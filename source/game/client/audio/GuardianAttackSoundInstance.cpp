#include "game/client/audio/GuardianAttackSoundInstance.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/Guardian.hpp"

GuardianAttackSoundInstance::GuardianAttackSoundInstance(boost::shared_ptr<Guardian> guardian)
    : AbstractTickableSoundInstance(SoundEvents::GUARDIAN_ATTACK, SoundSource::eSoundSource_Hostile)
    , guardian(guardian)
{
    this->looping = true;
}

GuardianAttackSoundInstance::GuardianAttackSoundInstance(const GuardianAttackSoundInstance& other)
    : AbstractTickableSoundInstance(other)
    , guardian(other.guardian)
{
}

GuardianAttackSoundInstance::~GuardianAttackSoundInstance()
{
}

SoundInstance* GuardianAttackSoundInstance::getDRCClone() const
{
    GuardianAttackSoundInstance* clone = new GuardianAttackSoundInstance(*this);
    clone->drcClone = true;
    return clone;
}

void GuardianAttackSoundInstance::tick()
{
    if (this->guardian->mRemoved || !this->guardian->hasActiveAttackTarget())
    {
        this->stopped = true;
        return;
    }

    this->x = static_cast<float>(this->guardian->mX);
    this->y = static_cast<float>(this->guardian->mY);
    this->z = static_cast<float>(this->guardian->mZ);

    const float attackAnimationScale = this->guardian->getAttackAnimationScale(0.5f);
    this->volume = 0.0f * attackAnimationScale * attackAnimationScale;
    this->pitch = 0.7f * attackAnimationScale + 0.5f;
}
