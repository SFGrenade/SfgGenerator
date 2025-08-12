#include "_folders.hpp"

#include <cstdlib>
#include <string>

#if defined( _WIN32 ) || defined( _WIN64 )
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN 1
#include <Objbase.h>
#include <Shlobj.h>
#endif

std::filesystem::path get_data_path( std::string const& app, std::string const& company ) {
#if defined( _WIN32 ) || defined( _WIN64 )
  // windows
  PWSTR path_tmp = nullptr;
  if( S_OK == SHGetKnownFolderPath( FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &path_tmp ) ) {
    std::filesystem::path base( path_tmp );
    CoTaskMemFree( path_tmp );
    return base / company / app;
  }
  CoTaskMemFree( path_tmp );
  if( char const* local_app_data = std::getenv( "LOCALAPPDATA" ) ) {
    return std::filesystem::path( local_app_data ) / company / app;
  }
#elif defined( __APPLE__ )
  // apple
  if( char const* home = std::getenv( "HOME" ) ) {
    return std::filesystem::path( home ) / "Library" / "Application Support" / company / app;
  }
#else
  // linux
  if( char const* xdg = std::getenv( "XDG_DATA_HOME" ) ) {
    return std::filesystem::path( xdg ) / company / app;
  }
  if( char const* home = std::getenv( "HOME" ) ) {
    return std::filesystem::path( home ) / ".local" / "share" / company / app;
  }
#endif
  return {};
}
