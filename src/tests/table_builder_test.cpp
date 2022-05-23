#include "table/table_builder.h"

#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>
#include "db/comparator.h"
#include "file/file.h"
#include "filter/bloomfilter.h"
#include "table/table.h"
#include "logger/log.h"

using namespace std;
using namespace corekv;
static const vector<string> kTestKeys = {"corekv", "corekv1", "corekv2","corekv3","corekv4","corekv7"};
TEST(table_builder_Test, Add) {
  corekv::LogConfig log_config;
  log_config.log_type = corekv::LogType::CONSOLE;
  log_config.rotate_size = 100;
  corekv::Log::GetInstance()->InitLog(log_config);
  static const std::string st = "d.txt";

  Options options;
  options.block_compress_type = kSnappyCompression;
  options.filter_policy = std::make_unique<BloomFilter>(30);
  options.comparator = std::make_unique<ByteComparator>();
  FileWriter* file_handler = new FileWriter(st);
  TableBuilder* tb = new TableBuilder(options, file_handler);
  for (const auto& item : kTestKeys) {
    tb->Add(item, item);
  }
  tb->Finish();
  delete file_handler;
  delete tb;
  FileReader file_reader(st);
  Table tab(&options, &file_reader);
  tab.Open(FileTool::GetFileSize(st));
}