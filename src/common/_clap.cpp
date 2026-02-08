// Header assigned to this source
#include "common/_clap.hpp"

std::filesystem::path ClapGlobals::PLUGIN_PATH;
std::shared_ptr< spdlog::logger > ClapGlobals::PLUGIN_LOGGER;

ClapStreamBuf::ClapStreamBuf( clap_istream_t const* stream ) : istream_( stream ) {
  setg( buffer_.data(), buffer_.data(), buffer_.data() );
}

ClapStreamBuf::ClapStreamBuf( clap_ostream_t const* stream ) : ostream_( stream ) {
  setg( buffer_.data(), buffer_.data(), buffer_.data() );
}

ClapStreamBuf::~ClapStreamBuf() {}

std::streamsize ClapStreamBuf::xsgetn( char* s, std::streamsize count ) {
  int64_t r = istream_->read( istream_, s, count );
  return r > 0 ? static_cast< std::streamsize >( r ) : 0;
}

ClapStreamBuf::int_type ClapStreamBuf::underflow() {
  if( gptr() < egptr() )
    return traits_type::to_int_type( *gptr() );

  int64_t r = istream_->read( istream_, buffer_.data(), buffer_.size() );
  if( r <= 0 )
    return traits_type::eof();

  setg( buffer_.data(), buffer_.data(), buffer_.data() + r );
  return traits_type::to_int_type( *gptr() );
}

std::streamsize ClapStreamBuf::xsputn( char const* s, std::streamsize count ) {
  int64_t r = ostream_->write( ostream_, s, count );
  return r > 0 ? static_cast< std::streamsize >( r ) : 0;
}

ClapStreamBuf::int_type ClapStreamBuf::overflow( ClapStreamBuf::int_type ch ) {
  if( ch == traits_type::eof() )
    return traits_type::eof();

  char c = static_cast< char >( ch );
  int64_t r = ostream_->write( ostream_, &c, 1 );
  return r == 1 ? ch : traits_type::eof();
}

int ClapStreamBuf::sync() {
  return 0;
}
