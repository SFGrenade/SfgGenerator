#pragma once
#define CLAP_HAS_THREAD

// Project includes
#include "common/_fmt.hpp"

// Other lib includes
#include <fftw3.h>

// C++ std includes
#include <memory>
#include <utility>

std::shared_ptr< fftwf_plan_s > make_fftw_shared_ptr( fftwf_plan s );

typedef std::pair< double, double > SplinePoint;
SplinePoint catmullRom( SplinePoint const& p0, SplinePoint const& p1, SplinePoint const& p2, SplinePoint const& p3, double t );
