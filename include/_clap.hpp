#pragma once
#define CLAP_HAS_THREAD

// Other lib includes
#include <clap/all.h>

// C++ std includes
#include <atomic>
#include <mutex>
#include <string>

static std::mutex g_entry_lock;
static std::atomic_int32_t g_entry_init_counter = 0;
static std::string g_plugin_path;
