#include "game/client/audio/ElytraOnPlayerSoundInstance.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/LivingEntity.hpp"

ElytraOnPlayerSoundInstance::ElytraOnPlayerSoundInstance(boost::shared_ptr<Player> player)
    : AbstractTickableSoundInstance(SoundEvents::ELYTRA_FLYING, SoundSource::eSoundSource_Players)
    , ticks(0)
    , player(player)
    , warningSoundDelay(0)
    , warning(false)
{
    this->volume = 1.0f;
    this->looping = true;
}

ElytraOnPlayerSoundInstance::ElytraOnPlayerSoundInstance(const ElytraOnPlayerSoundInstance& other)
    : AbstractTickableSoundInstance(other)
    , ticks(other.ticks)
    , player(other.player)
    , warningSoundDelay(other.warningSoundDelay)
    , warning(other.warning)
{
}

ElytraOnPlayerSoundInstance::~ElytraOnPlayerSoundInstance()
{
}

SoundInstance* ElytraOnPlayerSoundInstance::getDRCClone() const
{
    ElytraOnPlayerSoundInstance* clone = new ElytraOnPlayerSoundInstance(*this);
    clone->drcClone = true;
    return clone;
}

void ElytraOnPlayerSoundInstance::tick()
{
    LivingEntity* entity = (LivingEntity*)this->player.get();
    ++this->ticks;

    if (entity->mRemoved)
    {
        this->stopped = true;
        return;
    }

    if (this->ticks > 20 && !entity->isFallFlying())
    {
        this->stopped = true;
        return;
    }

    this->x = static_cast<float>(entity->mX);
    this->y = static_cast<float>(entity->mY);
    this->z = static_cast<float>(entity->mZ);

    float speed = static_cast<float>(Mth::sqrt(entity->mXd * entity->mXd +
                                               entity->mYd * entity->mYd +
                                               entity->mZd * entity->mZd));
    speed *= 0.5f;

    if (entity->getElytraProximityWarning())
    {
        this->warning = true;
        speed *= static_cast<float>(this->warningSoundDelay) / 10.0f + 0.8f;
        this->warningSoundDelay = this->warningSoundDelay < 10 ? this->warningSoundDelay + 1 : 10;
    }
    else if (this->warningSoundDelay > 0)
    {
        speed *= static_cast<float>(this->warningSoundDelay) / 10.0f + 0.8f;
        --this->warningSoundDelay;
    }

    if (speed > 0.01f)
        this->volume = Mth::clamp(speed * speed, 0.5f, 1.03499997f);
    else
        this->volume = 0.5f;
    this->pitch = 0.8f;
}
