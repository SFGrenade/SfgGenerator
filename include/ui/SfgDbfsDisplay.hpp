#pragma once

// Other lib includes
#include <QMouseEvent>
#include <QSlider>
#include <QWidget>

class SfgDbfsDisplay : public QWidget {
  Q_OBJECT;
  using _base_ = QWidget;
  using _base_::_base_;

  public:
  SfgDbfsDisplay( Qt::Orientation orientation, QWidget* parent = nullptr );
  virtual ~SfgDbfsDisplay();

  public Q_SLOTS:
  void setDBFS( double dBFS );
  void setValue( double value );

  protected:
  void paintEvent( QPaintEvent* event ) override;

  private:
  static constexpr double const MIN_DBFS_ = -70.0;
  static constexpr double const MAX_DBFS_ = 0.0;
  Qt::Orientation orientation_;
  double dBFS_ = -70.0;
};
