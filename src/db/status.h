#ifndef DB_STATUS_H_
#define DB_STATUS_H_
#include <stdint.h>
namespace corekv {

struct DBStatus {
  int32_t code;
  const char *message;
};
bool operator==(const DBStatus &x, const DBStatus &y);
bool operator!=(const DBStatus &x, const DBStatus &y);

struct Status {
  Status() = delete;
  ~Status() = delete;
  static constexpr DBStatus Success = {1000, "Success"};
  static constexpr DBStatus NotFound = {1001, "Not Found"};
  static constexpr DBStatus Interupt = {1002, "Interrupt"};
};

}  // namespace corekv

#endif