#pragma once
#include <stdint.h>

#include <string>
#include <string_view>

#include "../db/status.h"
namespace corekv {

struct OffSetSize {
  // 记录数据的起点
  uint64_t offset = 0;
  // 记录数据的长度
  uint64_t length = 0;
};
class OffsetBuilder final {
 public:
  // 按照varint编解码
  void Encode(const OffSetSize& offset_size, std::string& output);
  DBStatus Decode(const char* input, OffSetSize& offset_size);
  std::string DebugString(const OffSetSize& offset_size);
};

}  // namespace corekv
