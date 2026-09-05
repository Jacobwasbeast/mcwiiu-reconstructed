#include "game/client/audio/RidingMinecartSoundInstance.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"

RidingMinecartSoundInstance::RidingMinecartSoundInstance(boost::shared_ptr<Player> player,
                                                         boost::shared_ptr<Minecart> minecart)
    : AbstractTickableSoundInstance(SoundEvents::MINECART_RIDING, SoundSource::eSoundSource_Neutral)
    , player(player)
    , minecart(minecart)
{
    this->looping = true;
}

RidingMinecartSoundInstance::RidingMinecartSoundInstance(const RidingMinecartSoundInstance& other)
    : AbstractTickableSoundInstance(other)
    , player(other.player)
    , minecart(other.minecart)
{
}

RidingMinecartSoundInstance::~RidingMinecartSoundInstance()
{
}

SoundInstance* RidingMinecartSoundInstance::getDRCClone() const
{
    RidingMinecartSoundInstance* clone = new RidingMinecartSoundInstance(*this);
    clone->drcClone = true;
    return clone;
}

void RidingMinecartSoundInstance::tick()
{
    const float baseVolume = 0.0f;

    if (GameSettings::GetGameSettings((eGameSetting)0x23) != 0)
    {
        Entity* ridingPlayer = (Entity*)this->player.get();
        if (!this->minecart->mRemoved
            && ridingPlayer->isPassenger()
            && ridingPlayer->getVehicle().get() == this->minecart.get())
        {
            this->x = static_cast<float>(this->minecart->mX);
            this->y = static_cast<float>(this->minecart->mY);
            this->z = static_cast<float>(this->minecart->mZ);

            const double speed = Mth::sqrt(this->minecart->mXd * this->minecart->mXd
                                           + this->minecart->mZd * this->minecart->mZd);
            if (speed >= 0.01)
            {
                const float clampedSpeed = Mth::clamp(static_cast<float>(speed), 0.0f, 1.0f);
                this->volume = static_cast<float>(clampedSpeed * 0.75f);
                return;
            }
        }
        else
        {
            this->stopped = true;
            return;
        }
    }

    this->volume = baseVolume;
}
