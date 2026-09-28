#ifndef GAME_CORE_BODYCONTROL_HPP
#define GAME_CORE_BODYCONTROL_HPP

class LivingEntity;

class BodyControl
{
public:
    BodyControl(LivingEntity* mob);

    virtual ~BodyControl() {}
    virtual void clientTick();

protected:
    float clamp(float base, float value, float maxDifference);

    LivingEntity* mMob;
    int mHeadStableTime;
    float mLastStableYHeadRot;
};

#endif
