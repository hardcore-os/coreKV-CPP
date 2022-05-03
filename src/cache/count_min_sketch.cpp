#include "count_min_sketch.h"

#include <algorithm>

#include "../utils/random_util.h"
namespace corekv {

void CountMinSketch::SingleGroup::Init(uint32_t counter_num) {
  if (counter_num == 0) {
    counter_num = 1;
  }
  base_.resize(counter_num);
}
void CountMinSketch::SingleGroup::Increment(uint32_t counter_index) {
  if (base_.empty()) {
    return;
  }
  auto value = base_[counter_index].to_ulong();
  // 这里15表示的是4bit，最大值为15，
  if (!base_[counter_index].all()) {
    CounterType tmp(value + 1);
    base_[counter_index] = tmp;
  }
}
void CountMinSketch::SingleGroup::Reset() {
  for (auto& item : base_) {
    auto value = item.to_ulong();
    value = value >> 1 & 0x0111;
    CounterType tmp(value);
    item = tmp;
  }
}
void CountMinSketch::SingleGroup::Clear() {
  for (auto& item : base_) {
    item.reset();
  }
}
int32_t CountMinSketch::SingleGroup::Get(uint32_t counter_index) {
  static constexpr int32_t kEmpty = 0;
  if (base_.empty()) {
    return kEmpty;
  }
  const auto& value = base_[counter_index].to_ulong();
  return value;
}

// CountMinSketch
CountMinSketch::CountMinSketch(uint32_t counter_num) {
  if (counter_num > 0) {
    mask_ = counter_num - 1;
    RandomUtil rand_util;
    std::for_each(groups_.begin(), groups_.end(),
                  [counter_num](SingleGroup& base) { base.Init(counter_num); });
    std::for_each(seeds_.begin(), seeds_.end(), [&rand_util](uint64_t& seed) {
      seed = static_cast<uint64_t>(rand_util.GetSimpleRandomNum());
    });
  }
}

int32_t CountMinSketch::Estimate(uint32_t hash_val) {
  int32_t min_val = 0;
  for (int32_t index = 0; index < kCmsDepth; ++index) {
    min_val = std::min(groups_[index].Get((hash_val ^ seeds_[index]) & mask_),
                       min_val);
  }
  return min_val;
}
void CountMinSketch::Increment(uint32_t hash_val) {
  for (int32_t index = 0; index < kCmsDepth; ++index) {
    groups_[index].Increment((hash_val ^ seeds_[index]) & mask_);
  }
}
// 大于某个阈值之后，会进行降级操作
void CountMinSketch::Reset() {
  for (int32_t index = 0; index < kCmsDepth; ++index) {
    groups_[index].Reset();
  }
}
void CountMinSketch::Clear() {
  for (int32_t index = 0; index < kCmsDepth; ++index) {
    groups_[index].Clear();
  }
}
}  // namespace corekv
