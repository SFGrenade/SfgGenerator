// Header assigned to this source
#include "ui/SfgEngine.hpp"

SfgEngine::SfgEngine( QApplication* app, QWidget* hiddenWatcher, QObject* parent ) : _base_( parent ), app_( app ), hiddenWatcher_( hiddenWatcher ) {
  timer_ = Timer::createNative( 10, std::bind( &SfgEngine::timerReceiver, this ) );
}

SfgEngine::~SfgEngine() {
  stop();
}

void SfgEngine::start() {
  timer_->start();
}

void SfgEngine::stop() {
  timer_->stop();
}

void SfgEngine::timerReceiver() {
  emit timerTicked();
  app_->processEvents();
}
