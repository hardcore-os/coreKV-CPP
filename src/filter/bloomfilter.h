#pragma once
#include "filter_policy.h"
namespace corekv {
class BloomFilter final : public FilterPolicy {
 public:
  //直接给定每个key占的位数
  BloomFilter(int32_t bits_per_key);
  //通过误差率反算出每个key所占的位数
  BloomFilter(int32_t entries_num, float positive);
  ~BloomFilter() = default;
  const char* Name() override;
  void CreateFilter(const std::string_view* keys, int32_t n) override;
  bool MayMatch(const std::string_view& key, int32_t start_pos, int32_t len) override;
  uint32_t Size() override {
      return bloomfilter_data_.size();
  }
 private:
  void CalcBloomBitsPerKey(int32_t entries_num, float positive = 0.01);
  void CalcHashNum();
 private:
  // 每个key占用的bit位数
  int32_t bits_per_key_ = 0;
  // 哈希函数个数
  int32_t k_ = 0;
  std::string bloomfilter_data_;
};
}  // namespace corekv
