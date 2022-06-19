#include "db/skiplist.h"

#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>

#include "db/comparator.h"
#include "logger/log.h"
#include "memory/area.h"

using namespace std;
using namespace corekv;
static  vector<string> kTestKeys = {"corekv", "corekv1", "corekv2", "corekv3", "corekv4", "corekv5"};
TEST(skiplistTest, Insert) {
  corekv::LogConfig log_config;
  log_config.log_type = corekv::LogType::CONSOLE;
  log_config.rotate_size = 100;
  corekv::Log::GetInstance()->InitLog(log_config);
  using Table = SkipList<const char*, ByteComparator, SimpleVectorAlloc>;
  ByteComparator byte_comparator;
  Table tb(byte_comparator);
  for (int i = 0; i < 100; i++) {
    kTestKeys.emplace_back(std::to_string(i));
  }
  for (auto item : kTestKeys) {
    tb.Insert(item.c_str());
  }
  for (auto& item : kTestKeys) {
    cout << "[ key:" << item << ", has_existed:" << tb.Contains(item.c_str())
         << " ]" << endl;
  }
}