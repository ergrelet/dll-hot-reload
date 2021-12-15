#pragma once

#include <filesystem>
#include <string>

namespace dll_loader {

struct LoaderConfiguration {
  std::string dll_path_str{};
  bool use_manual_mapping{};

  bool LoadFromFile(const std::filesystem::path& configuration_path);
};

}  // namespace dll_loader