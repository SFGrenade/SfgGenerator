// Header assigned to this source
#include "common/_fftw.hpp"

// C++ std includes
#include <cmath>

std::shared_ptr< fftwf_plan_s > make_fftw_shared_ptr( fftwf_plan s ) {
  return std::shared_ptr< fftwf_plan_s >( s, []( fftwf_plan p ) { fftwf_destroy_plan( p ); } );
}

SplinePoint catmullRom( SplinePoint const& p0, SplinePoint const& p1, SplinePoint const& p2, SplinePoint const& p3, double t ) {
  double t2 = t * t;
  double t3 = t2 * t;

  return { 0.5
               * ( ( 2.0 * p1.first ) + ( -p0.first + p2.first ) * t + ( 2.0 * p0.first - 5.0 * p1.first + 4.0 * p2.first - p3.first ) * t2
                   + ( -p0.first + 3.0 * p1.first - 3.0 * p2.first + p3.first ) * t3 ),
           0.5
               * ( ( 2.0 * p1.second ) + ( -p0.second + p2.second ) * t + ( 2.0 * p0.second - 5.0 * p1.second + 4.0 * p2.second - p3.second ) * t2
                   + ( -p0.second + 3.0 * p1.second - 3.0 * p2.second + p3.second ) * t3 ) };
}
