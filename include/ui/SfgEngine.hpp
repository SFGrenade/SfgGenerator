#pragma once

// Project includes
#include "common/_fmt.hpp"
#include "common/Timer.hpp"

// Other lib includes
#include <QApplication>
#include <QEventLoop>
#include <QObject>
#include <QTimer>
#include <QWidget>

class SfgEngine : public QObject {
  Q_OBJECT;
  using _base_ = QObject;
  using _base_::_base_;

  public:
  SfgEngine( std::shared_ptr< spdlog::logger > logger, QApplication* app, QWidget* hiddenWatcher, QObject* parent = nullptr );
  virtual ~SfgEngine();

  public Q_SLOTS:
  void start();
  void stop();

  protected Q_SLOTS:
  void timerReceiver();

  Q_SIGNALS:
  void timerTicked();

  private:
  std::shared_ptr< spdlog::logger > logger_;
  QApplication* app_ = nullptr;
  QWidget* hiddenWatcher_ = nullptr;
  QTimer* timer_ = nullptr;
  // std::unique_ptr<Timer> timer_ = nullptr;
};
