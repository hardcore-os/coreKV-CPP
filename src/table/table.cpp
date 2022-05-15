#include "table.h"

#include "../logger/log.h"
#include "../utils/codec.h"
#include "footer.h"
#include "table_options.h"
#include "../utils/crc32.h"
namespace corekv {
using namespace util;
DBStatus Table::Open(const Options& options, const FileReader* file_reader,
                     uint64_t file_size) {
  if (file_size < kEncodedLength) {
    return Status::kInterupt;
  }
  //先解析出固定长度的footer
  std::string footer_str;
  footer_str.resize(kEncodedLength);
  auto status = file_reader->Read(file_size - kEncodedLength, kEncodedLength,
                                  &footer_str);
  if (status != Status::kSuccess) {
    return status;
  }
  Footer footer;
  std::string_view st = footer_str;
  status = footer.DecodeFrom(&st);
  ReadBlock(options, file_reader, footer.GetIndexBlockMetaData());
  ReadBlock(options, file_reader, footer.GetFilterBlockMetaData());
  return status;
}

DBStatus Table::ReadBlock(const Options& , const FileReader* file_reader,
                          const OffSetSize& offset_size) {
  std::string buf;
  buf.resize(offset_size.length);
  file_reader->Read(offset_size.offset, offset_size.length,
                    &buf);
  const auto& crc32_value = DecodeFixed32(buf.data());
  LOG(corekv::LogLevel::ERROR, "crc32_val:%d", crc32_value);
  return Status::kSuccess;
}
}  // namespace corekv