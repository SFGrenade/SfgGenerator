// Header assigned to this source
#include "common/_clap.hpp"

std::mutex ClapGlobals::ENTRY_LOCK;
std::atomic_int32_t ClapGlobals::ENTRY_INIT_COUNTER = 0;
std::string ClapGlobals::PLUGIN_PATH = "";
