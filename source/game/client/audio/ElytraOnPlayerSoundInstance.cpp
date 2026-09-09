#include "game/client/audio/ElytraOnPlayerSoundInstance.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

ElytraOnPlayerSoundInstance::ElytraOnPlayerSoundInstance(boost::shared_ptr<Player> player)
    : AbstractTickableSoundInstance(SoundEvents::ELYTRA_FLYING, SoundSource::eSoundSource_Players)
    , ticks(0)
    , player(player)
    , warningSoundDelay(0)
    , warning(false)
{
    this->volume = 1.0f;
    this->looping = true;
    this->delay = 0;
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
    ++this->ticks;

    if (this->player->mRemoved)
    {
        this->stopped = true;
        return;
    }

    if (this->ticks > 20 && !this->player->isFallFlying())
    {
        this->stopped = true;
        return;
    }

    this->x = static_cast<float>(this->player->mX);
    this->y = static_cast<float>(this->player->mY);
    this->z = static_cast<float>(this->player->mZ);

    float speed = 0.0f;
    if (!this->player->PositionLocked())
    {
        speed = Mth::sqrt(this->player->mXd * this->player->mXd +
                          this->player->mYd * this->player->mYd +
                          this->player->mZd * this->player->mZd);
    }

    float scaledSpeed = static_cast<float>(speed * 0.5);

    if (this->player->getElytraProximityWarning())
        this->warning = true;

    if (this->warning)
    {
        float warningScale = static_cast<float>(this->warningSoundDelay / 10.0) + 1.0f;
        scaledSpeed = scaledSpeed * warningScale;
        this->warningSoundDelay = this->warningSoundDelay < 10 ? this->warningSoundDelay + 1 : 10;
        if (!this->player->getElytraProximityWarning() && warningScale == 2.0f)
            this->warning = false;
    }
    else if (this->warningSoundDelay > 0)
    {
        scaledSpeed = scaledSpeed * (static_cast<float>(this->warningSoundDelay / 10.0) + 1.0f);
        --this->warningSoundDelay;
    }

    if (speed >= 0.01)
        this->volume = Mth::clamp(scaledSpeed * scaledSpeed, 0.0f, 0.8f);
    else
        this->volume = 0.0f;

    if (this->ticks < 20)
        this->volume = 0.0f;
    else if (this->ticks < 40)
        this->volume = this->volume * static_cast<float>((this->ticks - 20) / 20.0);

    if (this->volume > 0.8f)
        this->pitch = 1.0f + (this->volume - 0.8f);
    else
        this->pitch = 1.0f;

    if (this->player->GetType() == ENTITY_TYPE_LOCAL_PLAYER)
    {
        boost::shared_ptr<LocalPlayer> localPlayer = boost::static_pointer_cast<LocalPlayer>(this->player);
        if (Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
        {
            if (gConsoleUIController.IsSceneInStack(localPlayer->mUserIndex, static_cast<EUIScene>(5)))
                this->volume = 0.0f;
        }
    }
}
