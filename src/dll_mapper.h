#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace dll_loader {

class IDllMapper {
 public:
  using Ptr = std::unique_ptr<IDllMapper>;

  using OnLoadFunc = bool (*)();
  using OnUnloadFunc = bool (*)();

  IDllMapper(std::optional<std::string> on_load_function = {},
             std::optional<std::string> on_unload_function = {})
      : on_load_function_(std::move(on_load_function)),
        on_unload_function_(std::move(on_unload_function)) {}

  virtual ~IDllMapper() = default;
  virtual bool LoadDll(const std::filesystem::path& dll_path) = 0;
  virtual bool UnloadAllDlls() = 0;

 protected:
  std::optional<std::string> on_load_function_{};
  std::optional<std::string> on_unload_function_{};
};

}  // namespace dll_loader
