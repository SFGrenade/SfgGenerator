// Other lib includes
#define CLAP_HAS_THREAD
#include <clap/all.h>

// C++ std includes
#include <filesystem>
#include <mutex>

// System includes
#if defined( _WIN32 ) || defined( _WIN64 )
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
typedef HMODULE LoadedDllHandle;
#elif defined( __APPLE__ ) || defined( __linux__ )
#include <dlfcn.h>
typedef void* LoadedDllHandle;
#endif

typedef bool ( *ClapEntryInitFunc )( char const* plugin_path );
typedef void ( *ClapEntryDeinitFunc )( void );
typedef void const* ( *ClapEntryGetFactoryFunc )( char const* factory_id );

static std::mutex G_ENTRY_LOCK;
static std::atomic_int32_t G_ENTRY_INIT_COUNTER = 0;

LoadedDllHandle g_sfgGeneratorMainHandle;

ClapEntryInitFunc entry_init = nullptr;
ClapEntryDeinitFunc entry_deinit = nullptr;
ClapEntryGetFactoryFunc entry_get_factory = nullptr;

LoadedDllHandle sfg_load_library( std::filesystem::path const& library_path, std::string const& library_name ) {
#if defined( _WIN32 ) || defined( _WIN64 )
  return LoadLibraryW( ( library_path / ( library_name + ".dll" ) ).wstring().c_str() );
#elif defined( __APPLE__ ) || defined( __linux__ )
  return dlopen( ( library_path / ( "lib" + library_name + ".so" ) ).string().c_str(), RTLD_NOW | RTLD_GLOBAL );
#endif
}

void sfg_free_library( LoadedDllHandle library ) {
#if defined( _WIN32 ) || defined( _WIN64 )
  FreeLibrary( library );
#elif defined( __APPLE__ ) || defined( __linux__ )
  dlclose( library );
#endif
}

void* sfg_load_symbol( LoadedDllHandle library, const char* name ) {
#if defined( _WIN32 ) || defined( _WIN64 )
  return reinterpret_cast< void* >( GetProcAddress( library, name ) );
#elif defined( __APPLE__ ) || defined( __linux__ )
  return dlsym( library, name );
#endif
}

void load_additional_libraries( std::filesystem::path const& base_path ) {
#if defined( _WIN32 ) || defined( _WIN64 )
  SetDllDirectoryW( ( base_path / "qt" ).wstring().c_str() );
#endif

  g_sfgGeneratorMainHandle = sfg_load_library( base_path / "qt", "SfgGeneratorMain" );

  entry_init = reinterpret_cast< ClapEntryInitFunc >( sfg_load_symbol( g_sfgGeneratorMainHandle, "entry_init" ) );
  entry_deinit = reinterpret_cast< ClapEntryDeinitFunc >( sfg_load_symbol( g_sfgGeneratorMainHandle, "entry_deinit" ) );
  entry_get_factory = reinterpret_cast< ClapEntryGetFactoryFunc >( sfg_load_symbol( g_sfgGeneratorMainHandle, "entry_get_factory" ) );
}

// thread safe init counter
bool entry_init_guard( char const* plugin_path ) {
  std::lock_guard< std::mutex > _( G_ENTRY_LOCK );
  int const cnt = ++G_ENTRY_INIT_COUNTER;
  if( cnt > 1 )
    return true;
  load_additional_libraries( std::filesystem::path( plugin_path ).parent_path() );
  if( entry_init && entry_init( plugin_path ) )
    return true;
  G_ENTRY_INIT_COUNTER = 0;
  return false;
}

// thread safe deinit counter
void entry_deinit_guard( void ) {
  std::lock_guard< std::mutex > _( G_ENTRY_LOCK );
  int const cnt = --G_ENTRY_INIT_COUNTER;
  if( cnt == 0 ) {
    if( entry_deinit )
      entry_deinit();

    sfg_free_library( g_sfgGeneratorMainHandle );
  }
}

void const* entry_get_factory_guard( char const* factory_id ) {
  if( ( G_ENTRY_INIT_COUNTER <= 0 ) || !entry_get_factory )
    return nullptr;
  return entry_get_factory( factory_id );
}

#if __cplusplus
extern "C" {
#endif

CLAP_EXPORT clap_plugin_entry_t const clap_entry = {
    .clap_version = CLAP_VERSION_INIT,
    .init = entry_init_guard,
    .deinit = entry_deinit_guard,
    .get_factory = entry_get_factory_guard,
};

#if __cplusplus
}
#endif
