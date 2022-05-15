#pragma once
#include <stdint.h>
namespace corekv {
// echo http://www.hardcore.com/corekv | sha1sum
static constexpr uint64_t kTableMagicNumber = 0x04452b9527c24933ull;
static constexpr uint32_t kMaxVarInt64Length = 10;
static constexpr uint32_t kMaxOffSetSizeLength = 2 * kMaxVarInt64Length;
// footer的长度
// static constexpr uint64_t kEncodedLength =
//     2 * kMaxOffSetSizeLength + 8;
static constexpr uint64_t kEncodedLength = 40;
// 1-byte type + 32-bit crc
static constexpr size_t kBlockTrailerSize = 5;
}  // namespace corekv
