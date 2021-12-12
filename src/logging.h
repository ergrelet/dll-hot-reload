#pragma once

#ifdef _DEBUG
#define LOG(FMT, ...)                                    \
  {                                                      \
    const auto msg = std::format(FMT "\n", __VA_ARGS__); \
    ::OutputDebugStringA(msg.c_str());                   \
  }
#else
#define LOG(FMT, ..)
#endif
