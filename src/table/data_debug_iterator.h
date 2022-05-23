#pragma once
#include <string_view>
#include <string>
namespace corekv {
class DataDebugIterator final {
 public:
  std::string ParseData(const std::string_view&st);
};
}  // namespace  corekv
