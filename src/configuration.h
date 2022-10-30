#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace dll_loader {

struct LoaderConfiguration {
  std::string dll_path_str{};
  bool use_manual_mapping{};
  std::optional<std::string> on_load_function{};
  std::optional<std::string> on_unload_function{};

  bool LoadFromFile(const std::filesystem::path& configuration_path);
};

}  // namespace dll_loader