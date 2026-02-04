// Header assigned to this source
#include "ui/UiAudioLerpEffect.hpp"

// Other lib includes
#include <QSpacerItem>
#include <QWindow>

UiAudioLerpEffect::UiAudioLerpEffect( std::shared_ptr< spdlog::logger > logger, QWidget *parent ) : _base_( parent ), logger_( logger ) {
  setGeometry( 0, 0, 300, 200 );
  setMinimumSize( 1, 1 );

  mainLabel_ = new QLabel( "Audio A<=>B Interpolation", this );
  mainLabel_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  leftLabel_ = new QLabel( "A", this );
  leftLabel_->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  abSlider_ = new SfgSlider( 0, 1 << 30, Qt::Horizontal, this );
  abSlider_->setRange( abSliderMinValue_, abSliderMaxValue_ );
  abSlider_->setValue( abSliderMinValue_ + ( ( abSliderMaxValue_ - abSliderMinValue_ ) / 2 ) );
  rightLabel_ = new QLabel( "B", this );
  rightLabel_->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );

  // construct the main layout
  layout_ = new QGridLayout( this );
  layout_->addItem( new QSpacerItem( 1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding ), 0, 1, 1, 1 );
  layout_->addWidget( mainLabel_, 1, 0, 1, 3 );
  layout_->addWidget( leftLabel_, 2, 0, 1, 1 );
  layout_->addWidget( abSlider_, 2, 1, 1, 1 );
  layout_->addWidget( rightLabel_, 2, 2, 1, 1 );
  layout_->addItem( new QSpacerItem( 1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding ), 3, 1, 1, 1 );

  layout_->setRowStretch( 0, 1 );
  layout_->setRowStretch( 1, 0 );
  layout_->setRowStretch( 2, 0 );
  layout_->setRowStretch( 3, 1 );
  layout_->setColumnStretch( 0, 0 );
  layout_->setColumnStretch( 1, 1 );
  layout_->setColumnStretch( 2, 0 );

  this->setLayout( layout_ );

  connect( abSlider_, &SfgSlider::valueChanged, this, &UiAudioLerpEffect::abSliderNewValue, Qt::QueuedConnection );
  connect( abSlider_, &SfgSlider::sliderMoved, this, &UiAudioLerpEffect::abSliderNewValue, Qt::QueuedConnection );
}

UiAudioLerpEffect::~UiAudioLerpEffect() {}

void UiAudioLerpEffect::setAbValue( double value ) {
  abSlider_->setValue( int( abSliderMinValue_ + ( double( abSliderMaxValue_ - abSliderMinValue_ ) * value ) ) );
}

void UiAudioLerpEffect::abSliderNewValue( int value ) {
  double fraction = double( value - this->abSliderMinValue_ ) / double( this->abSliderMaxValue_ - this->abSliderMinValue_ );
  emit this->abAdjusted( fraction );
}
