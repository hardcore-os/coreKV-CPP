#include "filter/bloomfilter.h"

#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "utils/codec.h"
#include "utils/crc32.h"
using namespace std;
using namespace corekv;
using namespace corekv::crc32;
static const vector<string> kTestKeys = {"corekv",  "corekv1", "corekv2",
                                         "corekv3", "corekv4", "corekv7"};
TEST(bloomFilterTest, CreateFilter) {
  std::unique_ptr<FilterPolicy> filter_policy =
      std::make_unique<BloomFilter>(30);
  vector<string> tmp;
  for (const auto& item : kTestKeys) {
    tmp.emplace_back(item);
  }
  filter_policy->CreateFilter(&tmp[0], tmp.size());
  std::string src = filter_policy->Data();
  util::PutFixed32(&src, filter_policy->GetMeta().hash_num);
  char trailer[5];
  trailer[0] = static_cast<uint8_t>(0);
  uint32_t crc = crc32::Value(src.data(), src.size());
  cout << "crc:" << crc << endl;
  tmp.emplace_back("hardcore");
  for (const auto& item : tmp) {
    cout << "[ key:" << item
         << ", has_existed:" << filter_policy->MayMatch(item, 0, 0) << " ]"
         << endl;
  }
}