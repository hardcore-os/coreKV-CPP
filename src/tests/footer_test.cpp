#include "table/footer.h"

#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace corekv;
static const vector<string> kTestKeys = {"corekv", "corekv1", "corekv2"};
TEST(footerTest, EncodeTo) {
  OffSetSize filter_block;
  filter_block.offset = 11;
  filter_block.length = 12;
  OffSetSize index_block;
  index_block.offset = 34;
  index_block.length = 36;
  Footer footer;
  footer.SetFilterBlockMetaData(filter_block);
  footer.SetIndexBlockMetaData(index_block);
  std::string dst;
  footer.EncodeTo(&dst);
  std::string decode_str;
  std::string_view decode_view = decode_str;
  footer.DecodeFrom(&decode_view);
 std::cout<<"deseriablize:"<<footer.DebugString()<<std::endl;
}