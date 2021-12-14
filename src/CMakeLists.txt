add_library(dll_loader MODULE
  "dllmain.cc"
  "logging.h"
  "scope_guard.h"
  "configuration.h"
  "configuration.cc"
  "hot_reload.h"
  "hot_reload.cc"
)
target_link_libraries(dll_loader PRIVATE inipp::inipp)

if(BUILD_TEST_EXE)
  add_executable(test_host "test_host.cc")
endif()