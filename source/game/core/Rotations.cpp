#include "game/core/Rotations.hpp"

Rotations::Rotations(float x, float y, float z)
    : x(x)
    , y(y)
    , z(z)
{
}

float Rotations::getX() const
{
    return x;
}

float Rotations::getY() const
{
    return y;
}

float Rotations::getZ() const
{
    return z;
}
