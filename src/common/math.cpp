// Header assigned to this source
#include "common/math.hpp"

// C++ std includes
#include <cmath>
#include <stdexcept>

uint64_t sfg_upow( uint64_t a, int64_t b ) {
  b = b - 1;
  if( b <= -2 )
    throw std::range_error( "negatives not yet handled" );
  if( b >= 0 )
    return a << b;
  return a >> std::abs( b );
}

int64_t sfg_ipow( int64_t a, int64_t b ) {
  b = b - 1;
  if( b <= -2 )
    throw std::range_error( "negatives not yet handled" );
  if( b >= 0 )
    return a << b;
  return a >> std::abs( b );
}
