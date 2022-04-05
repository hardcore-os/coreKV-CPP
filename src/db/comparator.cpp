#include "comparator.h"

#include <string.h>
namespace corekv {
int32_t ByteComparator::Compare(const char* a, const char* b) {
  return strcmp(a, b);
}
}  // namespace corekv
