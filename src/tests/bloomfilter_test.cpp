#include "filter/bloomfilter.h"

#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

using namespace std;
using namespace corekv;
static const vector<string> kTestKeys = {"corekv", "corekv1", "corekv2"};
TEST(bloomFilterTest, CreateFilter) {
  std::unique_ptr<FilterPolicy> filter_policy =
      std::make_unique<BloomFilter>(30);
  vector<string_view> tmp;
  for (const auto& item : kTestKeys) {
    tmp.emplace_back(item);
  }
  filter_policy->CreateFilter(&tmp[0], tmp.size());
  tmp.emplace_back("hardcore");
  for (const auto& item : tmp) {
    cout << "[ key:" << item
         << ", has_existed:" << filter_policy->MayMatch(item, 0, 0) << " ]"
         << endl;
  }
}