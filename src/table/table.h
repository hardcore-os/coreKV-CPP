#pragma once
#include <memory>

#include "../db/options.h"
#include "../file/file.h"
#include "block_builder.h"
#include "offset_size.h"
#include "footer.h"
namespace corekv {

class Table final {
 public:
 Table(const Options* options, const FileReader* file_reader);
  DBStatus Open(uint64_t file_size);
  DBStatus ReadBlock(const OffSetSize&, std::string&);
  void ReadMeta(const Footer* footer);
  void ReadFilter(const std::string_view& filter_handle_value);
 private:
  const Options* options_;
  const FileReader* file_reader_;
  std::string bf_;
};
}  // namespace corekv
