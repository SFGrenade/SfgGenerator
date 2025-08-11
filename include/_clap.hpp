#pragma once
#define CLAP_HAS_THREAD

#include <atomic>
#include <clap/all.h>
#include <mutex>

static std::mutex g_entry_lock;
static std::atomic_int32_t g_entry_init_counter = 0;
static std::string g_plugin_path;
