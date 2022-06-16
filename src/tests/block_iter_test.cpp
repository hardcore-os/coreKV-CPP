#include <gtest/gtest.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "../db/options.h"
#include "../logger/log.h"
#include "db/comparator.h"
#include "file/file.h"
#include "filter/bloomfilter.h"
#include "table/block_builder.h"
#include "table/data_block.h"

using namespace std;
using namespace corekv;
TEST(ITER_TEST, block_iter) {
  vector<string> kTestKeys = {};
  for (int i = 0; i < 1900; ++i) {
    int idx = i;
    string k = "key";
    k += to_string(idx);
    string_view key = static_cast<string_view>(k);
    kTestKeys.emplace_back(key);
  }
  sort(kTestKeys.begin(), kTestKeys.end());

  corekv::LogConfig log_config;
  log_config.log_type = corekv::LogType::CONSOLE;
  log_config.rotate_size = 100;
  corekv::Log::GetInstance()->InitLog(log_config);
  Options options;
  options.block_compress_type = kSnappyCompression;
  options.filter_policy = std::make_unique<BloomFilter>(30);
  options.comparator = std::make_unique<ByteComparator>();
  DataBlockBuilder* blockBuilder = new DataBlockBuilder(&options);
  for (const auto& k : kTestKeys) {
    const string_view key = static_cast<string_view>(k);
    string v = "_value:";
    v += k;
    v += "_";
    const string_view value = static_cast<string_view>(v);
    blockBuilder->Add(key, value);
  }
  blockBuilder->Finish();
  string data = blockBuilder->Data();
  delete blockBuilder;
  string_view contents = data;
  DataBlock* dataBlock = new DataBlock(contents);
  Iterator* iter = dataBlock->NewIterator(options.comparator);
  iter->SeekToFirst();
  while (iter->Valid()) {
    cout << "[" << iter->key() << "," << iter->value() << "]"
         << " ";
    iter->Next();
  }
  cout << endl;
  iter->SeekToLast();
  while (iter->Valid()) {
    cout << "[" << iter->key() << "," << iter->value() << "]"
         << " ";
    iter->Prev();
  }
  cout << endl;
  for (int i = 0; i < 1900; i += 3) {
    string_view key = static_cast<string_view>(kTestKeys[i]);
    iter->Seek(key);
    cout << "[" << iter->key() << "," << iter->value() << "]"
         << " ";
  }
  cout << endl;
  delete dataBlock;
}
