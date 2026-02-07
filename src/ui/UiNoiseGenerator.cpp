// Header assigned to this source
#include "ui/UiNoiseGenerator.hpp"

// Other lib includes
#include <QWindow>

UiNoiseGenerator::UiNoiseGenerator( std::shared_ptr< spdlog::logger > logger, QWidget *parent ) : _base_( parent ), logger_( logger ) {
  setGeometry( 0, 0, 300, 200 );
  setMinimumSize( 1, 1 );

  frameSineWave_ = new QFrame( this );
  frameSineWave_->setFrameStyle( QFrame::Box | QFrame::Plain );
  frameSineWaveLayout_ = new QGridLayout( frameSineWave_ );
  frameSineWaveLayout_->addWidget( new QLabel( "Sine Wave" ), 0, 0, 1, 2, Qt::AlignCenter );
  comboBoxSineWaveType_ = new QComboBox( frameSineWave_ );
  comboBoxSineWaveType_->addItem( "StdSin", 0 );  // todo: fixme: adjust to NoiseGenerator::SineWaveType::StdSin
  comboBoxSineWaveType_->addItem( "CSin", 1 );    // todo: fixme: adjust to NoiseGenerator::SineWaveType::CSin
  frameSineWaveLayout_->addWidget( comboBoxSineWaveType_, 1, 0 );
  sliderSineWaveMix_ = new SfgSlider( abSliderMinValue_, abSliderMaxValue_, abSliderMinValue_, Qt::Vertical, frameSineWave_ );
  frameSineWaveLayout_->addWidget( sliderSineWaveMix_, 1, 1 );
  frameSineWave_->setLayout( frameSineWaveLayout_ );

  frameSquareWave_ = new QFrame( this );
  frameSquareWave_->setFrameStyle( QFrame::Box | QFrame::Plain );
  frameSquareWaveLayout_ = new QGridLayout( frameSquareWave_ );
  frameSquareWaveLayout_->addWidget( new QLabel( "Square Wave" ), 0, 0, 1, 3, Qt::AlignCenter );
  comboBoxSquareWaveType_ = new QComboBox( frameSquareWave_ );
  comboBoxSquareWaveType_->addItem( "PhaseWidth", 0 );         // todo: fixme: adjust to NoiseGenerator::SquareWaveType::PhaseWidth
  comboBoxSquareWaveType_->addItem( "InversePhaseWidth", 1 );  // todo: fixme: adjust to NoiseGenerator::SquareWaveType::InversePhaseWidth
  frameSquareWaveLayout_->addWidget( comboBoxSquareWaveType_, 1, 0 );
  sliderSquareWavePwm_ = new SfgSlider( abSliderMinValue_, abSliderMaxValue_, Qt::Vertical, frameSquareWave_ );
  frameSquareWaveLayout_->addWidget( sliderSquareWavePwm_, 1, 1 );
  sliderSquareWaveMix_ = new SfgSlider( abSliderMinValue_, abSliderMaxValue_, abSliderMinValue_, Qt::Vertical, frameSquareWave_ );
  frameSquareWaveLayout_->addWidget( sliderSquareWaveMix_, 1, 2 );
  frameSquareWave_->setLayout( frameSquareWaveLayout_ );

  frameSawWave_ = new QFrame( this );
  frameSawWave_->setFrameStyle( QFrame::Box | QFrame::Plain );
  frameSawWaveLayout_ = new QGridLayout( frameSawWave_ );
  frameSawWaveLayout_->addWidget( new QLabel( "Saw Wave" ), 0, 0, 1, 2, Qt::AlignCenter );
  comboBoxSawWaveType_ = new QComboBox( frameSawWave_ );
  comboBoxSawWaveType_->addItem( "Phase", 0 );         // todo: fixme: adjust to NoiseGenerator::SawWaveType::Phase
  comboBoxSawWaveType_->addItem( "InversePhase", 1 );  // todo: fixme: adjust to NoiseGenerator::SawWaveType::InversePhase
  frameSawWaveLayout_->addWidget( comboBoxSawWaveType_, 1, 0 );
  sliderSawWaveMix_ = new SfgSlider( abSliderMinValue_, abSliderMaxValue_, abSliderMinValue_, Qt::Vertical, frameSawWave_ );
  frameSawWaveLayout_->addWidget( sliderSawWaveMix_, 1, 1 );
  frameSawWave_->setLayout( frameSawWaveLayout_ );

  frameTriangleWave_ = new QFrame( this );
  frameTriangleWave_->setFrameStyle( QFrame::Box | QFrame::Plain );
  frameTriangleWaveLayout_ = new QGridLayout( frameTriangleWave_ );
  frameTriangleWaveLayout_->addWidget( new QLabel( "Triangle Wave" ), 0, 0, 1, 2, Qt::AlignCenter );
  comboBoxTriangleWaveType_ = new QComboBox( frameTriangleWave_ );
  comboBoxTriangleWaveType_->addItem( "ChunkLerp", 0 );  // todo: fixme: adjust to NoiseGenerator::TriangleWaveType::ChunkLerp
  frameTriangleWaveLayout_->addWidget( comboBoxTriangleWaveType_, 1, 0 );
  sliderTriangleWaveMix_ = new SfgSlider( abSliderMinValue_, abSliderMaxValue_, abSliderMinValue_, Qt::Vertical, frameTriangleWave_ );
  frameTriangleWaveLayout_->addWidget( sliderTriangleWaveMix_, 1, 1 );
  frameTriangleWave_->setLayout( frameTriangleWaveLayout_ );

  frameWhiteNoise_ = new QFrame( this );
  frameWhiteNoise_->setFrameStyle( QFrame::Box | QFrame::Plain );
  frameWhiteNoiseLayout_ = new QGridLayout( frameWhiteNoise_ );
  frameWhiteNoiseLayout_->addWidget( new QLabel( "White Noise" ), 0, 0, 1, 2, Qt::AlignCenter );
  comboBoxWhiteNoiseType_ = new QComboBox( frameWhiteNoise_ );
  comboBoxWhiteNoiseType_->addItem( "StdRandom", 0 );    // todo: fixme: adjust to NoiseGenerator::WhiteNoiseType::StdRandom
  comboBoxWhiteNoiseType_->addItem( "RandMaxRand", 1 );  // todo: fixme: adjust to NoiseGenerator::WhiteNoiseType::RandMaxRand
  frameWhiteNoiseLayout_->addWidget( comboBoxWhiteNoiseType_, 1, 0 );
  sliderWhiteNoiseMix_ = new SfgSlider( abSliderMinValue_, abSliderMaxValue_, abSliderMinValue_, Qt::Vertical, frameWhiteNoise_ );
  frameWhiteNoiseLayout_->addWidget( sliderWhiteNoiseMix_, 1, 1 );
  frameWhiteNoise_->setLayout( frameWhiteNoiseLayout_ );

  framePinkNoise_ = new QFrame( this );
  framePinkNoise_->setFrameStyle( QFrame::Box | QFrame::Plain );
  framePinkNoiseLayout_ = new QGridLayout( framePinkNoise_ );
  framePinkNoiseLayout_->addWidget( new QLabel( "Pink Noise" ), 0, 0, 1, 2, Qt::AlignCenter );
  comboBoxPinkNoiseType_ = new QComboBox( framePinkNoise_ );
  comboBoxPinkNoiseType_->addItem( "PaulKellettRefined", 0 );      // todo: fixme: adjust to NoiseGenerator::PinkNoiseType::PaulKellettRefined
  comboBoxPinkNoiseType_->addItem( "PaulKellettEconomy", 1 );      // todo: fixme: adjust to NoiseGenerator::PinkNoiseType::PaulKellettEconomy
  comboBoxPinkNoiseType_->addItem( "VossMcCartney", 2 );           // todo: fixme: adjust to NoiseGenerator::PinkNoiseType::VossMcCartney
  comboBoxPinkNoiseType_->addItem( "IirFilterApproximation", 3 );  // todo: fixme: adjust to NoiseGenerator::PinkNoiseType::IirFilterApproximation
  framePinkNoiseLayout_->addWidget( comboBoxPinkNoiseType_, 1, 0 );
  // todo: fixme: pink noise vossmccartney number thing
  sliderPinkNoiseMix_ = new SfgSlider( abSliderMinValue_, abSliderMaxValue_, abSliderMinValue_, Qt::Vertical, framePinkNoise_ );
  framePinkNoiseLayout_->addWidget( sliderPinkNoiseMix_, 1, 1 );
  framePinkNoise_->setLayout( framePinkNoiseLayout_ );

  frameRedNoise_ = new QFrame( this );
  frameRedNoise_->setFrameStyle( QFrame::Box | QFrame::Plain );
  frameRedNoiseLayout_ = new QGridLayout( frameRedNoise_ );
  frameRedNoiseLayout_->addWidget( new QLabel( "Red Noise" ), 0, 0, 1, 2, Qt::AlignCenter );
  comboBoxRedNoiseType_ = new QComboBox( frameRedNoise_ );
  comboBoxRedNoiseType_->addItem( "BasicIntegration", 0 );     // todo: fixme: adjust to NoiseGenerator::RedNoiseType::BasicIntegration
  comboBoxRedNoiseType_->addItem( "LeakyIntegration", 1 );     // todo: fixme: adjust to NoiseGenerator::RedNoiseType::LeakyIntegration
  comboBoxRedNoiseType_->addItem( "IntegerWalk", 2 );          // todo: fixme: adjust to NoiseGenerator::RedNoiseType::IntegerWalk
  comboBoxRedNoiseType_->addItem( "OnePoleIirFilter", 3 );     // todo: fixme: adjust to NoiseGenerator::RedNoiseType::OnePoleIirFilter
  comboBoxRedNoiseType_->addItem( "CumulativeWithClamp", 4 );  // todo: fixme: adjust to NoiseGenerator::RedNoiseType::CumulativeWithClamp
  frameRedNoiseLayout_->addWidget( comboBoxRedNoiseType_, 1, 0 );
  sliderRedNoiseMix_ = new SfgSlider( abSliderMinValue_, abSliderMaxValue_, abSliderMinValue_, Qt::Vertical, frameRedNoise_ );
  frameRedNoiseLayout_->addWidget( sliderRedNoiseMix_, 1, 1 );
  frameRedNoise_->setLayout( frameRedNoiseLayout_ );

  frameBlueNoise_ = new QFrame( this );
  frameBlueNoise_->setFrameStyle( QFrame::Box | QFrame::Plain );
  frameBlueNoiseLayout_ = new QGridLayout( frameBlueNoise_ );
  frameBlueNoiseLayout_->addWidget( new QLabel( "Blue Noise" ), 0, 0, 1, 2, Qt::AlignCenter );
  comboBoxBlueNoiseType_ = new QComboBox( frameBlueNoise_ );
  comboBoxBlueNoiseType_->addItem( "VoidAndCluster", 0 );         // todo: fixme: adjust to NoiseGenerator::BlueNoiseType::VoidAndCluster
  comboBoxBlueNoiseType_->addItem( "PoissonDiskSampling", 1 );    // todo: fixme: adjust to NoiseGenerator::BlueNoiseType::PoissonDiskSampling
  comboBoxBlueNoiseType_->addItem( "SimpleSpectralShaping", 2 );  // todo: fixme: adjust to NoiseGenerator::BlueNoiseType::SimpleSpectralShaping
  comboBoxBlueNoiseType_->addItem( "R2JitteredSampling", 3 );     // todo: fixme: adjust to NoiseGenerator::BlueNoiseType::R2JitteredSampling
  comboBoxBlueNoiseType_->addItem( "PermutedGradientNoise", 4 );  // todo: fixme: adjust to NoiseGenerator::BlueNoiseType::PermutedGradientNoise
  frameBlueNoiseLayout_->addWidget( comboBoxBlueNoiseType_, 1, 0 );
  sliderBlueNoiseMix_ = new SfgSlider( abSliderMinValue_, abSliderMaxValue_, abSliderMinValue_, Qt::Vertical, frameBlueNoise_ );
  frameBlueNoiseLayout_->addWidget( sliderBlueNoiseMix_, 1, 1 );
  frameBlueNoise_->setLayout( frameBlueNoiseLayout_ );

  frameVioletNoise_ = new QFrame( this );
  frameVioletNoise_->setFrameStyle( QFrame::Box | QFrame::Plain );
  frameVioletNoiseLayout_ = new QGridLayout( frameVioletNoise_ );
  frameVioletNoiseLayout_->addWidget( new QLabel( "Violet Noise" ), 0, 0, 1, 2, Qt::AlignCenter );
  comboBoxVioletNoiseType_ = new QComboBox( frameVioletNoise_ );
  comboBoxVioletNoiseType_->addItem( "FirstOrderDifference", 0 );  // todo: fixme: adjust to NoiseGenerator::VioletNoiseType::FirstOrderDifference
  comboBoxVioletNoiseType_->addItem( "FirstOrderIirFilter", 1 );   // todo: fixme: adjust to NoiseGenerator::VioletNoiseType::FirstOrderIirFilter
  frameVioletNoiseLayout_->addWidget( comboBoxVioletNoiseType_, 1, 0 );
  sliderVioletNoiseMix_ = new SfgSlider( abSliderMinValue_, abSliderMaxValue_, abSliderMinValue_, Qt::Vertical, frameVioletNoise_ );
  frameVioletNoiseLayout_->addWidget( sliderVioletNoiseMix_, 1, 1 );
  frameVioletNoise_->setLayout( frameVioletNoiseLayout_ );

  frameGreyNoise_ = new QFrame( this );
  frameGreyNoise_->setFrameStyle( QFrame::Box | QFrame::Plain );
  frameGreyNoiseLayout_ = new QGridLayout( frameGreyNoise_ );
  frameGreyNoiseLayout_->addWidget( new QLabel( "Grey Noise" ), 0, 0, 1, 2, Qt::AlignCenter );
  comboBoxGreyNoiseType_ = new QComboBox( frameGreyNoise_ );
  comboBoxGreyNoiseType_->addItem( "PsychoacousticFilter", 0 );        // todo: fixme: adjust to NoiseGenerator::GreyNoiseType::PsychoacousticFilter
  comboBoxGreyNoiseType_->addItem( "AweightingInversion", 1 );         // todo: fixme: adjust to NoiseGenerator::GreyNoiseType::AweightingInversion
  comboBoxGreyNoiseType_->addItem( "MultiBandpass", 2 );               // todo: fixme: adjust to NoiseGenerator::GreyNoiseType::MultiBandpass
  comboBoxGreyNoiseType_->addItem( "EqualLoudnessApproximation", 3 );  // todo: fixme: adjust to NoiseGenerator::GreyNoiseType::EqualLoudnessApproximation
  frameGreyNoiseLayout_->addWidget( comboBoxGreyNoiseType_, 1, 0 );
  sliderGreyNoiseMix_ = new SfgSlider( abSliderMinValue_, abSliderMaxValue_, abSliderMinValue_, Qt::Vertical, frameGreyNoise_ );
  frameGreyNoiseLayout_->addWidget( sliderGreyNoiseMix_, 1, 1 );
  frameGreyNoise_->setLayout( frameGreyNoiseLayout_ );

  frameVelvetNoise_ = new QFrame( this );
  frameVelvetNoise_->setFrameStyle( QFrame::Box | QFrame::Plain );
  frameVelvetNoiseLayout_ = new QGridLayout( frameVelvetNoise_ );
  frameVelvetNoiseLayout_->addWidget( new QLabel( "Velvet Noise" ), 0, 0, 1, 2, Qt::AlignCenter );
  comboBoxVelvetNoiseType_ = new QComboBox( frameVelvetNoise_ );
  comboBoxVelvetNoiseType_->addItem( "SporadicImpulse", 0 );  // todo: fixme: adjust to NoiseGenerator::VelvetNoiseType::SporadicImpulse
  frameVelvetNoiseLayout_->addWidget( comboBoxVelvetNoiseType_, 1, 0 );
  sliderVelvetNoiseMix_ = new SfgSlider( abSliderMinValue_, abSliderMaxValue_, abSliderMinValue_, Qt::Vertical, frameVelvetNoise_ );
  frameVelvetNoiseLayout_->addWidget( sliderVelvetNoiseMix_, 1, 1 );
  frameVelvetNoise_->setLayout( frameVelvetNoiseLayout_ );

  // construct the main layout
  layout_ = new QGridLayout( this );
  layout_->addWidget( frameSineWave_, 0, 0, 1, 2 );
  layout_->addWidget( frameSquareWave_, 0, 2, 1, 2 );
  layout_->addWidget( frameSawWave_, 0, 4, 1, 2 );
  layout_->addWidget( frameTriangleWave_, 1, 0, 1, 2 );
  layout_->addWidget( frameWhiteNoise_, 1, 2, 1, 2 );
  layout_->addWidget( framePinkNoise_, 1, 4, 1, 2 );
  layout_->addWidget( frameRedNoise_, 2, 0, 1, 2 );
  layout_->addWidget( frameBlueNoise_, 2, 2, 1, 2 );
  layout_->addWidget( frameVioletNoise_, 2, 4, 1, 2 );
  layout_->addWidget( frameGreyNoise_, 3, 0, 1, 3 );
  layout_->addWidget( frameVelvetNoise_, 3, 3, 1, 3 );

  layout_->setRowStretch( 0, 1 );
  layout_->setRowStretch( 1, 1 );
  layout_->setRowStretch( 2, 1 );
  layout_->setRowStretch( 3, 1 );
  layout_->setColumnStretch( 0, 1 );
  layout_->setColumnStretch( 1, 1 );
  layout_->setColumnStretch( 2, 1 );
  layout_->setColumnStretch( 3, 1 );
  layout_->setColumnStretch( 4, 1 );
  layout_->setColumnStretch( 5, 1 );

  setLayout( layout_ );

  connect( comboBoxSineWaveType_,
           QOverload< int >::of( &QComboBox::currentIndexChanged ),
           this,
           &UiNoiseGenerator::sineWaveTypeNewValue,
           Qt::QueuedConnection );
  connect( sliderSineWaveMix_, &SfgSlider::valueChanged, this, &UiNoiseGenerator::sineWaveMixNewValue, Qt::QueuedConnection );
  connect( sliderSineWaveMix_, &SfgSlider::sliderMoved, this, &UiNoiseGenerator::sineWaveMixNewValue, Qt::QueuedConnection );

  connect( comboBoxSquareWaveType_,
           QOverload< int >::of( &QComboBox::currentIndexChanged ),
           this,
           &UiNoiseGenerator::squareWaveTypeNewValue,
           Qt::QueuedConnection );
  connect( sliderSquareWavePwm_, &SfgSlider::valueChanged, this, &UiNoiseGenerator::squareWavePwmNewValue, Qt::QueuedConnection );
  connect( sliderSquareWavePwm_, &SfgSlider::sliderMoved, this, &UiNoiseGenerator::squareWavePwmNewValue, Qt::QueuedConnection );
  connect( sliderSquareWaveMix_, &SfgSlider::valueChanged, this, &UiNoiseGenerator::squareWaveMixNewValue, Qt::QueuedConnection );
  connect( sliderSquareWaveMix_, &SfgSlider::sliderMoved, this, &UiNoiseGenerator::squareWaveMixNewValue, Qt::QueuedConnection );

  connect( comboBoxSawWaveType_, QOverload< int >::of( &QComboBox::currentIndexChanged ), this, &UiNoiseGenerator::sawWaveTypeNewValue, Qt::QueuedConnection );
  connect( sliderSawWaveMix_, &SfgSlider::valueChanged, this, &UiNoiseGenerator::sawWaveMixNewValue, Qt::QueuedConnection );
  connect( sliderSawWaveMix_, &SfgSlider::sliderMoved, this, &UiNoiseGenerator::sawWaveMixNewValue, Qt::QueuedConnection );

  connect( comboBoxTriangleWaveType_,
           QOverload< int >::of( &QComboBox::currentIndexChanged ),
           this,
           &UiNoiseGenerator::triangleWaveTypeNewValue,
           Qt::QueuedConnection );
  connect( sliderTriangleWaveMix_, &SfgSlider::valueChanged, this, &UiNoiseGenerator::triangleWaveMixNewValue, Qt::QueuedConnection );
  connect( sliderTriangleWaveMix_, &SfgSlider::sliderMoved, this, &UiNoiseGenerator::triangleWaveMixNewValue, Qt::QueuedConnection );

  connect( comboBoxWhiteNoiseType_,
           QOverload< int >::of( &QComboBox::currentIndexChanged ),
           this,
           &UiNoiseGenerator::whiteNoiseTypeNewValue,
           Qt::QueuedConnection );
  connect( sliderWhiteNoiseMix_, &SfgSlider::valueChanged, this, &UiNoiseGenerator::whiteNoiseMixNewValue, Qt::QueuedConnection );
  connect( sliderWhiteNoiseMix_, &SfgSlider::sliderMoved, this, &UiNoiseGenerator::whiteNoiseMixNewValue, Qt::QueuedConnection );

  connect( comboBoxPinkNoiseType_,
           QOverload< int >::of( &QComboBox::currentIndexChanged ),
           this,
           &UiNoiseGenerator::pinkNoiseTypeNewValue,
           Qt::QueuedConnection );
  connect( sliderPinkNoiseMix_, &SfgSlider::valueChanged, this, &UiNoiseGenerator::pinkNoiseMixNewValue, Qt::QueuedConnection );
  connect( sliderPinkNoiseMix_, &SfgSlider::sliderMoved, this, &UiNoiseGenerator::pinkNoiseMixNewValue, Qt::QueuedConnection );

  connect( comboBoxRedNoiseType_,
           QOverload< int >::of( &QComboBox::currentIndexChanged ),
           this,
           &UiNoiseGenerator::redNoiseTypeNewValue,
           Qt::QueuedConnection );
  connect( sliderRedNoiseMix_, &SfgSlider::valueChanged, this, &UiNoiseGenerator::redNoiseMixNewValue, Qt::QueuedConnection );
  connect( sliderRedNoiseMix_, &SfgSlider::sliderMoved, this, &UiNoiseGenerator::redNoiseMixNewValue, Qt::QueuedConnection );

  connect( comboBoxBlueNoiseType_,
           QOverload< int >::of( &QComboBox::currentIndexChanged ),
           this,
           &UiNoiseGenerator::blueNoiseTypeNewValue,
           Qt::QueuedConnection );
  connect( sliderBlueNoiseMix_, &SfgSlider::valueChanged, this, &UiNoiseGenerator::blueNoiseMixNewValue, Qt::QueuedConnection );
  connect( sliderBlueNoiseMix_, &SfgSlider::sliderMoved, this, &UiNoiseGenerator::blueNoiseMixNewValue, Qt::QueuedConnection );

  connect( comboBoxVioletNoiseType_,
           QOverload< int >::of( &QComboBox::currentIndexChanged ),
           this,
           &UiNoiseGenerator::violetNoiseTypeNewValue,
           Qt::QueuedConnection );
  connect( sliderVioletNoiseMix_, &SfgSlider::valueChanged, this, &UiNoiseGenerator::violetNoiseMixNewValue, Qt::QueuedConnection );
  connect( sliderVioletNoiseMix_, &SfgSlider::sliderMoved, this, &UiNoiseGenerator::violetNoiseMixNewValue, Qt::QueuedConnection );

  connect( comboBoxGreyNoiseType_,
           QOverload< int >::of( &QComboBox::currentIndexChanged ),
           this,
           &UiNoiseGenerator::greyNoiseTypeNewValue,
           Qt::QueuedConnection );
  connect( sliderGreyNoiseMix_, &SfgSlider::valueChanged, this, &UiNoiseGenerator::greyNoiseMixNewValue, Qt::QueuedConnection );
  connect( sliderGreyNoiseMix_, &SfgSlider::sliderMoved, this, &UiNoiseGenerator::greyNoiseMixNewValue, Qt::QueuedConnection );

  connect( comboBoxVelvetNoiseType_,
           QOverload< int >::of( &QComboBox::currentIndexChanged ),
           this,
           &UiNoiseGenerator::velvetNoiseTypeNewValue,
           Qt::QueuedConnection );
  connect( sliderVelvetNoiseMix_, &SfgSlider::valueChanged, this, &UiNoiseGenerator::velvetNoiseMixNewValue, Qt::QueuedConnection );
  connect( sliderVelvetNoiseMix_, &SfgSlider::sliderMoved, this, &UiNoiseGenerator::velvetNoiseMixNewValue, Qt::QueuedConnection );
}

UiNoiseGenerator::~UiNoiseGenerator() {}

void UiNoiseGenerator::setSineWaveTypeValue( double value ) {
  comboBoxSineWaveType_->setCurrentIndex( int( std::round( value ) ) );
}

void UiNoiseGenerator::setSineWaveMixValue( double value ) {
  sliderSineWaveMix_->setValue( int( abSliderMinValue_ + std::round( double( abSliderMaxValue_ - abSliderMinValue_ ) * value ) ) );
}

void UiNoiseGenerator::setSquareWaveTypeValue( double value ) {
  comboBoxSquareWaveType_->setCurrentIndex( int( std::round( value ) ) );
}

void UiNoiseGenerator::setSquareWavePwmValue( double value ) {
  sliderSquareWavePwm_->setValue( int( abSliderMinValue_ + std::round( double( abSliderMaxValue_ - abSliderMinValue_ ) * value ) ) );
}

void UiNoiseGenerator::setSquareWaveMixValue( double value ) {
  sliderSquareWaveMix_->setValue( int( abSliderMinValue_ + std::round( double( abSliderMaxValue_ - abSliderMinValue_ ) * value ) ) );
}

void UiNoiseGenerator::setSawWaveTypeValue( double value ) {
  comboBoxSawWaveType_->setCurrentIndex( int( std::round( value ) ) );
}

void UiNoiseGenerator::setSawWaveMixValue( double value ) {
  sliderSawWaveMix_->setValue( int( abSliderMinValue_ + std::round( double( abSliderMaxValue_ - abSliderMinValue_ ) * value ) ) );
}

void UiNoiseGenerator::setTriangleWaveTypeValue( double value ) {
  comboBoxTriangleWaveType_->setCurrentIndex( int( std::round( value ) ) );
}

void UiNoiseGenerator::setTriangleWaveMixValue( double value ) {
  sliderTriangleWaveMix_->setValue( int( abSliderMinValue_ + std::round( double( abSliderMaxValue_ - abSliderMinValue_ ) * value ) ) );
}

void UiNoiseGenerator::setWhiteNoiseTypeValue( double value ) {
  comboBoxWhiteNoiseType_->setCurrentIndex( int( std::round( value ) ) );
}

void UiNoiseGenerator::setWhiteNoiseMixValue( double value ) {
  sliderWhiteNoiseMix_->setValue( int( abSliderMinValue_ + std::round( double( abSliderMaxValue_ - abSliderMinValue_ ) * value ) ) );
}

void UiNoiseGenerator::setPinkNoiseTypeValue( double value ) {
  comboBoxPinkNoiseType_->setCurrentIndex( int( std::round( value ) ) );
}

void UiNoiseGenerator::setPinkNoiseMixValue( double value ) {
  sliderPinkNoiseMix_->setValue( int( abSliderMinValue_ + std::round( double( abSliderMaxValue_ - abSliderMinValue_ ) * value ) ) );
}

void UiNoiseGenerator::setRedNoiseTypeValue( double value ) {
  comboBoxRedNoiseType_->setCurrentIndex( int( std::round( value ) ) );
}

void UiNoiseGenerator::setRedNoiseMixValue( double value ) {
  sliderRedNoiseMix_->setValue( int( abSliderMinValue_ + std::round( double( abSliderMaxValue_ - abSliderMinValue_ ) * value ) ) );
}

void UiNoiseGenerator::setBlueNoiseTypeValue( double value ) {
  comboBoxBlueNoiseType_->setCurrentIndex( int( std::round( value ) ) );
}

void UiNoiseGenerator::setBlueNoiseMixValue( double value ) {
  sliderBlueNoiseMix_->setValue( int( abSliderMinValue_ + std::round( double( abSliderMaxValue_ - abSliderMinValue_ ) * value ) ) );
}

void UiNoiseGenerator::setVioletNoiseTypeValue( double value ) {
  comboBoxVioletNoiseType_->setCurrentIndex( int( std::round( value ) ) );
}

void UiNoiseGenerator::setVioletNoiseMixValue( double value ) {
  sliderVioletNoiseMix_->setValue( int( abSliderMinValue_ + std::round( double( abSliderMaxValue_ - abSliderMinValue_ ) * value ) ) );
}

void UiNoiseGenerator::setGreyNoiseTypeValue( double value ) {
  comboBoxGreyNoiseType_->setCurrentIndex( int( std::round( value ) ) );
}

void UiNoiseGenerator::setGreyNoiseMixValue( double value ) {
  sliderGreyNoiseMix_->setValue( int( abSliderMinValue_ + std::round( double( abSliderMaxValue_ - abSliderMinValue_ ) * value ) ) );
}

void UiNoiseGenerator::setVelvetNoiseTypeValue( double value ) {
  comboBoxVelvetNoiseType_->setCurrentIndex( int( std::round( value ) ) );
}

void UiNoiseGenerator::setVelvetNoiseMixValue( double value ) {
  sliderVelvetNoiseMix_->setValue( int( abSliderMinValue_ + std::round( double( abSliderMaxValue_ - abSliderMinValue_ ) * value ) ) );
}

void UiNoiseGenerator::sineWaveTypeNewValue( int value ) {
  double fraction = double( value );
  emit sineWaveTypeAdjusted( fraction );
}

void UiNoiseGenerator::sineWaveMixNewValue( int value ) {
  double fraction = double( value - abSliderMinValue_ ) / double( abSliderMaxValue_ - abSliderMinValue_ );
  emit sineWaveMixAdjusted( fraction );
}

void UiNoiseGenerator::squareWaveTypeNewValue( int value ) {
  double fraction = double( value );
  emit squareWaveTypeAdjusted( fraction );
}

void UiNoiseGenerator::squareWavePwmNewValue( int value ) {
  double fraction = double( value - abSliderMinValue_ ) / double( abSliderMaxValue_ - abSliderMinValue_ );
  emit squareWavePwmAdjusted( fraction );
}

void UiNoiseGenerator::squareWaveMixNewValue( int value ) {
  double fraction = double( value - abSliderMinValue_ ) / double( abSliderMaxValue_ - abSliderMinValue_ );
  emit squareWaveMixAdjusted( fraction );
}

void UiNoiseGenerator::sawWaveTypeNewValue( int value ) {
  double fraction = double( value );
  emit sawWaveTypeAdjusted( fraction );
}

void UiNoiseGenerator::sawWaveMixNewValue( int value ) {
  double fraction = double( value - abSliderMinValue_ ) / double( abSliderMaxValue_ - abSliderMinValue_ );
  emit sawWaveMixAdjusted( fraction );
}

void UiNoiseGenerator::triangleWaveTypeNewValue( int value ) {
  double fraction = double( value );
  emit triangleWaveTypeAdjusted( fraction );
}

void UiNoiseGenerator::triangleWaveMixNewValue( int value ) {
  double fraction = double( value - abSliderMinValue_ ) / double( abSliderMaxValue_ - abSliderMinValue_ );
  emit triangleWaveMixAdjusted( fraction );
}

void UiNoiseGenerator::whiteNoiseTypeNewValue( int value ) {
  double fraction = double( value );
  emit whiteNoiseTypeAdjusted( fraction );
}

void UiNoiseGenerator::whiteNoiseMixNewValue( int value ) {
  double fraction = double( value - abSliderMinValue_ ) / double( abSliderMaxValue_ - abSliderMinValue_ );
  emit whiteNoiseMixAdjusted( fraction );
}

void UiNoiseGenerator::pinkNoiseTypeNewValue( int value ) {
  double fraction = double( value );
  emit pinkNoiseTypeAdjusted( fraction );
}

void UiNoiseGenerator::pinkNoiseMixNewValue( int value ) {
  double fraction = double( value - abSliderMinValue_ ) / double( abSliderMaxValue_ - abSliderMinValue_ );
  emit pinkNoiseMixAdjusted( fraction );
}

void UiNoiseGenerator::redNoiseTypeNewValue( int value ) {
  double fraction = double( value );
  emit redNoiseTypeAdjusted( fraction );
}

void UiNoiseGenerator::redNoiseMixNewValue( int value ) {
  double fraction = double( value - abSliderMinValue_ ) / double( abSliderMaxValue_ - abSliderMinValue_ );
  emit redNoiseMixAdjusted( fraction );
}

void UiNoiseGenerator::blueNoiseTypeNewValue( int value ) {
  double fraction = double( value );
  emit blueNoiseTypeAdjusted( fraction );
}

void UiNoiseGenerator::blueNoiseMixNewValue( int value ) {
  double fraction = double( value - abSliderMinValue_ ) / double( abSliderMaxValue_ - abSliderMinValue_ );
  emit blueNoiseMixAdjusted( fraction );
}

void UiNoiseGenerator::violetNoiseTypeNewValue( int value ) {
  double fraction = double( value );
  emit violetNoiseTypeAdjusted( fraction );
}

void UiNoiseGenerator::violetNoiseMixNewValue( int value ) {
  double fraction = double( value - abSliderMinValue_ ) / double( abSliderMaxValue_ - abSliderMinValue_ );
  emit violetNoiseMixAdjusted( fraction );
}

void UiNoiseGenerator::greyNoiseTypeNewValue( int value ) {
  double fraction = double( value );
  emit greyNoiseTypeAdjusted( fraction );
}

void UiNoiseGenerator::greyNoiseMixNewValue( int value ) {
  double fraction = double( value - abSliderMinValue_ ) / double( abSliderMaxValue_ - abSliderMinValue_ );
  emit greyNoiseMixAdjusted( fraction );
}

void UiNoiseGenerator::velvetNoiseTypeNewValue( int value ) {
  double fraction = double( value );
  emit velvetNoiseTypeAdjusted( fraction );
}

void UiNoiseGenerator::velvetNoiseMixNewValue( int value ) {
  double fraction = double( value - abSliderMinValue_ ) / double( abSliderMaxValue_ - abSliderMinValue_ );
  emit velvetNoiseMixAdjusted( fraction );
}
