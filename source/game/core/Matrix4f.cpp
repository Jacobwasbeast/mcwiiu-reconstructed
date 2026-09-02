#include "game/core/Matrix4f.hpp"

#include "game/core/Mth.hpp"

Matrix4f::Matrix4f()
{
    m[0] = 0.0f;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 0.0f;
    m[5] = 0.0f;
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = 0.0f;
    m[11] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 0.0f;
}

void Matrix4f::setZYXRot(float z, float y, float x)
{
    float sinZ = 0.0f;
    float cosZ = 1.0f;
    float sinY = 0.0f;
    float cosY = 1.0f;
    float sinX = 0.0f;
    float cosX = 1.0f;

    if (z != 0.0f)
    {
        sinZ = Mth::sin(z);
        cosZ = Mth::cos(z);
    }

    if (y != 0.0f)
    {
        sinY = Mth::sin(y);
        cosY = Mth::cos(y);
    }

    if (x != 0.0f)
    {
        sinX = Mth::sin(x);
        cosX = Mth::cos(x);
    }

    m[0] = cosZ * cosY;
    m[1] = sinZ * cosY;
    m[2] = -sinY;
    m[3] = 0.0f;
    m[4] = cosZ * sinY * sinX - sinZ * cosX;
    m[5] = sinZ * sinY * sinX + cosZ * cosX;
    m[6] = cosY * sinX;
    m[7] = 0.0f;
    m[8] = cosZ * sinY * cosX + sinZ * sinX;
    m[9] = sinZ * sinY * cosX - cosZ * sinX;
    m[10] = cosY * cosX;
    m[11] = 0.0f;
}
