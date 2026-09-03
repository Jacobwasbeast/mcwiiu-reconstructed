#include "game/core/BorderStatus.hpp"

BorderStatus::BorderStatus(int color)
    : color(color)
{
}

BorderStatus::~BorderStatus()
{
}

int BorderStatus::getColor()
{
    return color;
}
