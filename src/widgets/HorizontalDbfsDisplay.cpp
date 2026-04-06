// Header assigned to this source
#include "widgets/HorizontalDbfsDisplay.hpp"

HorizontalDbfsDisplay::HorizontalDbfsDisplay( std::string const& text, SDL_FRect position ) : _base_( position ), textDisplay_( text ) {}

HorizontalDbfsDisplay::~HorizontalDbfsDisplay() {}

void HorizontalDbfsDisplay::InitUi( std::shared_ptr< Widget > parent ) {
  _base_::InitUi( parent );
  std::shared_ptr< Widget > self = shared_from_this();

  textLabel_ = std::make_shared< Label >( textDisplay_, SDL_FRect{ 0.0f, 0.0f, 0.125f, 0.5f } );
  valueLabel_ = std::make_shared< Label >( "Silence", SDL_FRect{ 0.0f, 0.5f, 0.125f, 0.5f } );
  for( std::shared_ptr< Label > tmp : { textLabel_, valueLabel_ } ) {
    tmp->InitUi( self );
    tmp->SetHorizontalAlignment( Label::HorizontalAlignment::Centered );
    tmp->SetFontFile( ClapGlobals::PLUGIN_PATH.parent_path() / "SfgGenerator" / "fonts" / "NotoSerif-Regular.ttf" );
    tmp->SetFontSize( 12 );
    tmp->SetFontColourActive( SDL_Color{ 0x40, 0xC0, 0xff, 0xff } );
    tmp->SetFontColourInactive( SDL_Color{ 0x40, 0xC0, 0xff, 0x80 } );
    tmp->SetPadding( 0.0f );
  }
  textLabel_->SetVerticalAlignment( Label::VerticalAlignment::Bottom );
  valueLabel_->SetVerticalAlignment( Label::VerticalAlignment::Top );

  barDisplay_ = std::make_shared< Bar >( SDL_FRect{ 0.125f, 0.0f, 0.875f, 1.0f } );
  barDisplay_->InitUi( self );
  barDisplay_->SetPadding( 0.0f );
}

void HorizontalDbfsDisplay::OnRender( std::shared_ptr< SDL_Renderer > renderer ) {
  _base_::OnRender( renderer );
  if( !IsVisibleHierarchy() ) {
    return;
  }
}

float HorizontalDbfsDisplay::GetValue() {
  return std::pow( 10.0f, GetDbfs() / 20.0f );
}

void HorizontalDbfsDisplay::SetValue( float value ) {
  SetDbfs( 20.0f * std::log10( value ) );
}

float HorizontalDbfsDisplay::GetDbfs() {
  return dBFS_;
}

void HorizontalDbfsDisplay::SetDbfs( float dBFS ) {
  dBFS_ = std::clamp( dBFS, MIN_DBFS_, MAX_DBFS_ );
  if( dBFS_ <= MIN_DBFS_ ) {
    valueLabel_->SetText( "Silence" );
  } else {
    valueLabel_->SetText( fmt::format( "{:.2f} {:s}", dBFS_, unit_ ) );
  }
  barDisplay_->SetValue( ( dBFS_ - MIN_DBFS_ ) / ( MAX_DBFS_ - MIN_DBFS_ ) );
}

std::string HorizontalDbfsDisplay::GetUnit() {
  return unit_;
}

void HorizontalDbfsDisplay::SetUnit( std::string const& value ) {
  unit_ = value;
  if( dBFS_ <= MIN_DBFS_ ) {
    valueLabel_->SetText( "Silence" );
  } else {
    valueLabel_->SetText( fmt::format( "{:.2f} {:s}", dBFS_, unit_ ) );
  }
}
