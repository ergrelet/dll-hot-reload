#include "hot_reload.h"

#include <chrono>
#include <filesystem>

#include "configuration.h"
#include "dll_mappers/manual.h"
#include "dll_mappers/win32.h"
#include "logging.h"
#include "scope_guard.h"

#undef min

namespace dll_loader {

using namespace std::chrono_literals;
namespace fs = std::filesystem;

// Used only in the main thread
struct LoaderContext {
  const fs::path watched_dll_path;
  fs::file_time_type previous_write_time;
  std::chrono::time_point<std::chrono::system_clock> last_reload_time;
  IDllMapper::Ptr p_dll_mapper;
};

static DWORD WINAPI BackgroundThreadRoutine(LPVOID p_parameter);
static DWORD HotReloadServiceRoutine(ThreadSync* p_sync);
static bool ReloadDLL(LoaderContext* p_ctx);

bool HotReloadService::Initialize() {
  LOG("Entered '{}'", __FUNCTION__);
  h_thread_ = ::CreateThread(
      nullptr, 0,
      reinterpret_cast<LPTHREAD_START_ROUTINE>(BackgroundThreadRoutine),
      &thread_sync_, 0, nullptr);
  if (h_thread_ == nullptr) {
    return false;
  }

  LOG("Exiting '{}'", __FUNCTION__);
  return true;
}

void HotReloadService::Cleanup() {
  LOG("Entered '{}'", __FUNCTION__);

  TerminateChildThreadProperly();

  LOG("Exiting '{}'", __FUNCTION__);
}

void HotReloadService::TerminateChildThreadProperly() {
  if (h_thread_ == nullptr) {
    return;
  }

  // Wait for the background thread to finish its business
  thread_sync_.should_stop.store(true);
  thread_sync_.is_stopped.wait(false);

  // Ensure the thread does not execute any more code
  ::TerminateThread(h_thread_, 0);
  ::CloseHandle(h_thread_);
  h_thread_ = nullptr;
}

static DWORD WINAPI BackgroundThreadRoutine(LPVOID p_parameter) {
  LOG("Entered 'BackgroundThreadRoutine'");
  if (p_parameter == nullptr) {
    return 1;
  }
  auto* p_sync = reinterpret_cast<ThreadSync*>(p_parameter);

  const auto on_exit = sg::make_scope_guard([p_sync]() {
    p_sync->is_stopped.store(true);
    p_sync->is_stopped.notify_one();
  });
  try {
    return HotReloadServiceRoutine(p_sync);
  } catch ([[maybe_unused]] const std::exception& ex) {
    LOG("Unhandled exception in background thread: {}", ex.what());
    return 2;
  }
}

static DWORD HotReloadServiceRoutine(ThreadSync* p_sync) {
  LoaderConfiguration configuration{};
  if (!configuration.LoadFromFile("dll_loader.ini")) {
    LOG("Failed to load configuration");
    return 1;
  }

  std::error_code err{};
  LoaderContext ctx{
      .watched_dll_path = fs::absolute(configuration.dll_path_str),
      .previous_write_time = fs::file_time_type::min(),
      .p_dll_mapper = nullptr};
  if (configuration.use_manual_mapping) {
    ctx.p_dll_mapper = std::make_unique<mappers::ManualDllMapper>();
  } else {
    ctx.p_dll_mapper = std::make_unique<mappers::Win32DllMapper>();
  }

  if (!ReloadDLL(&ctx)) {
    LOG("Failed to load '{}'.", ctx.watched_dll_path.string());
    return 1;
  }
  const auto unload_on_exit =
      sg::make_scope_guard([p_sync, &configuration, &ctx]() {
        if (!configuration.use_manual_mapping && p_sync->should_stop.load()) {
          // Note: Cannot call `FreeLibrary` here if we were asked to stop since
          // it would mean loader lock's already been taken and we would
          // deadlock.
          return;
        }
        ctx.p_dll_mapper->UnloadAllDlls();
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
        // Change notification received, something has changed in the directory
        // we're watching
        constexpr auto kUpdateCooldown = 5s;

        std::error_code err{};
        const auto dll_last_write_time =
            fs::last_write_time(ctx.watched_dll_path, err);
        const auto current_time = std::chrono::system_clock::now();
        // Check last write time and last reload time
        if (dll_last_write_time >= ctx.previous_write_time + kUpdateCooldown &&
            current_time >= ctx.last_reload_time + kUpdateCooldown) {
          if (!ReloadDLL(&ctx)) {
            return 1;
          }
          ctx.last_reload_time = current_time;
          ctx.previous_write_time = dll_last_write_time;
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

static bool ReloadDLL(LoaderContext* p_ctx) {
  LOG("Entered '{}'", __FUNCTION__);
  if (p_ctx == nullptr) {
    return false;
  }

  // FIXME: Only works because we watch one DLL at a time
  if (!p_ctx->p_dll_mapper->UnloadAllDlls()) {
    return false;
  }

  return p_ctx->p_dll_mapper->LoadDll(p_ctx->watched_dll_path);
}

}  // namespace dll_loader