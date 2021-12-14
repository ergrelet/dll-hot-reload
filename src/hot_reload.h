#pragma once

#include <windows.h>

#include <atomic>

namespace dll_loader {

// Shared between threads
struct ThreadSync {
  HANDLE h_thread;
  std::atomic_bool should_stop;
  std::atomic_bool is_stopped;
};

bool InitializeHotReload(ThreadSync* p_context);

void CleanupHotReload(ThreadSync* p_context);

}  // namespace dll_loader
