// Header assigned to this source
#include "libraryExtensions/_fftw.hpp"

// C++ std includes
#include <cmath>

std::shared_ptr< fftwf_plan_s > make_fftw_shared_ptr( fftwf_plan s ) {
  return std::shared_ptr< fftwf_plan_s >( s, []( fftwf_plan p ) { fftwf_destroy_plan( p ); } );
}
