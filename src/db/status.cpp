#include "status.h"

namespace corekv {

bool operator==(const DBStatus &x, const DBStatus &y) {
  return x.code == y.code;
}
bool operator!=(const DBStatus &x, const DBStatus &y) {
  return x.code != y.code;
}

}  // namespace corekv
