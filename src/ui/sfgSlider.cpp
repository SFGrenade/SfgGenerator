// Header assigned to this source
#include "ui/sfgSlider.hpp"

SfgSlider::SfgSlider( int minVal, int maxVal, Qt::Orientation orientation, QWidget* parent )
    : _base_( orientation, parent ), minValue_( minVal ), maxValue_( maxVal ) {}

SfgSlider::~SfgSlider() {}

void SfgSlider::mouseDoubleClickEvent( QMouseEvent* ) {
  setValue( minValue_ + ( ( maxValue_ - minValue_ ) / 2 ) );
}

void SfgSlider::setRange( int min, int max ) {
  _base_::setRange( min, max );
  minValue_ = min;
  maxValue_ = max;
}
