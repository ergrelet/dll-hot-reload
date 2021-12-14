#include <windows.h>

#include "hot_reload.h"
#include "logging.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
  LOG("Entered '{}'", __FUNCTION__);
  static dll_loader::ThreadSync context{};

  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      ::DisableThreadLibraryCalls(hinstDLL);
      if (!dll_loader::InitializeHotReload(&context)) {
        return FALSE;
      }
      break;
    case DLL_PROCESS_DETACH:
      dll_loader::CleanupHotReload(&context);
      break;
  }
  return TRUE;
}