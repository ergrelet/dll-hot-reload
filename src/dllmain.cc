#include <windows.h>

#include "hot_reload.h"
#include "logging.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
  LOG("Entered '{}' - Reason 0x{:x}", __FUNCTION__, fdwReason);
  static dll_loader::HotReloadService context{};

  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      if (!context.Initialize()) {
        return FALSE;
      }
      break;
    case DLL_PROCESS_DETACH:
      context.Cleanup();
      break;
  }
  return TRUE;
}