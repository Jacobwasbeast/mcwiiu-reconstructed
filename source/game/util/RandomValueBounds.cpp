#include "game/util/RandomValueBounds.hpp"

RandomValueBounds::RandomValueBounds()
    : min(0.0f), max(0.0f)
{
}

RandomValueBounds::RandomValueBounds(int min, int max)
    : min((float)min), max((float)max)
{
}

RandomValueBounds::RandomValueBounds(double min, double max)
    : min((float)min), max((float)max)
{
}

RandomValueBounds::RandomValueBounds(int min, double max)
    : min((float)min), max((float)max)
{
}

RandomValueBounds::RandomValueBounds(int value)
    : min((float)value), max((float)value)
{
}
