// Header assigned to this source
#include "ui/sfgEngine.hpp"

SfgEngine::SfgEngine( QApplication* app, QWidget* hiddenWatcher, QObject* parent ) : _base_( parent ), app_( app ), hiddenWatcher_( hiddenWatcher ) {
  timer_ = new QTimer( this );
  timer_->setInterval( std::chrono::milliseconds( 1000 / 60 ) );
  timer_->setTimerType( Qt::TimerType::PreciseTimer );
  timer_->setSingleShot( false );

  connect( timer_, &QTimer::timeout, this, &SfgEngine::timerReceiver, Qt::QueuedConnection );
}

SfgEngine::~SfgEngine() {}

void SfgEngine::start() {
  timer_->start();
  app_->processEvents();
}

void SfgEngine::stop() {
  timer_->stop();
  app_->processEvents();
}

void SfgEngine::timerReceiver() {
  if( hiddenWatcher_->isHidden() ) {
    timer_->stop();
  }
  app_->processEvents();
}
