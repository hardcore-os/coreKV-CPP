#pragma once
#include <string>
#include <string_view>
// 用于过滤
namespace corekv {
struct FilterPolicyMeta {
  uint32_t hash_num;
};
class FilterPolicy {
 public:
  FilterPolicy() = default;
  virtual ~FilterPolicy() = default;
  // 当前过滤器的名字
  virtual const char* Name() = 0;
  virtual void CreateFilter(const std::string* keys, int n) = 0;
  virtual bool MayMatch(const std::string_view& key, int32_t start_pos,
                        int32_t len) = 0;
  virtual bool MayMatch(const std::string_view& key,
                        const std::string_view& datas) = 0;
  virtual const std::string& Data() = 0;
  virtual const FilterPolicyMeta& GetMeta() = 0;
  virtual uint32_t Size() = 0;
};
}  // namespace corekv
