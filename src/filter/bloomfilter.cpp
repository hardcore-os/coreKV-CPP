#include "bloomfilter.h"

#include <cmath>

#include "utils/codec.h"
#include "utils/hash_util.h"

namespace corekv {
BloomFilter::BloomFilter(int32_t bits_per_key) : bits_per_key_(bits_per_key) {
  CalcHashNum();
}

BloomFilter::BloomFilter(int32_t entries_num, float positive) {
  if (entries_num > 0) {
    CalcBloomBitsPerKey(entries_num, positive);
  }
  CalcHashNum();
}
void BloomFilter::CalcHashNum() {
  if (bits_per_key_ < 0) {
    bits_per_key_ = 0;
  }
  filter_policy_meta_.hash_num = static_cast<int32_t>(bits_per_key_ *
                            0.69314718056);  // 0.69314718056 =~ ln(2)
  filter_policy_meta_.hash_num = filter_policy_meta_.hash_num < 1 ? 1 : filter_policy_meta_.hash_num;
  filter_policy_meta_.hash_num = filter_policy_meta_.hash_num > 30 ? 30 : filter_policy_meta_.hash_num;
}
void BloomFilter::CalcBloomBitsPerKey(int32_t entries_num, float positive) {
  float size = -1 * entries_num * logf(positive) / powf(0.69314718056, 2.0);
  bits_per_key_ = static_cast<int32_t>(ceilf(size / entries_num));
}
const char* BloomFilter::Name() { return "general_bloomfilter"; }
void BloomFilter::CreateFilter(const std::string* keys, int32_t n) {
  if (n <= 0 || !keys) {
    return;
  }

  int32_t bits = n * bits_per_key_;
  bits = bits < 64 ? 64 : bits;
  const int32_t bytes = (bits + 7) / 8;
  bits = bytes * 8;
  //这里主要是在corekv场景下，可能多个bf共用一个底层bloomfilter_data_对象
  const int32_t init_size = bloomfilter_data_.size();
  bloomfilter_data_.resize(init_size + bytes, 0);
  // 转成数组使用起来更方便
  char* array = &(bloomfilter_data_)[init_size];
  for (int i = 0; i < n; i++) {
    // Use double-hashing to generate a sequence of hash values.
    // See analysis in [Kirsch,Mitzenmacher 2006].
    uint32_t hash_val =
        hash_util::SimMurMurHash(keys[i].data(), keys[i].size());
    const uint32_t delta =
        (hash_val >> 17) | (hash_val << 15);  // Rotate right 17 bits
    for (size_t j = 0; j < filter_policy_meta_.hash_num; j++) {
      const uint32_t bitpos = hash_val % bits;
      array[bitpos / 8] |= (1 << (bitpos % 8));
      hash_val += delta;
    }
  }
}
bool BloomFilter::MayMatch(const std::string_view& key, int32_t start_pos,
                           int32_t len) {
  if (key.empty() || bloomfilter_data_.empty()) {
    return false;
  }
  // 转成
  const char* array = bloomfilter_data_.data();
  const size_t total_len = bloomfilter_data_.size();
  if (start_pos >= total_len) {
    return false;
  }
  if (len == 0) {
    len = total_len - start_pos;
  }
  std::string_view bloom_filter(array + start_pos, len);
  const char* cur_array = bloom_filter.data();
  const int32_t bits = len * 8;
  if (filter_policy_meta_.hash_num > 30) {
    return true;
  }

  uint32_t hash_val = hash_util::SimMurMurHash(key.data(), key.size());
  const uint32_t delta =
      (hash_val >> 17) | (hash_val << 15);  // Rotate right 17 bits
  for (int32_t j = 0; j < filter_policy_meta_.hash_num; j++) {
    const uint32_t bitpos = hash_val % bits;
    if ((cur_array[bitpos / 8] & (1 << (bitpos % 8))) == 0) {
      return false;
    }
    hash_val += delta;
  }
  return true;
}

bool BloomFilter::MayMatch(const std::string_view& key,
                           const std::string_view& bf_datas) {
  static constexpr uint32_t kFixedSize = 4;
  // 先恢复k_
  const auto& size = bf_datas.size();
  if (size < kFixedSize || key.empty()) {
    return false;
  }
  uint32_t k = util::DecodeFixed32(bf_datas.data() + size - kFixedSize);
  if (k > 30) {
    return true;
  }
  const int32_t bits = (size - kFixedSize) * 8;
  std::string_view bloom_filter(bf_datas.data(), size - kFixedSize);
  const char* cur_array = bloom_filter.data();
  uint32_t hash_val = hash_util::SimMurMurHash(key.data(), key.size());
  const uint32_t delta =
      (hash_val >> 17) | (hash_val << 15);  // Rotate right 17 bits
  for (int32_t j = 0; j < k; j++) {
    const uint32_t bitpos = hash_val % bits;
    if ((cur_array[bitpos / 8] & (1 << (bitpos % 8))) == 0) {
      return false;
    }
    hash_val += delta;
  }
  return true;
}
}  // namespace corekv
