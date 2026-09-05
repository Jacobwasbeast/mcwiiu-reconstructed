#include "game/entity/mob/InstantenousMobEffect.hpp"

InstantenousMobEffect::InstantenousMobEffect(bool harmful, eMinecraftColour color)
    : MobEffect(harmful, color)
{
}

InstantenousMobEffect::~InstantenousMobEffect()
{
}

bool InstantenousMobEffect::isDurationEffectTick(int duration, int)
{
    return duration > 0;
}

bool InstantenousMobEffect::isInstantenous()
{
    return true;
}
