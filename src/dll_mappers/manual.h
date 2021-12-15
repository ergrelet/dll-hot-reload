#pragma once

#include <BlackBone/Process/Process.h>

#include "dll_mapper.h"

namespace dll_loader::mappers {

class ManualDllMapper final : public IDllMapper {
 public:
  ManualDllMapper();

  bool LoadDll(const std::filesystem::path& dll_path) override;
  bool UnloadAllDlls() override;

 private:
  blackbone::Process current_process_;
};

}  // namespace dll_loader::mappers
