#pragma once
#include <stdint.h>

#include <atomic>

namespace corekv {
namespace util {
class IntIdGenerator {
 public:
  uint64_t NewId(uint64_t step) : init_id_(0) {
    init_id_.fetch_add(step, std::memory_order_relaxed);
    return init_id_;
  }

 private:
  std::atomic<uint64_t> init_id_;
};
}  // namespace util

}  // namespace corekv