#include "string_util.h"

#include <sstream>
namespace corekv {
namespace string_util {

void Split(const std::string& input, const char delimiter,
           std::vector<std::string>& output) {
  if (input.empty()) {
    return;
  }
  std::istringstream stream(input);
  std::string item;
  while (std::getline(stream, item, delimiter))
    output.emplace_back(std::move(item));
}
}  // namespace string_util

}  // namespace corekv
