#include "util.h"

#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <chrono>
#include <iomanip>
#include <iostream>
namespace corekv {
namespace util {
uint64_t GetCurrentTime() {
  const auto& now = std::chrono::high_resolution_clock::now();
  auto d = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch());
  return d.count();
}
void GetCurrentTimeString(std::string& output) {
  const auto& now = std::chrono::system_clock::now();
  const auto& tp = std::chrono::time_point_cast<std::chrono::seconds>(now);
  std::time_t t = std::chrono::system_clock::to_time_t(tp);

  char buffer[30] = {0};
  struct tm tm;
  localtime_r(&t, &tm);
  strftime(buffer, sizeof(buffer), "%Y-%m-%d-%H%M%S", &tm);
  output.append(buffer);
}
int64_t GetCurrentTid() {
  thread_local int64_t tid = 0;
  if (tid != 0) {
    return tid;
  }
  tid = syscall(SYS_gettid);
  return tid;
}
int64_t GetCurrentPid() { return getpid(); }
}  // namespace util

}  // namespace corekv
