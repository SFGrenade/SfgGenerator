#pragma once

// Project includes
#include "common/_fmt.hpp"
#include "ui/SfgSlider.hpp"

// Other lib includes
#include <QGridLayout>
#include <QLabel>
#include <QVector>
#include <QWidget>

class UiAudioAnalysis : public QWidget {
  Q_OBJECT;
  using _base_ = QWidget;
  using _base_::_base_;

  public:
  UiAudioAnalysis( std::shared_ptr< spdlog::logger > logger, QWidget* parent = nullptr );
  virtual ~UiAudioAnalysis();

  public Q_SLOTS:
  void pushRmsMomentary( double value );
  void pushLufsMomentary( double value );
  Q_SIGNALS:
  void emitRmsMomentary( double value );
  void emitLufsMomentary( double value );
  protected Q_SLOTS:
  void rmsMomentaryReceived( double value );
  void lufsMomentaryReceived( double value );

  public Q_SLOTS:
  // void setTimeWindowValue( double value );

  protected Q_SLOTS:
  // void timeWindowSliderNewValue( int value );

  Q_SIGNALS:
  // void timeWindowAdjusted( double value );

  private:
  std::shared_ptr< spdlog::logger > logger_;
  QGridLayout* layout_ = nullptr;
  QLabel* momentaryRmsLabel_ = nullptr;
  SfgSlider* momentaryRmsSlider_ = nullptr;
  QLabel* momentaryLufsLabel_ = nullptr;
  SfgSlider* momentaryLufsSlider_ = nullptr;
  // SfgSlider* timeWindowSlider_ = nullptr;

  private:
  int const sliderMinValue_ = 0;
  int const sliderMaxValue_ = 1 << 30;
};
