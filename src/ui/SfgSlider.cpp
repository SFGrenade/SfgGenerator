// Header assigned to this source
#include "ui/SfgSlider.hpp"

SfgSlider::SfgSlider( int minVal, int maxVal, Qt::Orientation orientation, QWidget* parent )
    : SfgSlider( minVal, maxVal, ( minVal + maxVal ) / 2, orientation, parent ) {}

SfgSlider::SfgSlider( int minVal, int maxVal, int defaultVal, Qt::Orientation orientation, QWidget* parent )
    : _base_( orientation, parent ), minValue_( minVal ), maxValue_( maxVal ), defaultValue_( defaultVal ) {}

SfgSlider::~SfgSlider() {}

void SfgSlider::mouseDoubleClickEvent( QMouseEvent* ) {
  setValue( defaultValue_ );
}

void SfgSlider::setRange( int min, int max ) {
  setRange( min, max, ( min + max ) / 2 );
}

void SfgSlider::setRange( int min, int max, int def ) {
  _base_::setRange( min, max );
  minValue_ = min;
  maxValue_ = max;
  defaultValue_ = def;
}
