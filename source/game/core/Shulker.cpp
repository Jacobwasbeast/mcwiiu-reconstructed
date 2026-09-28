#include "game/core/Shulker.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <vector>

#include "game/core/AttributeModifier.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/world/block/Material.hpp"
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
#include "game/item/ItemId.hpp"

const EntityDataAccessor<const Direction*>* Shulker::DATA_ATTACH_FACE = NULL;
const EntityDataAccessor<BlockPos*>* Shulker::DATA_ATTACH_POSITION = NULL;
const EntityDataAccessor<unsigned char>* Shulker::DATA_PEEK = NULL;
const EntityDataAccessor<unsigned char>* Shulker::DATA_COLOR = NULL;
const DyeColor* Shulker::DEFAULT_COLOR = NULL;
AttributeModifier* Shulker::COVERED_ARMOR_MODIFIER =
    (new AttributeModifier(eMODIFIER_ID_ArmorBonus, 20.0, 0))->setSerialize(false);

Shulker::ShulkerAttackGoal::ShulkerAttackGoal(Shulker* shulker)
    : Goal(), mShulker(shulker)
{
    setRequiredControlFlags(3);
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
    return mShulker->mLevel->getDifficulty() != Difficulty::PEACEFUL && NearestAttackableTargetGoal::canUse();
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
    : Golem(level), mOldPeekAmount(0.0f), mPeekAmount(0.0f), mOldAttachPosition(NULL)
{
    fjDerivedCtorCalls();
    setSize(1.0f, 1.0f);
    mOldAttachPosition = NULL;
    mFireImmune = true;
    mYHeadRot = 180.0f;
    mYRotO = 180.0f;
    mYHeadRotO = 180.0f;
    mXRot = 0.0f;
    mXRotO = 0.0f;
    mYBodyRot = 180.0f;
    mYBodyRotO = 180.0f;
    mExperienceReward = 5;
    mYRot = 180.0f;
}

Shulker::~Shulker()
{
}

void Shulker::create(Level* level)
{
    new Shulker(level);
}

void Shulker::staticCtor()
{
    DATA_ATTACH_FACE = SynchedEntityData::defineId<const Direction*>(EntityType::SHULKER, EntityType::GOLEM, EntityDataSerializers::DIRECTION);
    DATA_ATTACH_POSITION = SynchedEntityData::defineId<BlockPos*>(EntityType::SHULKER, EntityType::GOLEM, EntityDataSerializers::OPTIONAL_BLOCK_POS);
    DATA_PEEK = SynchedEntityData::defineId<unsigned char>(EntityType::SHULKER, EntityType::GOLEM, EntityDataSerializers::BYTE);
    DATA_COLOR = SynchedEntityData::defineId<unsigned char>(EntityType::SHULKER, EntityType::GOLEM, EntityDataSerializers::BYTE);
    DEFAULT_COLOR = DyeColor::PURPLE;
}

MobGroupData* Shulker::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& name)
{
    mYBodyRot = 180.0f;
    mYBodyRotO = 180.0f;
    mYRot = 180.0f;
    mYRotO = 180.0f;
    mYHeadRot = 180.0f;
    mYHeadRotO = 180.0f;
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
    mEntityData->createDateItem(DATA_ATTACH_FACE, Direction::DOWN);
    mEntityData->createDateItem(DATA_ATTACH_POSITION, static_cast<BlockPos*>(NULL));
    mEntityData->createDateItem(DATA_PEEK, static_cast<unsigned char>(0));
    mEntityData->createDateItem(DATA_COLOR, static_cast<unsigned char>(DEFAULT_COLOR->getBlockData()));
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
    Mob::addDataWalkers(fixer, EntityType::SHULKER);
}

void Shulker::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag->contains(L"AttachFace"))
        mEntityData->set(DATA_ATTACH_FACE, Direction::from3DDataValue(tag->getByte(L"AttachFace")), true);
    if (tag->contains(L"Color"))
        mEntityData->set(DATA_PEEK, tag->getByte(L"Peek"), true);
    if (tag->contains(L"Color"))
        mEntityData->set(DATA_COLOR, tag->getByte(L"Color"), true);
    if (tag->contains(L"APX"))
    {
        int x = tag->getInt(L"APX");
        int y = tag->getInt(L"APY");
        int z = tag->getInt(L"APZ");
        mEntityData->set(DATA_ATTACH_POSITION, new BlockPos(x, y, z), true);
    }
    else
    {
        mEntityData->set(DATA_ATTACH_POSITION, static_cast<BlockPos*>(NULL), true);
    }
}

void Shulker::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putByte(L"AttachFace", static_cast<unsigned char>(mEntityData->get(DATA_ATTACH_FACE)->get3DDataValue()));
    tag->putByte(L"Peek", mEntityData->get(DATA_PEEK));
    tag->putByte(L"Color", mEntityData->get(DATA_COLOR));
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
    BlockPos* attachPos = mEntityData->get(DATA_ATTACH_POSITION);
    if (attachPos == NULL && !mLevel->mIsClientSide)
    {
        attachPos = new BlockPos(this);
        mEntityData->set(DATA_ATTACH_POSITION, new BlockPos(*attachPos), true);
    }

    if (isPassenger())
    {
        attachPos = NULL;
        float vehicleYRot = getVehicle()->mYRot;
        mYBodyRot = vehicleYRot;
        mYRot = vehicleYRot;
        mYBodyRotO = vehicleYRot;
        mClientSideTeleportInterpolation = 0;
    }
    else if (!mLevel->mIsClientSide)
    {
        const BlockState* state = mLevel->getBlockState(*attachPos);
        if (state->getMaterial() != Material::AIR)
        {
            Block* block = state->getBlock();
            if (block == Blocks::piston_extension)
            {
                const Direction* facing = state->getValue<const Direction*>(PistonMagicBlock::FACING);
                if (mLevel->isEmptyBlock(attachPos->relative(facing)))
                {
                    *attachPos = attachPos->relative(facing);
                    mEntityData->set(DATA_ATTACH_POSITION, new BlockPos(*attachPos), true);
                }
                else
                {
                    teleportSomewhere();
                }
            }
            else if (block == Blocks::piston_head)
            {
                const Direction* facing = state->getValue<const Direction*>(PistonHeadBlock::FACING);
                if (mLevel->isEmptyBlock(attachPos->relative(facing)))
                {
                    *attachPos = attachPos->relative(facing);
                    mEntityData->set(DATA_ATTACH_POSITION, new BlockPos(*attachPos), true);
                }
                else
                {
                    teleportSomewhere();
                }
            }
            else
            {
                teleportSomewhere();
            }
        }

        if (!mLevel->isSolidBlockingBlockInLoadedChunk(attachPos->relative(getAttachFace()), false))
        {
            bool attached = false;
            for (unsigned int i = 0; i < Direction::VALUES.size(); ++i)
            {
                const Direction* direction = Direction::VALUES[i];
                if (mLevel->isSolidBlockingBlockInLoadedChunk(attachPos->relative(direction), false))
                {
                    mEntityData->set(DATA_ATTACH_FACE, direction, true);
                    attached = true;
                    break;
                }
            }
            if (!attached) teleportSomewhere();
        }

        if (mLevel->isSolidBlockingBlockInLoadedChunk(attachPos->relative(getAttachFace()->getOpposite()), false))
            teleportSomewhere();
    }

    float targetPeek = getRawPeekAmount() * 0.01f;
    mOldPeekAmount = mPeekAmount;
    if (mPeekAmount > targetPeek)
        mPeekAmount = Mth::clamp(mPeekAmount - 0.05f, targetPeek, 1.0f);
    else if (mPeekAmount < targetPeek)
        mPeekAmount = Mth::clamp(mPeekAmount + 0.05f, 0.0f, targetPeek);

    if (attachPos == NULL)
        return;

    if (mLevel->mIsClientSide)
    {
        if (mClientSideTeleportInterpolation > 0 && mOldAttachPosition != NULL)
            --mClientSideTeleportInterpolation;
        else
            mOldAttachPosition = attachPos;
    }

    double x = attachPos->x + 0.5;
    double y = attachPos->y;
    double z = attachPos->z + 0.5;
    mX = x;
    mY = y;
    mZ = z;
    mXo = x;
    mYo = y;
    mZo = z;
    mXOld = x;
    mYOld = y;
    mZOld = z;

    double peekOffset = 0.5 - Mth::sin((mPeekAmount + 0.5f) * Mth::PI) * 0.5;
    double oldPeekOffset = 0.5 - Mth::sin((mOldPeekAmount + 0.5f) * Mth::PI) * 0.5;
    double pushAmount = peekOffset - oldPeekOffset;
    double pushX = 0.0;
    double pushY = 0.0;
    double pushZ = 0.0;
    switch (getAttachFace()->get3DDataValue())
    {
    case Direction::eDirection_Down:
        setBoundingBox(AABB::newTemp(mX - 0.5, mY, mZ - 0.5, mX + 0.5, mY + 1.0 + peekOffset, mZ + 0.5));
        pushY = pushAmount;
        break;
    case Direction::eDirection_Up:
        setBoundingBox(AABB::newTemp(mX - 0.5, mY - peekOffset, mZ - 0.5, mX + 0.5, mY + 1.0, mZ + 0.5));
        pushY = -pushAmount;
        break;
    case Direction::eDirection_North:
        setBoundingBox(AABB::newTemp(mX - 0.5, mY, mZ - 0.5, mX + 0.5, mY + 1.0, mZ + 0.5 + peekOffset));
        pushZ = pushAmount;
        break;
    case Direction::eDirection_South:
        setBoundingBox(AABB::newTemp(mX - 0.5, mY, mZ - 0.5 - peekOffset, mX + 0.5, mY + 1.0, mZ + 0.5));
        pushZ = -pushAmount;
        break;
    case Direction::eDirection_West:
        setBoundingBox(AABB::newTemp(mX - 0.5, mY, mZ - 0.5, mX + 0.5 + peekOffset, mY + 1.0, mZ + 0.5));
        pushX = pushAmount;
        break;
    case Direction::eDirection_East:
        setBoundingBox(AABB::newTemp(mX - 0.5 - peekOffset, mY, mZ - 0.5, mX + 0.5, mY + 1.0, mZ + 0.5));
        pushX = -pushAmount;
        break;
    }

    if (pushAmount > 0.0)
    {
        std::vector<boost::shared_ptr<Entity> > entities;
        mLevel->getEntities(shared_from_this(), mBoundingBox, &entities);
        for (unsigned int i = 0; i < entities.size(); ++i)
        {
            boost::shared_ptr<Entity> entity = entities[i];
            if (entity->GetType() != EntityType::SHULKER && !entity->mNoPhysics)
                entity->move(eMoverType_Self, pushX, pushY, pushZ, false);
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
    if (mEntityData != NULL && mTickCount != 0)
    {
        BlockPos* attachPos = mEntityData->get(DATA_ATTACH_POSITION);
        BlockPos* pos = new BlockPos(x, y, z);
        if (!pos->equals(attachPos))
        {
            mEntityData->set(DATA_ATTACH_POSITION, pos, true);
            mEntityData->set(DATA_PEEK, static_cast<unsigned char>(0), true);
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

    BlockPos* origin = new BlockPos(this);
    for (int attempt = 0; attempt < 5; ++attempt)
    {
        int dx = 8 - mRandom->nextInt(17);
        int dy = 8 - mRandom->nextInt(17);
        int dz = 8 - mRandom->nextInt(17);
        BlockPos candidate = origin->offset(dx, dy, dz);
        if (candidate.y > 0 && mLevel->isEmptyBlock(candidate) && mLevel->isInsideWorldBorder(shared_from_this())
            && mLevel->getCollisionAABBs(shared_from_this(), AABB::newTemp(candidate), false, false, false)->empty())
        {
            for (unsigned int i = 0; i < Direction::VALUES.size(); ++i)
            {
                const Direction* direction = Direction::VALUES[i];
                if (mLevel->isSolidBlockingBlockInLoadedChunk(candidate.relative(direction), false))
                {
                    mEntityData->set(DATA_ATTACH_FACE, direction, true);
                    playSound(SoundEvents::SHULKER_TELEPORT, 1.0f, 1.0f);
                    mEntityData->set(DATA_ATTACH_POSITION, new BlockPos(candidate), true);
                    mEntityData->set(DATA_PEEK, static_cast<unsigned char>(0), true);
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
    mYBodyRotO = 180.0f;
    mYBodyRot = 180.0f;
    mYRot = 180.0f;
}

void Shulker::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (accessor == DATA_ATTACH_POSITION && mLevel->mIsClientSide && !isPassenger())
    {
        BlockPos* attachPos = getAttachPosition();
        if (attachPos != NULL)
        {
            if (mOldAttachPosition == NULL)
                mOldAttachPosition = attachPos;
            else
                mClientSideTeleportInterpolation = 6;
            double x = attachPos->x + 0.5;
            double y = attachPos->y;
            double z = attachPos->z + 0.5;
            mX = x;
            mY = y;
            mZ = z;
            mXo = x;
            mYo = y;
            mZo = z;
            mXOld = x;
            mYOld = y;
            mZOld = z;
        }
    }
    LivingEntity::onSyncedDataUpdated(accessor);
}

bool Shulker::hurt(const DamageSource* source, float damage)
{
    if (isClosed())
    {
        boost::shared_ptr<Entity> attacker = source->getDirectEntity();
        if (attacker != NULL && (attacker->GetType() & EntityType::PROJECTILE) == EntityType::PROJECTILE)
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
    return mEntityData->get(DATA_ATTACH_FACE);
}

BlockPos* Shulker::getAttachPosition()
{
    return mEntityData->get(DATA_ATTACH_POSITION);
}

void Shulker::setAttachPosition(BlockPos* position)
{
    mEntityData->set(DATA_ATTACH_POSITION, position, true);
}

unsigned char Shulker::getRawPeekAmount()
{
    return mEntityData->get(DATA_PEEK);
}

void Shulker::setRawPeekAmount(int amount)
{
    if (!mLevel->mIsClientSide)
    {
        getAttribute(SharedMonsterAttributes::ARMOR)->removeModifier(COVERED_ARMOR_MODIFIER);
        if (amount == 0)
        {
            getAttribute(SharedMonsterAttributes::ARMOR)->addModifier(new AttributeModifier(*COVERED_ARMOR_MODIFIER));
            playSound(SoundEvents::SHULKER_CLOSE, 1.0f, 1.0f);
        }
        else
        {
            playSound(SoundEvents::SHULKER_OPEN, 1.0f, 1.0f);
        }
    }
    mEntityData->set(DATA_PEEK, static_cast<unsigned char>(amount), true);
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

float Shulker::getPickRadius()
{
    return 0.0f;
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
    return DyeColor::byBlockData(mEntityData->get(DATA_COLOR));
}

void Shulker::setColor(const DyeColor* color)
{
    mEntityData->set(DATA_COLOR, static_cast<unsigned char>(color->getBlockData()), true);
}

unsigned int Shulker::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (Item::getId(holder.m_pItemInstance->getItem()) != ItemId::SPAWN_EGG)
        return Mob::GetInteractTooltip(holder);
    return IDS_NONE;
}

unsigned long long Shulker::GetType()
{
    return EntityType::SHULKER;
}
