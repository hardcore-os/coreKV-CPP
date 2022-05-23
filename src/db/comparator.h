#ifndef DB_COMPARATOR_H_
#define DB_COMPARATOR_H_
#include <stdint.h>

#include <string>
#include <string_view>
namespace corekv {
class Comparator {
 public:
  virtual ~Comparator() = default;
  virtual const char* Name() = 0;

  virtual int32_t Compare(const char* a, const char* b) = 0;

  virtual void FindShortest(std::string& start, const std::string_view& limit) = 0;

};
// 按照字典序列
class ByteComparator final : public Comparator {
 public:
  const char* Name() override;
  int32_t Compare(const char* a, const char* b) override;
  void FindShortest(std::string& start, const std::string_view& limit) override;
};
}  // namespace corekv

#endif