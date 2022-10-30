#include "dll_mappers/win32.h"

#include "logging.h"
#include "scope_guard.h"

namespace dll_loader::mappers {

namespace fs = std::filesystem;

Win32DllMapper::Win32DllMapper(std::optional<std::string> on_load_function,
                               std::optional<std::string> on_unload_function)
    : IDllMapper(std::move(on_load_function), std::move(on_unload_function)),
      current_dll_handle_(),
      current_dll_path_() {}

bool Win32DllMapper::LoadDll(const fs::path& dll_path) {
  LOG("Entered '{}'", __FUNCTION__);

  std::error_code err{};
  const auto new_dll_path = fs::temp_directory_path() / dll_path.filename();
  fs::copy_file(dll_path, new_dll_path, fs::copy_options::overwrite_existing,
                err);
  if (err) {
    LOG("copy_file failed. Error: {}", err.message());
    return false;
  }
  auto remove_dll_on_error =
      sg::make_scope_guard([&]() { fs::remove(new_dll_path); });

  current_dll_handle_ = ::LoadLibraryW(new_dll_path.wstring().c_str());
  if (current_dll_handle_ == nullptr) {
    LOG("LoadLibraryW failed. LastError=0x{:x}", ::GetLastError());
    return false;
  }

  // Call the 'OnLoad' function if needed
  if (on_load_function_.has_value()) {
    const std::string& on_load_name = on_load_function_.value();

    const auto on_load = reinterpret_cast<OnLoadFunc>(
        ::GetProcAddress(current_dll_handle_, on_load_name.c_str()));
    if (on_load == nullptr) {
      LOG("'OnLoad' function '{}' not found", on_load_name);
      return false;
    }

    if (!on_load()) {
      LOG("'OnLoad' function '{}' failed", on_load_name);
      return false;
    }
  }

  // Do not remove the DLL in case of success
  remove_dll_on_error.dismiss();
  current_dll_path_ = new_dll_path;
  return true;
}

bool Win32DllMapper::UnloadAllDlls() {
  LOG("Entered '{}'", __FUNCTION__);

  // Call the 'OnUnload' function if needed
  if (on_unload_function_.has_value()) {
    const std::string& on_unload_name = on_unload_function_.value();

    const auto on_unload = reinterpret_cast<OnUnloadFunc>(
        ::GetProcAddress(current_dll_handle_, on_unload_name.c_str()));
    if (on_unload == nullptr) {
      LOG("'OnUnload' function '{}' not found", on_unload_name);
      // Proceed with the freeing anyway
    } else if (!on_unload()) {
      LOG("'OnUnload' function '{}' failed", on_unload_name);
      // Proceed with the freeing anyway
    }
  }

  // Free (and unload) the DLL
  if (current_dll_handle_ != nullptr) {
    if (::FreeLibrary(current_dll_handle_) == FALSE) {
      LOG("FreeLibrary failed. LastError=0x{:x}", ::GetLastError());
      return false;
    }
    current_dll_handle_ = nullptr;
  }

  std::error_code err{};
  fs::remove(current_dll_path_, err);
  if (err) {
    LOG("remove failed. Error: {}", err.message());
    return false;
  }
  current_dll_path_.clear();

  return true;
}

}  // namespace dll_loader::mappers