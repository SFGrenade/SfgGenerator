// Header assigned to this source
#include "ui/UiNgHolder.hpp"

// Project includes
#include "ui/SfgEngine.hpp"
#include "ui/UiNoiseGenerator.hpp"

// Other lib includes
#include <QApplication>
#include <QWindow>

struct UiNgHolder::Impl {
  std::shared_ptr< spdlog::logger > logger;
  clap_host_t const* host;
  SfgGenerator::Proto::NoiseGenerator* state;

  bool initialized = false;
  QApplication* qtApp = nullptr;
  SfgEngine* qtEngine = nullptr;
  UiNoiseGenerator* qtWindow = nullptr;
  QWindow* qtNativeParent = nullptr;

  // qt things, not used
  int argc = 1;
  char arg[6] = { 'd', 'u', 'm', 'm', 'y', 0 };
  char* argv[2] = { arg, nullptr };

  bool windowParentSet = false;
  clap_window_t windowParent;
};

UiNgHolder::UiNgHolder() : impl_( std::make_unique< UiNgHolder::Impl >() ) {}

UiNgHolder::~UiNgHolder() {}

bool UiNgHolder::clap_create( std::string const& api, bool is_floating ) {
  impl_->logger->trace( "[{:s}] [{:p}] enter( api={:?}, is_floating={} )", __FUNCTION__, static_cast< void* >( this ), api, is_floating );
  if( impl_->initialized ) {
    return false;
  }
  QApplication::setLibraryPaths( {
    QString::fromStdString( ( ClapGlobals::PLUGIN_PATH.parent_path() / "qt" ).string() ),
    QString::fromStdString( ( ClapGlobals::PLUGIN_PATH.parent_path() / "qt" / "iconengines" ).string() ),
    QString::fromStdString( ( ClapGlobals::PLUGIN_PATH.parent_path() / "qt" / "imageformats" ).string() ),
    QString::fromStdString( ( ClapGlobals::PLUGIN_PATH.parent_path() / "qt" / "platforms" ).string() ),
    QString::fromStdString( ( ClapGlobals::PLUGIN_PATH.parent_path() / "qt" / "styles" ).string() ),
  } );
  QApplication::setAttribute( Qt::AA_PluginApplication );
  impl_->qtApp = new QApplication( impl_->argc, impl_->argv );
  impl_->qtWindow = new UiNoiseGenerator( impl_->logger->clone( "UiNoiseGenerator" ), nullptr );
  impl_->qtEngine = new SfgEngine( impl_->qtApp, impl_->qtWindow );

  impl_->qtWindow->connect( impl_->qtEngine, &SfgEngine::timerTicked, [this]() {
    static double last_sineWaveType = -1.0;
    static double last_sineWaveMix = -1.0;
    static double last_squareWaveType = -1.0;
    static double last_squareWavePwm = -1.0;
    static double last_squareWaveMix = -1.0;
    static double last_sawWaveType = -1.0;
    static double last_sawWaveMix = -1.0;
    static double last_triangleWaveType = -1.0;
    static double last_triangleWaveMix = -1.0;
    static double last_whiteNoiseType = -1.0;
    static double last_whiteNoiseMix = -1.0;
    static double last_pinkNoiseType = -1.0;
    static double last_pinkNoiseMix = -1.0;
    static double last_redNoiseType = -1.0;
    static double last_redNoiseMix = -1.0;
    static double last_blueNoiseType = -1.0;
    static double last_blueNoiseMix = -1.0;
    static double last_violetNoiseType = -1.0;
    static double last_violetNoiseMix = -1.0;
    static double last_greyNoiseType = -1.0;
    static double last_greyNoiseMix = -1.0;
    static double last_velvetNoiseType = -1.0;
    static double last_velvetNoiseMix = -1.0;
    if( last_sineWaveType != double( this->impl_->state->synth_sine_wave_type() ) ) {
      last_sineWaveType = double( this->impl_->state->synth_sine_wave_type() );
      this->impl_->qtWindow->setSineWaveTypeValue( last_sineWaveType );
    }
    if( last_sineWaveMix != this->impl_->state->synth_sine_wave_mix() ) {
      last_sineWaveMix = this->impl_->state->synth_sine_wave_mix();
      this->impl_->qtWindow->setSineWaveMixValue( last_sineWaveMix );
    }
    if( last_squareWaveType != double( this->impl_->state->synth_square_wave_type() ) ) {
      last_squareWaveType = double( this->impl_->state->synth_square_wave_type() );
      this->impl_->qtWindow->setSquareWaveTypeValue( last_squareWaveType );
    }
    if( last_squareWavePwm != this->impl_->state->synth_square_wave_pwm() ) {
      last_squareWavePwm = this->impl_->state->synth_square_wave_pwm();
      this->impl_->qtWindow->setSquareWavePwmValue( last_squareWavePwm );
    }
    if( last_squareWaveMix != this->impl_->state->synth_square_wave_mix() ) {
      last_squareWaveMix = this->impl_->state->synth_square_wave_mix();
      this->impl_->qtWindow->setSquareWaveMixValue( last_squareWaveMix );
    }
    if( last_sawWaveType != double( this->impl_->state->synth_saw_wave_type() ) ) {
      last_sawWaveType = double( this->impl_->state->synth_saw_wave_type() );
      this->impl_->qtWindow->setSawWaveTypeValue( last_sawWaveType );
    }
    if( last_sawWaveMix != this->impl_->state->synth_saw_wave_mix() ) {
      last_sawWaveMix = this->impl_->state->synth_saw_wave_mix();
      this->impl_->qtWindow->setSawWaveMixValue( last_sawWaveMix );
    }
    if( last_triangleWaveType != double( this->impl_->state->synth_triangle_wave_type() ) ) {
      last_triangleWaveType = double( this->impl_->state->synth_triangle_wave_type() );
      this->impl_->qtWindow->setTriangleWaveTypeValue( last_triangleWaveType );
    }
    if( last_triangleWaveMix != this->impl_->state->synth_triangle_wave_mix() ) {
      last_triangleWaveMix = this->impl_->state->synth_triangle_wave_mix();
      this->impl_->qtWindow->setTriangleWaveMixValue( last_triangleWaveMix );
    }
    if( last_whiteNoiseType != double( this->impl_->state->synth_white_noise_type() ) ) {
      last_whiteNoiseType = double( this->impl_->state->synth_white_noise_type() );
      this->impl_->qtWindow->setWhiteNoiseTypeValue( last_whiteNoiseType );
    }
    if( last_whiteNoiseMix != this->impl_->state->synth_white_noise_mix() ) {
      last_whiteNoiseMix = this->impl_->state->synth_white_noise_mix();
      this->impl_->qtWindow->setWhiteNoiseMixValue( last_whiteNoiseMix );
    }
    if( last_pinkNoiseType != double( this->impl_->state->synth_pink_noise_type() ) ) {
      last_pinkNoiseType = double( this->impl_->state->synth_pink_noise_type() );
      this->impl_->qtWindow->setPinkNoiseTypeValue( last_pinkNoiseType );
    }
    if( last_pinkNoiseMix != this->impl_->state->synth_pink_noise_mix() ) {
      last_pinkNoiseMix = this->impl_->state->synth_pink_noise_mix();
      this->impl_->qtWindow->setPinkNoiseMixValue( last_pinkNoiseMix );
    }
    if( last_redNoiseType != double( this->impl_->state->synth_red_noise_type() ) ) {
      last_redNoiseType = double( this->impl_->state->synth_red_noise_type() );
      this->impl_->qtWindow->setRedNoiseTypeValue( last_redNoiseType );
    }
    if( last_redNoiseMix != this->impl_->state->synth_red_noise_mix() ) {
      last_redNoiseMix = this->impl_->state->synth_red_noise_mix();
      this->impl_->qtWindow->setRedNoiseMixValue( last_redNoiseMix );
    }
    if( last_blueNoiseType != double( this->impl_->state->synth_blue_noise_type() ) ) {
      last_blueNoiseType = double( this->impl_->state->synth_blue_noise_type() );
      this->impl_->qtWindow->setBlueNoiseTypeValue( last_blueNoiseType );
    }
    if( last_blueNoiseMix != this->impl_->state->synth_blue_noise_mix() ) {
      last_blueNoiseMix = this->impl_->state->synth_blue_noise_mix();
      this->impl_->qtWindow->setBlueNoiseMixValue( last_blueNoiseMix );
    }
    if( last_violetNoiseType != double( this->impl_->state->synth_violet_noise_type() ) ) {
      last_violetNoiseType = double( this->impl_->state->synth_violet_noise_type() );
      this->impl_->qtWindow->setVioletNoiseTypeValue( last_violetNoiseType );
    }
    if( last_violetNoiseMix != this->impl_->state->synth_violet_noise_mix() ) {
      last_violetNoiseMix = this->impl_->state->synth_violet_noise_mix();
      this->impl_->qtWindow->setVioletNoiseMixValue( last_violetNoiseMix );
    }
    if( last_greyNoiseType != double( this->impl_->state->synth_grey_noise_type() ) ) {
      last_greyNoiseType = double( this->impl_->state->synth_grey_noise_type() );
      this->impl_->qtWindow->setGreyNoiseTypeValue( last_greyNoiseType );
    }
    if( last_greyNoiseMix != this->impl_->state->synth_grey_noise_mix() ) {
      last_greyNoiseMix = this->impl_->state->synth_grey_noise_mix();
      this->impl_->qtWindow->setGreyNoiseMixValue( last_greyNoiseMix );
    }
    if( last_velvetNoiseType != double( this->impl_->state->synth_velvet_noise_type() ) ) {
      last_velvetNoiseType = double( this->impl_->state->synth_velvet_noise_type() );
      this->impl_->qtWindow->setVelvetNoiseTypeValue( last_velvetNoiseType );
    }
    if( last_velvetNoiseMix != this->impl_->state->synth_velvet_noise_mix() ) {
      last_velvetNoiseMix = this->impl_->state->synth_velvet_noise_mix();
      this->impl_->qtWindow->setVelvetNoiseMixValue( last_velvetNoiseMix );
    }
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::sineWaveTypeAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_sine_wave_type( SfgGenerator::Proto::NoiseGenerator_SineWaveType( std::round( value ) ) );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::sineWaveMixAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_sine_wave_mix( value );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::squareWaveTypeAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_square_wave_type( SfgGenerator::Proto::NoiseGenerator_SquareWaveType( std::round( value ) ) );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::squareWavePwmAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_square_wave_pwm( value );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::squareWaveMixAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_square_wave_mix( value );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::sawWaveTypeAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_saw_wave_type( SfgGenerator::Proto::NoiseGenerator_SawWaveType( std::round( value ) ) );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::sawWaveMixAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_saw_wave_mix( value );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::triangleWaveTypeAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_triangle_wave_type( SfgGenerator::Proto::NoiseGenerator_TriangleWaveType( std::round( value ) ) );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::triangleWaveMixAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_triangle_wave_mix( value );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::whiteNoiseTypeAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_white_noise_type( SfgGenerator::Proto::NoiseGenerator_WhiteNoiseType( std::round( value ) ) );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::whiteNoiseMixAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_white_noise_mix( value );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::pinkNoiseTypeAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_pink_noise_type( SfgGenerator::Proto::NoiseGenerator_PinkNoiseType( std::round( value ) ) );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::pinkNoiseMixAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_pink_noise_mix( value );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::redNoiseTypeAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_red_noise_type( SfgGenerator::Proto::NoiseGenerator_RedNoiseType( std::round( value ) ) );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::redNoiseMixAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_red_noise_mix( value );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::blueNoiseTypeAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_blue_noise_type( SfgGenerator::Proto::NoiseGenerator_BlueNoiseType( std::round( value ) ) );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::blueNoiseMixAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_blue_noise_mix( value );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::violetNoiseTypeAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_violet_noise_type( SfgGenerator::Proto::NoiseGenerator_VioletNoiseType( std::round( value ) ) );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::violetNoiseMixAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_violet_noise_mix( value );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::greyNoiseTypeAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_grey_noise_type( SfgGenerator::Proto::NoiseGenerator_GreyNoiseType( std::round( value ) ) );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::greyNoiseMixAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_grey_noise_mix( value );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::velvetNoiseTypeAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_velvet_noise_type( SfgGenerator::Proto::NoiseGenerator_VelvetNoiseType( std::round( value ) ) );
    this->impl_->host->request_callback( this->impl_->host );
  } );
  impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::velvetNoiseMixAdjusted, [this]( double value ) {
    this->impl_->state->set_synth_velvet_noise_mix( value );
    this->impl_->host->request_callback( this->impl_->host );
  } );

  impl_->qtEngine->start();

  if( ( !is_floating ) && impl_->windowParentSet ) {
    clap_set_parent( &impl_->windowParent );
  }

  impl_->initialized = true;
  return true;
}

void UiNgHolder::clap_destroy( void ) {
  impl_->logger->trace( "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  impl_->qtNativeParent = nullptr;
  if( impl_->qtWindow ) {
    delete impl_->qtWindow;
    impl_->qtWindow = nullptr;
  }
  if( impl_->qtEngine ) {
    delete impl_->qtEngine;
    impl_->qtEngine = nullptr;
  }
  if( impl_->qtApp ) {
    impl_->qtApp->quit();
    delete impl_->qtApp;
    impl_->qtApp = nullptr;
  }
  impl_->initialized = false;
}

bool UiNgHolder::clap_set_scale( double scale ) {
  impl_->logger->trace( "[{:s}] [{:p}] enter( scale={:f} )", __FUNCTION__, static_cast< void* >( this ), scale );
  // todo: fixme: uh, how?
  // if( impl_->qtWindow ) {
  //   emit impl_->qtWindow->resize(impl_->qtWindow->size() * scale);
  //   return impl_->qtWindow->clap_set_scale( scale );
  // }
  return false;
}

bool UiNgHolder::clap_get_size( uint32_t* out_width, uint32_t* out_height ) {
  impl_->logger->trace( "[{:s}] [{:p}] enter( out_width={:p}, out_height={:p} )",
                  __FUNCTION__,
                  static_cast< void* >( this ),
                  static_cast< void* >( out_width ),
                  static_cast< void* >( out_height ) );
  if( out_width && out_height && impl_->qtWindow ) {
    *out_width = impl_->qtWindow->width();
    *out_height = impl_->qtWindow->height();
    return true;
  }
  return false;
}

bool UiNgHolder::clap_can_resize( void ) {
  impl_->logger->trace( "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  if( impl_->qtWindow ) {
    return impl_->qtWindow->minimumSize() != impl_->qtWindow->maximumSize();
  }
  return false;
}

bool UiNgHolder::clap_get_resize_hints( clap_gui_resize_hints_t* out_hints ) {
  impl_->logger->trace( "[{:s}] [{:p}] enter( out_hints={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void* >( out_hints ) );
  if( out_hints && impl_->qtWindow ) {
    out_hints->can_resize_horizontally = impl_->qtWindow->minimumSize().width() != impl_->qtWindow->maximumSize().width();
    out_hints->can_resize_vertically = impl_->qtWindow->minimumSize().height() != impl_->qtWindow->maximumSize().height();
    out_hints->preserve_aspect_ratio = impl_->qtWindow->hasHeightForWidth();
    if( out_hints->preserve_aspect_ratio ) {
      out_hints->aspect_ratio_width = 1000;
      out_hints->aspect_ratio_height = impl_->qtWindow->heightForWidth( 1000 );
    }
    return true;
  }
  return false;
}

bool UiNgHolder::clap_adjust_size( uint32_t* out_width, uint32_t* out_height ) {
  impl_->logger->trace( "[{:s}] [{:p}] enter( out_width={:p}, out_height={:p} )",
                  __FUNCTION__,
                  static_cast< void* >( this ),
                  static_cast< void* >( out_width ),
                  static_cast< void* >( out_height ) );
  if( out_width && out_height && impl_->qtWindow ) {
    impl_->qtWindow->resize( *out_width, *out_height );
    *out_width = impl_->qtWindow->width();
    *out_height = impl_->qtWindow->height();
    return true;
  }
  return false;
}

bool UiNgHolder::clap_set_size( uint32_t width, uint32_t height ) {
  impl_->logger->trace( "[{:s}] [{:p}] enter( width={:d}, height={:d} )", __FUNCTION__, static_cast< void* >( this ), width, height );
  if( impl_->qtWindow ) {
    impl_->qtWindow->resize( width, height );
    return true;
  }
  return false;
}

bool UiNgHolder::clap_set_parent( clap_window_t const* window ) {
  impl_->logger->trace( "[{:s}] [{:p}] enter( window={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( window ) );
  if( !window ) {
    return false;
  }
  impl_->windowParent.api = window->api;

  if( impl_->windowParent.api == CLAP_WINDOW_API_WIN32 )
    impl_->windowParent.win32 = window->win32;
  else if( impl_->windowParent.api == CLAP_WINDOW_API_COCOA )
    impl_->windowParent.cocoa = window->cocoa;
  else if( impl_->windowParent.api == CLAP_WINDOW_API_X11 )
    impl_->windowParent.x11 = window->x11;
  else if( impl_->windowParent.api == CLAP_WINDOW_API_WAYLAND )
    impl_->windowParent.ptr = window->ptr;
  else
    impl_->windowParent.ptr = window->ptr;

  if( ( impl_->windowParent.api == CLAP_WINDOW_API_WIN32 ) && impl_->qtWindow )
    impl_->qtNativeParent = QWindow::fromWinId( WId( impl_->windowParent.win32 ) );
  else if( ( impl_->windowParent.api == CLAP_WINDOW_API_COCOA ) && impl_->qtWindow )
    impl_->qtNativeParent = QWindow::fromWinId( WId( impl_->windowParent.cocoa ) );
  else if( ( impl_->windowParent.api == CLAP_WINDOW_API_X11 ) && impl_->qtWindow )
    impl_->qtNativeParent = QWindow::fromWinId( WId( impl_->windowParent.x11 ) );
  else if( ( impl_->windowParent.api == CLAP_WINDOW_API_WAYLAND ) && impl_->qtWindow )
    impl_->qtNativeParent = QWindow::fromWinId( WId( impl_->windowParent.ptr ) );
  else if( impl_->qtWindow )
    impl_->qtNativeParent = QWindow::fromWinId( WId( impl_->windowParent.ptr ) );

  impl_->windowParentSet = true;
  if( impl_->qtWindow && impl_->qtNativeParent ) {
    if( !impl_->qtWindow->windowHandle() ) {
      impl_->qtWindow->show();
      impl_->qtWindow->hide();
    }
    impl_->qtWindow->windowHandle()->setParent( impl_->qtNativeParent );
  }
  return true;
}

bool UiNgHolder::clap_set_transient( clap_window_t const* window ) {
  impl_->logger->trace( "[{:s}] [{:p}] enter( window={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( window ) );
  if( impl_->qtWindow ) {
    impl_->qtWindow->raise();
    return true;
  }
  return false;
}

void UiNgHolder::clap_suggest_title( std::string const& title ) {
  impl_->logger->trace( "[{:s}] [{:p}] enter( title={:?} )", __FUNCTION__, static_cast< void* >( this ), title );
  if( impl_->qtWindow ) {
    impl_->qtWindow->setWindowTitle( QString::fromStdString( title ) );
  }
}

bool UiNgHolder::clap_show( void ) {
  impl_->logger->trace( "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  if( impl_->qtWindow ) {
    impl_->qtWindow->show();
    impl_->qtEngine->start();
    return true;
  }
  return false;
}

bool UiNgHolder::clap_hide( void ) {
  impl_->logger->trace( "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  if( impl_->qtWindow ) {
    impl_->qtWindow->hide();
    impl_->qtEngine->stop();
    return true;
  }
  return false;
}

void UiNgHolder::set_host( clap_host_t const* host ) {
  impl_->host = host;
}

void UiNgHolder::set_state( SfgGenerator::Proto::NoiseGenerator* state ) {
  impl_->state = state;
}

void UiNgHolder::set_logger( std::shared_ptr< spdlog::logger > logger ) {
  impl_->logger = logger;
}
