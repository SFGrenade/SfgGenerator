#pragma once
#define CLAP_HAS_THREAD

// Project includes
#include "common/_fmt.hpp"

// Other lib includes
#include <clap/all.h>

// C++ std includes
#include <array>
#include <filesystem>
#include <istream>
#include <ostream>
#include <streambuf>

class ClapGlobals {
  public:
  static std::filesystem::path PLUGIN_PATH;
  static std::shared_ptr< spdlog::logger > PLUGIN_LOGGER;
};

class ClapStreamBuf final : public std::streambuf {
  public:
  explicit ClapStreamBuf( clap_istream_t const* stream );
  explicit ClapStreamBuf( clap_ostream_t const* stream );
  virtual ~ClapStreamBuf();

  protected:
  // read
  std::streamsize xsgetn( char* s, std::streamsize count ) override;
  int_type underflow() override;
  // write
  std::streamsize xsputn( char const* s, std::streamsize count ) override;
  int_type overflow( int_type ch ) override;

  int sync() override;

  private:
  clap_istream_t const* istream_ = nullptr;
  clap_ostream_t const* ostream_ = nullptr;
  std::array< char, 4096 > buffer_;
};

class ClapIStream : public std::istream {
  public:
  explicit ClapIStream( clap_istream_t const* stream ) : std::istream( &buf_ ), buf_( stream ) {}
  virtual ~ClapIStream() {}

  private:
  ClapStreamBuf buf_;
};

class ClapOStream : public std::ostream {
  public:
  explicit ClapOStream( clap_ostream_t const* stream ) : std::ostream( &buf_ ), buf_( stream ) {}
  virtual ~ClapOStream() {}

  private:
  ClapStreamBuf buf_;
};
