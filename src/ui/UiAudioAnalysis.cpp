// Header assigned to this source
#include "ui/UiAudioAnalysis.hpp"

// Other lib includes
#include <QSpacerItem>
#include <QWindow>

// C++ std includes
#include <numeric>

UiAudioAnalysis::UiAudioAnalysis( std::shared_ptr< spdlog::logger > logger, double sampleRate, QWidget* parent )
    : _base_( parent ), logger_( logger ), sampleRate_( sampleRate ), rmsShortTermValueBuffer_( 30 ), lufsShortTermValueBuffer_( 30 ) {
  setGeometry( 0, 0, 300, 200 );
  setMinimumSize( 1, 1 );

  for( uint32_t c = 0; c < 2; c++ ) {
    rmsMomentaryValueBuffer_[c].set_capacity( sampleRate * 0.4 );
    lufsMomentaryValueBuffer_[c].set_capacity( sampleRate * 0.4 );
    kWeightingFilterHighShelf_[c].setup( sampleRate, 2000.0, 4.0 );
    kWeightingFilterHighPass_[c].setup( sampleRate, 100.0 );
    rmsMomentaryValueBuffer_[c].resize( rmsMomentaryValueBuffer_[c].capacity(), 0.0 );
    lufsMomentaryValueBuffer_[c].resize( lufsMomentaryValueBuffer_[c].capacity(), 0.0 );
  }

  rmsShortTermValueBuffer_.resize( rmsShortTermValueBuffer_.capacity(), 0.0 );
  lufsShortTermValueBuffer_.resize( lufsShortTermValueBuffer_.capacity(), 0.0 );

  // CLAP audio -> QT
  connect( this, &UiAudioAnalysis::emitSample, this, &UiAudioAnalysis::sampleReceived, Qt::QueuedConnection );
  // Passing of values
  connect( this, &UiAudioAnalysis::emitRmsSample, this, &UiAudioAnalysis::rmsSampleReceived, Qt::QueuedConnection );
  connect( this, &UiAudioAnalysis::emitLufsSample, this, &UiAudioAnalysis::lufsSampleReceived, Qt::QueuedConnection );
  connect( this, &UiAudioAnalysis::emitRmsMomentaryValue, this, &UiAudioAnalysis::rmsMomentaryValueReceived, Qt::QueuedConnection );
  connect( this, &UiAudioAnalysis::emitLufsMomentaryValue, this, &UiAudioAnalysis::lufsMomentaryValueReceived, Qt::QueuedConnection );
  connect( this, &UiAudioAnalysis::emitRmsShortTermValue, this, &UiAudioAnalysis::rmsShortTermValueReceived, Qt::QueuedConnection );
  connect( this, &UiAudioAnalysis::emitLufsShortTermValue, this, &UiAudioAnalysis::lufsShortTermValueReceived, Qt::QueuedConnection );

  momentaryRmsLabel_ = new QLabel( "Momentary RMS", this );
  momentaryRmsLabel_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  momentaryRmsValueLabel_ = new QLabel( "Silence", this );
  momentaryRmsValueLabel_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  momentaryRmsSlider_ = new SfgDbfsDisplay( Qt::Horizontal, this );

  shortTermRmsLabel_ = new QLabel( "Short-Term RMS", this );
  shortTermRmsLabel_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  shortTermRmsValueLabel_ = new QLabel( "Silence", this );
  shortTermRmsValueLabel_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  shortTermRmsSlider_ = new SfgDbfsDisplay( Qt::Horizontal, this );

  momentaryLufsLabel_ = new QLabel( "Momentary LUFS", this );
  momentaryLufsLabel_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  momentaryLufsValueLabel_ = new QLabel( "Silence", this );
  momentaryLufsValueLabel_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  momentaryLufsSlider_ = new SfgDbfsDisplay( Qt::Horizontal, this );

  shortTermLufsLabel_ = new QLabel( "Short-Term LUFS", this );
  shortTermLufsLabel_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  shortTermLufsValueLabel_ = new QLabel( "Silence", this );
  shortTermLufsValueLabel_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  shortTermLufsSlider_ = new SfgDbfsDisplay( Qt::Horizontal, this );

  // construct the main layout
  layout_ = new QGridLayout( this );
  layout_->addWidget( momentaryRmsLabel_, 0, 0, 1, 1 );
  layout_->addWidget( momentaryRmsValueLabel_, 1, 0, 1, 1 );
  layout_->addWidget( momentaryRmsSlider_, 0, 1, 2, 1 );
  layout_->addWidget( shortTermRmsLabel_, 2, 0, 1, 1 );
  layout_->addWidget( shortTermRmsValueLabel_, 3, 0, 1, 1 );
  layout_->addWidget( shortTermRmsSlider_, 2, 1, 2, 1 );
  layout_->addWidget( momentaryLufsLabel_, 4, 0, 1, 1 );
  layout_->addWidget( momentaryLufsValueLabel_, 5, 0, 1, 1 );
  layout_->addWidget( momentaryLufsSlider_, 4, 1, 2, 1 );
  layout_->addWidget( shortTermLufsLabel_, 6, 0, 1, 1 );
  layout_->addWidget( shortTermLufsValueLabel_, 7, 0, 1, 1 );
  layout_->addWidget( shortTermLufsSlider_, 6, 1, 2, 1 );

  layout_->setRowStretch( 0, 0 );
  layout_->setRowStretch( 1, 0 );
  layout_->setRowStretch( 2, 0 );
  layout_->setRowStretch( 3, 0 );
  layout_->setRowStretch( 4, 0 );
  layout_->setRowStretch( 5, 0 );
  layout_->setRowStretch( 6, 0 );
  layout_->setRowStretch( 7, 0 );
  layout_->setColumnStretch( 0, 0 );
  layout_->setColumnStretch( 1, 1 );

  setLayout( layout_ );

  // connect( timeWindowSlider_, &SfgSlider::valueChanged, this, &UiAudioAnalysis::timeWindowSliderNewValue, Qt::QueuedConnection );
  // connect( timeWindowSlider_, &SfgSlider::sliderMoved, this, &UiAudioAnalysis::timeWindowSliderNewValue, Qt::QueuedConnection );
}

UiAudioAnalysis::~UiAudioAnalysis() {}

// called from the audio thread, so we push it with a queued connection to get it into the actual QT thread
void UiAudioAnalysis::pushSample( double sample, uint32_t channel ) {
  // logger_->trace( "[{:s}] [{:p}] enter( value={:f} )", __FUNCTION__, static_cast< void* >( this ), value );
  Q_EMIT emitSample( sample, channel );
}
// QT thread, hopefully
void UiAudioAnalysis::sampleReceived( double sample, uint32_t channel ) {
  // logger_->trace( "[{:s}] [{:p}] enter( value={:f} )", __FUNCTION__, static_cast< void* >( this ), value );
  Q_EMIT emitRmsSample( sample, channel );
  Q_EMIT emitLufsSample( kWeightingFilterHighPass_[channel].filter( kWeightingFilterHighShelf_[channel].filter( sample ) ), channel );
}

void UiAudioAnalysis::rmsSampleReceived( double rmsSample, uint32_t channel ) {
  rmsSamplesReceived_[channel]++;
  rmsMomentaryValueBuffer_[channel].push_back( rmsSample );
  if( ( rmsSamplesReceived_[0] >= std::ceil( sampleRate_ * 0.1 ) ) && ( rmsSamplesReceived_[1] >= std::ceil( sampleRate_ * 0.1 ) ) ) {
    rmsSamplesReceived_[0] = 0;
    rmsSamplesReceived_[1] = 0;
    Q_EMIT emitRmsMomentaryValue( ( averageOf( rmsMomentaryValueBuffer_[0] ) + averageOf( rmsMomentaryValueBuffer_[1] ) ) / 2.0 );
  }
}

void UiAudioAnalysis::lufsSampleReceived( double lufsSample, uint32_t channel ) {
  lufsSamplesReceived_[channel]++;
  lufsMomentaryValueBuffer_[channel].push_back( lufsSample );
  if( ( lufsSamplesReceived_[0] >= std::ceil( sampleRate_ * 0.1 ) ) && ( lufsSamplesReceived_[1] >= std::ceil( sampleRate_ * 0.1 ) ) ) {
    lufsSamplesReceived_[0] = 0;
    lufsSamplesReceived_[1] = 0;
    Q_EMIT emitLufsMomentaryValue( ( averageOf( lufsMomentaryValueBuffer_[0] ) + averageOf( lufsMomentaryValueBuffer_[1] ) ) / 2.0 );
  }
}

void UiAudioAnalysis::rmsMomentaryValueReceived( double rmsMomentaryValue ) {
  momentaryRmsValueLabel_->setText( valueToFs( rmsMomentaryValue, "dBFS" ) );
  momentaryRmsSlider_->setValue( rmsMomentaryValue );
  rmsShortTermValueBuffer_.push_back( rmsMomentaryValue );
  Q_EMIT emitRmsShortTermValue( averageOf( rmsShortTermValueBuffer_ ) );
}

void UiAudioAnalysis::lufsMomentaryValueReceived( double lufsMomentaryValue ) {
  momentaryLufsValueLabel_->setText( valueToFs( lufsMomentaryValue, "LUFS" ) );
  momentaryLufsSlider_->setValue( lufsMomentaryValue );
  lufsShortTermValueBuffer_.push_back( lufsMomentaryValue );
  Q_EMIT emitLufsShortTermValue( averageOf( lufsShortTermValueBuffer_ ) );
}

void UiAudioAnalysis::rmsShortTermValueReceived( double rmsShortTermValue ) {
  shortTermRmsValueLabel_->setText( valueToFs( rmsShortTermValue, "dBFS" ) );
  shortTermRmsSlider_->setValue( rmsShortTermValue );
}

void UiAudioAnalysis::lufsShortTermValueReceived( double lufsShortTermValue ) {
  shortTermLufsValueLabel_->setText( valueToFs( lufsShortTermValue, "LUFS" ) );
  shortTermLufsSlider_->setValue( lufsShortTermValue );
}

// void UiAudioAnalysis::setTimeWindowValue( double value ) {
//   timeWindowSlider_->setValue( int( sliderMinValue_ + std::round( double( sliderMaxValue_ - sliderMinValue_ ) * value ) ) );
// }

// void UiAudioAnalysis::timeWindowSliderNewValue( int value ) {
//   double fraction = double( value - sliderMinValue_ ) / double( sliderMaxValue_ - sliderMinValue_ );
//   emit timeWindowAdjusted( fraction );
// }

double UiAudioAnalysis::averageOf( boost::circular_buffer< double > const& buffer ) {
  return std::accumulate( buffer.begin(), buffer.end(), 0.0, []( double a, double b ) { return std::abs( a ) + std::abs( b ); } ) / double( buffer.size() );
}

QString UiAudioAnalysis::valueToFs( double value, std::string const& unit ) {
  double dBFS = 20.0 * std::log10( value );
  if( dBFS <= -70.0 ) {
    return "Silence";
  }
  return QString::fromStdString( fmt::format( "{:.2f} {:s}", dBFS, unit ) );
}
