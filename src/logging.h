#pragma once

#ifdef _DEBUG
#include <windows.h>

#include <format>

#define LOG(FMT, ...)                                    \
  {                                                      \
    const auto msg = std::format(FMT "\n", __VA_ARGS__); \
    ::OutputDebugStringA(msg.c_str());                   \
  }
#else
#define LOG(FMT, ..)
#endif
