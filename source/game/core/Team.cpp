#include "game/core/Team.hpp"

Team::~Team()
{
}

bool Team::isAlliedTo(Team* other)
{
    if (other != NULL && this == other)
    {
        return true;
    }

    return false;
}
