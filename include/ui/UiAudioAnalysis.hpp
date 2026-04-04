#pragma once

// Project includes
#include "common/_fmt.hpp"
#include "ui/SfgSlider.hpp"

// Other lib includes
#include <Iir.h>
#include <QGridLayout>
#include <QLabel>
#include <QVector>
#include <QWidget>
#if !defined( Q_MOC_RUN )
#include <boost/circular_buffer.hpp>
#endif

class UiAudioAnalysis : public QWidget {
  Q_OBJECT;
  using _base_ = QWidget;
  using _base_::_base_;

  public:
  UiAudioAnalysis( std::shared_ptr< spdlog::logger > logger, double sampleRate, QWidget* parent = nullptr );
  virtual ~UiAudioAnalysis();

  // to get from CLAP's audio thread into our QT thread
  public Q_SLOTS:
  void pushSample( double sample, uint32_t channel );
  Q_SIGNALS:
  void emitSample( double sample, uint32_t channel );
  protected Q_SLOTS:
  void sampleReceived( double sample, uint32_t channel );

  public Q_SLOTS:
  // void setTimeWindowValue( double value );

  protected Q_SLOTS:
  // void timeWindowSliderNewValue( int value );
  void rmsSampleReceived( double rmsSample, uint32_t channel );
  void lufsSampleReceived( double lufsSample, uint32_t channel );
  void rmsMomentaryValueReceived( double rmsMomentaryValue );
  void lufsMomentaryValueReceived( double lufsMomentaryValue );
  void rmsShortTermValueReceived( double rmsShortTermValue );
  void lufsShortTermValueReceived( double lufsShortTermValue );

  Q_SIGNALS:
  void emitRmsSample( double rmsSample, uint32_t channel );
  void emitLufsSample( double lufsSample, uint32_t channel );
  void emitRmsMomentaryValue( double rmsMomentaryValue );
  void emitLufsMomentaryValue( double lufsMomentaryValue );
  void emitRmsShortTermValue( double rmsShortTermValue );
  void emitLufsShortTermValue( double lufsShortTermValue );

  private:
  static double averageOf( boost::circular_buffer< double > const& buffer );
  static QString valueToFs( double value, std::string const& unit );

  private:
  std::shared_ptr< spdlog::logger > logger_;
  double sampleRate_;
  std::array< Iir::Butterworth::HighShelf< 2 >, 2 > kWeightingFilterHighShelf_;  // for LUFS: 4 dB highshelf at 2 kHz
  std::array< Iir::Butterworth::HighPass< 2 >, 2 > kWeightingFilterHighPass_;    // for LUFS: 12 dB/oct highpass at 100 Hz
  std::array< boost::circular_buffer< double >, 2 > rmsMomentaryValueBuffer_;    // stores the last 400 ms of samples (1 new value every 0.1 seconds)
  std::array< uint32_t, 2 > rmsSamplesReceived_{};
  std::array< boost::circular_buffer< double >, 2 > lufsMomentaryValueBuffer_;  // stores the last 400 ms of samples (1 new value every 0.1 seconds)
  std::array< uint32_t, 2 > lufsSamplesReceived_{};
  boost::circular_buffer< double > rmsShortTermValueBuffer_;   // stores the last 3 seconds of momentary values (1 value every 0.1 seconds => 30 values)
  boost::circular_buffer< double > lufsShortTermValueBuffer_;  // stores the last 3 seconds of momentary values (1 value every 0.1 seconds => 30 values)
  QGridLayout* layout_ = nullptr;
  QLabel* momentaryRmsLabel_ = nullptr;
  QLabel* momentaryRmsValueLabel_ = nullptr;
  SfgSlider* momentaryRmsSlider_ = nullptr;
  QLabel* shortTermRmsLabel_ = nullptr;
  QLabel* shortTermRmsValueLabel_ = nullptr;
  SfgSlider* shortTermRmsSlider_ = nullptr;
  QLabel* momentaryLufsLabel_ = nullptr;
  QLabel* momentaryLufsValueLabel_ = nullptr;
  SfgSlider* momentaryLufsSlider_ = nullptr;
  QLabel* shortTermLufsLabel_ = nullptr;
  QLabel* shortTermLufsValueLabel_ = nullptr;
  SfgSlider* shortTermLufsSlider_ = nullptr;
  // SfgSlider* timeWindowSlider_ = nullptr;

  private:
  int const sliderMinValue_ = 0;
  int const sliderMaxValue_ = 1 << 30;
};
