#include "dll_mappers/win32.h"

#include "logging.h"

namespace dll_loader::mappers {

namespace fs = std::filesystem;

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

  current_dll_handle_ = ::LoadLibraryW(new_dll_path.wstring().c_str());
  if (current_dll_handle_ == nullptr) {
    LOG("LoadLibraryW failed. LastError=0x{:x}", ::GetLastError());
    fs::remove(new_dll_path);
    return false;
  }

  current_dll_path_ = new_dll_path;
  return true;
}

bool Win32DllMapper::UnloadAllDlls() {
  LOG("Entered '{}'", __FUNCTION__);
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