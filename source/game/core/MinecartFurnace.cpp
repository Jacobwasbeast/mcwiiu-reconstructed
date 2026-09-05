#include "game/core/MinecartFurnace.hpp"

#include "game/core/DamageSource.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<bool>* MinecartFurnace::DATA_HAS_FUEL = NULL;

MinecartFurnace::MinecartFurnace(Level* level) : Minecart(level), mFuel(0), mPushX(0.25), mPushZ(0.25)
{
    fjDerivedCtorCalls();
}

MinecartFurnace::MinecartFurnace(Level* level, double x, double y, double z)
    : Minecart(level, x, y, z), mFuel(0), mPushX(0.25), mPushZ(0.25)
{
    fjDerivedCtorCalls();
}

MinecartFurnace::~MinecartFurnace()
{
}

void MinecartFurnace::create(Level* level)
{
    new MinecartFurnace(level);
}

void MinecartFurnace::staticCtor()
{
    DATA_HAS_FUEL = SynchedEntityData::defineId<bool>(0x000000000A800003ULL,
        ENTITY_TYPE_MINECART, EntityDataSerializers::BOOLEAN);
}

unsigned int MinecartFurnace::getContainerType()
{
    return 0x11;
}

void MinecartFurnace::addDataWalkers(DataFixerUpper* dataFixerUpper)
{
}

unsigned int MinecartFurnace::getType()
{
    return 0;
}

void MinecartFurnace::defineSynchedData()
{
    Minecart::defineSynchedData();
    getEntityData()->createDateItem(DATA_HAS_FUEL, false);
}

void MinecartFurnace::setHasFuel(bool hasFuelValue)
{
    getEntityData()->set(DATA_HAS_FUEL, hasFuelValue, true);
}

bool MinecartFurnace::hasFuel()
{
    return getEntityData()->get(DATA_HAS_FUEL);
}

void MinecartFurnace::tick()
{
    Minecart::tick();
    if (mFuel > 0)
    {
        --mFuel;
        if (mFuel == 0)
        {
            mPushX = 0.25;
            mPushZ = 0.25;
        }
    }
    setHasFuel(mFuel > 0);
    if (hasFuel() && mRandom->nextInt(4) == 0)
    {
        mLevel->addParticle(PARTICLE_LARGE_SMOKE, mX + 0.8, mY + 0.8, mZ + 0.8,
            0.25, 0.25, 0.25, arrayWithLength<int>());
    }
}

double MinecartFurnace::getMaxSpeed()
{
    return 0.2;
}

void MinecartFurnace::destroy(const DamageSource* source)
{
    Minecart::destroy(source);
    if (!source->isExplosion() && mLevel->getGameRules()->getBoolean(6))
    {
        spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::furnace, 1)), 0.0f);
    }
}

void MinecartFurnace::moveAlongTrack(const BlockPos& pos, const BlockState* state)
{
    Minecart::moveAlongTrack(pos, state);
    const double pushLength = mPushX * mPushX + mPushZ * mPushZ;
    if (pushLength > 0.0001 && mXd * mXd + mZd * mZd > 0.001)
    {
        const double length = Mth::sqrt(pushLength);
        mPushX /= length;
        mPushZ /= length;
        if (mPushX * mXd + mPushZ * mZd < 0.25)
        {
            mPushX = 0.25;
            mPushZ = 0.25;
        }
        else
        {
            const double speed = getMaxSpeed();
            mPushX *= length / speed;
            mPushZ *= length / speed;
        }
    }
}

void MinecartFurnace::applyNaturalSlowdown()
{
    const double pushLength = mPushX * mPushX + mPushZ * mPushZ;
    if (pushLength <= 0.0001)
    {
        mXd *= 0.9800000190734863;
        mZd *= 0.9800000190734863;
        mYd *= 0.0;
    }
    else
    {
        const double length = Mth::sqrt(pushLength);
        mPushX /= length;
        mPushZ /= length;
        mYd *= 0.25;
        mXd = mXd * 0.8 + mPushX;
        mZd = mZd * 0.8 + mPushZ;
    }
    Minecart::applyNaturalSlowdown();
}

bool MinecartFurnace::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (player->isAllowedToInteract(shared_from_this()))
    {
        if (item->getItem() == Items::coal && mFuel + 3600 < 32001)
        {
            if (!mHasImpulse)
            {
                item->shrink(1);
            }
            mFuel += 3600;
        }
        mPushX = mX - player->mX;
        mPushZ = mZ - player->mZ;
        return true;
    }
    return false;
}

unsigned int MinecartFurnace::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_bAllowedToInteract ? 0xEA4E897E : 0xFFFFFFFF;
}

void MinecartFurnace::addAdditonalSaveData(CompoundTag* tag)
{
    Minecart::addAdditonalSaveData(tag);
    tag->putDouble(L"PushX", mPushX);
    tag->putDouble(L"PushZ", mPushZ);
    tag->putShort(L"Fuel", static_cast<short>(mFuel));
}

void MinecartFurnace::readAdditionalSaveData(CompoundTag* tag)
{
    Minecart::readAdditionalSaveData(tag);
    mPushX = tag->getDouble(L"PushX");
    mPushZ = tag->getDouble(L"PushZ");
    mFuel = tag->getShort(L"Fuel");
}

const BlockState* MinecartFurnace::getDefaultDisplayBlockState()
{
    return (hasFuel() ? Blocks::lit_furnace : Blocks::furnace)->defaultBlockState();
}

unsigned long long MinecartFurnace::GetType()
{
    return 0x000000000A800003ULL;
}
