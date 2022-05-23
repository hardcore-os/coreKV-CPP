#include "util.h"

#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <chrono>
#include <iomanip>

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

//判断是否是小端(可能也是一道笔试题)
bool CheckLittleEndian() {
  union {
    short inum = 0x1234;
    char cnum;
  } dummy;
  return dummy.cnum != 0x12;
}
uint32_t DecodeFixed32(const char* ptr) {
  if (!ptr) {
    return 0;
  }
  {
    return ((static_cast<uint32_t>(static_cast<unsigned char>(ptr[0]))) |
            (static_cast<uint32_t>(static_cast<unsigned char>(ptr[1])) << 8) |
            (static_cast<uint32_t>(static_cast<unsigned char>(ptr[2])) << 16) |
            (static_cast<uint32_t>(static_cast<unsigned char>(ptr[3])) << 24));
  }
}

int64_t Next2Power(int64_t value) {
  value--;
  value |= value >> 1;
  value |= value >> 2;
  value |= value >> 4;
  value |= value >> 8;
  value |= value >> 16;
  value |= value >> 32;
  value++;
  return value;
}
}  // namespace util

}  // namespace corekv
