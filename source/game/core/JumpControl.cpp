#include "game/core/JumpControl.hpp"

#include "game/entity/mob/Mob.hpp"

JumpControl::JumpControl(Mob* mob)
{
    JumpControl* obj = this;
    if (obj != NULL || (obj = static_cast<JumpControl*>(operator new(0xc)), obj != NULL))
    {
        obj->mMob = mob;
        obj->mJump = false;
    }
}

void JumpControl::jump()
{
    mJump = true;
}

void JumpControl::tick()
{
    mMob->setJumping(mJump);
    mJump = false;
}
