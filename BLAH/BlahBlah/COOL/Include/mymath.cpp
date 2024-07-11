#include "framework.h"

XMFLOAT3 LerpValue(const XMFLOAT3 a, const XMFLOAT3 b, float factor)
{
    XMFLOAT3 result;
    result.x = a.x + (b.x - a.x) * factor;
    result.y = a.y + (b.y - a.y) * factor;
    result.z = a.z + (b.z - a.z) * factor;
    return result;
}

XMFLOAT4 LerpValue(const XMFLOAT4 a, const XMFLOAT4 b, float factor)
{
    XMFLOAT4 result;
    result.x = a.x + (b.x - a.x) * factor;
    result.y = a.y + (b.y - a.y) * factor;
    result.z = a.z + (b.z - a.z) * factor;
    result.w = a.w + (b.w - a.w) * factor;
    return result;
}

float LerpValue(float a, float b, float factor)
{
    float result;
    result = a + (b - a) * factor;
    return result;
}
