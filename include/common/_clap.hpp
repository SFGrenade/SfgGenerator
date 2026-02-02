#pragma once
#define CLAP_HAS_THREAD

// Other lib includes
#include <clap/all.h>

// C++ std includes
#include <atomic>
#include <mutex>
#include <string>

class ClapGlobals {
  public:
  static std::mutex ENTRY_LOCK;
  static std::atomic_int32_t ENTRY_INIT_COUNTER;
  static std::string PLUGIN_PATH;
};
