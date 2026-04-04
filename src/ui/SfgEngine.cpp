// Header assigned to this source
#include "ui/SfgEngine.hpp"

SfgEngine::SfgEngine( std::shared_ptr< spdlog::logger > logger, QApplication* app, QWidget* hiddenWatcher, QObject* parent )
    : _base_( parent ), logger_( logger ), app_( app ), hiddenWatcher_( hiddenWatcher ) {
  logger_->trace( "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  timer_ = new QTimer( this );
  timer_->setInterval( std::chrono::milliseconds( 10 ) );
  timer_->setTimerType( Qt::TimerType::PreciseTimer );
  timer_->setSingleShot( false );
  connect( timer_, &QTimer::timeout, this, &SfgEngine::timerReceiver, Qt::QueuedConnection );
  // timer_ = Timer::createNative( 10, std::bind( &SfgEngine::timerReceiver, this ) );
}

SfgEngine::~SfgEngine() {
  logger_->trace( "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  stop();
}

void SfgEngine::start() {
  logger_->trace( "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  stop();
  timer_->start();
  app_->processEvents();  // todo: only for QT
}

void SfgEngine::stop() {
  logger_->trace( "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  timer_->stop();
  app_->processEvents();  // todo: only for QT
}

void SfgEngine::timerReceiver() {
  logger_->trace( "[{:s}] [{:p}] enter()", __FUNCTION__, static_cast< void* >( this ) );
  Q_EMIT timerTicked();
  app_->processEvents();
}
