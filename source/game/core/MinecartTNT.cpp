#include "game/core/MinecartTNT.hpp"

#include <cmath>
#include <string>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/DamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BaseRailBlock.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/biome/arrayWithLength.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

extern CMinecraftApp* g_pApp;

void MinecartTNT::create(Level* level)
{
    new MinecartTNT(level);
}

MinecartTNT::MinecartTNT(Level* level)
    : Minecart(level)
{
    _init();
}

MinecartTNT::MinecartTNT(Level* level, double x, double y, double z)
    : Minecart(level, x, y, z)
{
    _init();
}

MinecartTNT::~MinecartTNT()
{
}

void MinecartTNT::_init()
{
    fjDerivedCtorCalls();
    mFuse = -1;
}

void MinecartTNT::addDataWalkers(DataFixerUpper*)
{
}

unsigned int MinecartTNT::getType()
{
    return 0;
}

const BlockState* MinecartTNT::getDefaultDisplayBlockState()
{
    return Blocks::tnt->defaultBlockState();
}

void MinecartTNT::tick()
{
    PIXBeginNamedEvent(4.5f, "Minecart tnt tick");
    Minecart::tick();

    if (mFuse >= 0)
    {
        if (mFuse > 0)
        {
            --mFuse;
            mLevel->addParticle(ePARTICLE_SMOKE, false, mX, mY + 0.5, mZ,
                                0.0, 0.0, 0.0, arrayWithLength<int>());
        }
        else
        {
            explode(mXd * mXd + mZd * mZd);
        }
    }

    if (mOnGround && mXd * mXd + mZd * mZd >= 0.01)
    {
        explode(mXd * mXd + mZd * mZd);
    }
    PIXEndNamedEvent();
}

bool MinecartTNT::hurt(const DamageSource* source, float damage)
{
    boost::shared_ptr<Entity> entity = source->getDirectEntity();
    if (entity && entity->isOnFire())
    {
        explode(entity->mXd * mXd + entity->mYd * mYd + entity->mZd * mZd);
    }
    return Minecart::hurt(source, damage);
}

void MinecartTNT::destroy(const DamageSource* source)
{
    if (g_pApp->GetGameHostOption(eGameHostOption_13) != 0 &&
        (source->isFire() || source->isExplosion() || mXd * mXd + mZd * mZd >= 0.01))
    {
        if (mFuse < 0)
        {
            primeFuse();
            mFuse = mRandom->nextInt(20) + mRandom->nextInt(20);
        }
        return;
    }

    Minecart::destroy(source);
    if (g_pApp->GetGameHostOption(eGameHostOption_13) != 0 && !source->isExplosion() &&
        mLevel->getGameRules()->getBoolean(6))
    {
        spawnAtLocation(Item::byBlock(Blocks::tnt), 1);
    }
}

void MinecartTNT::explode(double power)
{
    if (!mLevel->mIsClientSide)
    {
        if (g_pApp->GetGameHostOption(eGameHostOption_13) != 0)
        {
            double radius = std::sqrt(power);
            if (radius > 5.0)
                radius = 5.0;
            radius = (float)(mRandom->nextDouble() * 1.5 * radius - 1.0);
            mLevel->explode(shared_from_this(), mX, mY, mZ, (float)radius,
                            false, false, true, true, true, -1.0f);
            remove();
        }
        else
        {
            remove();
        }
    }
}

void MinecartTNT::causeFallDamage(float fallDistance, float damageMultiplier)
{
    if (fallDistance >= 3.0f)
    {
        explode((float)(fallDistance / 10.0f) * (float)(fallDistance / 10.0f));
    }
    Entity::causeFallDamage(fallDistance, damageMultiplier);
}

void MinecartTNT::activateMinecart(int, int, int, bool powered)
{
    if (powered && mFuse < 0)
        primeFuse();
}

void MinecartTNT::handleEntityEvent(unsigned char event, int data)
{
    if (data == 40)
        primeFuse();
    else
        Entity::handleEntityEvent(event, data);
}

void MinecartTNT::primeFuse()
{
    if (g_pApp->GetGameHostOption(eGameHostOption_13) != 0)
    {
        mFuse = 80;
        if (!mLevel->mIsClientSide)
            mLevel->broadcastEntityEvent(shared_from_this(), 40, 0);
        if (!isSilent())
            mLevel->addParticle(ePARTICLE_SMOKE, false, mX, mY, mZ, 0.0, 0.0, 0.0,
                                arrayWithLength<int>());
    }
}

int MinecartTNT::getFuse()
{
    return mFuse;
}

bool MinecartTNT::isPrimed()
{
    return mFuse >= 0;
}

float MinecartTNT::getBlockExplosionResistance(Explosion* explosion, Level* level, const BlockPos& pos)
{
    if (ignoreExplosion())
    {
        const BlockState* state = level->getBlockState(pos);
        if (BaseRailBlock::isRail(state) || BaseRailBlock::isRail(level, pos.above()))
        {
            if (!isInvulnerableTo(NULL))
                return 4.5;
        }
    }
    return Entity::getBlockExplosionResistance(explosion, level, pos);
}

bool MinecartTNT::shouldBlockExplode(Explosion* explosion, Level* level, const BlockPos& pos, float radius)
{
    if (g_pApp->GetGameHostOption(eGameHostOption_13) != 0)
    {
        const BlockState* state = level->getBlockState(pos);
        if (BaseRailBlock::isRail(state) || BaseRailBlock::isRail(level, pos.above()))
            return false;
    }
    return Entity::shouldBlockExplode(explosion, level, pos, radius);
}

void MinecartTNT::readAdditionalSaveData(CompoundTag* tag)
{
    Minecart::readAdditionalSaveData(tag);
    std::wstring key(L"TNTFuse");
    if (tag->contains(key, 99))
        mFuse = tag->getInt(key);
}

void MinecartTNT::addAdditonalSaveData(CompoundTag* tag)
{
    Minecart::addAdditonalSaveData(tag);
    tag->putInt(std::wstring(L"TNTFuse"), mFuse);
}

unsigned long long MinecartTNT::GetType()
{
    return 0x0A80000400000000ULL;
}
