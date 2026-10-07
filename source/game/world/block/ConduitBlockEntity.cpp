#include "game/world/block/ConduitBlockEntity.hpp"
#include "game/core/EntityType.hpp"

#include <stdlib.h>
#include <typeinfo>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerList.hpp"

ConduitBlockEntity::ConduitBlockEntity()
    : BlockEntity(),
      mActive(false),
      mHunting(false),
      mCheckTime(0),
      mNextAmbientSoundActivation(0LL),
      mTickCount(0),
      mAnimationValue(0.0f),
      mActiveRotation(0.0f),
      mRotationTick(0),
      mWindLevel(0),
      mTargetId(-1),
      mBlocks()
{
    mBlocks.reserve(42);
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
    double x = position.x + 0.5;
    double y = position.y + 1.5;
    double z = position.z + 0.5;

    if (mLevel->mRandom->nextInt(5) != 0)
        return;

    if (mActive)
    {
        for (std::vector<BlockPos>::iterator it = mBlocks.begin(); it != mBlocks.end(); ++it)
        {
            if (mLevel->mRandom->nextInt(5) == 0)
            {
                double ox = mLevel->mRandom->nextFloat(-0.5f, 0.5f);
                double oy = mLevel->mRandom->nextFloat(-2.0f, -1.0f);
                double oz = mLevel->mRandom->nextFloat(-0.5f, 0.5f);
                mLevel->addParticle(PARTICLE_CONDUIT, x, y, z, (it->x - position.x) + ox, (it->y - position.y) + oy,
                                    (it->z - position.z) + oz, arrayWithLength<int>());
            }
        }
    }

    boost::shared_ptr<Entity> target = mLevel->getEntity(mTargetId);
    if (target)
    {
        for (int i = 0; i < 6; ++i)
        {
            float xa = (mLevel->mRandom->nextFloat() + -0.5f) * (target->mBbWidth + 3.0f);
            float ya = mLevel->mRandom->nextFloat() * target->mBbHeight + -1.0f;
            float za = (mLevel->mRandom->nextFloat() + -0.5f) * (target->mBbWidth + 3.0f);
            mLevel->addParticle(PARTICLE_CONDUIT, x, y, z, xa, ya, za, arrayWithLength<int>());
        }
    }
}

void ConduitBlockEntity::setEffectRange(int range)
{
    BlockEntity::setChanged();
    mEffectRange = range;
}

void ConduitBlockEntity::_checkShape()
{
    bool wasActive = mActive;
    mBlocks.clear();
    mActive = false;
    mHunting = false;

    bool dry = false;
    BlockPos center = getBlockPos();
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            for (int z = -1; z <= 1; ++z)
            {
                if (x == 0 && y == 0 && z == 0)
                    continue;
                if (mLevel->getLiquidBlock(BlockPos(center.x + x, center.y + y, center.z + z))->mMaterial
                    != Material::WATER)
                {
                    dry = true;
                }
            }
        }
    }

    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            if ((abs(x) == 1 || x == 0) && (abs(y) == 1 || y == 0))
                continue;
            BlockPos framePos(center.x + x, center.y + y, center.z);
            Block* block = mLevel->getBlock(framePos);
            if (block == Blocks::prismarine)
                mBlocks.push_back(framePos);
            if (block == Blocks::sea_lantern)
                mBlocks.push_back(framePos);
        }
    }

    for (int x = -2; x <= 2; ++x)
    {
        for (int z = -2; z <= 2; ++z)
        {
            if ((abs(x) == 1 || x == 0) && (abs(z) == 1 || z == 0))
                continue;
            if (abs(x) == 2 && z == 0)
                continue;
            BlockPos framePos(center.x + x, center.y, center.z + z);
            Block* block = mLevel->getBlock(framePos);
            if (block == Blocks::prismarine)
                mBlocks.push_back(framePos);
            if (block == Blocks::sea_lantern)
                mBlocks.push_back(framePos);
        }
    }

    for (int z = -2; z <= 2; ++z)
    {
        for (int y = -2; y <= 2; ++y)
        {
            if ((abs(z) == 1 || z == 0) && (abs(y) == 1 || y == 0))
                continue;
            if (abs(z) == 2 && y == 0)
                continue;
            if (abs(y) == 2 && z == 0)
                continue;
            BlockPos framePos(center.x, center.y + y, center.z + z);
            Block* block = mLevel->getBlock(framePos);
            if (block == Blocks::prismarine)
                mBlocks.push_back(framePos);
            if (block == Blocks::sea_lantern)
                mBlocks.push_back(framePos);
        }
    }

    if (!dry)
    {
        unsigned int count = mBlocks.size();
        setEffectRange((static_cast<int>(count) / 7) * 16);
        if (count > 15)
        {
            mActive = true;
            if (!wasActive)
            {
                mLevel->playLocalSound(getBlockPos().x, getBlockPos().y, getBlockPos().z,
                                       SoundEvents::CONDUIT_ACTIVATE, SoundSource::eSoundSource_Blocks, 1.0f,
                                       1.0f, false, 16.0f);
                if (!mLevel->mIsClientSide)
                {
                    int range = mEffectRange;
                    PlayerList* playerList = gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers;
                    std::vector<boost::shared_ptr<ServerPlayer> >* players = playerList->getPlayers();
                    for (unsigned int i = 0; i < players->size(); ++i)
                    {
                        boost::shared_ptr<ServerPlayer> player = players->at(i);
                        Vec3* position = player->getPos(0.0f);
                        if (center.x - range < position->x && position->x < center.x + range
                            && center.y - range < position->y && position->y < center.y + range
                            && center.z - range < position->z && position->z < center.z + range)
                        {
                            player->awardStat(GenericStats::moskstraumen(), GenericStats::param_moskstraumen());
                        }
                    }
                }
            }
            if (count > 41)
                mHunting = true;
        }
    }

    if (wasActive && !mActive)
    {
        mLevel->playLocalSound(getBlockPos().x, getBlockPos().y, getBlockPos().z, SoundEvents::CONDUIT_DEACTIVATE,
                               SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, false, 16.0f);
    }
}

void ConduitBlockEntity::_applyEffects()
{
    if (mActive && !mLevel->mIsClientSide)
    {
        double maxBuildHeight = mLevel->getMaxBuildHeight();
        AABB* area = AABB::newTemp((double)mPos.x, (double)mPos.y, (double)mPos.z,
                                    (double)(mPos.x + 1), (double)(mPos.y + 1), (double)(mPos.z + 1));
        area = area->grow((double)mEffectRange);
        area = area->expand(0.0, maxBuildHeight, 0.0);
        std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(Player), area);
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
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
    if (mLevel->mIsClientSide)
        return;

    int previousTarget = mTargetId;
    if (!mHunting)
        mTargetId = -1;

    boost::shared_ptr<Entity> target = mLevel->getEntity(mTargetId);
    if (target)
    {
        if (target->isAlive() && target->isInWaterOrRain() && target->distanceToSqr(getBlockPos()) < 64.0)
        {
            mLevel->playSound(boost::shared_ptr<Player>(), getBlockPos(), SoundEvents::CONDUIT_ATTACK,
                              SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, 16.0f);
            target->hurt(DamageSource::MAGIC, 4.0f);
            if (mTargetId != previousTarget)
                BlockEntity::setChanged();
        }
        else
        {
            mTargetId = -1;
            if (previousTarget != -1)
                BlockEntity::setChanged();
        }
        return;
    }

    if (mHunting)
    {
        std::vector<boost::shared_ptr<Entity> > candidates;
        double maxBuildHeight = mLevel->getMaxBuildHeight();
        const AABB* area = AABB::newTemp(mPos.x, mPos.y, mPos.z, mPos.x + 1, mPos.y + 1, mPos.z + 1)
                               ->grow(8.0)
                               ->expand(0.0, maxBuildHeight, 0.0);
        std::vector<boost::shared_ptr<Entity> >* monsters = mLevel->getEntitiesOfClass(typeid(Monster), area);
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = monsters->begin(); it != monsters->end(); ++it)
        {
            boost::shared_ptr<Entity> monster = *it;
            if (monster && monster->isAlive() && monster->isInWaterOrRain())
                candidates.push_back(monster);
        }
        delete monsters;
        if (!candidates.empty())
            mTargetId = candidates[mLevel->mRandom->nextInt(candidates.size())]->getId();
    }
    if (mTargetId != previousTarget)
        BlockEntity::setChanged();
}

void ConduitBlockEntity::tick()
{
    ++mTickCount;
    BlockEntity::tick();
    if (mActive)
        _animateTick();

    if (mLevel->mIsClientSide)
    {
        Random random(getBlockPos().hashCode());
        int tick = mTickCount + random.nextInt(9001);
        if (tick % 80 == 0 && mActive)
        {
            mLevel->playLocalSound(getBlockPos().x, getBlockPos().y, getBlockPos().z, SoundEvents::CONDUIT_AMBIENT,
                                   SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, false, 16.0f);
        }
        if (tick > mNextAmbientSoundActivation && mActive)
        {
            mNextAmbientSoundActivation = tick + 60 + mLevel->mRandom->nextInt(40);
            mLevel->playLocalSound(getBlockPos().x, getBlockPos().y, getBlockPos().z, SoundEvents::CONDUIT_SHORT,
                                   SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, false, 16.0f);
        }
    }

    if (--mCheckTime > 0)
        return;
    mCheckTime = 40;
    _checkShape();
    if (mActive)
    {
        _applyEffects();
        _updateTarget();
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
    boost::shared_ptr<ClientboundBlockEntityDataPacket> packet(new ClientboundBlockEntityDataPacket(
        mPos, 13, mLevel->mDimension->getType()->getId(), getUpdateTag()));
    return packet;
}

CompoundTag* ConduitBlockEntity::getUpdateTag()
{
    return save(new CompoundTag());
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
    return BlockEntityType::CONDUIT;
}
