// Header assigned to this source
#include "common/Timer.hpp"

// C++ std includes
#include <stdexcept>

Timer::~Timer() = default;

std::unique_ptr< Timer > Timer::createNative( uint32_t durationMs, std::function< void() > const &callback ) {
#if defined( SFG_GEN_IS_LINUX )
  return std::make_unique< LinuxTimer >( durationMs, callback );
#elif defined( SFG_GEN_IS_MACOS )
  return std::make_unique< MacOsTimer >( durationMs, callback );
#elif defined( SFG_GEN_IS_WINDOWS )
  return std::make_unique< WindowsTimer >( durationMs, callback );
#endif
  return nullptr;
}

#if defined( SFG_GEN_IS_LINUX )
LinuxTimer::LinuxTimer( uint32_t durationMs, const std::function< void() > &callback ) : durationMs_( durationMs ), cb_( callback ) {
  if( !cb_ ) {
    throw std::invalid_argument( "callback can't be null" );
  }

  // establish signal handler
  signalAction_.sa_flags = SA_SIGINFO;
  signalAction_.sa_sigaction = &LinuxTimer::onTimer;
  sigemptyset( &signalAction_.sa_mask );
  if( 0 != sigaction( SIGRTMIN, &signalAction_, nullptr ) ) {
    // error establishing signal handler
  }

  // block timer signal
  sigemptyset( &signalMask_ );
  sigaddset( &signalMask_, SIGRTMIN );
  if( 0 != sigprocmask( SIG_SETMASK, &signalMask_, NULL ) ) {
    // error blocking timer signal
  }

  // create the timer
  signalEvent_.sigev_notify = SIGEV_SIGNAL;
  signalEvent_.sigev_signo = SIGRTMIN;
  signalEvent_.sigev_value.sival_ptr = this;
  if( 0 != timer_create( CLOCK_REALTIME, &signalEvent_, &timer_ ) ) {
    // error creating the timer
  }

  timerSpecification_.it_value.tv_sec = durationMs / 1'000;
  timerSpecification_.it_value.tv_nsec = ( durationMs % 1'000 ) * 1'000'000;
  timerSpecification_.it_interval.tv_sec = timerSpecification_.it_value.tv_sec;
  timerSpecification_.it_interval.tv_nsec = timerSpecification_.it_value.tv_nsec;
}

LinuxTimer::~LinuxTimer() {
  // destroy the timer
  if( 0 != timer_delete( timer_ ) ) {
    // error destroying the timer
  }

  // unblock timer signal
  if( 0 != sigprocmask( SIG_UNBLOCK, &signalMask_, NULL ) ) {
    // error unblocking timer signal
  }

  // restore signal handler
  signalAction_.sa_flags = 0;
  signalAction_.sa_handler = SIG_DFL;
  signalAction_.sa_sigaction = nullptr;
  if( 0 != sigaction( SIGRTMIN, &signalAction_, nullptr ) ) {
    // error restoring signal handler
  }
}

void LinuxTimer::start() {
  if( 0 != timer_settime( timer_, 0, &timerSpecification_, nullptr ) ) {
    // error setting the timer time
  }
}

void LinuxTimer::stop() {
  timerSpecification_.it_value.tv_sec = 0;
  timerSpecification_.it_value.tv_nsec = 0;
  timerSpecification_.it_interval.tv_sec = timerSpecification_.it_value.tv_sec;
  timerSpecification_.it_interval.tv_nsec = timerSpecification_.it_value.tv_nsec;
  if( 0 != timer_settime( timer_, 0, &timerSpecification_, nullptr ) ) {
    // error setting the timer time
  }
}

void LinuxTimer::onTimer( int signal, siginfo_t *signalInfo, void *ucontext ) {
  auto self = static_cast< LinuxTimer * >( signalInfo->si_value.sival_ptr );
  self->cb_();
}
#endif

#if defined( SFG_GEN_IS_MACOS )
MacOsTimer::MacOsTimer( uint32_t durationMs, const std::function< void() > &callback ) : durationMs_( durationMs ), cb_( callback ) {
  if( !cb_ ) {
    throw std::invalid_argument( "callback can't be null" );
  }

  ctx_.copyDescription = nullptr;
  ctx_.info = this;
  ctx_.release = nullptr;
  ctx_.retain = nullptr;
  ctx_.version = 0;
}

MacOsTimer::~MacOsTimer() {
  if( timer_ ) {
    CFRelease( timer_ );
    timer_ = nullptr;
  }
}

void MacOsTimer::start() {
  auto runLoop = CFRunLoopGetCurrent();

  timer_ = CFRunLoopTimerCreate( kCFAllocatorDefault, 0, durationMs_ * 0.0001, 0, 0, &MacOsTimer::onTimer, &ctx_ );
  if( timer_ ) {
    CFRunLoopAddTimer( runLoop, timer_, kCFRunLoopCommonModes );
  }
}

void MacOsTimer::stop() {
  auto runLoop = CFRunLoopGetCurrent();

  if( timer_ ) {
    CFRunLoopRemoveTimer( runLoop, timer_, kCFRunLoopCommonModes );
  }
}

void MacOsTimer::onTimer( CFRunLoopTimerRef /*timer*/, void *ctx ) {
  auto self = static_cast< MacOsTimer * >( ctx );
  self->cb_();
}
#endif

#if defined( SFG_GEN_IS_WINDOWS )
std::mutex WindowsTimer::timersLock_;
std::unordered_map< UINT_PTR, WindowsTimer * > WindowsTimer::timers_;

WindowsTimer::WindowsTimer( uint32_t durationMs, const std::function< void() > &callback ) : durationMs_( durationMs ), cb_( callback ) {
  if( !cb_ ) {
    throw std::invalid_argument( "callback can't be null" );
  }
}

WindowsTimer::~WindowsTimer() {
  stop();
}

void WindowsTimer::start() {
  timer_ = SetTimer( nullptr, 0, durationMs_, &WindowsTimer::onTimer );

  if( timer_ ) {
    std::lock_guard< std::mutex > guard( timersLock_ );
    auto it = timers_.emplace( timer_, this );
  }
}

void WindowsTimer::stop() {
  if( timer_ ) {
    {
      std::lock_guard< std::mutex > guard( timersLock_ );
      timers_.erase( timer_ );
    }
    KillTimer( nullptr, timer_ );
    timer_ = 0;
  }
}

void WindowsTimer::onTimer( HWND /*hwnd*/, UINT /*message*/, UINT_PTR idTimer, DWORD /*dwTime*/ ) {
  WindowsTimer *self = nullptr;
  {
    std::lock_guard< std::mutex > guard( timersLock_ );
    auto it = timers_.find( idTimer );
    if( it == timers_.end() ) {
      return;
    }
    self = it->second;
  }

  if( self ) {
    self->cb_();
  }
}
#endif
