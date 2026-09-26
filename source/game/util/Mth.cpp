#include "game/util/Mth.hpp"

double fclamp(float value, float min, float max)
{
    if (value < min)
    {
        return min;
    }

    if (value - max < 0.0f)
    {
        max = value;
    }

    return max;
}
