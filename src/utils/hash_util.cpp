#include "hash_util.h"

#include "util.h"
namespace corekv {
namespace hash_util {
uint32_t SimMurMurHash(const char *data, uint32_t len) {
  if (len <= 0 || !data) {
    return 0;
  }
  static constexpr uint32_t seed = 0xbc9f1d34;
  // Similar to murmur hash
  static constexpr uint32_t m = 0xc6a4a793;
  static constexpr uint32_t r = 24;
  const char *limit = data + len;
  uint32_t h = seed ^ (len * m);

  // Pick up four bytes at a time
  while (data + 4 <= limit) {
    uint32_t w = util::DecodeFixed32(data);
    data += 4;
    h += w;
    h *= m;
    h ^= (h >> 16);
  }
  switch (limit - data) {
    case 3:
      h += static_cast<uint8_t>(data[2]) << 16;
    case 2:
      h += static_cast<uint8_t>(data[1]) << 8;
    case 1:
      h += static_cast<uint8_t>(data[0]);
      h *= m;
      h ^= (h >> r);
      break;
  }
  return h;
}
}  // namespace hash_util

}  // namespace corekv
