// Header assigned to this source
#include "ui/SfgDbfsDisplay.hpp"

// Other lib includes
#include <QPainter>
#include <QSizePolicy>

// C++ std inlcudes
#include <cmath>

SfgDbfsDisplay::SfgDbfsDisplay( Qt::Orientation orientation, QWidget* parent ) : _base_( parent ), orientation_( orientation ) {
  setSizePolicy( QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::MinimumExpanding );
}

SfgDbfsDisplay::~SfgDbfsDisplay() {}

void SfgDbfsDisplay::setDBFS( double dBFS ) {
  dBFS_ = std::clamp( dBFS, MIN_DBFS_, MAX_DBFS_ );
  update();
}

void SfgDbfsDisplay::setValue( double value ) {
  setDBFS( 20.0 * std::log10( value ) );
}

void SfgDbfsDisplay::paintEvent( QPaintEvent* /*event*/ ) {
  // _base_::paintEvent( event );

  double normalizedDBFS = ( dBFS_ - MIN_DBFS_ ) / ( MAX_DBFS_ - MIN_DBFS_ );

  QColor bg( 0, 0, 0, 255 );
  QColor bar( 255, 255, 255, 255 );

  QPainter painter( this );
  painter.setRenderHint( QPainter::Antialiasing );

  // painter.save();
  QRectF bgRect = rect();
  painter.fillRect( bgRect, bg );
  bgRect.setWidth( width() * normalizedDBFS );
  painter.fillRect( bgRect, bar );
  // painter.restore();
}
