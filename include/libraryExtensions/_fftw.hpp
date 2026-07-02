#pragma once
#define CLAP_HAS_THREAD

// Project includes
#include "libraryExtensions/logging.hpp"

// Other lib includes
#include <fftw3.h>

// C++ std includes
#include <memory>

std::shared_ptr< fftwf_plan_s > make_fftw_shared_ptr( fftwf_plan s );
