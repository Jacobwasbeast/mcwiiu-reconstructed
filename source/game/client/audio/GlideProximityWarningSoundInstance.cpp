#include "game/client/audio/GlideProximityWarningSoundInstance.hpp"

#include "game/entity/mob/LivingEntity.hpp"

GlideProximityWarningSoundInstance::GlideProximityWarningSoundInstance(boost::shared_ptr<LocalPlayer> player,
                                                                       const SoundEvent* sound)
    : AbstractTickableSoundInstance(sound, SoundSource::eSoundSource_Players)
    , player(player)
    , ticks(0)
{
    this->volume = 1.0f;
    this->looping = true;
}

GlideProximityWarningSoundInstance::GlideProximityWarningSoundInstance(
    const GlideProximityWarningSoundInstance& other)
    : AbstractTickableSoundInstance(other)
    , player(other.player)
    , ticks(other.ticks)
{
}

GlideProximityWarningSoundInstance::~GlideProximityWarningSoundInstance()
{
}

SoundInstance* GlideProximityWarningSoundInstance::getDRCClone() const
{
    GlideProximityWarningSoundInstance* clone = new GlideProximityWarningSoundInstance(*this);
    clone->drcClone = true;
    return clone;
}

void GlideProximityWarningSoundInstance::tick()
{
    LivingEntity* livingEntity = (LivingEntity*)this->player.get();
    ++this->ticks;

    if (livingEntity->mRemoved)
    {
        this->stopped = true;
        return;
    }

    if (this->ticks > 20 && !livingEntity->isFallFlying())
    {
        this->stopped = true;
        return;
    }

    this->x = static_cast<float>(livingEntity->mX);
    this->y = static_cast<float>(livingEntity->mY);
    this->z = static_cast<float>(livingEntity->mZ);

    if (livingEntity->getElytraProximityWarning() && livingEntity->getElytraCollisionDistance() < 2.0)
    {
        this->volume = 0.8f;
    }
}
