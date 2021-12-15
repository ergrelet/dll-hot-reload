add_library(dll_loader MODULE
  "dllmain.cc"
  "logging.h"
  "scope_guard.h"
  "configuration.h"
  "configuration.cc"
  "hot_reload.h"
  "hot_reload.cc"
  "dll_mapper.h"

  "dll_mappers/win32.h"
  "dll_mappers/win32.cc"
  "dll_mappers/manual.h"
  "dll_mappers/manual.cc"
)
target_link_libraries(dll_loader PRIVATE inipp::inipp BlackBone)
target_include_directories(dll_loader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TEST_EXE)
  add_executable(test_host "test_host.cc")
endif()