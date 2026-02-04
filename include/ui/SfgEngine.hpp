#pragma once

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
  SfgEngine( QApplication* app, QWidget* hiddenWatcher, QObject* parent = nullptr );
  virtual ~SfgEngine();

  public Q_SLOTS:
  void start();
  void stop();

  protected Q_SLOTS:
  void timerReceiver();

  Q_SIGNALS:
  void timerTicked();

  private:
  QApplication* app_ = nullptr;
  QWidget* hiddenWatcher_ = nullptr;
  QTimer* timer_ = nullptr;
};
