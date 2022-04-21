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
  //当前可用的内存(这里需要注意一下，两个内存之间相减是有意义的)
  uint32_t bytes_left = free_list_end_pos_ - free_list_start_pos_;

  if (bytes_left >= total_bytes) {
    // 剩余空间满足，那么此时直接使用剩余的空间
    result = free_list_start_pos_;
    //这里需要进行更新，因为部分有一部分给了链表内存管理
    free_list_start_pos_ += total_bytes;
    memory_usage_.fetch_add(total_bytes, std::memory_order_relaxed);
    return result;
  } else if (bytes_left >= bytes) {
    // 内存池剩余空间不能完全满足需求量，但足够供应一个以上区块
    nobjs = bytes_left / bytes;
    total_bytes = bytes * nobjs;
    result = free_list_start_pos_;
    //这里需要进行更新，因为部分有一部分给了链表内存管理
    free_list_start_pos_ += total_bytes;
    memory_usage_.fetch_add(total_bytes, std::memory_order_relaxed);
    return result;
  } else {
    //内存池剩余空间一个都没法分配时
    //在这里又分配了2倍，见uint32_t total_bytes = bytes * nobjs;
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
    //如果分配失败，尝试已经存在的slot能不能装下来
    if (!free_list_start_pos_) {
      FreeList *volatile *my_free_list = nullptr, *p = nullptr;
      // 尝试从freelist中查找是不是有足够大的没有用过的区块
      for (int32_t index = bytes; index <= kSmallObjectBytes;
           index += kAlignBytes) {
        my_free_list = freelist_ + M_FreelistIndex(index);
        p = *my_free_list;
        if (!p) {
          //说明找到了，此时在进行下一轮循环时候，我们就能直接返回
          *my_free_list = p->next;
          free_list_start_pos_ = (char*)p;
          free_list_end_pos_ = free_list_start_pos_ + index;
          return M_ChunkAlloc(bytes, nobjs);
        }
      }
      //如果未找到，此时我们再重新尝试分配一次，如果分配失败，此时将终止程序
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
  //默认一次先分配10个block，分配太多，导致浪费，太少可能不够用
  static const int32_t kInitBlockCount = 10;  //一次先分配10个，STL默认是20个
  int32_t real_block_count = kInitBlockCount;  //初始化，先按理想值来分配
  char* address = M_ChunkAlloc(bytes, real_block_count);
  do {
    //当前内存池刚好只够分配一个block块时
    if (real_block_count == 1) {
      break;
    }
    FreeList* next = nullptr;
    FreeList* cur = nullptr;
    FreeList* new_free_list = nullptr;
    //我们将第一个给申请者，剩下的9个放到对应的链表上，但是我们在M_ChunkAlloc
    //函数中，分配的是2倍，因此剩下的10个放在内存池中，供我们使用
    new_free_list = next = reinterpret_cast<FreeList*>(address + bytes);
    freelist_[M_FreelistIndex(bytes)] = new_free_list;

    for (uint32_t index = 1;; ++index) {
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
  //如果超过4kb，此时直接使用glibc自带的malloc，因为我们假设大多数时候都是小对象
  //针对大对象，可能会存在大key，在上层我们就应该尽可能的规避
  if (n > kSmallObjectBytes) {
    memory_usage_.fetch_add(n, std::memory_order_relaxed);
    return (char*)malloc(n);
  }
  FreeList** select_free_list = nullptr;
  //根据对象大小，定位位于哪个slot，对应内存分配策略其实最佳适配原则
  select_free_list = freelist_ + M_FreelistIndex(n);
  FreeList* result = *select_free_list;
  //默认情况下，我们的slot不能提前分配内存，因为他为空
  if (!result) {
    //如果为空，此时我们需要分配内存来进行填充
    void* ret = (char*)M_Refill(M_Roundup(n));
    return ret;
  }
  //更新下一个可用
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
    //可用内存挂在最前端
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