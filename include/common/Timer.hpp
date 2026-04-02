#pragma once

// C++ std includes
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

class Timer {
  public:
  virtual ~Timer();

  virtual void start() = 0;
  virtual void stop() = 0;

  static std::unique_ptr< Timer > createNative( uint32_t durationMs, std::function< void() > const& callback );
};

#if defined( SFG_GEN_IS_LINUX )
// System includes
#include <signal.h>
#include <time.h>

class LinuxTimer final : public Timer {
  public:
  LinuxTimer( uint32_t durationMs, std::function< void() > const& callback );
  ~LinuxTimer() override;

  void start() override;
  void stop() override;

  private:
  static void onTimer( int signal, siginfo_t* signalInfo, void* uc );

  uint32_t const durationMs_;
  std::function< void() > const cb_;
  struct sigaction signalAction_;
  sigset_t signalMask_;
  sigevent signalEvent_;
  timer_t timer_;
  itimerspec timerSpecification_;
};
#endif

#if defined( SFG_GEN_IS_MACOS )
// System includes
#include <CoreFoundation/CoreFoundation.h>

class MacOsTimer final : public Timer {
  public:
  MacOsTimer( uint32_t durationMs, std::function< void() > const& callback );
  ~MacOsTimer() override;

  void start() override;
  void stop() override;

  private:
  static void onTimer( CFRunLoopTimerRef timer, void* ctx );

  uint32_t const durationMs_;
  std::function< void() > const cb_;
  CFRunLoopTimerRef timer_ = nullptr;
  CFRunLoopTimerContext ctx_;
};
#endif

#if defined( SFG_GEN_IS_WINDOWS )
// System includes
#if !defined( WIN32_LEAN_AND_MEAN )
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

class WindowsTimer final : public Timer {
  public:
  WindowsTimer( uint32_t durationMs, std::function< void() > const& callback );
  ~WindowsTimer() override;

  void start() override;
  void stop() override;

  private:
  static void onTimer( HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime );

  uint32_t const durationMs_;
  std::function< void() > const cb_;
  UINT_PTR timer_ = 0;
  static std::mutex timersLock_;
  static std::unordered_map< UINT_PTR, WindowsTimer* > timers_;
};
#endif
