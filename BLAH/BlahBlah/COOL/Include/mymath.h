#pragma once
#include <random>

#define MACHINE_EPSILON std::numeric_limits<float>::epsilon()

inline bool IsZero(float value) { return((fabsf(value) < 0.000001f)); }
inline bool IsEqual(float a, float b) { return(::IsZero(a - b)); }


XMFLOAT3 LerpValue(const XMFLOAT3 a, const XMFLOAT3 b, float factor);
XMFLOAT4 LerpValue(const XMFLOAT4 a, const XMFLOAT4 b, float factor);
float LerpValue(float a, float b, float factor);
