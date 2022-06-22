#include "manifest/manifest.h"

#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>

#include "logger/log.h"
#include "manifest/manifest_change_edit.h"

using namespace std;
using namespace corekv;
TEST(manifestTest, Insert) {
  corekv::LogConfig log_config;
  log_config.log_type = corekv::LogType::CONSOLE;
  log_config.rotate_size = 100;
  corekv::Log::GetInstance()->InitLog(log_config);
  ManifestChangeEdit manifest_change_edit;
  ManifestHandler manifest_handler("./");
  manifest_handler.AddTableMeta(1, 1111);
}