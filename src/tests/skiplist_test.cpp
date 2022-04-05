#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "db/skiplist.h"

#include "db/comparator.h"
#include "logger/log.h"
#include "memory/area.h"

using namespace std;
using namespace corekv;
static const vector<string> kTestKeys = {"corekv", "corekv1", "corekv2"};
TEST(skiplistTest, Insert) {
  using Table = SkipList<const char*, ByteComparator, SimpleVectorAlloc>;
  ByteComparator byte_comparator;
  Table tb(byte_comparator);
  tb.Insert("corekv");
  tb.Insert("corekv");
  tb.Insert("kuihuabaodian");
  for (const auto& item : kTestKeys) {
    cout << "[ key:" << item << ", has_existed:" << tb.Contains(item.c_str())
         << " ]" << endl;
  }
}