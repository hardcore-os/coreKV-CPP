#include "offset_size.h"
#include "../utils/codec.h"
namespace corekv {
using namespace util;

void OffsetBuilder::Encode(const OffSetSize& offset_size, std::string& output) {
  PutFixed64(&output, offset_size.offset);
  PutFixed64(&output, offset_size.length);
}
DBStatus OffsetBuilder::Decode(const char* input, OffSetSize& offset_size) {
  offset_size.offset = DecodeFixed64(input);
  offset_size.length = DecodeFixed64(input + 8);
  return Status::kSuccess;
}
std::string OffsetBuilder::DebugString(const OffSetSize& offset_size) {
  return "[offset_size: offset = " +
         std::to_string(offset_size.offset) +
         ", size = " + std::to_string(offset_size.length) + " ]";
}
}  // namespace corekv
