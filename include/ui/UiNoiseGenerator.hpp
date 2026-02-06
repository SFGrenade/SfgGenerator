#pragma once

// Project includes
#include "ui/SfgSlider.hpp"

// Project includes
#include <common/_fmt.hpp>

// Other lib includes
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QWidget>

class UiNoiseGenerator : public QWidget {
  Q_OBJECT;
  using _base_ = QWidget;
  using _base_::_base_;

  public:
  UiNoiseGenerator( std::shared_ptr< spdlog::logger > logger, QWidget *parent = nullptr );
  virtual ~UiNoiseGenerator();

  public Q_SLOTS:
  void setSineWaveTypeValue( double value );
  void setSineWaveMixValue( double value );
  void setSquareWaveTypeValue( double value );
  void setSquareWavePwmValue( double value );
  void setSquareWaveMixValue( double value );
  void setSawWaveTypeValue( double value );
  void setSawWaveMixValue( double value );
  void setTriangleWaveTypeValue( double value );
  void setTriangleWaveMixValue( double value );
  void setWhiteNoiseTypeValue( double value );
  void setWhiteNoiseMixValue( double value );
  void setPinkNoiseTypeValue( double value );
  void setPinkNoiseMixValue( double value );
  void setRedNoiseTypeValue( double value );
  void setRedNoiseMixValue( double value );
  void setBlueNoiseTypeValue( double value );
  void setBlueNoiseMixValue( double value );
  void setVioletNoiseTypeValue( double value );
  void setVioletNoiseMixValue( double value );
  void setGreyNoiseTypeValue( double value );
  void setGreyNoiseMixValue( double value );
  void setVelvetNoiseTypeValue( double value );
  void setVelvetNoiseMixValue( double value );

  protected Q_SLOTS:
  void sineWaveTypeNewValue( int value );
  void sineWaveMixNewValue( int value );
  void squareWaveTypeNewValue( int value );
  void squareWavePwmNewValue( int value );
  void squareWaveMixNewValue( int value );
  void sawWaveTypeNewValue( int value );
  void sawWaveMixNewValue( int value );
  void triangleWaveTypeNewValue( int value );
  void triangleWaveMixNewValue( int value );
  void whiteNoiseTypeNewValue( int value );
  void whiteNoiseMixNewValue( int value );
  void pinkNoiseTypeNewValue( int value );
  void pinkNoiseMixNewValue( int value );
  void redNoiseTypeNewValue( int value );
  void redNoiseMixNewValue( int value );
  void blueNoiseTypeNewValue( int value );
  void blueNoiseMixNewValue( int value );
  void violetNoiseTypeNewValue( int value );
  void violetNoiseMixNewValue( int value );
  void greyNoiseTypeNewValue( int value );
  void greyNoiseMixNewValue( int value );
  void velvetNoiseTypeNewValue( int value );
  void velvetNoiseMixNewValue( int value );

  Q_SIGNALS:
  void sineWaveTypeAdjusted( double value );
  void sineWaveMixAdjusted( double value );
  void squareWaveTypeAdjusted( double value );
  void squareWavePwmAdjusted( double value );
  void squareWaveMixAdjusted( double value );
  void sawWaveTypeAdjusted( double value );
  void sawWaveMixAdjusted( double value );
  void triangleWaveTypeAdjusted( double value );
  void triangleWaveMixAdjusted( double value );
  void whiteNoiseTypeAdjusted( double value );
  void whiteNoiseMixAdjusted( double value );
  void pinkNoiseTypeAdjusted( double value );
  void pinkNoiseMixAdjusted( double value );
  void redNoiseTypeAdjusted( double value );
  void redNoiseMixAdjusted( double value );
  void blueNoiseTypeAdjusted( double value );
  void blueNoiseMixAdjusted( double value );
  void violetNoiseTypeAdjusted( double value );
  void violetNoiseMixAdjusted( double value );
  void greyNoiseTypeAdjusted( double value );
  void greyNoiseMixAdjusted( double value );
  void velvetNoiseTypeAdjusted( double value );
  void velvetNoiseMixAdjusted( double value );

  private:
  std::shared_ptr< spdlog::logger > logger_;
  QGridLayout* layout_ = nullptr;

  // sine wave
  QFrame* frameSineWave_ = nullptr;
  QGridLayout* frameSineWaveLayout_ = nullptr;
  QComboBox* comboBoxSineWaveType_ = nullptr;
  SfgSlider* sliderSineWaveMix_ = nullptr;
  // square wave
  QFrame* frameSquareWave_ = nullptr;
  QGridLayout* frameSquareWaveLayout_ = nullptr;
  QComboBox* comboBoxSquareWaveType_ = nullptr;
  SfgSlider* sliderSquareWavePwm_ = nullptr;
  SfgSlider* sliderSquareWaveMix_ = nullptr;
  // saw wave
  QFrame* frameSawWave_ = nullptr;
  QGridLayout* frameSawWaveLayout_ = nullptr;
  QComboBox* comboBoxSawWaveType_ = nullptr;
  SfgSlider* sliderSawWaveMix_ = nullptr;
  // triangle wave
  QFrame* frameTriangleWave_ = nullptr;
  QGridLayout* frameTriangleWaveLayout_ = nullptr;
  QComboBox* comboBoxTriangleWaveType_ = nullptr;
  SfgSlider* sliderTriangleWaveMix_ = nullptr;
  // white noise
  QFrame* frameWhiteNoise_ = nullptr;
  QGridLayout* frameWhiteNoiseLayout_ = nullptr;
  QComboBox* comboBoxWhiteNoiseType_ = nullptr;
  SfgSlider* sliderWhiteNoiseMix_ = nullptr;
  // pink noise
  QFrame* framePinkNoise_ = nullptr;
  QGridLayout* framePinkNoiseLayout_ = nullptr;
  QComboBox* comboBoxPinkNoiseType_ = nullptr;
  SfgSlider* sliderPinkNoiseMix_ = nullptr;
  // red noise
  QFrame* frameRedNoise_ = nullptr;
  QGridLayout* frameRedNoiseLayout_ = nullptr;
  QComboBox* comboBoxRedNoiseType_ = nullptr;
  SfgSlider* sliderRedNoiseMix_ = nullptr;
  // blue noise
  QFrame* frameBlueNoise_ = nullptr;
  QGridLayout* frameBlueNoiseLayout_ = nullptr;
  QComboBox* comboBoxBlueNoiseType_ = nullptr;
  SfgSlider* sliderBlueNoiseMix_ = nullptr;
  // violet noise
  QFrame* frameVioletNoise_ = nullptr;
  QGridLayout* frameVioletNoiseLayout_ = nullptr;
  QComboBox* comboBoxVioletNoiseType_ = nullptr;
  SfgSlider* sliderVioletNoiseMix_ = nullptr;
  // grey noise
  QFrame* frameGreyNoise_ = nullptr;
  QGridLayout* frameGreyNoiseLayout_ = nullptr;
  QComboBox* comboBoxGreyNoiseType_ = nullptr;
  SfgSlider* sliderGreyNoiseMix_ = nullptr;
  // velvet noise
  QFrame* frameVelvetNoise_ = nullptr;
  QGridLayout* frameVelvetNoiseLayout_ = nullptr;
  QComboBox* comboBoxVelvetNoiseType_ = nullptr;
  SfgSlider* sliderVelvetNoiseMix_ = nullptr;

  private:
  int const abSliderMinValue_ = 0;
  int const abSliderMaxValue_ = 1 << 30;
};
