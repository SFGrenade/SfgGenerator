#pragma once

// C++ std includes
#include <utility>

typedef std::pair< double, double > SplinePoint;
SplinePoint catmullRom( SplinePoint const& p0, SplinePoint const& p1, SplinePoint const& p2, SplinePoint const& p3, double t );
