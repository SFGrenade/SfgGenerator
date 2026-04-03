// Header assigned to this source
#include "ui/UiAudioAnalysis.hpp"

// Other lib includes
#include <QSpacerItem>
#include <QWindow>

// C++ std includes
#include <numeric>

UiAudioAnalysis::UiAudioAnalysis( std::shared_ptr< spdlog::logger > logger, QWidget* parent ) : _base_( parent ), logger_( logger ) {
  setGeometry( 0, 0, 300, 200 );
  setMinimumSize( 1, 1 );

  connect( this, &UiAudioAnalysis::emitRmsMomentary, this, &UiAudioAnalysis::rmsMomentaryReceived, Qt::QueuedConnection );
  connect( this, &UiAudioAnalysis::emitLufsMomentary, this, &UiAudioAnalysis::lufsMomentaryReceived, Qt::QueuedConnection );

  momentaryRmsLabel_ = new QLabel( "Momentary RMS", this );
  momentaryRmsLabel_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  momentaryRmsSlider_ = new SfgSlider( 0, 1 << 30, Qt::Horizontal, this );
  momentaryRmsSlider_->setRange( sliderMinValue_, sliderMaxValue_ );
  momentaryRmsSlider_->setValue( sliderMinValue_ );
  momentaryRmsSlider_->setEnabled( false );
  momentaryLufsLabel_ = new QLabel( "Momentary LUFS", this );
  momentaryLufsLabel_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  momentaryLufsSlider_ = new SfgSlider( 0, 1 << 30, Qt::Horizontal, this );
  momentaryLufsSlider_->setRange( sliderMinValue_, sliderMaxValue_ );
  momentaryLufsSlider_->setValue( sliderMinValue_ );
  momentaryLufsSlider_->setEnabled( false );

  // construct the main layout
  layout_ = new QGridLayout( this );
  layout_->addWidget( momentaryRmsLabel_, 0, 0, 1, 1 );
  layout_->addWidget( momentaryRmsSlider_, 0, 1, 1, 1 );
  layout_->addWidget( momentaryLufsLabel_, 1, 0, 1, 1 );
  layout_->addWidget( momentaryLufsSlider_, 1, 1, 1, 1 );

  layout_->setRowStretch( 0, 0 );
  layout_->setRowStretch( 1, 0 );
  layout_->setColumnStretch( 0, 0 );
  layout_->setColumnStretch( 1, 1 );

  setLayout( layout_ );

  // connect( timeWindowSlider_, &SfgSlider::valueChanged, this, &UiAudioAnalysis::timeWindowSliderNewValue, Qt::QueuedConnection );
  // connect( timeWindowSlider_, &SfgSlider::sliderMoved, this, &UiAudioAnalysis::timeWindowSliderNewValue, Qt::QueuedConnection );
}

UiAudioAnalysis::~UiAudioAnalysis() {}

// called from the audio thread, so we push it with a queued connection to get it into the actual QT thread
void UiAudioAnalysis::pushRmsMomentary( double value ) {
  // logger_->trace( "[{:s}] [{:p}] enter( value={:f} )", __FUNCTION__, static_cast< void* >( this ), value );
  Q_EMIT emitRmsMomentary( value );
}
void UiAudioAnalysis::pushLufsMomentary( double value ) {
  // logger_->trace( "[{:s}] [{:p}] enter( value={:f} )", __FUNCTION__, static_cast< void* >( this ), value );
  Q_EMIT emitLufsMomentary( value );
}
// QT thread, hopefully
void UiAudioAnalysis::rmsMomentaryReceived( double value ) {
  // logger_->trace( "[{:s}] [{:p}] enter( value={:f} )", __FUNCTION__, static_cast< void* >( this ), value );
  momentaryRmsSlider_->setValue( sliderMinValue_ + int( double( sliderMaxValue_ - sliderMinValue_ ) * value ) );
}
void UiAudioAnalysis::lufsMomentaryReceived( double value ) {
  // logger_->trace( "[{:s}] [{:p}] enter( value={:f} )", __FUNCTION__, static_cast< void* >( this ), value );
  momentaryLufsSlider_->setValue( sliderMinValue_ + int( double( sliderMaxValue_ - sliderMinValue_ ) * value ) );
}

// void UiAudioAnalysis::setTimeWindowValue( double value ) {
//   timeWindowSlider_->setValue( int( sliderMinValue_ + std::round( double( sliderMaxValue_ - sliderMinValue_ ) * value ) ) );
// }

// void UiAudioAnalysis::timeWindowSliderNewValue( int value ) {
//   double fraction = double( value - sliderMinValue_ ) / double( sliderMaxValue_ - sliderMinValue_ );
//   emit timeWindowAdjusted( fraction );
// }
