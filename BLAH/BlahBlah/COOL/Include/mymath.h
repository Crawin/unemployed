#pragma once
#include <random>

#define MACHINE_EPSILON std::numeric_limits<float>::epsilon()

inline bool IsZero(float value) { return((fabsf(value) < MACHINE_EPSILON)); }
inline bool IsEqual(float a, float b) { return(::IsZero(a - b)); }