#include <windows.h>

#include <cstdio>

int main(int argc, char* argv[]) {
  const HMODULE h_library = ::LoadLibraryA("dll_loader.dll");
  if (h_library == nullptr) {
    return 1;
  }

  ::printf("Press any key to exit...\n");
  int _res = ::getc(stdin);

  ::FreeLibrary(h_library);
  return 0;
}