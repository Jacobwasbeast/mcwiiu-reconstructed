#include "game/core/WitherSkull.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GameRules.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

static const EntityDataAccessor<bool>* sWitherSkullDangerous = NULL;

WitherSkull::WitherSkull(Level* level) : Fireball(level)
{
    fjDerivedCtorCalls();
    setSize(0.3125f, 0.3125f);
}

WitherSkull::WitherSkull(Level* level, boost::shared_ptr<LivingEntity> owner,
                         double xd, double yd, double zd)
    : Fireball(level, owner, xd, yd, zd)
{
    fjDerivedCtorCalls();
    setSize(0.3125f, 0.3125f);
}

WitherSkull::WitherSkull(Level* level, double x, double y, double z,
                         double xd, double yd, double zd)
    : Fireball(level, x, y, z, xd, yd, zd)
{
    fjDerivedCtorCalls();
    setSize(0.3125f, 0.3125f);
}

WitherSkull::~WitherSkull()
{
}

void WitherSkull::create(Level* level)
{
    new WitherSkull(level);
}

void WitherSkull::staticCtor()
{
    sWitherSkullDangerous = SynchedEntityData::defineId(0x8a400002ULL,
                                                         0x8a400000ULL,
                                                         EntityDataSerializers::BOOLEAN);
}

bool WitherSkull::isOnFire()
{
    return false;
}

float WitherSkull::getBlockExplosionResistance(Explosion* explosion, Level* level, const BlockPos& pos)
{
    float resistance = Entity::getBlockExplosionResistance(explosion, level, pos);
    Block* block = level->getBlockState(pos)->getBlock();
    if (isDangerous() && WitherBoss::canDestroy(block) && resistance < 0.8f)
        resistance = 0.8f;
    return resistance;
}

void WitherSkull::addDataWalkers(DataFixerUpper* fixer)
{
    Fireball::addDataWalkers(fixer, 0x8a400002ULL);
}

bool WitherSkull::isPickable()
{
    return isDangerous();
}

bool WitherSkull::isDangerous()
{
    return getEntityData()->get(sWitherSkullDangerous);
}

void WitherSkull::setDangerous(bool dangerous)
{
    getEntityData()->set(sWitherSkullDangerous, dangerous, true);
}

double WitherSkull::getInertia()
{
    return isDangerous() ? 0.75 : Fireball::getInertia();
}

void WitherSkull::onHit(const HitResult* result)
{
    if (mLevel->mIsClientSide)
        return;

    if (result->mEntity)
    {
        bool hurt = result->mEntity->hurt(NULL, 5.0f);
        if (hurt && mOwner)
        {
            doEnchantDamageEffects(mOwner, result->mEntity);
        }
    }
    bool mobGriefing = mLevel->getGameRules()->getBoolean(1);
    mLevel->explode(shared_from_this(), mX, mY, mZ, 1.0f, false, true,
                    mobGriefing, true, true, 0.0f);
    remove();
}

void WitherSkull::defineSynchedData()
{
    getEntityData()->createDateItem(sWitherSkullDangerous, false);
}

bool WitherSkull::shouldBurn()
{
    return false;
}

std::vector<unsigned long long>* WitherSkull::getEntityTypesToIgnoreCollisionsWith()
{
    std::vector<unsigned long long>* types = new std::vector<unsigned long long>();
    types->push_back(GetType());
    return types;
}

unsigned long long WitherSkull::GetType()
{
    return 0x8a400002ULL;
}
