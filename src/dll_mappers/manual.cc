#include "dll_mappers/manual.h"

#include "logging.h"

namespace dll_loader::mappers {

namespace fs = std::filesystem;

ManualDllMapper::ManualDllMapper(std::optional<std::string> on_load_function,
                                 std::optional<std::string> on_unload_function)
    : IDllMapper(std::move(on_load_function), std::move(on_unload_function)),
      current_process_(),
      loaded_module_() {
  current_process_.Attach(::GetCurrentProcessId());
}

bool ManualDllMapper::LoadDll(const fs::path& dll_path) {
  LOG("Entered '{}'", __FUNCTION__);
  auto result = current_process_.mmap().MapImage(dll_path.wstring());
  if (!result.success()) {
    LOG("MapImage failed. Result=0x{:x}", result.status);
    return false;
  }
  loaded_module_ = result.result();

  // Call the 'OnLoad' function if needed
  if (on_load_function_.has_value()) {
    const std::string& on_load_name = on_load_function_.value();

    auto getexport_result = current_process_.modules().GetExport(
        loaded_module_, on_load_name.c_str());
    if (!getexport_result.success()) {
      LOG("'OnLoad' function '{}' not found", on_load_name);
      return false;
    }

    const auto on_load =
        reinterpret_cast<OnLoadFunc>(getexport_result.result().procAddress);
    if (!on_load()) {
      LOG("'OnLoad' function '{}' failed", on_load_name);
      return false;
    }
  }

  return true;
}

bool ManualDllMapper::UnloadAllDlls() {
  LOG("Entered '{}'", __FUNCTION__);

  // Call the 'OnUnload' function if needed
  if (on_unload_function_.has_value()) {
    const std::string& on_unload_name = on_unload_function_.value();

    auto getexport_result = current_process_.modules().GetExport(
        loaded_module_, on_unload_name.c_str());
    if (!getexport_result.success()) {
      LOG("'OnUnload' function '{}' not found", on_unload_name);
      // Proceed with the freeing anyway
    } else {
      const auto on_unload =
          reinterpret_cast<OnUnloadFunc>(getexport_result.result().procAddress);
      if (!on_unload()) {
        LOG("'OnUnload' function '{}' failed", on_unload_name);
        // Proceed with the freeing anyway
      }
    }
  }

  loaded_module_.reset();
  return NT_SUCCESS(current_process_.mmap().UnmapAllModules());
}

}  // namespace dll_loader::mappers