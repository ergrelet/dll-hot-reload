#pragma once

#include <windows.h>

#include <filesystem>
#include <optional>
#include <string>

#include "dll_mapper.h"

namespace dll_loader::mappers {

class Win32DllMapper final : public IDllMapper {
 public:
  Win32DllMapper(std::optional<std::string> on_load_function = {},
                 std::optional<std::string> on_unload_function = {});

  bool LoadDll(const std::filesystem::path& dll_path) override;
  bool UnloadAllDlls() override;

 private:
  HMODULE current_dll_handle_;
  std::filesystem::path current_dll_path_;
};

}  // namespace dll_loader::mappers
