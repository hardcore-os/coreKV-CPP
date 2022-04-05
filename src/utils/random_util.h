#ifndef UTILS_RANDOM_UTIL_H_
#define UTILS_RANDOM_UTIL_H_
#include <math.h>

#include <cstdint>
namespace corekv {
class RandomUtil final {
  public:
    RandomUtil() = default;
    ~RandomUtil() = default;
    RandomUtil(uint32_t seed) : seed_val_(seed) {}
    int64_t GetSimpleRandomNum() { return rand(); }

  private:
    uint32_t seed_val_;
};
}  // namespace corekv

#endif