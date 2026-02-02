// Header assigned to this source
#include "ui/audio_lerp_effect.hpp"

// Project includes
#include "ui/ui_audio_lerp_effect.hpp"

// Other lib includes
#include <QApplication>
#include <QTimer>
#include <QWindow>

// C++ std includes
#include <chrono>
#include <thread>

struct UiAleHolder::Impl {
  bool initialized = false;
  QApplication* qtApp = nullptr;
  QTimer* qtTimer = nullptr;
  UiAudioLerpEffect* qtWindow = nullptr;
  QWindow* qtNativeParent = nullptr;

  // qt things, not used
  int argc = 1;
  char arg[6] = { 'd', 'u', 'm', 'm', 'y', 0 };
  char* argv[2] = { arg, nullptr };

  bool windowParentSet = false;
  clap_window_t windowParent;
  uint32_t out_width;
  uint32_t out_height;
  clap_gui_resize_hints_t out_hints;
};

UiAleHolder::UiAleHolder() : impl_( std::make_unique< UiAleHolder::Impl >() ) {}

UiAleHolder::~UiAleHolder() {}

bool UiAleHolder::clap_create( std::string const& api, bool is_floating ) {
  logger_->trace( "[{:s}] [{:p}] enter( api={:?}, is_floating={} )", __FUNCTION__, static_cast< void* >( this ), api, is_floating );
  if( impl_->initialized ) {
    return false;
  }
  QApplication::setAttribute( Qt::AA_PluginApplication );
  impl_->qtApp = new QApplication( impl_->argc, impl_->argv );
  impl_->qtTimer = new QTimer();
  impl_->qtTimer->setInterval( std::chrono::milliseconds( 1000 / 60 ) );
  impl_->qtTimer->setTimerType( Qt::TimerType::PreciseTimer );
  impl_->qtTimer->setSingleShot( false );
  impl_->qtWindow = new UiAudioLerpEffect( this->logger_->clone( "UiAudioLerpEffect" ), nullptr );

  impl_->qtTimer->connect( impl_->qtTimer, &QTimer::timeout, [this]() {
    if( this->impl_->qtWindow->isHidden() ) {
      this->impl_->qtTimer->stop();
    }
    this->impl_->qtApp->processEvents();
  } );
  impl_->qtTimer->start();
  impl_->qtApp->processEvents();

  if( ( !is_floating ) && impl_->windowParentSet ) {
    clap_set_parent( &impl_->windowParent );
  }

  impl_->initialized = true;
  return true;
}

void UiAleHolder::clap_destroy( void ) {
  logger_->trace( "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  impl_->qtNativeParent = nullptr;
  if( impl_->qtWindow ) {
    delete impl_->qtWindow;
    impl_->qtWindow = nullptr;
  }
  if( impl_->qtTimer ) {
    delete impl_->qtTimer;
    impl_->qtTimer = nullptr;
  }
  if( impl_->qtApp ) {
    impl_->qtApp->quit();
    delete impl_->qtApp;
    impl_->qtApp = nullptr;
  }
  impl_->initialized = false;
}

bool UiAleHolder::clap_set_scale( double scale ) {
  logger_->trace( "[{:s}] [{:p}] enter( scale={:f} )", __FUNCTION__, static_cast< void* >( this ), scale );
  // todo: fixme: uh, how?
  // if( impl_->qtWindow ) {
  //   emit impl_->qtWindow->resize(impl_->qtWindow->size() * scale);
  //   return impl_->qtWindow->clap_set_scale( scale );
  // }
  return false;
}

bool UiAleHolder::clap_get_size( uint32_t* out_width, uint32_t* out_height ) {
  logger_->trace( "[{:s}] [{:p}] enter( out_width={:p}, out_height={:p} )",
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

bool UiAleHolder::clap_can_resize( void ) {
  logger_->trace( "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  if( impl_->qtWindow ) {
    return impl_->qtWindow->minimumSize() != impl_->qtWindow->maximumSize();
  }
  return false;
}

bool UiAleHolder::clap_get_resize_hints( clap_gui_resize_hints_t* out_hints ) {
  logger_->trace( "[{:s}] [{:p}] enter( out_hints={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void* >( out_hints ) );
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

bool UiAleHolder::clap_adjust_size( uint32_t* out_width, uint32_t* out_height ) {
  logger_->trace( "[{:s}] [{:p}] enter( out_width={:p}, out_height={:p} )",
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

bool UiAleHolder::clap_set_size( uint32_t width, uint32_t height ) {
  logger_->trace( "[{:s}] [{:p}] enter( width={:d}, height={:d} )", __FUNCTION__, static_cast< void* >( this ), width, height );
  if( impl_->qtWindow ) {
    impl_->qtWindow->resize( width, height );
    return true;
  }
  return false;
}

bool UiAleHolder::clap_set_parent( clap_window_t const* window ) {
  logger_->trace( "[{:s}] [{:p}] enter( window={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( window ) );
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

bool UiAleHolder::clap_set_transient( clap_window_t const* window ) {
  logger_->trace( "[{:s}] [{:p}] enter( window={:p} )", __FUNCTION__, static_cast< void* >( this ), static_cast< void const* >( window ) );
  if( impl_->qtWindow ) {
    impl_->qtWindow->raise();
    return true;
  }
  return false;
}

void UiAleHolder::clap_suggest_title( std::string const& title ) {
  logger_->trace( "[{:s}] [{:p}] enter( title={:?} )", __FUNCTION__, static_cast< void* >( this ), title );
  if( impl_->qtWindow ) {
    impl_->qtWindow->setWindowTitle( QString::fromStdString( title ) );
  }
}

bool UiAleHolder::clap_show( void ) {
  logger_->trace( "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  if( impl_->qtWindow ) {
    impl_->qtWindow->show();

    impl_->qtTimer->start();
    impl_->qtApp->processEvents();
    return true;
  }
  return false;
}

bool UiAleHolder::clap_hide( void ) {
  logger_->trace( "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  if( impl_->qtWindow ) {
    impl_->qtWindow->hide();

    impl_->qtTimer->stop();
    impl_->qtApp->processEvents();
    return true;
  }
  return false;
}

void UiAleHolder::set_logger( std::shared_ptr< spdlog::logger > logger ) {
  logger_ = logger;
}
