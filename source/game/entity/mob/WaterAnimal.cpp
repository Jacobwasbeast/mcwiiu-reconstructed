#include "game/entity/mob/WaterAnimal.hpp"
#include "game/client/audio/SoundEvents.hpp"

#include <algorithm>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/ai/WaterBoundPathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"

WaterAnimal::WaterAnimal(Level* level)
    : PathfinderMob(level)
{
    mWaterValueA = 0.0f;
    mWaterValueB = 0.0f;
}

WaterAnimal::~WaterAnimal()
{
}

bool WaterAnimal::isWaterMob()
{
    return true;
}

bool WaterAnimal::checkSpawnRules()
{
    BlockPos position(getPos(0.0f));
    while (mLevel->getBlockState(position.above())->getMaterial() == Material::WATER)
    {
        MutableBlockPos above(position.above());
        position = above;
    }

    int y = position.y - mRandom->nextInt(1, 5);
    MutableBlockPos spawnPosition(position);
    spawnPosition.setY(std::max(static_cast<int>(getPos(0.0f)->y), y));
    setPos(spawnPosition.x, spawnPosition.y, spawnPosition.z);

    return !mLevel->containsAnysolidBlocking(mBoundingBox)
        && mLevel->containsMaterial(mBoundingBox, Material::WATER);
}

bool WaterAnimal::checkSpawnObstruction()
{
    return mLevel->isUnobstructed(mBoundingBox, shared_from_this());
}

unsigned int WaterAnimal::getAmbientSoundInterval()
{
    return 120;
}

bool WaterAnimal::removeWhenFarAway()
{
    return true;
}

int WaterAnimal::getExperienceReward(boost::shared_ptr<Player>)
{
    return mLevel->mRandom->nextInt(3) + 1;
}

WaterBoundPathNavigation* WaterAnimal::createNavigation(Level* level)
{
    return new WaterBoundPathNavigation(this, level, true, false);
}

const SoundEvent* WaterAnimal::getFlopSound() const
{
    return SoundEvents::FISH_FLOP;
}

void WaterAnimal::aiStep()
{
    Mob::aiStep();
    if (mOnGround && !isInWater())
    {
        mYd += 0.5;
        mXd += (static_cast<double>(mRandom->nextFloat() * 2.0f) - 1.0) * 0.2;
        mZd += (static_cast<double>(mRandom->nextFloat() * 2.0f) - 1.0) * 0.2;
        mOnGround = false;
        mYRot = mRandom->nextFloat() * 360.0f;
    }
}

void WaterAnimal::baseTick()
{
    int airSupply = getAirSupply();
    Mob::baseTick();
    if (!isAlive() || isInWaterOrBubble())
    {
        setAirSupply(300);
    }
    else
    {
        setAirSupply(airSupply - 1);
        if (getAirSupply() == -20)
        {
            setAirSupply(0);
            hurt(DamageSource::DROWN, 2.0f);
        }
    }
}

bool WaterAnimal::isPushedByWater()
{
    return false;
}

bool WaterAnimal::breathesAir()
{
    return false;
}
