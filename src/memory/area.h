#ifndef MEMORY_AREA_H_
#define MEMORY_AREA_H_
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace corekv {
class SimpleVectorAlloc final {
 public:
  SimpleVectorAlloc();

  SimpleVectorAlloc(const SimpleVectorAlloc&) = delete;
  SimpleVectorAlloc& operator=(const SimpleVectorAlloc&) = delete;

  ~SimpleVectorAlloc();

  // Return a pointer to a newly allocated memory block of "bytes" bytes.
  void* Allocate(uint32_t bytes);


  // Returns an estimate of the total memory usage of data allocated
  // by the arena.
  uint32_t MemoryUsage() const {
    return memory_usage_.load(std::memory_order_relaxed);
  }
  void Deallocate(void* p, int32_t n);  //释放内存
 private:
  char* AllocateFallback(uint32_t bytes);
  char* AllocateNewBlock(uint32_t block_bytes);

  // Allocation state
  char* alloc_ptr_;
  uint32_t alloc_bytes_remaining_;

  // Array of new[] allocated memory blocks
  std::vector<char*> blocks_;
  std::atomic<uint32_t> memory_usage_;
};
}  // namespace corekv

#endif