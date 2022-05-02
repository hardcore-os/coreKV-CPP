#pragma once
#include <stdint.h>

#include <array>
#include <bitset>
#include <vector>
namespace corekv {
// CountMinSketch简称cms算法
class CountMinSketch final {
  static constexpr uint8_t kCmsDepth = 4;

 public:
  explicit CountMinSketch(uint32_t counter_num);
  ~CountMinSketch() = default;
  void Increment(uint32_t hash_val);
  void Reset();
  void Clear();
  // 每次取出最小的那个值
  int32_t Estimate(uint32_t hash_val);

 private:
  class SingleGroup final {
    // 4bit作为一个counter，char是1个字节，等于8位，相当于是2个counter
    static constexpr uint8_t kCounterBitLen = 4;
    using CounterType = std::bitset<kCounterBitLen>;
    // 每一行一组
   public:
    SingleGroup(uint32_t counter_num) {
        Init(counter_num);
    }
    SingleGroup() = default;
    ~SingleGroup() = default;
    void Init(uint32_t counter_num);
    void Increment(uint32_t counter_index);
    void Reset();
    void Clear();
    int32_t Get(uint32_t counter_index);

   private:
    std::vector<CounterType> base_;
  };

 private:
  uint32_t mask_;
  std::array<uint64_t, kCmsDepth> seeds_;
  std::array<SingleGroup, kCmsDepth> groups_;
};

}  // namespace corekv
