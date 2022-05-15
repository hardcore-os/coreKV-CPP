#pragma once
#include "../file/file.h"
#include "../db/options.h"
#include "offset_size.h"
namespace corekv {

class Table final {
 public:
  DBStatus Open(const Options& options, const FileReader* file_reader,
                uint64_t file_size);
  DBStatus ReadBlock(const Options& options,const FileReader* file_reader,const OffSetSize& );
};
}  // namespace corekv
