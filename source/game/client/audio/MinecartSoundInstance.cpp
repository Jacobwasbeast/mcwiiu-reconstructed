#include "game/client/audio/MinecartSoundInstance.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Mth.hpp"

MinecartSoundInstance::MinecartSoundInstance(boost::shared_ptr<Minecart> minecart)
    : AbstractTickableSoundInstance(SoundEvents::MINECART_RIDING, SoundSource::eSoundSource_Neutral)
    , minecart(minecart)
    , volumeAdjustment(0.0f)
{
    this->looping = true;
    this->volume = 0.0f;
    this->tick();
}

MinecartSoundInstance::MinecartSoundInstance(const MinecartSoundInstance& other)
    : AbstractTickableSoundInstance(other)
    , minecart(other.minecart)
    , volumeAdjustment(other.volumeAdjustment)
{
}

MinecartSoundInstance::~MinecartSoundInstance()
{
}

SoundInstance* MinecartSoundInstance::getDRCClone() const
{
    MinecartSoundInstance* clone = new MinecartSoundInstance(*this);
    clone->drcClone = true;
    return clone;
}

void MinecartSoundInstance::tick()
{
    const float baseVolume = 0.0f;

    if (GameSettings::GetGameSettings((eGameSetting)0x23) == 0)
    {
        this->volume = baseVolume;
        return;
    }

    if (this->minecart->mRemoved)
    {
        this->stopped = true;
        return;
    }

    this->x = static_cast<float>(this->minecart->mX);
    this->y = static_cast<float>(this->minecart->mY);
    this->z = static_cast<float>(this->minecart->mZ);

    const double speed = std::sqrt(this->minecart->mXd * this->minecart->mXd +
                                   this->minecart->mZd * this->minecart->mZd);
    if (speed >= 0.01)
    {
        this->volumeAdjustment = Mth::clamp(this->volumeAdjustment + 0.0025f, 0.0f, 1.0f);
        this->volume = static_cast<float>(Mth::clamp(speed, 0.0, 1.75) * 0.7);
    }
    else
    {
        this->volumeAdjustment = 0.0f;
        this->volume = baseVolume;
    }
}
