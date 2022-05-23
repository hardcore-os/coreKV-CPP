#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "../db/options.h"
#include "../filter/filter_policy.h"
namespace corekv {
class DataBlockBuilder final {
 public:
  DataBlockBuilder(const Options* options);
  void Add(const std::string_view& key, const std::string_view& value);
  void Finish();
  
  // 思考一下这里为什么不是直接buffer.size呢？
  const uint64_t CurrentSize() {
    return buffer_.size() + restarts_.size() * sizeof(uint32_t) +
           sizeof(uint32_t);
  }

  const std::string& Data() { return buffer_; }
  void Reset() {
      restarts_.clear();
      restarts_.emplace_back(0);
      is_finished_ = false;
      buffer_.clear();
      pre_key_ = "";
      restart_pointer_counter_ = 0;
  }
  private:
  void AddRestartPointers();
 private:
  // 判断是否结束了
  bool is_finished_ = false;
  const Options* options_;
  std::string buffer_;                    // 目标buffer
  std::vector<uint32_t> restarts_;        // 记录重启点的位置
  uint32_t restart_pointer_counter_ = 0;  // 记录什么时候需要进行restart
  std::string pre_key_;  // 记录前一个key(需要进行深度复制，不能使用string_view)
};
// filter_block_builder的话
class FilterBlockBuilder final {
 public:
  FilterBlockBuilder(const Options& options);
  bool Availabe() { return policy_filter_ != nullptr; }
  void Add(const std::string_view& key);
  void CreateFilter();
  bool MayMatch(const std::string_view& key);
  bool MayMatch(const std::string_view& key,
                const std::string_view& bf_datas);
  const std::string& Data();
  void Finish(); 
 private:
 std::string buffer_;
  std::vector<std::string> datas_;
  FilterPolicy* policy_filter_ = nullptr;
};
}  // namespace corekv
