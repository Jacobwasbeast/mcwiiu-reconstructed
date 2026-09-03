#include "game/entity/ai/Goal.hpp"

Goal::Goal()
{
    mRequiredControlFlags = 0;
}

Goal::~Goal()
{
}

bool Goal::canContinueToUse()
{
    return canUse();
}

bool Goal::canInterrupt()
{
    return true;
}

void Goal::start()
{
}

void Goal::stop()
{
}

void Goal::tick()
{
}

void Goal::setRequiredControlFlags(int flags)
{
    mRequiredControlFlags = flags;
}

int Goal::getRequiredControlFlags()
{
    return mRequiredControlFlags;
}

void Goal::setLevel(Level* level)
{
    (void)level;
}

void Goal::toString()
{
    getName();
}
