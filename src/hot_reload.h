#pragma once

#include <windows.h>

#include <atomic>

namespace dll_loader {

// Shared between threads
struct ThreadSync {
  std::atomic_bool should_stop;
  std::atomic_bool is_stopped;
};

class HotReloadService {
 public:
  bool Initialize();
  void Cleanup();

 private:
  void TerminateChildThreadProperly();

 private:
  HANDLE h_thread_;
  ThreadSync thread_sync_;
};

}  // namespace dll_loader
