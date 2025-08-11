#pragma once

#include <mutex>
#include <string>

#include "_clap.hpp"
#include "_spdlog.hpp"

class ClapSink : public spdlog::sinks::base_sink< std::mutex > {
  public:
  explicit ClapSink( clap_host_t const* host, clap_host_log_t const* host_log );

  protected:
  void sink_it_( const spdlog::details::log_msg& msg ) override;
  void flush_() override;

  private:
  clap_host_t const* host_ = nullptr;
  clap_host_log_t const* host_log_ = nullptr;
};
