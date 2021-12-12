add_library(dll_loader MODULE
  "dllmain.cc"
  "logging.h"
  "scope_guard.h"
)

if(BUILD_TEST_EXE)
  add_executable(test_host "test_host.cc")
endif()