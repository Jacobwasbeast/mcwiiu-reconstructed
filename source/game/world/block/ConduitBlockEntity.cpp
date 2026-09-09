#include "game/world/block/ConduitBlockEntity.hpp"

#include <typeinfo>
#include <boost/shared_ptr.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"

ConduitBlockEntity::ConduitBlockEntity()
    : BlockEntity(),
      mActive(false),
      mHunting(false),
      mCheckTime(0),
      mNextTargetCheck(0LL),
      mTickCount(0),
      mAnimationValue(0.0f),
      mActiveRotation(0.0f),
      mRotationTick(0),
      mWindLevel(0),
      mEffectRange(0),
      mTargetId(-1),
      mBlocks()
{
    mBlocks.reserve(42);
}

ConduitBlockEntity::~ConduitBlockEntity()
{
}

BlockEntity* ConduitBlockEntity::create()
{
    return new ConduitBlockEntity();
}

boost::shared_ptr<BlockEntity> ConduitBlockEntity::clone()
{
    boost::shared_ptr<BlockEntity> result(new ConduitBlockEntity());
    BlockEntity::clone(result);
    return result;
}

void ConduitBlockEntity::_animateTick()
{
    if (!mActive || mLevel == NULL)
    {
        return;
    }

    BlockPos position = getBlockPos();
    double x = (double)position.x + 0.5 + (double)mLevel->mRandom->nextFloat(-0.5f, 0.5f);
    double y = (double)position.y + 0.5 + (double)mLevel->mRandom->nextFloat(-1.0f, 1.0f);
    double z = (double)position.z + 0.5 + (double)mLevel->mRandom->nextFloat(-0.5f, 0.5f);
    mLevel->addParticle(PARTICLE_CONDUIT, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
}

void ConduitBlockEntity::setEffectRange(int range)
{
    setChanged();
    mEffectRange = range;
}

void ConduitBlockEntity::_checkShape()
{
    const BlockPos center = getBlockPos();
    const int oldRange = mEffectRange;
    const bool oldActive = mActive;
    mBlocks.clear();

    int count = 0;
    int y;
    int x;
    int z;
    for (y = -2; y <= 2; ++y)
    {
        for (x = -2; x <= 2; ++x)
        {
            for (z = -2; z <= 2; ++z)
            {
                bool frame = false;
                if ((y == -2 || y == 2) && x >= -1 && x <= 1 && z >= -1 && z <= 1)
                {
                    frame = true;
                }
                if ((x == -2 || x == 2) && y >= -1 && y <= 1 && z >= -1 && z <= 1)
                {
                    frame = true;
                }
                if ((z == -2 || z == 2) && y == 0 && x >= -1 && x <= 1)
                {
                    frame = true;
                }
                if (frame)
                {
                    BlockPos position = center.offset(x, y, z);
                    int id = Block::getId(mLevel->getBlock(position));
                    if (id == 168 || id == 169)
                    {
                        mBlocks.push_back(position);
                        ++count;
                    }
                }
            }
        }
    }

    setEffectRange((count / 7) * 16);
    mActive = count > 16;
    mHunting = count > 41;
    if (oldActive != mActive || oldRange != mEffectRange)
    {
        setChanged();
    }
}

void ConduitBlockEntity::_applyEffects()
{
    if (mActive && mLevel != NULL && !mLevel->mIsClientSide)
    {
        AABB* area = AABB::newTemp(getBlockPos());
        area->grow((double)mEffectRange)->expand(1.0, 1.0, 1.0);
        std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(Entity), area);
        delete entities;
    }
}

void ConduitBlockEntity::_updateTarget()
{
    if (mLevel == NULL || mLevel->mIsClientSide)
    {
        return;
    }

    boost::shared_ptr<Entity> target = mLevel->getEntity(mTargetId);
    if (target && target->isInWaterOrRain() && target->distanceToSqr(getBlockPos()) < 64.0)
    {
        return;
    }

    mTargetId = -1;
    if (!mHunting)
    {
        setChanged();
        return;
    }

    AABB* area = AABB::newTemp(getBlockPos());
    area->grow(8.0)->expand(0.0, 1.0, 0.0);
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(Entity), area);
    if (entities != NULL && !entities->empty())
    {
        int index = mLevel->mRandom->nextInt((int)entities->size());
        if ((*entities)[index])
        {
            mTargetId = (*entities)[index]->getId();
        }
    }
    delete entities;
    setChanged();
}

void ConduitBlockEntity::tick()
{
    ++mTickCount;
    BlockEntity::tick();
    if (mActive)
    {
        _animateTick();
    }
    if (--mCheckTime <= 0)
    {
        mCheckTime = 40;
        _checkShape();
        if (mActive)
        {
            _applyEffects();
            _updateTarget();
        }
    }
}

void ConduitBlockEntity::loadFromPacket(Level*, const BlockPos&, CompoundTag* tag)
{
    BlockEntity::load(tag);
    mTargetId = tag->getInt(L"Target");
    mActive = tag->getBoolean(L"Active");
}

void ConduitBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mTargetId = tag->getInt(L"Target");
    mActive = tag->getBoolean(L"Active");
    mActiveRotation = tag->getFloat(L"Rotation");
}

CompoundTag* ConduitBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putInt(L"Target", mTargetId);
    tag->putBoolean(L"Active", mActive);
    tag->putFloat(L"Rotation", mActiveRotation);
    return tag;
}

boost::shared_ptr<Packet> ConduitBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>(new ClientboundBlockEntityDataPacket(getBlockPos(), 13, 0, getUpdateTag()));
}

CompoundTag* ConduitBlockEntity::getUpdateTag()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return tag;
}

void ConduitBlockEntity::incrementAnimationValue(float amount)
{
    mAnimationValue += amount;
    if (mAnimationValue >= 1.0f)
    {
        mAnimationValue = 0.0f;
    }
}

void ConduitBlockEntity::incrementWindLevel()
{
    ++mWindLevel;
    if (mWindLevel > 2)
    {
        mWindLevel = 0;
    }
}

unsigned long long ConduitBlockEntity::GetType()
{
    return 0x0000000001200212ULL;
}
