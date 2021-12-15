#include "dll_mappers/manual.h"

#include "logging.h"

namespace dll_loader::mappers {

namespace fs = std::filesystem;

ManualDllMapper::ManualDllMapper() : current_process_() {
  current_process_.Attach(::GetCurrentProcessId());
}

bool ManualDllMapper::LoadDll(const fs::path& dll_path) {
  LOG("Entered '{}'", __FUNCTION__);
  const auto result = current_process_.mmap().MapImage(dll_path.wstring());
  return result.success();
}

bool ManualDllMapper::UnloadAllDlls() {
  LOG("Entered '{}'", __FUNCTION__);
  return NT_SUCCESS(current_process_.mmap().UnmapAllModules());
}

}  // namespace dll_loader::mappers