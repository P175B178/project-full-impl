#pragma once

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string_view>

namespace warden::test {

/// Parses protocol/examples/<name>: the messages both programs must agree on. The Pi has no source
/// tree, so tools/pi/test.sh passes the folder's path there.
inline nlohmann::json protocolExample(std::string_view name) {
  const char *copiedExamples = std::getenv("WARDEN_PROTOCOL_EXAMPLES"); // NOLINT(concurrency-mt-unsafe)
  const std::filesystem::path folder{copiedExamples != nullptr ? copiedExamples : WARDEN_PROTOCOL_EXAMPLES};
  std::ifstream file{folder / name};
  if (!file) {
    throw std::runtime_error{"cannot open protocol example " + (folder / name).string()};
  }
  return nlohmann::json::parse(file);
}

} // namespace warden::test
