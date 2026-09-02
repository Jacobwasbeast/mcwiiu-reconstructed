#include "game/core/SmoothFloat.hpp"

SmoothFloat::SmoothFloat()
{
    targetValue = 0.0f;
    lastAmount = 0.0f;
    remainingValue = 0.0f;
}

float SmoothFloat::getNewDeltaValue(float amount, float scale)
{
    targetValue += amount;
    float delta = (targetValue - remainingValue) * scale;
    float smoothedDelta = lastAmount + (delta - lastAmount) * 0.5f;
    lastAmount = smoothedDelta;
    if ((delta > 0.0f && smoothedDelta < delta) || (delta < 0.0f && smoothedDelta > delta))
    {
        delta = smoothedDelta;
    }
    remainingValue += delta;
    return delta;
}
