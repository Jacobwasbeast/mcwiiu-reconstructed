#include "game/core/ThrownExpBottle.hpp"

#include "game/core/ExperienceOrb.hpp"
#include "game/core/HitResult.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

ThrownExpBottle::ThrownExpBottle(Level* level)
    : ThrowableProjectile(level)
{
    _init();
}

ThrownExpBottle::ThrownExpBottle(Level* level, boost::shared_ptr<LivingEntity> owner)
    : ThrowableProjectile(level, owner)
{
    _init();
}

ThrownExpBottle::ThrownExpBottle(Level* level, double x, double y, double z)
    : ThrowableProjectile(level, x, y, z)
{
    _init();
}

ThrownExpBottle::~ThrownExpBottle()
{
}

void ThrownExpBottle::create(Level* level)
{
    new ThrownExpBottle(level);
}

bool ThrownExpBottle::onHit(const HitResult* result)
{
    Projectile::onHit(result);
    if (!mLevel->mIsClientSide)
    {
        BlockPos pos(this);
        mLevel->levelEvent(2002, pos, PotionUtils::getColor(Potions::EMPTY));
        int experience = mRandom->nextInt(5) + mRandom->nextInt(5) + 3;
        while (experience > 0)
        {
            int value = ExperienceOrb::getExperienceValue(experience);
            experience -= value;
            mLevel->addEntity(boost::shared_ptr<Entity>(new ExperienceOrb(
                mLevel, mX, mY, mZ, value)));
        }
        remove();
    }
    return true;
}

void ThrownExpBottle::addDataWalkers(DataFixerUpper*)
{
}

void ThrownExpBottle::_init()
{
    fjDerivedCtorCalls();
}

double ThrownExpBottle::getGravity()
{
    return 0.07000000029802322;
}

unsigned long long ThrownExpBottle::GetType()
{
    return 0x8A200005ULL;
}
