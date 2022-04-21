#include "area.h"

namespace corekv {
SimpleVectorAlloc::SimpleVectorAlloc()
    : alloc_ptr_(nullptr), alloc_bytes_remaining_(0), memory_usage_(0) {}

SimpleVectorAlloc::~SimpleVectorAlloc() {
  for (uint32_t i = 0; i < blocks_.size(); i++) {
    delete[] blocks_[i];
  }
}
void SimpleVectorAlloc::Deallocate(void*, int32_t) {
  //暂时不支持这个操作
}
char* SimpleVectorAlloc::AllocateFallback(uint32_t bytes) {
  if (bytes > kBlockSize / 4) {
    // Object is more than a quarter of our block size.  Allocate it separately
    // to avoid wasting too much space in leftover bytes.
    char* result = AllocateNewBlock(bytes);
    return result;
  }

  // We waste the remaining space in the current block.
  alloc_ptr_ = AllocateNewBlock(kBlockSize);
  alloc_bytes_remaining_ = kBlockSize;

  char* result = alloc_ptr_;
  alloc_ptr_ += bytes;
  alloc_bytes_remaining_ -= bytes;
  return result;
}

void* SimpleVectorAlloc::Allocate(uint32_t bytes) {
  const int align = (sizeof(void*) > 8) ? sizeof(void*) : 8;
  static_assert((align & (align - 1)) == 0,
                "Pointer size should be a power of 2");
  //保证是8的倍
  uint32_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align - 1);
  uint32_t slop = (current_mod == 0 ? 0 : align - current_mod);
  uint32_t needed = bytes + slop;
  char* result = nullptr;
  //如果是当前剩余的内存足够，我们直接使用即可
  if (needed <= alloc_bytes_remaining_) {
    result = alloc_ptr_ + slop;
    alloc_ptr_ += needed;
    alloc_bytes_remaining_ -= needed;
  } else {
    //如果不够我们开辟新内存
    AllocateFallback(bytes);
    //再一次调用，将内存碎片充分利用
    alloc_bytes_remaining_+=bytes;
    Allocate(bytes);
  }
  assert((reinterpret_cast<uintptr_t>(result) & (align - 1)) == 0);
  return result;
}

char* SimpleVectorAlloc::AllocateNewBlock(uint32_t block_bytes) {
  char* result = new char[block_bytes];
  blocks_.push_back(result);
  memory_usage_.fetch_add(block_bytes + sizeof(char*),
                          std::memory_order_relaxed);
  return result;
}
}  // namespace corekv
