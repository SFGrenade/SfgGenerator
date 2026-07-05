#pragma once

// C++ std includes
#include <string>

class StandardFolders {
  public:
  StandardFolders() = delete;
  StandardFolders( StandardFolders const& other ) = delete;
  StandardFolders( StandardFolders&& other ) = delete;
  StandardFolders& operator=( StandardFolders const& other ) = delete;
  StandardFolders& operator=( StandardFolders&& other ) = delete;

  public:
  static std::string ResolveEmbeddedEnvVarsSinglePass( std::string const& source );
  static std::string ResolveEmbeddedEnvVars( std::string const& source );

  // home, userprofile
  static std::string Home();
  static std::string UserProfile();

  // home, userprofile
  static std::string AppData();
  static std::string Cache();
  static std::string Cookies();
  static std::string Desktop();
  static std::string Downloads();
  static std::string Favorites();
  static std::string History();
  static std::string LocalAppData();
  static std::string Music();
  static std::string Pictures();
  static std::string Projects();
  static std::string Videos();
};
