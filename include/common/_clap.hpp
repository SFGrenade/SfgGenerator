#pragma once
#define CLAP_HAS_THREAD

// Project includes
#include <common/_fmt.hpp>

// Other lib includes
#include <clap/all.h>

// C++ std includes
#include <atomic>
#include <filesystem>
#include <mutex>

class ClapGlobals {
  public:
  static std::filesystem::path PLUGIN_PATH;
  static std::shared_ptr< spdlog::logger > PLUGIN_LOGGER;
};
