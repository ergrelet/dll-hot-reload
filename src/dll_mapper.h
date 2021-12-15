#pragma once

#include <filesystem>
#include <memory>

namespace dll_loader {

class IDllMapper {
 public:
  using Ptr = std::unique_ptr<IDllMapper>;

  virtual ~IDllMapper() = default;
  virtual bool LoadDll(const std::filesystem::path& dll_path) = 0;
  virtual bool UnloadAllDlls() = 0;
};

}  // namespace dll_loader
