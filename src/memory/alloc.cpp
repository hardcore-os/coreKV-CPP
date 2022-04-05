#include "alloc.h"

#include <stdlib.h>
#include <iostream>
#include <cassert>
namespace corekv {

SimpleFreeListAlloc::~SimpleFreeListAlloc() {
  //释放二级内存，最后统一释放
  if(!free_list_start_pos_) {
    FreeList* p = (FreeList*)free_list_start_pos_;
    while(p) {
      FreeList* next = p->next;
      free(p);
      p = next;
    }
  }
}
int32_t SimpleFreeListAlloc::M_FreelistIndex(int32_t bytes) {
  // first fit策略
  return (bytes + kAlignBytes - 1) / kAlignBytes - 1;
}
int32_t SimpleFreeListAlloc::M_Roundup(int32_t bytes) {
  //向上取整
  return (bytes + kAlignBytes - 1) & ~(kAlignBytes - 1);
}
char* SimpleFreeListAlloc::M_ChunkAlloc(int32_t bytes, int32_t& nobjs) {
  char* result = nullptr;
  //总的字节大小
  uint32_t total_bytes = bytes * nobjs;
  //当前可用的
  uint32_t bytes_left = free_list_end_pos_ - free_list_start_pos_;
  if (bytes_left >= total_bytes) {
    // 剩余空间满足，那么此时直接使用剩余的空间
    result = free_list_start_pos_;
    free_list_start_pos_ += total_bytes;
    memory_usage_.fetch_add(total_bytes, std::memory_order_relaxed);
    return result;
  } else if (bytes_left >= bytes) {
    // 内存池剩余空间不能完全满足需求量，但足够供应一个以上区块
    nobjs = bytes_left / bytes;
    total_bytes = bytes * nobjs;
    result = free_list_start_pos_;
    free_list_start_pos_ += total_bytes;
    memory_usage_.fetch_add(total_bytes, std::memory_order_relaxed);
    return result;
  } else {
    // 内存池剩余空间一个都没法分配时，此时我们按照默认2倍来分配
    int32_t bytes_to_get = 2 * total_bytes + M_Roundup(heap_size_ >> 4);
    if (bytes_left > 0) {
      // 内存池中还有剩余，先配给适当的freelist，否则这部分会浪费掉
      FreeList* volatile* cur_free_list =
          freelist_ + M_FreelistIndex(bytes_left);
      // 调整freelist，将内存池剩余空间编入
      ((FreeList*)free_list_start_pos_)->next = *cur_free_list;
      *cur_free_list = ((FreeList*)free_list_start_pos_);
    }

    // 分配新的空间
    free_list_start_pos_ = (char*)malloc(bytes_to_get);
    if (!free_list_start_pos_) {
      FreeList *volatile *my_free_list = nullptr, *p = nullptr;
      // 尝试从freelist中查找是不是有足够大的没有用过的区块
      for (int32_t index = bytes; index <= kSmallObjectBytes;
           index += kAlignBytes) {
        my_free_list = freelist_ + M_FreelistIndex(index);
        p = *my_free_list;
        if (!p) {
          *my_free_list = p->next;
          free_list_start_pos_ = (char*)p;
          free_list_end_pos_ = free_list_start_pos_ + index;
          return M_ChunkAlloc(bytes, nobjs);
        }
      }
      free_list_end_pos_ = nullptr;
      free_list_start_pos_ = (char*)malloc(bytes_to_get);
      if (!free_list_start_pos_) {
        exit(1);
      }
    }
    heap_size_ += bytes_to_get;
    free_list_end_pos_ = free_list_start_pos_ + bytes_to_get;
    memory_usage_.fetch_add(bytes_to_get, std::memory_order_relaxed);
    return M_ChunkAlloc(bytes, nobjs);
  }
}

void* SimpleFreeListAlloc::M_Refill(int32_t bytes) {
  static const int32_t kInitBlockCount = 10;  //一次先分配10个，STL默认是20个
  int32_t real_block_count = kInitBlockCount;  //初始化，先按理想值来分配
  char* address = M_ChunkAlloc(bytes, real_block_count);
  do {
    if (real_block_count == 1) {
      break;
    }
    FreeList* next = nullptr;
    FreeList* cur = nullptr;
    FreeList* new_free_list = nullptr;

    new_free_list = next = reinterpret_cast<FreeList*>(address + bytes);
    freelist_[M_FreelistIndex(bytes)] = new_free_list;

    for (uint32_t index = 1; index < real_block_count; ++index) {
      cur = next;
      next = (FreeList*)((char*)next + bytes);  //下一个块的首地址
      if (index != real_block_count - 1) {
        cur = cur->next;
      } else {
        cur->next = nullptr;
      }
    }
  } while (0);
  FreeList* result = reinterpret_cast<FreeList*>(address);
  return result;
}

void* SimpleFreeListAlloc::Allocate(int32_t n) {
  assert(n > 0);
  if (n > kSmallObjectBytes) {
    memory_usage_.fetch_add(n, std::memory_order_relaxed);
    return (char*)malloc(n);
  }
  FreeList** select_free_list = nullptr;
  select_free_list = freelist_ + M_FreelistIndex(n);
  FreeList* result = *select_free_list;
  if (!result) {
    void* ret = (char*)M_Refill(M_Roundup(n));
    return ret;
  }
  *select_free_list = result->next;
  return result;
}

void SimpleFreeListAlloc::Deallocate(void* address, int32_t n) {
  if (address) {
    FreeList* p = (FreeList*)address;
    FreeList* volatile* cur_free_list = nullptr;
    memory_usage_.fetch_sub(n, std::memory_order_relaxed);
    if (n > kAlignBytes) {
      free(address);

      address = nullptr;
      return;
    }
    cur_free_list = freelist_ + M_FreelistIndex(n);
    p->next = *cur_free_list;
    *cur_free_list = p;
  }
}
void* SimpleFreeListAlloc::Reallocate(void* address, int32_t old_size,
                                      int32_t new_size) {
  Deallocate(address, old_size);
  address = Allocate(new_size);
  return address;
}

}  // namespace corekv