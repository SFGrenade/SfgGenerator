// Header assigned to this source
#include "common/StandardFolders.hpp"

// C++ std includes
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

// System includes
#if defined( SFG_GEN_IS_LINUX )
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#elif defined( SFG_GEN_IS_MACOS )
#include <CoreFoundation/CoreFoundation.h>
#include <limits.h>
#include <pwd.h>
#include <unistd.h>
#elif defined( SFG_GEN_IS_WINDOWS )
#if !defined( WIN32_LEAN_AND_MEAN )
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined( NOMINMAX )
#define NOMINMAX
#endif
#include <ShlObj.h>
#include <Shlwapi.h>
#include <Windows.h>
#endif

inline std::vector< char > ALLOWED_CHARACTERS{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
                                               'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
                                               'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_' };

std::string StandardFolders::ResolveEmbeddedEnvVarsSinglePass( std::string const& source ) {
  std::string ret = source;

#if defined( SFG_GEN_IS_LINUX ) || defined( SFG_GEN_IS_MACOS )
  // env vars are e.g. `$HOME`
  for( auto startOfVarNameIter = ret.begin();
       ( startOfVarNameIter = std::find_if( ret.begin(), ret.end(), []( char c ) { return c == '$'; } ) ) != ret.end(); ) {
    auto endOfVarNameIter = std::find_if( startOfVarNameIter + 1, ret.end(), []( char c ) {
      return std::find_if( ALLOWED_CHARACTERS.begin(), ALLOWED_CHARACTERS.end(), [c]( char a ) { return a == c; } ) == ALLOWED_CHARACTERS.end();
    } );
    std::string varName = std::string( startOfVarNameIter + 1, endOfVarNameIter );
    std::string envValue;
    char* rawEnvValue = std::getenv( varName.c_str() );
    if( rawEnvValue ) {
      envValue = std::string( rawEnvValue );
    }
    ret.replace( startOfVarNameIter, endOfVarNameIter, envValue );
    startOfVarNameIter = endOfVarNameIter;
  }
#elif defined( SFG_GEN_IS_WINDOWS )
  // env vars are e.g. `%USERPROFILE%`
  for( auto startOfVarNameIter = ret.begin();
       ( startOfVarNameIter = std::find_if( ret.begin(), ret.end(), []( char c ) { return c == '%'; } ) ) != ret.end(); ) {
    auto endOfVarNameIter = std::find_if( startOfVarNameIter + 1, ret.end(), []( char c ) {
      return std::find_if( ALLOWED_CHARACTERS.begin(), ALLOWED_CHARACTERS.end(), [c]( char a ) { return a == c; } ) == ALLOWED_CHARACTERS.end();
    } );
    std::string varName = std::string( startOfVarNameIter + 1, endOfVarNameIter );
    std::string envValue;
    char* rawEnvValue = std::getenv( varName.c_str() );
    if( rawEnvValue ) {
      envValue = std::string( rawEnvValue );
    }
    // + 1 because of the trailing %
    ret.replace( startOfVarNameIter, endOfVarNameIter + 1, envValue );
    startOfVarNameIter = endOfVarNameIter + 1;
  }
#else
  return ret;
#endif
}

std::string StandardFolders::ResolveEmbeddedEnvVars( std::string const& source ) {
  std::string result = source;
  std::string previous;

  do {
    previous = result;
    result = ResolveEmbeddedEnvVarsSinglePass( previous );
  } while( result != previous );

  return result;
}

// Helper to convert wide char to UTF-8 string on Windows
#if defined( SFG_GEN_IS_WINDOWS )
std::string WideCharToUTF8( const wchar_t* wstr ) {
  if( !wstr || wstr[0] == L'\0' )
    return "";
  int len = WideCharToMultiByte( CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr );
  if( len <= 0 )
    return "";
  std::string result( len, '\0' );
  WideCharToMultiByte( CP_UTF8, 0, wstr, -1, &result[0], len, nullptr, nullptr );
  // Remove trailing null if present
  if( !result.empty() && result.back() == '\0' )
    result.pop_back();
  return result;
}

std::string GetKnownFolderPath( REFKNOWNFOLDERID rfid ) {
  // SHGetKnownFolderPath requires COM to be initialized on the thread
  HRESULT hr = CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE );
  if( FAILED( hr ) && hr != S_FALSE && hr != RPC_E_CHANGED_MODE ) {
    return "";
  }

  PWSTR path = nullptr;
  hr = SHGetKnownFolderPath( rfid, 0, nullptr, &path );
  CoUninitialize();

  if( SUCCEEDED( hr ) ) {
    std::string result = WideCharToUTF8( path );
    CoTaskMemFree( path );
    return result;
  }
  return "";
}
#endif

// Helper for macOS/Linux to get home directory
#if defined( SFG_GEN_IS_LINUX ) || defined( SFG_GEN_IS_MACOS )
static std::string GetHomeDirectory() {
#if defined( SFG_GEN_IS_LINUX )
  // Try HOME first
  if( const char* home = std::getenv( "HOME" ) ) {
    if( home && home[0] != '\0' )
      return home;
  }
  // Fall back to getpwuid
  struct passwd* pw = getpwuid( getuid() );
  if( pw && pw->pw_dir && pw->pw_dir[0] != '\0' )
    return pw->pw_dir;
#elif defined( SFG_GEN_IS_MACOS )
  // Try HOME first
  if( const char* home = std::getenv( "HOME" ) ) {
    if( home && home[0] != '\0' )
      return home;
  }
  // Fall back to CoreFoundation
  CFURLRef homeURL = CFCopyHomeDirectoryURLForCurrentUser();
  if( homeURL ) {
    CFStringRef path = CFURLCopyFileSystemPath( homeURL, kCFURLPOSIXPathStyle );
    if( path ) {
      char buffer[PATH_MAX];
      if( CFStringGetCString( path, buffer, sizeof( buffer ), kCFStringEncodingUTF8 ) ) {
        CFRelease( homeURL );
        CFRelease( path );
        return buffer;
      }
      CFRelease( path );
    }
    CFRelease( homeURL );
  }
  // Last resort: use getpwuid like Linux
  struct passwd* pw = getpwuid( getuid() );
  if( pw && pw->pw_dir && pw->pw_dir[0] != '\0' )
    return pw->pw_dir;
#endif
  return "";
}

static std::string GetXDGPath( const char* envVar, const char* defaultRelative ) {
  if( const char* val = std::getenv( envVar ) ) {
    if( val && val[0] != '\0' )
      return val;
  }
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += defaultRelative;
    return home;
  }
  return "";
}
#endif

std::string StandardFolders::Home() {
#if defined( SFG_GEN_IS_WINDOWS )
  if( const char* home = std::getenv( "USERPROFILE" ) )
    return ResolveEmbeddedEnvVars( home );
  return GetKnownFolderPath( FOLDERID_Profile );
#elif defined( SFG_GEN_IS_LINUX ) || defined( SFG_GEN_IS_MACOS )
  return GetHomeDirectory();
#else
  return "";
#endif
}

std::string StandardFolders::UserProfile() {
#if defined( SFG_GEN_IS_WINDOWS )
  if( const char* profile = std::getenv( "USERPROFILE" ) )
    return ResolveEmbeddedEnvVars( profile );
  return GetKnownFolderPath( FOLDERID_Profile );
#elif defined( SFG_GEN_IS_LINUX ) || defined( SFG_GEN_IS_MACOS )
  return GetHomeDirectory();
#else
  return "";
#endif
}

std::string StandardFolders::AppData() {
#if defined( SFG_GEN_IS_WINDOWS )
  if( const char* appdata = std::getenv( "APPDATA" ) )
    return ResolveEmbeddedEnvVars( appdata );
  return GetKnownFolderPath( FOLDERID_RoamingAppData );
#elif defined( SFG_GEN_IS_LINUX )
  // XDG config home
  return GetXDGPath( "XDG_CONFIG_HOME", ".config" );
#elif defined( SFG_GEN_IS_MACOS )
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += "Library/Application Support";
    return home;
  }
  return "";
#else
  return "";
#endif
}

std::string StandardFolders::Cache() {
#if defined( SFG_GEN_IS_WINDOWS )
  if( const char* localAppData = std::getenv( "LOCALAPPDATA" ) )
    return ResolveEmbeddedEnvVars( localAppData + std::string( "\\Microsoft\\Windows\\INetCache" ) );
  std::string path = GetKnownFolderPath( FOLDERID_LocalAppData );
  if( !path.empty() )
    return path + "\\Microsoft\\Windows\\INetCache";
  return "";
#elif defined( SFG_GEN_IS_LINUX )
  return GetXDGPath( "XDG_CACHE_HOME", ".cache" );
#elif defined( SFG_GEN_IS_MACOS )
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += "Library/Caches";
    return home;
  }
  return "";
#else
  return "";
#endif
}

std::string StandardFolders::Cookies() {
#if defined( SFG_GEN_IS_WINDOWS )
  return GetKnownFolderPath( FOLDERID_Cookies );
#elif defined( SFG_GEN_IS_LINUX )
  // Cookies are typically in cache directory
  std::string cache = Cache();
  if( !cache.empty() ) {
    if( cache.back() != '/' )
      cache += '/';
    cache += "cookies";
    return cache;
  }
  return "";
#elif defined( SFG_GEN_IS_MACOS )
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += "Library/Cookies";
    return home;
  }
  return "";
#else
  return "";
#endif
}

std::string StandardFolders::Desktop() {
#if defined( SFG_GEN_IS_WINDOWS )
  if( const char* desktop = std::getenv( "USERPROFILE" ) ) {
    std::string path = ResolveEmbeddedEnvVars( desktop );
    if( !path.empty() ) {
      if( path.back() != '\\' )
        path += '\\';
      path += "Desktop";
      return path;
    }
  }
  return GetKnownFolderPath( FOLDERID_Desktop );
#elif defined( SFG_GEN_IS_LINUX )
  return GetXDGPath( "XDG_DESKTOP_DIR", "Desktop" );
#elif defined( SFG_GEN_IS_MACOS )
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += "Desktop";
    return home;
  }
  return "";
#else
  return "";
#endif
}

std::string StandardFolders::Downloads() {
#if defined( SFG_GEN_IS_WINDOWS )
  if( const char* userProfile = std::getenv( "USERPROFILE" ) ) {
    std::string path = ResolveEmbeddedEnvVars( userProfile );
    if( !path.empty() ) {
      if( path.back() != '\\' )
        path += '\\';
      path += "Downloads";
      return path;
    }
  }
  return GetKnownFolderPath( FOLDERID_Downloads );
#elif defined( SFG_GEN_IS_LINUX )
  return GetXDGPath( "XDG_DOWNLOAD_DIR", "Downloads" );
#elif defined( SFG_GEN_IS_MACOS )
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += "Downloads";
    return home;
  }
  return "";
#else
  return "";
#endif
}

std::string StandardFolders::Favorites() {
#if defined( SFG_GEN_IS_WINDOWS )
  return GetKnownFolderPath( FOLDERID_Favorites );
#elif defined( SFG_GEN_IS_LINUX )
  // No standard XDG favorites directory, but some DEs use ~/.local/share/bookmarks
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += ".local/share/bookmarks";
    return home;
  }
  return "";
#elif defined( SFG_GEN_IS_MACOS )
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += "Library/Favorites";
    return home;
  }
  return "";
#else
  return "";
#endif
}

std::string StandardFolders::History() {
#if defined( SFG_GEN_IS_WINDOWS )
  return GetKnownFolderPath( FOLDERID_History );
#elif defined( SFG_GEN_IS_LINUX )
  // History is typically in cache
  std::string cache = Cache();
  if( !cache.empty() ) {
    if( cache.back() != '/' )
      cache += '/';
    cache += "history";
    return cache;
  }
  return "";
#elif defined( SFG_GEN_IS_MACOS )
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += "Library/Safari/History.db";
    return home;
  }
  return "";
#else
  return "";
#endif
}

std::string StandardFolders::LocalAppData() {
#if defined( SFG_GEN_IS_WINDOWS )
  if( const char* localAppData = std::getenv( "LOCALAPPDATA" ) )
    return ResolveEmbeddedEnvVars( localAppData );
  return GetKnownFolderPath( FOLDERID_LocalAppData );
#elif defined( SFG_GEN_IS_LINUX )
  // On Linux, LocalAppData is typically the same as cache or a subdirectory
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += ".local/share";
    return home;
  }
  return "";
#elif defined( SFG_GEN_IS_MACOS )
  // macOS doesn't distinguish between AppData and LocalAppData in the same way
  // Return Application Support which is closest equivalent
  return AppData();
#else
  return "";
#endif
}

std::string StandardFolders::Music() {
#if defined( SFG_GEN_IS_WINDOWS )
  return GetKnownFolderPath( FOLDERID_Music );
#elif defined( SFG_GEN_IS_LINUX )
  return GetXDGPath( "XDG_MUSIC_DIR", "Music" );
#elif defined( SFG_GEN_IS_MACOS )
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += "Music";
    return home;
  }
  return "";
#else
  return "";
#endif
}

std::string StandardFolders::Pictures() {
#if defined( SFG_GEN_IS_WINDOWS )
  return GetKnownFolderPath( FOLDERID_Pictures );
#elif defined( SFG_GEN_IS_LINUX )
  return GetXDGPath( "XDG_PICTURES_DIR", "Pictures" );
#elif defined( SFG_GEN_IS_MACOS )
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += "Pictures";
    return home;
  }
  return "";
#else
  return "";
#endif
}

std::string StandardFolders::Projects() {
#if defined( SFG_GEN_IS_WINDOWS )
  // Windows doesn't have a standard Projects folder, but we can use Documents
  return GetKnownFolderPath( FOLDERID_Documents );
#elif defined( SFG_GEN_IS_LINUX )
  return GetXDGPath( "XDG_PROJECTS_DIR", "Projects" );
#elif defined( SFG_GEN_IS_MACOS )
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += "Projects";
    return home;
  }
  return "";
#else
  return "";
#endif
}

std::string StandardFolders::Videos() {
#if defined( SFG_GEN_IS_WINDOWS )
  return GetKnownFolderPath( FOLDERID_Videos );
#elif defined( SFG_GEN_IS_LINUX )
  return GetXDGPath( "XDG_VIDEOS_DIR", "Videos" );
#elif defined( SFG_GEN_IS_MACOS )
  std::string home = GetHomeDirectory();
  if( !home.empty() ) {
    if( home.back() != '/' )
      home += '/';
    home += "Movies";
    return home;
  }
  return "";
#else
  return "";
#endif
}
