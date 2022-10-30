#pragma once

#include <BlackBone/Process/Process.h>

#include <optional>
#include <string>

#include "dll_mapper.h"

namespace dll_loader::mappers {

class ManualDllMapper final : public IDllMapper {
 public:
  ManualDllMapper(std::optional<std::string> on_load_function = {},
                  std::optional<std::string> on_unload_function = {});

  bool LoadDll(const std::filesystem::path& dll_path) override;
  bool UnloadAllDlls() override;

 private:
  blackbone::Process current_process_;
  blackbone::ModuleDataPtr loaded_module_;
};

}  // namespace dll_loader::mappers
