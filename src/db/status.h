#ifndef DB_STATUS_H_
#define DB_STATUS_H_
#include <stdint.h>
namespace corekv {

struct DBStatus {
  int32_t code = 1000;
  const char *message;
};
bool operator==(const DBStatus &x, const DBStatus &y);
bool operator!=(const DBStatus &x, const DBStatus &y);

struct Status {
  Status() = delete;
  ~Status() = delete;
  static constexpr DBStatus kSuccess = {1000, "Success"};
  static constexpr DBStatus kNotFound = {1001, "Not Found"};
  static constexpr DBStatus kInterupt = {1002, "Interrupt"};
  static constexpr DBStatus kBadBlock = {1003, "Bad Block"};
  static constexpr DBStatus kWriteFileFailed = {1004, "WriteFile Failed"};
  static constexpr DBStatus kReadFileFailed = {1005, "ReadFile Failed"};
  static constexpr DBStatus kInvalidObject = {1006, "Invalid Object"};
};

}  // namespace corekv

#endif