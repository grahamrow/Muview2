// This part is originally created and released into the public
// domain by Gunnar Selke <gselke@physnet.uni-hamburg.de>.
// Modified by Graham Rowlands <grahamrow@gmail.com>

#ifndef CONTAINER_H
#define CONTAINER_H

#include <vector>
#include "boost/multi_array.hpp"
#include "boost/smart_ptr.hpp"
// #include <boost/algorithm/minmax.hpp>
// #include "OMFHeader.h"

typedef boost::multi_array<float, 4> array_type; // 4 dimensions
typedef boost::shared_ptr<array_type> array_ptr;
// typedef boost::shared_ptr<OMFHeader> header_ptr;

#endif
