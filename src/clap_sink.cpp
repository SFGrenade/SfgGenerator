#include "clap_sink.hpp"

ClapSink::ClapSink( clap_host_t const* host, clap_host_log_t const* host_log ) : host_( host ), host_log_( host_log ) {}

void ClapSink::sink_it_( const spdlog::details::log_msg& msg ) {
  // Format the message first
  spdlog::memory_buf_t formatted;
  base_sink< std::mutex >::formatter_->format( msg, formatted );

  // Call the callback with the resulting string
  std::string output_string = fmt::to_string( formatted );
  if( host_log_ && host_ )
    host_log_->log( host_, CLAP_LOG_DEBUG, output_string.c_str() );
}

void ClapSink::flush_() {
  // No flushing needed for function callback
}
