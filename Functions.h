// Header for utility functions that are independent of the simulation protocol.

#pragma once

#include <cmath>
#include <cstdlib>

#include "Definitions.h"

template <typename Numerical>
Numerical Truncate(Numerical n, Numerical lower, Numerical upper) {
	if (n < lower) {
		return lower;
	} else if (n > upper) {
		return upper;
	} else {
		return n;
	}
}

// Inclusive of lower and upper bound.
template <typename Numerical>
inline bool IsBetween(Numerical n, Numerical lower, Numerical upper) {
	return n >= lower && n <= upper;
}

inline uint GetRow(uint index, uint column_count) {
	return index / column_count;
}

inline uint GetColumn(uint index, uint column_count) {
	return index % column_count;
}

inline bool Executes(uint percent_probability) {
	return (rand() % 100) < percent_probability;
}