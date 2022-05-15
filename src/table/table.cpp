#include "table.h"
#include <memory>
#include "../db/comparator.h"
#include "../logger/log.h"
#include "../utils/codec.h"
#include "../utils/crc32.h"
#include "data_block.h"
#include "footer.h"
#include "table_options.h"
namespace corekv {
using namespace util;
Table::Table(const Options* options, const FileReader* file_reader)
    : options_(options), file_reader_(file_reader) {}
DBStatus Table::Open(uint64_t file_size) {
  if (file_size < kEncodedLength) {
    return Status::kInterupt;
  }
  std::string footer_space;
  footer_space.resize(kEncodedLength);
  auto status = file_reader_->Read(file_size - kEncodedLength, kEncodedLength,
                                   &footer_space);
  if (status != Status::kSuccess) {
    return status;
  }
  Footer footer;
  std::string_view st = footer_space;
  status = footer.DecodeFrom(&st);
  std::string index_meta_data;
  ReadBlock(footer.GetIndexBlockMetaData(), index_meta_data);
  ReadMeta(&footer);
  return status;
}

DBStatus Table::ReadBlock(const OffSetSize& offset_size, std::string& buf) {
  buf.resize(offset_size.length + kBlockTrailerSize);
  file_reader_->Read(offset_size.offset, offset_size.length + kBlockTrailerSize,
                     &buf);
  const char* data = buf.data();
  const uint32_t crc =
      crc32::Unmask(DecodeFixed32(data + offset_size.length + 1));
  const uint32_t actual = crc32::Value(data, offset_size.length + 1);

  if (crc != actual) {
    LOG(corekv::LogLevel::ERROR, "Invalid Block");
    return Status::kInvalidObject;
  }
  switch (data[offset_size.length]) {
    case kSnappyCompression:
      LOG(corekv::LogLevel::ERROR, "kSnappyCompression");
      break;
    default:
      LOG(corekv::LogLevel::ERROR, "kNonCompress");
      break;
  }
  return Status::kSuccess;
}
void Table::ReadMeta(const Footer* footer) {
  if (options_->filter_policy == nullptr) {
    return;
  }
  std::string filter_meta_data;
  ReadBlock(footer->GetFilterBlockMetaData(), filter_meta_data);
  std::string_view real_data(filter_meta_data.data(),
                             footer->GetFilterBlockMetaData().length);
  std::unique_ptr<DataBlock> meta = std::make_unique<DataBlock>(real_data);
  Iterator* iter = meta->NewIterator(new ByteComparator());
  std::string_view key = options_->filter_policy->Name();
  iter->Seek(key);
  if (iter->Valid() && iter->key() == key) {
    LOG(corekv::LogLevel::ERROR, "Hit Key=%s",key.data());
    ReadFilter(iter->value());
  }
  delete iter;
}
void Table::ReadFilter(const std::string_view& filter_handle_value) {
  OffSetSize offset_size;
  OffsetBuilder offset_builder;
  offset_builder.Decode(filter_handle_value.data(), offset_size);
  ReadBlock(offset_size, bf_);
}
}  // namespace corekv