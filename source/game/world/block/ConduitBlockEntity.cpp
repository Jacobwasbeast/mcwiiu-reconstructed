#include "game/world/block/ConduitBlockEntity.hpp"

#include <typeinfo>
#include <boost/shared_ptr.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

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
    BlockPos position = getBlockPos();
    double x = (double)position.x + 0.5;
    double y = (double)position.y + 1.5;
    double z = (double)position.z + 0.5;

    if (mLevel->mRandom->nextInt(5) != 0)
    {
        return;
    }

    if (mActive && !mBlocks.empty())
    {
        std::vector<BlockPos>::iterator it;
        for (it = mBlocks.begin(); it != mBlocks.end(); ++it)
        {
            if (mLevel->mRandom->nextInt(5) == 0)
            {
                double ox = (double)mLevel->mRandom->nextFloat(-0.5f, 0.5f);
                double oy = (double)mLevel->mRandom->nextFloat(-2.0f, -1.0f);
                double oz = (double)mLevel->mRandom->nextFloat(-0.5f, 0.5f);
                mLevel->addParticle(PARTICLE_CONDUIT, x, y, z,
                                     (double)(it->x - position.x) + ox,
                                     (double)(it->y - position.y) + oy,
                                     (double)(it->z - position.z) + oz,
                                     arrayWithLength<int>());
            }
        }
    }
}

void ConduitBlockEntity::setEffectRange(int range)
{
    setChanged();
    mEffectRange = range;
}

void ConduitBlockEntity::_checkShape()
{
    const BlockPos center = getBlockPos();
    mBlocks.clear();
    mActive = false;
    mHunting = false;

    bool submerged = true;
    int dx;
    int dy;
    int dz;
    for (dx = -1; dx <= 1; ++dx)
    {
        for (dy = -1; dy <= 1; ++dy)
        {
            for (dz = -1; dz <= 1; ++dz)
            {
                if (dx == 0 && dy == 0 && dz == 0)
                {
                    continue;
                }
                if (mLevel->getLiquidBlock(center.offset(dx, dy, dz))->mMaterial != Material::WATER)
                {
                    submerged = false;
                }
            }
        }
    }

    if (submerged)
    {
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
                        Block* block = mLevel->getBlock(position);
                        if (block == Blocks::prismarine || block == Blocks::sea_lantern)
                        {
                            mBlocks.push_back(position);
                            ++count;
                        }
                    }
                }
            }
        }

        setEffectRange((count / 7) * 16);
        if (count > 15)
        {
            mActive = true;
            mHunting = count > 41;
        }
    }
}

void ConduitBlockEntity::_applyEffects()
{
    if (mActive && !mLevel->mIsClientSide)
    {
        AABB* area = AABB::newTemp((double)mPos.x, (double)mPos.y, (double)mPos.z,
                                    (double)(mPos.x + 1), (double)(mPos.y + 1), (double)(mPos.z + 1));
        area = area->grow((double)mEffectRange);
        area = area->expand(1.0, (double)mLevel->getMaxBuildHeight(), 1.0);
        std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(Entity), area);
        std::vector<boost::shared_ptr<Entity> >::iterator it;
        for (it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(*it);
            if (player->isInWaterOrRain())
            {
                player->addEffect(new MobEffectInstance(MobEffects::CONDUIT_POWER, 260, 0, true, true),
                                  boost::shared_ptr<Entity>());
            }
        }
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
    return boost::shared_ptr<Packet>(new ClientboundBlockEntityDataPacket(mPos, 13, mLevel->mDimension->getType()->getId(), getUpdateTag()));
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
