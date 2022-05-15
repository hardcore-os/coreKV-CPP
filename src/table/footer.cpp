#include "footer.h"

#include "../utils/codec.h"
#include "table_options.h"
namespace corekv {
using namespace util;
void Footer::EncodeTo(std::string* dst) {
  // 16
  offset_builder_.Encode(filter_block_, *dst);
  //16
  offset_builder_.Encode(index_block_, *dst);
  // 生成magic num
  PutFixed32(dst, static_cast<uint32_t>(kTableMagicNumber & 0xffffffffu));
  PutFixed32(dst, static_cast<uint32_t>(kTableMagicNumber >> 32));
}

DBStatus Footer::DecodeFrom(std::string_view* input) {
  const char* magic_ptr = input->data() + kEncodedLength - 8;
  const uint32_t magic_lo = DecodeFixed32(magic_ptr);
  const uint32_t magic_hi = DecodeFixed32(magic_ptr + 4);
  const uint64_t magic = ((static_cast<uint64_t>(magic_hi) << 32) |
                          (static_cast<uint64_t>(magic_lo)));
  if (magic != kTableMagicNumber) {
    return Status::kBadBlock;
  }

  DBStatus result = offset_builder_.Decode(input->data(), filter_block_);
  if (result == Status::kSuccess) {
    result = offset_builder_.Decode(input->data()+16, index_block_);
  }
  if (result == Status::kSuccess) {
    // We skip over any leftover data (just padding for now) in "input"
    const char* end = magic_ptr + 8;
    *input = std::string_view(end, input->data() + input->size() - end);
  }
  return result;
}

std::string Footer::DebugString() {
  return "[filter_block_meta: offset = " +
         std::to_string(filter_block_.offset) +
         ", size = " + std::to_string(filter_block_.length) + " ], " +
         "[index_block_meta: offset = " + std::to_string(index_block_.offset) +
         ", size = " + std::to_string(index_block_.length) + " ]";
}
}  // namespace corekv
