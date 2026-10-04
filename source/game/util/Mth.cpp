#include "game/util/Mth.hpp"

double fclamp(float value, float min, float max)
{
    if (value < min)
    {
        return min;
    }

    return (value - max < 0.0f) ? value : max;
}
