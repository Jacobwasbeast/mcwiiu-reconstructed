#include "game/core/BodyControl.hpp"

#include <vector>

#include <boost/weak_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"

const unsigned long long ENTITY_TYPE_WITHER_BOSS = 0x000000005D540047ULL;

const float MIN_MOVEMENT_DISTANCE_SQR = 2.5000003e-07f;
const float MAX_Y_ROT_FROM_BODY = 75.0f;
const float MAX_Y_ROT_FROM_BODY_WITHER = 360.0f;
const float MAX_STABLE_HEAD_ROT_DIFFERENCE = 15.0f;
const int HEAD_STABLE_TIME_LIMIT = 10;

BodyControl::BodyControl(LivingEntity* mob)
    : mMob(mob)
    , mHeadStableTime(0)
    , mLastStableYHeadRot(0.0f)
{
}

BodyControl::~BodyControl()
{
}

float BodyControl::clamp(float base, float value, float maxDifference)
{
    float difference = Mth::wrapDegrees(base - value);

    if (difference < -maxDifference)
    {
        difference = -maxDifference;
    }

    if (difference >= maxDifference)
    {
        difference = maxDifference;
    }

    return base - difference;
}

void BodyControl::clientTick()
{
    if (mMob->GetType() == ENTITY_TYPE_WITHER_BOSS
        && static_cast<WitherBoss*>(mMob)->IsChargeAttacking())
    {
        mMob->mYBodyRot = mMob->mYHeadRot;
        return;
    }

    double xd = mMob->mX - mMob->mXo;
    double zd = mMob->mZ - mMob->mZo;
    float maxYRotFromBody = MAX_Y_ROT_FROM_BODY;

    if (xd * xd + zd * zd > MIN_MOVEMENT_DISTANCE_SQR)
    {
        mMob->mYBodyRot = mMob->mYRot;

        if (mMob->GetType() != ENTITY_TYPE_WITHER_BOSS)
        {
            mMob->mYHeadRot = clamp(mMob->mYBodyRot, mMob->mYHeadRot, maxYRotFromBody);
        }

        mHeadStableTime = 0;
        mLastStableYHeadRot = mMob->mYHeadRot;
        return;
    }

    std::vector<boost::weak_ptr<Entity> >* passengers = mMob->getPassengers();

    if (passengers->empty() || mMob->controllingPassengerIsA(ENTITY_TYPE_MOB))
    {
        if (mMob->GetType() == ENTITY_TYPE_WITHER_BOSS)
        {
            maxYRotFromBody = MAX_Y_ROT_FROM_BODY_WITHER;
        }

        float yHeadRot = mMob->mYHeadRot;

        if (Mth::abs(yHeadRot - mLastStableYHeadRot) > MAX_STABLE_HEAD_ROT_DIFFERENCE)
        {
            mLastStableYHeadRot = yHeadRot;
            mHeadStableTime = 0;
        }
        else
        {
            ++mHeadStableTime;

            if (mHeadStableTime > HEAD_STABLE_TIME_LIMIT)
            {
                float stableScale = 1.0f
                    - (float)(mHeadStableTime - HEAD_STABLE_TIME_LIMIT) / 10.0f;

                if (stableScale < 0.0f)
                {
                    stableScale = 0.0f;
                }

                maxYRotFromBody = maxYRotFromBody * stableScale;
            }
        }

        mMob->mYBodyRot = clamp(yHeadRot, mMob->mYBodyRot, maxYRotFromBody);

        if (mMob->GetType() == ENTITY_TYPE_WITHER_BOSS)
        {
            mMob->mYRot = mMob->mYBodyRot;
        }
    }
}
