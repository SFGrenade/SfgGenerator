#include "param_multiplex.hpp"

#include <exception>
#include <numbers>

#include "clap/events.h"


namespace SfPb = SfgGenerator::Proto;

ParamMultiplex::ParamMultiplex() : _base_() {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

ParamMultiplex::~ParamMultiplex() {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
}

std::string ParamMultiplex::get_name( void ) const {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void const* >( this ) );
  return "ParamMultiplex";
}

#pragma region shit to override

bool ParamMultiplex::init( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  bool ret = _base_::init();

  state_.Clear();
  state_.set_output_param( 0.0 );
  state_.set_amount_params( 16 );
  state_.set_selected_param( 1 );
  state_.mutable_params()->Resize( state_.amount_params(), 0.0 );

  ret = ret && true;
  return ret;
}

void ParamMultiplex::reset( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  _base_::reset();

  state_.Clear();
  state_.set_output_param( 0.0 );
  state_.set_amount_params( 16 );
  state_.set_selected_param( 1 );
  state_.mutable_params()->Resize( state_.amount_params(), 0.0 );
}

void ParamMultiplex::process_event( clap_event_header_t const* hdr, clap_output_events_t const* out_events ) {
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( hdr={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( hdr ) );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->size    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->size );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->time    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->time );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->space_id={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->space_id );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->type    ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->type );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] hdr->flags   ={:d} )", __FUNCTION__, static_cast< void* >( this ), hdr->flags );
  if( hdr->space_id != CLAP_CORE_EVENT_SPACE_ID ) {
    return;
  }
  if( hdr->type != CLAP_EVENT_PARAM_VALUE ) {
    return;
  }
  clap_event_param_value_t const* ev = reinterpret_cast< clap_event_param_value_t const* >( hdr );
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] CLAP_EVENT_PARAM_VALUE - param_id={:d}, cookie={:p}, note_id={:d}, port_index={:d}, channel={:d}, key={:d}, value={:f}",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 ev->param_id,
                 ev->cookie,
                 ev->note_id,
                 ev->port_index,
                 ev->channel,
                 ev->key,
                 ev->value );
  if( ev->param_id == 1 ) {
    // this is output
    state_.set_output_param( ev->value );
  } /*else if( ev->param_id == 2 ) {
    // we don't support resizing the amount of params
    state_.set_amount_params( ev->value );
    state_.mutable_params()->Resize( state_.amount_params(), 0.0 );
    host_params_->rescan( host_, CLAP_PARAM_RESCAN_ALL );
  }*/
  else if( ev->param_id == 3 ) {
    state_.set_selected_param( ev->value );
    {
      // send event to check for output param
      clap_event_param_value_t out_ev{};
      out_ev.header.size = sizeof( out_ev );
      out_ev.header.time = hdr->time;
      out_ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
      out_ev.header.type = CLAP_EVENT_PARAM_VALUE;
      out_ev.header.flags = CLAP_EVENT_IS_LIVE;
      out_ev.param_id = 0;
      this->params_get_value( 0, &out_ev.value );
      bool success = out_events->try_push( out_events, &out_ev.header );
      SFG_LOG_DEBUG( host_, host_log_, "[{:s}] [{:p}] sending event = {}", __FUNCTION__, static_cast< void* >( this ), success );
    }
  } else if( ( ev->param_id - 4 ) < state_.amount_params() ) {
    state_.set_params( ev->param_id - 4, ev->value );
    if( ( ev->param_id - 4 ) == ( state_.selected_param() - 1 ) ) {
      // if selected param was changed, have it rescan for the output param
      {
        // send event to check for output param
        clap_event_param_value_t out_ev{};
        out_ev.header.size = sizeof( out_ev );
        out_ev.header.time = hdr->time;
        out_ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
        out_ev.header.type = CLAP_EVENT_PARAM_VALUE;
        out_ev.header.flags = CLAP_EVENT_IS_LIVE;
        out_ev.param_id = 0;
        this->params_get_value( 0, &out_ev.value );
        bool success = out_events->try_push( out_events, &out_ev.header );
        SFG_LOG_DEBUG( host_, host_log_, "[{:s}] [{:p}] sending event = {}", __FUNCTION__, static_cast< void* >( this ), success );
      }
    }
  }
}

clap_process_status ParamMultiplex::process( clap_process_t const* process ) {
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( process={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( process )
  // );
  const uint32_t nframes = process->frames_count;
  const uint32_t nev = process->in_events->size( process->in_events );
  uint32_t ev_index = 0;
  uint32_t next_ev_frame = nev > 0 ? 0 : nframes;

  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] nframes      ={:d}", __FUNCTION__, static_cast< void* >( this ), nframes );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] nev          ={:d}", __FUNCTION__, static_cast< void* >( this ), nev );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] ev_index     ={:d}", __FUNCTION__, static_cast< void* >( this ), ev_index );
  // SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] next_ev_frame={:d}", __FUNCTION__, static_cast< void* >( this ), next_ev_frame );

  for( uint32_t i = 0; i < nframes; ) {
    /* handle every events that happrens at the frame "i" */
    while( ev_index < nev && next_ev_frame == i ) {
      const clap_event_header_t* hdr = process->in_events->get( process->in_events, ev_index );
      if( hdr->time != i ) {
        next_ev_frame = hdr->time;
        break;
      }
      process_event( hdr, process->out_events );
      process->out_events->try_push( process->out_events, hdr );
      ++ev_index;
      if( ev_index == nev ) {
        // we reached the end of the event list
        next_ev_frame = nframes;
        break;
      }
    }
    i = next_ev_frame;
  }

  return CLAP_PROCESS_CONTINUE;
}

#pragma endregion

#pragma region CLAP extensions

uint32_t ParamMultiplex::params_count( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] params_count()", __FUNCTION__, static_cast< void* >( this ) );
  // adjust according to param_multiplex.proto
  // while we could make it dynamic, without explicit gui i'd rather not
  return 2 + state_.amount_params();
}

bool ParamMultiplex::params_get_info( uint32_t param_index, clap_param_info_t* out_param_info ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( param_index={:d}, out_param_info={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 param_index,
                 static_cast< void* >( out_param_info ) );
  if( param_index >= params_count() )
    return false;
  if( !out_param_info )
    return false;
  switch( param_index ) {
    case 0:
      // param output
      out_param_info->id = 1;
      out_param_info->flags = CLAP_PARAM_IS_READONLY | CLAP_PARAM_IS_AUTOMATABLE;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "out" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "out" );
      out_param_info->min_value = 0.0;
      out_param_info->max_value = 1.0;
      out_param_info->default_value = out_param_info->min_value;
      break;
    // case 1:
    //   // param amount
    //   out_param_info->id = 2;
    //   out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE;
    //   out_param_info->cookie = nullptr;
    //   std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Param Amount" );
    //   std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Main" );
    //   out_param_info->min_value = 1;
    //   out_param_info->max_value = 128;
    //   out_param_info->default_value = out_param_info->min_value;
    //   break;
    case 1:
      // param select
      out_param_info->id = 3;
      out_param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_AUTOMATABLE;
      out_param_info->cookie = nullptr;
      std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%s", "Param Select" );
      std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Main" );
      out_param_info->min_value = 1;
      out_param_info->max_value = state_.amount_params();
      out_param_info->default_value = out_param_info->min_value;
      break;
  }
  uint32_t adjusted_param_index = param_index - 2;
  if( adjusted_param_index >= state_.amount_params() ) {
    return false;
  }
  out_param_info->id = 4 + adjusted_param_index;
  out_param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
  out_param_info->cookie = nullptr;
  std::snprintf( out_param_info->name, sizeof( out_param_info->name ), "%d", adjusted_param_index + 1 );
  std::snprintf( out_param_info->module, sizeof( out_param_info->module ), "%s", "Params" );
  out_param_info->min_value = 0.0;
  out_param_info->max_value = 1.0;
  out_param_info->default_value = 0.0;
  return true;
}

bool ParamMultiplex::params_get_value( clap_id param_id, double* out_value ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( param_id={:d}, out_value={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 param_id,
                 static_cast< void* >( out_value ) );
  if( !out_value )
    return false;
  if( param_id == 1 ) {
    ( *out_value ) = state_.params( state_.selected_param() - 1 );
    return true;
  } /*else if( param_id == 2 ) {
    ( *out_value ) = state_.amount_params();
    return true;
  }*/
  else if( param_id == 3 ) {
    ( *out_value ) = state_.selected_param();
    return true;
  } else if( ( param_id - 4 ) < state_.amount_params() ) {
    ( *out_value ) = state_.params( param_id - 4 );
    return true;
  }
  return false;
}

bool ParamMultiplex::params_value_to_text( clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( param_id={:d}, value={:f}, out_buffer={:p}, out_info={:d} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 param_id,
                 value,
                 static_cast< void* >( out_buffer ),
                 out_buffer_capacity );
  if( !out_buffer || ( out_buffer_capacity == 0 ) )
    return false;
  if( param_id == 1 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  }
  /*if( param_id == 2 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  }*/
  if( param_id == 3 ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  }
  if( ( param_id - 4 ) < state_.amount_params() ) {
    std::fill( out_buffer, out_buffer + out_buffer_capacity, 0 );
    std::string tmp_str = std::to_string( value );
    tmp_str.copy( out_buffer, std::min( static_cast< uint32_t >( tmp_str.size() ), out_buffer_capacity ) );
    return true;
  }
  return false;
}

bool ParamMultiplex::params_text_to_value( clap_id param_id, std::string const& param_value_text, double* out_value ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( param_id={:d}, param_value_text={:?}, out_value={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 param_id,
                 param_value_text,
                 static_cast< void* >( out_value ) );
  if( !out_value )
    return false;
  std::function< bool( std::string const&, double* ) > text_to_double = []( std::string const& param_value_text, double* out_value ) {
    try {
      ( *out_value ) = std::stod( param_value_text );
      if( ( *out_value ) > 1.0 ) {
        ( *out_value ) = ( *out_value ) / 100.0;
      }
      return true;
    } catch( std::exception const& ) {
      return false;
    }
  };
  std::function< bool( std::string const&, double* ) > text_to_double_no_scale = []( std::string const& param_value_text, double* out_value ) {
    try {
      ( *out_value ) = std::stod( param_value_text );
      return true;
    } catch( std::exception const& ) {
      return false;
    }
  };
  if( param_id == 1 ) {
    return text_to_double( param_value_text, out_value );
  }
  /*if( param_id == 2 ) {
    return text_to_double_no_scale( param_value_text, out_value );
  }*/
  if( param_id == 3 ) {
    return text_to_double_no_scale( param_value_text, out_value );
  }
  if( ( param_id - 4 ) < state_.amount_params() ) {
    return text_to_double( param_value_text, out_value );
  }
  return false;
}

void ParamMultiplex::params_flush( clap_input_events_t const* in, clap_output_events_t const* out ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( in={:p}, out={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 static_cast< void const* >( in ),
                 static_cast< void const* >( out ) );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] in_size={:d}", __FUNCTION__, static_cast< void* >( this ), in->size( in ) );

  for( uint32_t i = 0; i < in->size( in ); i++ ) {
    process_event( in->get( in, i ), out );
    out->try_push( out, in->get( in, i ) );
  }
  return;
}

uint32_t ParamMultiplex::remote_controls_count( void ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] remote_controls_count()", __FUNCTION__, static_cast< void* >( this ) );
  return 1;
}

bool ParamMultiplex::remote_controls_get( uint32_t page_index, clap_remote_controls_page_t* out_page ) {
  SFG_LOG_TRACE( host_,
                 host_log_,
                 "[{:s}] [{:p}] enter( page_index={:d}, out_page={:p} )",
                 __FUNCTION__,
                 static_cast< void* >( this ),
                 page_index,
                 static_cast< void* >( out_page ) );
  if( page_index >= remote_controls_count() )
    return false;
  if( !out_page )
    return false;
  switch( page_index ) {
    case 0:
      // param output
      std::snprintf( out_page->section_name, sizeof( out_page->section_name ), "%s", "main" );
      out_page->page_id = 0;
      std::snprintf( out_page->page_name, sizeof( out_page->page_name ), "%s", "out" );
      out_page->param_ids[0] = 0;
      break;
  }
  return true;
}

bool ParamMultiplex::state_save( clap_ostream_t const* stream ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( stream={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( stream ) );
  bool ret = true;
  std::string buffer{};
  if( !state_.SerializeToString( &buffer ) ) {
    ret = false;
  } else {
    int64_t written = 0;
    while( written < static_cast< int64_t >( buffer.size() ) ) {
      int64_t tmp = stream->write( stream, buffer.data() + written, buffer.size() - written );
      if( tmp < 0 ) {
        return false;
      }
      written += tmp;
    }
    ret = written == static_cast< int64_t >( buffer.size() );
  }
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

bool ParamMultiplex::state_load( clap_istream_t const* stream ) {
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] enter( stream={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( stream ) );
  bool ret = true;
  std::string buffer{};
  std::array< char, 4096 > temp;
  while( true ) {
    int64_t n = stream->read( stream, temp.data(), temp.size() );
    if( n <= 0 ) {
      break;
    }
    buffer.append( temp.data(), static_cast< size_t >( n ) );
  }
  ret = ret && ( buffer.size() > 0 );
  ret = ret && state_.ParseFromString( buffer );
  SFG_LOG_TRACE( host_, host_log_, "[{:s}] [{:p}] exit( ret={} )", __FUNCTION__, static_cast< void* >( this ), ret );
  return ret;
}

#pragma endregion

#pragma region CLAP extensions, wether or not to pointer things to clap

bool ParamMultiplex::supports_params() const {
  return true;
}

bool ParamMultiplex::supports_remote_controls() const {
  return true;
}

bool ParamMultiplex::supports_state() const {
  return true;
}

#pragma endregion

#pragma region shit for the factory

clap_plugin_t* ParamMultiplex::s_create( clap_host_t const* host ) {
  clap_plugin_t* plugin = new clap_plugin_t();
  ParamMultiplex* noise_gen = new ParamMultiplex();
  noise_gen->host_ = host;
  plugin->desc = descriptor_get();
  plugin->plugin_data = noise_gen;
  plugin->init = s_init;
  plugin->destroy = s_destroy;
  plugin->activate = s_activate;
  plugin->deactivate = s_deactivate;
  plugin->start_processing = s_start_processing;
  plugin->stop_processing = s_stop_processing;
  plugin->reset = s_reset;
  plugin->process = s_process;
  plugin->get_extension = s_get_extension;
  plugin->on_main_thread = s_on_main_thread;
  return plugin;
};

clap_plugin_descriptor_t* ParamMultiplex::descriptor_get( void ) {
  static clap_plugin_descriptor_t clap_descriptor_;
  static std::vector< char const* > clap_descriptor_features_;

  static std::string const c_plugin_id_ = "com.SFGrenade.ParamMultiplex";
  static std::string const c_plugin_name_ = "SFG-ParamMultiplex";
  static std::string const c_plugin_vendor_ = "SFGrenade";
  static std::string const c_plugin_url_ = "https://sfgrena.de";
  static std::string const c_plugin_manual_url_ = "https://sfgrena.de";
  static std::string const c_plugin_support_url_ = "https://sfgrena.de";
  static std::string const c_plugin_version_ = "0.0.1";
  static std::string const c_plugin_description_ = "Awesome ParamMultiplex. By SFGrenade.";
  static std::vector< std::string > const c_plugin_features_ = { CLAP_PLUGIN_FEATURE_ANALYZER, CLAP_PLUGIN_FEATURE_UTILITY, CLAP_PLUGIN_FEATURE_MIXING };

  clap_descriptor_.clap_version = CLAP_VERSION_INIT;
  clap_descriptor_.id = c_plugin_id_.c_str();
  clap_descriptor_.name = c_plugin_name_.c_str();
  clap_descriptor_.vendor = c_plugin_vendor_.c_str();
  clap_descriptor_.url = c_plugin_url_.c_str();
  clap_descriptor_.manual_url = c_plugin_manual_url_.c_str();
  clap_descriptor_.support_url = c_plugin_support_url_.c_str();
  clap_descriptor_.version = c_plugin_version_.c_str();
  clap_descriptor_.description = c_plugin_description_.c_str();
  // + 1 so we leave last entry as nullptr
  clap_descriptor_features_.resize( c_plugin_features_.size() + 1, nullptr );
  for( size_t i = 0; i < c_plugin_features_.size(); i++ ) {
    clap_descriptor_features_[i] = c_plugin_features_[i].c_str();
  }
  clap_descriptor_.features = clap_descriptor_features_.data();

  return &clap_descriptor_;
}

#pragma endregion
