#include "game/core/Shulker.hpp"

#include <vector>

#include "game/core/AttributeModifier.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/Item.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Mth.hpp"
#include "game/core/ShulkerBullet.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"
#include "game/world/block/PistonMagicBlock.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<const Direction*>* Shulker::DATA_ATTACH_FACE = NULL;
const EntityDataAccessor<BlockPos>* Shulker::DATA_ATTACH_POSITION = NULL;
const EntityDataAccessor<unsigned char>* Shulker::DATA_PEEK = NULL;
const EntityDataAccessor<unsigned char>* Shulker::DATA_COLOR = NULL;

Shulker::ShulkerAttackGoal::ShulkerAttackGoal(Shulker* shulker)
    : Goal(), mShulker(shulker), mAttackTime(0)
{
}

Shulker::ShulkerAttackGoal::~ShulkerAttackGoal()
{
}

bool Shulker::ShulkerAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mShulker->getTarget();
    if (target == NULL || !target->isAlive())
        return false;
    return mShulker->mLevel->getDifficulty() != Difficulty::PEACEFUL;
}

void Shulker::ShulkerAttackGoal::start()
{
    mAttackTime = 20;
    mShulker->setRawPeekAmount(100);
}

void Shulker::ShulkerAttackGoal::stop()
{
    mAttackTime = 0;
}

void Shulker::ShulkerAttackGoal::tick()
{
    if (mShulker->mLevel->getDifficulty() == Difficulty::PEACEFUL)
        return;

    --mAttackTime;
    boost::shared_ptr<LivingEntity> target = mShulker->getTarget();
    mShulker->getLookControl()->setLookAt(target, 180.0f, 180.0f);

    if (mShulker->distanceToSqr(target) >= 400.0)
    {
        mShulker->setTarget(boost::shared_ptr<LivingEntity>());
    }
    else if (mAttackTime <= 0)
    {
        mAttackTime = mShulker->mRandom->nextInt(10) * 10 + 20;
        const Direction* attachFace = mShulker->getAttachFace();
        Direction::Axis* axis = attachFace->getAxis();
        boost::shared_ptr<Entity> bullet(new ShulkerBullet(mShulker->mLevel,
            boost::static_pointer_cast<LivingEntity>(mShulker->shared_from_this()), target, axis));
        mShulker->mLevel->addEntity(bullet);
        float pitch = (mShulker->mRandom->nextFloat() - mShulker->mRandom->nextFloat()) * 0.2f + 1.0f;
        mShulker->playSound(SoundEvents::SHULKER_SHOOT, 2.0f, pitch);
    }

    Goal::tick();
}

std::wstring Shulker::ShulkerAttackGoal::getName()
{
    return std::wstring(L"ShulkerAttack");
}

Shulker::ShulkerBodyControl::ShulkerBodyControl(Shulker* shulker)
    : BodyControl(shulker), mShulker(shulker)
{
}

Shulker::ShulkerBodyControl::~ShulkerBodyControl()
{
}

void Shulker::ShulkerBodyControl::clientTick()
{
}

Shulker::ShulkerDefenseAttackGoal::ShulkerDefenseAttackGoal(Shulker* shulker)
    : NearestAttackableTargetGoal(shulker, typeid(LivingEntity), 10, true, false, NULL, false, false), mShulker(shulker)
{
}

Shulker::ShulkerDefenseAttackGoal::~ShulkerDefenseAttackGoal()
{
}

bool Shulker::ShulkerDefenseAttackGoal::canUse()
{
    return mShulker != NULL && mShulker->getTeam() != NULL && NearestAttackableTargetGoal::canUse();
}

AABB* Shulker::ShulkerDefenseAttackGoal::getTargetSearchArea(double distance)
{
    const Direction* attachFace = mShulker->getAttachFace();
    Direction::Axis* axis = attachFace->getAxis();
    if (axis == Direction::Axis::X)
        return mShulker->mBoundingBox->grow(4.0, distance, distance);
    if (axis == Direction::Axis::Y)
        return mShulker->mBoundingBox->grow(distance, distance, 4.0);
    return mShulker->mBoundingBox->grow(distance, 4.0, distance);
}

std::wstring Shulker::ShulkerDefenseAttackGoal::getName()
{
    return std::wstring(L"ShulkerDefenseAttack");
}

Shulker::ShulkerNearestAttackGoal::ShulkerNearestAttackGoal(Shulker* shulker)
    : NearestAttackableTargetGoal(shulker, typeid(LivingEntity), true, false, false), mShulker(shulker)
{
}

Shulker::ShulkerNearestAttackGoal::~ShulkerNearestAttackGoal()
{
}

bool Shulker::ShulkerNearestAttackGoal::canUse()
{
    return mShulker != NULL && mShulker->mLevel != NULL && !NearestAttackableTargetGoal::canUse();
}

AABB* Shulker::ShulkerNearestAttackGoal::getTargetSearchArea(double distance)
{
    const Direction* attachFace = mShulker->getAttachFace();
    Direction::Axis* axis = attachFace->getAxis();
    if (axis == Direction::Axis::X)
        return mShulker->mBoundingBox->grow(4.0, distance, distance);
    if (axis == Direction::Axis::Y)
        return mShulker->mBoundingBox->grow(distance, distance, 4.0);
    return mShulker->mBoundingBox->grow(distance, 4.0, distance);
}

std::wstring Shulker::ShulkerNearestAttackGoal::getName()
{
    return std::wstring(L"ShulkerNearestAttack");
}

Shulker::ShulkerPeekGoal::ShulkerPeekGoal(Shulker* shulker)
    : Goal(), mShulker(shulker), mPeekTime(0)
{
}

Shulker::ShulkerPeekGoal::~ShulkerPeekGoal()
{
}

bool Shulker::ShulkerPeekGoal::canUse()
{
    return mShulker != NULL && mShulker->getTarget() == NULL && mShulker->mRandom->nextInt(40) == 0;
}

bool Shulker::ShulkerPeekGoal::canContinueToUse()
{
    bool result = false;
    boost::shared_ptr<LivingEntity> target = mShulker->getTarget();
    if (target.get() == NULL && mPeekTime > 0)
        result = true;
    return result;
}

void Shulker::ShulkerPeekGoal::start()
{
    mPeekTime = mShulker->mRandom->nextInt(3) * 20 + 20;
    mShulker->setRawPeekAmount(30);
}

void Shulker::ShulkerPeekGoal::stop()
{
    boost::shared_ptr<LivingEntity> target = mShulker->getTarget();
    if (target.get() == NULL)
        mShulker->setTarget(boost::shared_ptr<LivingEntity>());
}

void Shulker::ShulkerPeekGoal::tick()
{
    --mPeekTime;
}

std::wstring Shulker::ShulkerPeekGoal::getName()
{
    return std::wstring(L"ShulkerPeak");
}

Shulker::Shulker(Level* level)
    : Golem(level), mOldPeekAmount(0.0f), mPeekAmount(0.0f), mOldAttachPosition(NULL), mAttachPosition(NULL), mClientSideTeleportInterpolation(0)
{
    fjDerivedCtorCalls();
    setSize(1.0f, 1.0f);
}

Shulker::~Shulker()
{
    delete mOldAttachPosition;
    delete mAttachPosition;
}

void Shulker::create(Level* level)
{
    new Shulker(level);
}

void Shulker::staticCtor()
{
    DATA_ATTACH_FACE = SynchedEntityData::defineId<const Direction*>(ENTITY_TYPE_SHULKER, 0x0d580000ULL, EntityDataSerializers::DIRECTION);
    DATA_ATTACH_POSITION = SynchedEntityData::defineId<BlockPos>(ENTITY_TYPE_SHULKER, 0x0d580000ULL, EntityDataSerializers::BLOCK_POS);
    DATA_PEEK = SynchedEntityData::defineId<unsigned char>(ENTITY_TYPE_SHULKER, 0x0d580000ULL, EntityDataSerializers::BYTE);
    DATA_COLOR = SynchedEntityData::defineId<unsigned char>(ENTITY_TYPE_SHULKER, 0x0d580000ULL, EntityDataSerializers::BYTE);
}

MobGroupData* Shulker::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& name)
{
    setRawPeekAmount(0);
    return Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
}

void Shulker::registerGoals()
{
    mGoalSelector.addGoal(1, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(4, new ShulkerAttackGoal(this), true);
    mGoalSelector.addGoal(7, new ShulkerPeekGoal(this), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(2, new ShulkerNearestAttackGoal(this), true);
    mTargetSelector.addGoal(3, new ShulkerDefenseAttackGoal(this), true);
}

bool Shulker::makeStepSound()
{
    return false;
}

SoundSource::ESoundSource Shulker::getSoundSource()
{
    return SoundSource::eSoundSource_Hostile;
}

const SoundEvent* Shulker::getAmbientSound()
{
    return SoundEvents::SHULKER_AMBIENT;
}

bool Shulker::isClosed()
{
    return getRawPeekAmount() == 0;
}

void Shulker::playAmbientSound()
{
    if (!isClosed())
        Mob::playAmbientSound();
}

const SoundEvent* Shulker::getDeathSound()
{
    return SoundEvents::SHULKER_DEATH;
}

const SoundEvent* Shulker::getHurtSound(const DamageSource* source)
{
    return isClosed() ? SoundEvents::SHULKER_HURT_CLOSED : SoundEvents::SHULKER_HURT;
}

void Shulker::defineSynchedData()
{
    Mob::defineSynchedData();
    if (mEntityData != NULL)
    {
        mEntityData->createDateItem(DATA_ATTACH_FACE, Direction::DOWN);
        mEntityData->createDateItem(DATA_ATTACH_POSITION, BlockPos::ZERO);
        mEntityData->createDateItem(DATA_PEEK, (unsigned char)0);
        mEntityData->createDateItem(DATA_COLOR, (unsigned char)0);
    }
}

void Shulker::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(30.0);
}

BodyControl* Shulker::createBodyControl()
{
    return new ShulkerBodyControl(this);
}

void Shulker::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, ENTITY_TYPE_SHULKER);
}

void Shulker::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);

    if (tag->contains(L"AttachFace") && mEntityData != NULL)
    {
        const Direction* direction = Direction::from3DDataValue(tag->getByte(L"AttachFace"));
        mEntityData->set(DATA_ATTACH_FACE, direction, true);
    }

    if (tag->contains(L"Peek") && mEntityData != NULL)
    {
        mEntityData->set(DATA_PEEK, tag->getByte(L"Peek"), true);
    }

    if (tag->contains(L"Color") && mEntityData != NULL)
    {
        mEntityData->set(DATA_COLOR, tag->getByte(L"Color"), true);
    }

    if (mEntityData != NULL)
    {
        if (tag->contains(L"APX"))
        {
            BlockPos attachPos(tag->getInt(L"APX"), tag->getInt(L"APY"), tag->getInt(L"APZ"));
            mEntityData->set(DATA_ATTACH_POSITION, attachPos, true);
        }
        else
        {
            mEntityData->set(DATA_ATTACH_POSITION, BlockPos::ZERO, true);
        }
    }
}

void Shulker::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);

    if (mEntityData != NULL)
    {
        tag->putByte(L"AttachFace", (unsigned char)getAttachFace()->get3DDataValue());
        tag->putByte(L"Peek", getRawPeekAmount());
        tag->putByte(L"Color", mEntityData->get(DATA_COLOR));
    }

    BlockPos* attachPos = getAttachPosition();
    if (attachPos != NULL)
    {
        tag->putInt(L"APX", attachPos->x);
        tag->putInt(L"APY", attachPos->y);
        tag->putInt(L"APZ", attachPos->z);
    }
}

void Shulker::tick()
{
    Mob::tick();

    BlockPos attachPos;
    bool hasAttachPos = false;
    if (mAttachPosition != NULL)
    {
        attachPos = *mAttachPosition;
        hasAttachPos = true;
    }
    else if (mEntityData != NULL)
    {
        attachPos = mEntityData->get(DATA_ATTACH_POSITION);
        hasAttachPos = true;
    }
    if (mLevel != NULL && !mLevel->mIsClientSide && mAttachPosition == NULL)
    {
        setAttachPosition(new BlockPos(this));
        attachPos = *mAttachPosition;
        hasAttachPos = true;
    }

    if (isAlive())
    {
        if (mLevel != NULL && !mLevel->mIsClientSide && hasAttachPos)
        {
            const BlockState* state = mLevel->getBlockState(attachPos);
            Block* block = state != NULL ? state->getBlock() : NULL;
            if (block == Blocks::AIR)
            {
                const Direction* attachFace = getAttachFace();
                BlockPos front = attachPos.relative(attachFace);
                if (!mLevel->isSolidBlockingBlockInLoadedChunk(front, false))
                {
                    for (unsigned int i = 0; i < Direction::VALUES.size(); ++i)
                    {
                        const Direction* direction = Direction::VALUES[i];
                        BlockPos candidate = attachPos.relative(direction);
                        if (mLevel->isSolidBlockingBlockInLoadedChunk(candidate, false))
                        {
                            if (mEntityData != NULL)
                                mEntityData->set(DATA_ATTACH_FACE, direction, true);
                            break;
                        }
                    }
                }
            }
            else if (block == Blocks::piston_head)
            {
                const Direction* facing = state->getValue(PistonHeadBlock::FACING);
                BlockPos moved = attachPos.relative(facing);
                if (mLevel->isEmptyBlock(moved))
                {
                    setAttachPosition(new BlockPos(moved));
                    attachPos = moved;
                }
                else
                {
                    const Direction* attachFace = getAttachFace();
                    if (!mLevel->isSolidBlockingBlockInLoadedChunk(attachPos.relative(attachFace), false))
                    {
                        for (unsigned int i = 0; i < Direction::VALUES.size(); ++i)
                        {
                            const Direction* direction = Direction::VALUES[i];
                            if (mLevel->isSolidBlockingBlockInLoadedChunk(attachPos.relative(direction), false))
                            {
                                if (mEntityData != NULL)
                                    mEntityData->set(DATA_ATTACH_FACE, direction, true);
                                break;
                            }
                        }
                    }
                }
            }
            else if (block == Blocks::piston || block == Blocks::sticky_piston)
            {
                const Direction* facing = state->getValue(PistonBaseBlock::FACING);
                BlockPos moved = attachPos.relative(facing);
                if (mLevel->isEmptyBlock(moved))
                {
                    setAttachPosition(new BlockPos(moved));
                    attachPos = moved;
                }
                else
                {
                    const Direction* attachFace = getAttachFace();
                    if (!mLevel->isSolidBlockingBlockInLoadedChunk(attachPos.relative(attachFace), false))
                    {
                        for (unsigned int i = 0; i < Direction::VALUES.size(); ++i)
                        {
                            const Direction* direction = Direction::VALUES[i];
                            if (mLevel->isSolidBlockingBlockInLoadedChunk(attachPos.relative(direction), false))
                            {
                                if (mEntityData != NULL)
                                    mEntityData->set(DATA_ATTACH_FACE, direction, true);
                                break;
                            }
                        }
                    }
                }
            }
            else if (block == Blocks::piston_extension)
            {
                const Direction* facing = state->getValue(PistonMagicBlock::FACING);
                BlockPos moved = attachPos.relative(facing);
                if (mLevel->isEmptyBlock(moved))
                {
                    setAttachPosition(new BlockPos(moved));
                    attachPos = moved;
                }
                else
                {
                    const Direction* attachFace = getAttachFace();
                    if (!mLevel->isSolidBlockingBlockInLoadedChunk(attachPos.relative(attachFace), false))
                    {
                        for (unsigned int i = 0; i < Direction::VALUES.size(); ++i)
                        {
                            const Direction* direction = Direction::VALUES[i];
                            if (mLevel->isSolidBlockingBlockInLoadedChunk(attachPos.relative(direction), false))
                            {
                                if (mEntityData != NULL)
                                    mEntityData->set(DATA_ATTACH_FACE, direction, true);
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                const Direction* attachFace = getAttachFace();
                if (!mLevel->isSolidBlockingBlockInLoadedChunk(attachPos.relative(attachFace), false))
                {
                    for (unsigned int i = 0; i < Direction::VALUES.size(); ++i)
                    {
                        const Direction* direction = Direction::VALUES[i];
                        if (mLevel->isSolidBlockingBlockInLoadedChunk(attachPos.relative(direction), false))
                        {
                            if (mEntityData != NULL)
                                mEntityData->set(DATA_ATTACH_FACE, direction, true);
                            break;
                        }
                    }
                }
            }

            const Direction* attachFace = getAttachFace();
            BlockPos behind = attachPos.relative(attachFace->getOpposite());
            if (!mLevel->isSolidBlockingBlockInLoadedChunk(behind, false))
                hurt(DamageSource::IN_WALL, 1.0f);
        }
    }
    else
    {
        boost::shared_ptr<LivingEntity> target = getTarget();
        if (target)
        {
            mXOld = target->mX;
            mYOld = target->mY;
            mZOld = target->mZ;
        }
        mClientSideTeleportInterpolation = 0;
    }

    float targetPeek = (float)getRawPeekAmount() * 0.01f;
    mOldPeekAmount = mPeekAmount;
    if (targetPeek < mPeekAmount)
        mPeekAmount = Mth::clamp(mPeekAmount - 0.05f, targetPeek, 1.0f);
    else if (mPeekAmount < targetPeek)
        mPeekAmount = Mth::clamp(mPeekAmount + 0.05f, 0.0f, targetPeek);

    if (hasAttachPos && mLevel != NULL)
    {
        if (mLevel->mIsClientSide)
        {
            if (mClientSideTeleportInterpolation < 1 || mOldAttachPosition == NULL)
            {
                delete mOldAttachPosition;
                mOldAttachPosition = new BlockPos(attachPos);
            }
            else
                --mClientSideTeleportInterpolation;
        }

        double x = (double)attachPos.x + 0.5;
        double y = (double)attachPos.y + 0.5;
        double z = (double)attachPos.z + 0.5;
        mX = x;
        mY = y;
        mZ = z;
        mXOld = x;
        mYOld = y;
        mZOld = z;

        const float pi = 3.1415927f;
        double peekOffset = -(double)(Mth::sin((mPeekAmount + 0.5f) * pi) * 0.5f - 0.5f);
        double oldPeekOffset = -(double)(Mth::sin((mOldPeekAmount + 0.5f) * pi) * 0.5f - 0.5f);
        double pushX = 0.0;
        double pushY = 0.0;
        double pushZ = 0.0;
        double pushAmount = oldPeekOffset - peekOffset;

        const Direction* attachFace = getAttachFace();
        switch (attachFace->get3DDataValue())
        {
        case Direction::eDirection_Down:
            setBoundingBox(AABB::newTemp(x - 0.5, y - 0.5, z - 0.5, x + 0.5, y + 1.0 + peekOffset, z + 0.5));
            pushY = pushAmount;
            break;
        case Direction::eDirection_Up:
            setBoundingBox(AABB::newTemp(x - 0.5, y - peekOffset - 0.5, z - 0.5, x + 0.5, y + 0.5, z + 0.5));
            pushY = -pushAmount;
            break;
        case Direction::eDirection_North:
            setBoundingBox(AABB::newTemp(x - 0.5, y - 0.5, z - 0.5, x + 0.5, y + 0.5, z + 0.5 + peekOffset));
            pushZ = pushAmount;
            break;
        case Direction::eDirection_South:
            setBoundingBox(AABB::newTemp(x - 0.5, y - 0.5, z - peekOffset - 0.5, x + 0.5, y + 0.5, z + 0.5));
            pushZ = -pushAmount;
            break;
        case Direction::eDirection_West:
            setBoundingBox(AABB::newTemp(x - 0.5, y - 0.5, z - 0.5 - peekOffset, x + 0.5, y + 0.5, z + 0.5));
            pushX = pushAmount;
            break;
        case Direction::eDirection_East:
            setBoundingBox(AABB::newTemp(x - peekOffset - 0.5, y - 0.5, z - 0.5, x + 0.5, y + 0.5, z + 0.5));
            pushX = -pushAmount;
            break;
        default:
            break;
        }

        if (pushAmount > 0.0)
        {
            std::vector<boost::shared_ptr<Entity> > entities;
            mLevel->getEntities(shared_from_this(), mBoundingBox, &entities);
            for (unsigned int i = 0; i < entities.size(); ++i)
            {
                boost::shared_ptr<Entity> entity = entities[i];
                if (entity == NULL || entity->GetType() == ENTITY_TYPE_SHULKER || entity->mNoPhysics)
                    continue;
                entity->push(pushX, pushY, pushZ);
            }
        }
    }
}

void Shulker::move(MoverType type, double xa, double ya, double za, bool onGround)
{
    if (type == eMoverType_ShulkerBox)
    {
        teleportSomewhere();
        return;
    }
    Entity::move(type, xa, ya, za, onGround);
}

void Shulker::setPos(double x, double y, double z)
{
    Entity::setPos(x, y, z);

    if (mEntityData != NULL && mLevel != NULL)
    {
        BlockPos pos(x, y, z);
        BlockPos currentAttachPos = mEntityData->get(DATA_ATTACH_POSITION);
        if (!pos.equals(currentAttachPos))
        {
            mEntityData->set(DATA_ATTACH_POSITION, pos, true);
            mEntityData->set(DATA_PEEK, (unsigned char)0, true);
            mHasImpulse = true;
        }
    }
}

double Shulker::getRidingHeight()
{
    return 0.3;
}

bool Shulker::teleportSomewhere()
{
    if (isNoAi() || !isAlive())
        return true;

    BlockPos origin(this);
    for (int attempt = 0; attempt < 5; ++attempt)
    {
        int rx = mRandom->nextInt(17);
        int ry = mRandom->nextInt(17);
        int rz = mRandom->nextInt(17);
        BlockPos candidate = origin.offset(8 - rx, 8 - ry, 8 - rz);

        bool foundSpot = false;
        if (candidate.y > 0 && mLevel->isEmptyBlock(candidate))
        {
            if (mLevel->isInsideWorldBorder(shared_from_this()))
            {
                std::vector<AABB*>* boxes = mLevel->getCollisionAABBs(shared_from_this(), AABB::newTemp(candidate), false, false, false);
                if (boxes->empty())
                    foundSpot = true;
            }
        }

        if (foundSpot)
        {
            for (unsigned int i = 0; i < Direction::VALUES.size(); ++i)
            {
                const Direction* direction = Direction::VALUES[i];
                BlockPos adjacent = candidate.relative(direction);
                if (mLevel->isSolidBlockingBlockInLoadedChunk(adjacent, false))
                {
                    mEntityData->set(DATA_ATTACH_FACE, direction, true);
                    playSound(SoundEvents::SHULKER_TELEPORT, 1.0f, 1.0f);
                    mEntityData->set(DATA_ATTACH_POSITION, candidate, true);
                    mEntityData->set(DATA_PEEK, (unsigned char)0, true);
                    setTarget(boost::shared_ptr<LivingEntity>());
                    return true;
                }
            }
        }
    }
    return false;
}

void Shulker::aiStep()
{
    Mob::aiStep();
    mXd = 0.0;
    mYd = 0.0;
    mZd = 0.0;
    mYHeadRot = 180.0f;
    mYBodyRot = 180.0f;
    mYHeadRotO = 180.0f;
}

void Shulker::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (DATA_ATTACH_POSITION != NULL && accessor == DATA_ATTACH_POSITION
        && mLevel != NULL && mLevel->mIsClientSide && isAlive())
    {
        BlockPos attachPosValue;
        BlockPos* attachPos = getAttachPosition();
        if (attachPos == NULL && mEntityData != NULL)
        {
            attachPosValue = mEntityData->get(DATA_ATTACH_POSITION);
            attachPos = &attachPosValue;
        }

        if (attachPos != NULL)
        {
            double x = (double)attachPos->x + 0.5;
            double y = (double)attachPos->y - 0.5;
            double z = (double)attachPos->z + 0.5;

            if (mAttachPosition == NULL)
                mAttachPosition = new BlockPos(*attachPos);
            else
                mClientSideTeleportInterpolation = 6;

            mX = x;
            mY = y;
            mZ = z;
            mXOld = x;
            mYOld = y;
            mZOld = z;
            mXo = x;
            mYo = y;
            mZo = z;
        }
    }

    LivingEntity::onSyncedDataUpdated(accessor);
}

bool Shulker::hurt(const DamageSource* source, float damage)
{
    if (isClosed())
    {
        boost::shared_ptr<Entity> attacker = source->getDirectEntity();
        if (attacker != NULL && (attacker->GetType() & 0x8000000000000000ULL) != 0)
            return false;
    }

    if (!LivingEntity::hurt(source, damage))
        return false;

    if (getHealth() < getMaxHealth() * 0.5f && mRandom->nextInt(4) == 0)
        teleportSomewhere();

    return true;
}

AABB* Shulker::getCollideBox()
{
    return isAlive() ? mBoundingBox : NULL;
}

const Direction* Shulker::getAttachFace()
{
    return mEntityData != NULL ? mEntityData->get(DATA_ATTACH_FACE) : Direction::DOWN;
}

BlockPos* Shulker::getAttachPosition()
{
    if (mEntityData == NULL)
        return mAttachPosition;

    if (mAttachPosition == NULL)
        mAttachPosition = new BlockPos(mEntityData->get(DATA_ATTACH_POSITION));

    return mAttachPosition;
}

void Shulker::setAttachPosition(BlockPos* position)
{
    if (mEntityData != NULL)
        mEntityData->set(DATA_ATTACH_POSITION, position != NULL ? *position : BlockPos::ZERO, true);
    delete mAttachPosition;
    mAttachPosition = position != NULL ? new BlockPos(*position) : NULL;
}

unsigned char Shulker::getRawPeekAmount()
{
    return mEntityData != NULL ? mEntityData->get(DATA_PEEK) : 0;
}

void Shulker::setRawPeekAmount(int amount)
{
    mEntityData->set(DATA_PEEK, (unsigned char)amount, true);
}

double Shulker::getClientPeekAmount(float partialTick)
{
    return (double)(mOldPeekAmount + (mPeekAmount - mOldPeekAmount) * partialTick);
}

int Shulker::getClientSideTeleportInterpolation()
{
    return mClientSideTeleportInterpolation;
}

BlockPos* Shulker::getOldAttachPosition()
{
    return mOldAttachPosition;
}

double Shulker::getEyeHeight()
{
    return 0.5;
}

int Shulker::getMaxHeadXRot()
{
    return 180;
}

int Shulker::getMaxHeadYRot()
{
    return 180;
}

void Shulker::push(boost::shared_ptr<Entity> entity)
{
}

double Shulker::getPickRadius()
{
    return 0.0;
}

bool Shulker::hasValidInterpolationPositions()
{
    return mOldAttachPosition != NULL && getAttachPosition() != NULL;
}

ResourceLocation* Shulker::getDefaultLootTable()
{
    return &BuiltInLootTables::SHULKER;
}

const DyeColor* Shulker::getColor()
{
    return DyeColor::byBlockData(mEntityData != NULL ? mEntityData->get(DATA_COLOR) : 0);
}

void Shulker::setColor(const DyeColor* color)
{
    if (mEntityData != NULL && color != NULL)
    {
        mEntityData->set(DATA_COLOR, static_cast<unsigned char>(color->getBlockData()), true);
    }
}

unsigned int Shulker::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (Item::getId(holder.m_pItemInstance->getItem()) != 0x17f)
        return Mob::GetInteractTooltip(holder);
    return 0xffffffffU;
}

unsigned long long Shulker::GetType()
{
    return ENTITY_TYPE_SHULKER;
}
