#pragma once

#include <windows.h>

#include <filesystem>

#include "dll_mapper.h"

namespace dll_loader::mappers {

class Win32DllMapper final : public IDllMapper {
 public:
  bool LoadDll(const std::filesystem::path& dll_path) override;
  bool UnloadAllDlls() override;

 private:
  HMODULE current_dll_handle_;
  std::filesystem::path current_dll_path_;
};

}  // namespace dll_loader::mappers
