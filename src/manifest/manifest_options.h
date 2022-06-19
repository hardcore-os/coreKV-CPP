#pragma once
#include <stdint.h>

#include <string>

namespace corekv {
// manifest相关的文件
namespace ManifestOptions {
  // manifest后缀名
  static const std::string kManifestName = "MANIFEST";
  static const std::string kManifestRewriteFilename = "REWRITEMANIFEST";
  static constexpr uint32_t kManifestDeletionsRewriteThreshold = 10000;
  static constexpr uint32_t kManifestDeletionsRatio = 10;
};
}  // namespace corekv
