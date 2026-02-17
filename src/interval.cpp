#include "interval.h"
#include <limits>

const Interval Interval::empty = Interval(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
const Interval Interval::universe = Interval(std::numeric_limits<float>::min(), std::numeric_limits<float>::max());