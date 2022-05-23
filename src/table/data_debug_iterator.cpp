#include "data_debug_iterator.h"
#include <stdint.h>

#include <iostream>

#include "../utils/codec.h"

using namespace std;
namespace corekv {
using namespace util;

static const char* DecodeEntry(const char* p, const char* limit,
                               uint32_t* shared, uint32_t* non_shared,
                               uint32_t* value_length) {
  if (limit - p < 3) return nullptr;
  *shared = reinterpret_cast<const uint8_t*>(p)[0];
  *non_shared = reinterpret_cast<const uint8_t*>(p)[1];
  *value_length = reinterpret_cast<const uint8_t*>(p)[2];
  if ((*shared | *non_shared | *value_length) < 128) {
    // Fast path: all three values are encoded in one byte each
    p += 3;
  } else {
    if ((p = GetVarint32Ptr(p, limit, shared)) == nullptr) return nullptr;
    if ((p = GetVarint32Ptr(p, limit, non_shared)) == nullptr) return nullptr;
    if ((p = GetVarint32Ptr(p, limit, value_length)) == nullptr) return nullptr;
  }

  if (static_cast<uint32_t>(limit - p) < (*non_shared + *value_length)) {
    return nullptr;
  }
  return p;
}

std::string DataDebugIterator::ParseData(const std::string_view& st) {
  if (st.empty()) {
    return "";
  }
  int size = st.size();
  const char* data = st.data();
  int restart_num = DecodeFixed32(data + size - 4);
  const char* end = data + size - 4 - 4 * restart_num;
  uint32_t shared;
  uint32_t non_shared;
  uint32_t value_length;
  const char* key_ptr =
      DecodeEntry(data, end, &shared, &non_shared, &value_length);
  std::string_view mid_key(key_ptr, non_shared);
  cout << "shared=" << shared << ",non_shared=" << non_shared << ",value_length"
       << value_length << ",mid_key:" << mid_key << endl;
  std::string value_key(key_ptr + non_shared, value_length);
  return value_key;
}
}  // namespace corekv
