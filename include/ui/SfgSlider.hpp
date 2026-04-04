#pragma once

// Other lib includes
#include <QMouseEvent>
#include <QSlider>
#include <QWidget>

class SfgSlider : public QSlider {
  Q_OBJECT;
  using _base_ = QSlider;
  using _base_::_base_;

  public:
  SfgSlider( int minVal, int maxVal, Qt::Orientation orientation, QWidget* parent = nullptr );
  SfgSlider( int minVal, int maxVal, int defaultVal, Qt::Orientation orientation, QWidget* parent = nullptr );
  virtual ~SfgSlider();

  public Q_SLOTS:
  void setRange( int min, int max );
  void setRange( int min, int max, int def );

  protected:
  void mouseDoubleClickEvent( QMouseEvent* event ) override;

  private:
  int minValue_;
  int maxValue_;
  int defaultValue_;
};
