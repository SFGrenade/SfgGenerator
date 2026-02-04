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
    // static double last_ab = -1.0;
    // if( last_ab != this->impl_->state->a_b() ) {
    //   last_ab = this->impl_->state->a_b();
    //   this->impl_->qtWindow->setAbValue( last_ab );
    // }
  } );
  // impl_->qtWindow->connect( impl_->qtWindow, &UiNoiseGenerator::abAdjusted, [this]( double value ) {
  //   this->impl_->state->set_a_b( value );
  //   this->impl_->host->request_callback( this->impl_->host );
  // } );

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
