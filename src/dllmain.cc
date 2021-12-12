#include <windows.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <format>
#include <string>
#include <thread>

#include "logging.h"
#include "scope_guard.h"

#undef min

using namespace std::chrono_literals;

namespace dll_loader {

namespace fs = std::filesystem;

// Shared between threads
struct ThreadSync {
  HANDLE h_thread;
  std::atomic_bool should_stop;
  std::atomic_bool is_stopped;
};

// Used only in the main thread
struct LoaderContext {
  const fs::path watched_dll_path;
  const fs::path temp_dll_path;
  HMODULE current_dll_handle;
  fs::file_time_type previous_write_time;
};

static DWORD HotReloadService(ThreadSync* p_parameter);
static bool ReloadDLL(LoaderContext* p_ctx);
static bool UnloadAndDeleteTempDLL(LoaderContext* p_ctx);

DWORD WINAPI BackgroundThreadRoutine(LPVOID p_parameter) {
  LOG("Entered 'BackgroundThreadRoutine'");
  if (p_parameter == nullptr) {
    return 1;
  }
  auto* p_sync = reinterpret_cast<ThreadSync*>(p_parameter);

  const auto on_exit = sg::make_scope_guard([p_sync]() {
    p_sync->is_stopped.store(true);
    p_sync->is_stopped.notify_one();
    LOG("Exiting 'BackgroundThreadRoutine'");
  });
  try {
    return HotReloadService(p_sync);
  } catch (const std::exception& ex) {
    LOG("Unhandled exception in background thread: {}", ex.what());
    return 2;
  }
}

DWORD HotReloadService(ThreadSync* p_sync) {
  // TODO: Parse config
  std::error_code err{};
  const auto dll_path = fs::absolute("reloaded.dll");
  const auto new_dll_path = fs::temp_directory_path() / dll_path.filename();
  LoaderContext ctx{.watched_dll_path = dll_path,
                    .temp_dll_path = new_dll_path,
                    .current_dll_handle = nullptr,
                    .previous_write_time = fs::file_time_type::min()};
  if (!ReloadDLL(&ctx)) {
    LOG("Failed to load '{}'. LastError=0x{:x}", ctx.watched_dll_path.string(),
        ::GetLastError());
    return 1;
  }
  const auto unload_on_exit = sg::make_scope_guard([p_sync, &ctx]() {
    if (!p_sync->should_stop.load()) {
      // Note: Cannot call `FreeLibrary` here if we were asked to stop since it
      // would mean loader lock's already been taken and we would deadlock.
      UnloadAndDeleteTempDLL(&ctx);
    }
  });

  const auto parent_directory_path = ctx.watched_dll_path.parent_path();
  LOG("Watching '{}' for modifications", parent_directory_path.string());
  const HANDLE h_change = ::FindFirstChangeNotificationA(
      parent_directory_path.string().c_str(),  // directory to watch
      FALSE,                                   // do not watch subtree
      FILE_NOTIFY_CHANGE_LAST_WRITE);
  if (h_change == INVALID_HANDLE_VALUE) {
    return 1;
  }
  const auto close_on_exit = sg::make_scope_guard([h_change]() {
    if (h_change != INVALID_HANDLE_VALUE) {
      ::FindCloseChangeNotification(h_change);
    }
  });

  while (true) {
    constexpr DWORD kWaitTimeoutMs = 200;
    const auto wait_status = ::WaitForSingleObject(h_change, kWaitTimeoutMs);
    switch (wait_status) {
      case WAIT_TIMEOUT:
        // Exit if needed
        if (p_sync->should_stop.load()) {
          return 1;
        }
        break;
      case WAIT_OBJECT_0: {
        // Change notification received
        constexpr auto kUpdateCooldown = 5s;
        // Check last write time
        std::error_code err{};
        const auto last_write_time =
            fs::last_write_time(ctx.watched_dll_path, err);
        if (last_write_time >= ctx.previous_write_time + kUpdateCooldown) {
          if (!ReloadDLL(&ctx)) {
            return 1;
          }
          ctx.previous_write_time = last_write_time;
        }
        // Register to future notifications
        if (::FindNextChangeNotification(h_change) == FALSE) {
          LOG("FindNextChangeNotification failed. LastError=0x{:x}",
              ::GetLastError());
          return 1;
        }
      } break;
      default:
        // Error
        return 1;
    }
  }

  return 0;
}

bool Initialize(ThreadSync* p_context) {
  LOG("Entered '{}'", __FUNCTION__);
  if (p_context == nullptr) {
    return false;
  }

  p_context->h_thread = ::CreateThread(nullptr, 0,
                                       reinterpret_cast<LPTHREAD_START_ROUTINE>(
                                           dll_loader::BackgroundThreadRoutine),
                                       p_context, 0, nullptr);
  if (p_context->h_thread == nullptr) {
    return false;
  }

  LOG("Exiting '{}'", __FUNCTION__);
  return true;
}

void Cleanup(ThreadSync* p_context) {
  LOG("Entered '{}'", __FUNCTION__);
  if (p_context == nullptr) {
    return;
  }

  // Wait for the background thread to finish its business
  p_context->should_stop.store(true);
  p_context->is_stopped.wait(false);
  ::CloseHandle(p_context->h_thread);
  LOG("Exiting '{}'", __FUNCTION__);
}

bool ReloadDLL(LoaderContext* p_ctx) {
  LOG("Entered '{}'", __FUNCTION__);
  if (p_ctx == nullptr) {
    return false;
  }

  if (!UnloadAndDeleteTempDLL(p_ctx)) {
    return false;
  }

  std::error_code err{};
  fs::copy_file(p_ctx->watched_dll_path, p_ctx->temp_dll_path, err);
  if (err) {
    LOG("copy_file failed. Error: {}", err.message());
    return false;
  }

  p_ctx->current_dll_handle =
      ::LoadLibraryA(p_ctx->temp_dll_path.string().c_str());
  LOG("Exiting '{}'", __FUNCTION__);
  return p_ctx->current_dll_handle != nullptr;
}

bool UnloadAndDeleteTempDLL(LoaderContext* p_ctx) {
  LOG("Entered '{}'", __FUNCTION__);
  if (p_ctx == nullptr) {
    return {};
  }

  // Free (and unload) the DLL
  if (p_ctx->current_dll_handle != nullptr) {
    if (::FreeLibrary(p_ctx->current_dll_handle) == FALSE) {
      LOG("FreeLibrary failed. LastError=0x{:x}", ::GetLastError());
      return false;
    }
    p_ctx->current_dll_handle = nullptr;
  }

  std::error_code err{};
  fs::remove(p_ctx->temp_dll_path, err);
  if (err) {
    LOG("remove failed. Error: {}", err.message());
    return false;
  }

  return true;
}

}  // namespace dll_loader

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
  LOG("Entered '{}'", __FUNCTION__);
  static dll_loader::ThreadSync context{};

  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      ::DisableThreadLibraryCalls(hinstDLL);
      if (!dll_loader::Initialize(&context)) {
        return FALSE;
      }
      break;
    case DLL_PROCESS_DETACH:
      dll_loader::Cleanup(&context);
      break;
  }
  return TRUE;
}