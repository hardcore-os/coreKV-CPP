#include "memory/alloc.h"

#include <gtest/gtest.h>

#include <iostream>

#include "logger/log.h"

using namespace std;

TEST(allocTest, Allocate) {
  int nCount = 4;
  corekv::SimpleFreeListAlloc simple_freelist_alloc;
  int *a = (int *)simple_freelist_alloc.Allocate(sizeof(int) * nCount);
  for (int i = 0; i < nCount; ++i) {
    a[i] = i;
  }

  std::cout << "a[](" << a << "): ";
  for (int i = 0; i < nCount; ++i) {
    std::cout << a[i] << " ";
  }

  simple_freelist_alloc.Deallocate(a, sizeof(int) * nCount);

  std::cout << std::endl;
  std::cout << "a[](" << a << "): ";
  for (int i = 0; i < nCount; ++i) {
    std::cout << a[i] << " ";
  }
}


TEST(logTest, Log) {
  corekv::LogConfig log_config;
  log_config.log_path="./tests";
  log_config.log_type = corekv::LogType::FILE;
  log_config.rotate_size = 100;
  corekv::Log::GetInstance()->InitLog(log_config);
  LOG(corekv::LogLevel::ERROR, "name:%s,owner:%s,course:%s", "hardcore","logic,chen","corekv");
}