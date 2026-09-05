#include "game/core/Shulker.hpp"

#include "game/core/AttributeModifier.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

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
    return mShulker != NULL && mShulker->getTarget() && !mShulker->isClosed();
}

void Shulker::ShulkerAttackGoal::start()
{
    mAttackTime = 20;
}

void Shulker::ShulkerAttackGoal::stop()
{
    mAttackTime = 0;
}

void Shulker::ShulkerAttackGoal::tick()
{
    if (mAttackTime > 0)
        --mAttackTime;
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
    return NearestAttackableTargetGoal::getTargetSearchArea(distance);
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
    return NearestAttackableTargetGoal::getTargetSearchArea(distance);
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
    return mShulker != NULL && mShulker->getTarget() == NULL && mPeekTime > 0;
}

void Shulker::ShulkerPeekGoal::start()
{
    mPeekTime = mShulker->mRandom->nextInt(3) * 20 + 20;
    mShulker->setRawPeekAmount(30);
}

void Shulker::ShulkerPeekGoal::stop()
{
    mPeekTime = 0;
    if (mShulker != NULL)
        mShulker->setRawPeekAmount(0);
}

void Shulker::ShulkerPeekGoal::tick()
{
    --mPeekTime;
}

std::wstring Shulker::ShulkerPeekGoal::getName()
{
    return std::wstring(L"ShulkerPeek");
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
    DATA_ATTACH_FACE = SynchedEntityData::defineId<const Direction*>(ENTITY_TYPE_SHULKER, ENTITY_TYPE_SHULKER, EntityDataSerializers::DIRECTION);
    DATA_ATTACH_POSITION = SynchedEntityData::defineId<BlockPos>(ENTITY_TYPE_SHULKER, ENTITY_TYPE_SHULKER, EntityDataSerializers::BLOCK_POS);
    DATA_PEEK = SynchedEntityData::defineId<unsigned char>(ENTITY_TYPE_SHULKER, ENTITY_TYPE_SHULKER, EntityDataSerializers::BYTE);
    DATA_COLOR = SynchedEntityData::defineId<unsigned char>(ENTITY_TYPE_SHULKER, ENTITY_TYPE_SHULKER, EntityDataSerializers::BYTE);
}

MobGroupData* Shulker::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& name)
{
    setRawPeekAmount(0);
    return Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
}

void Shulker::registerGoals()
{
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
    return NULL;
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
    return NULL;
}

const SoundEvent* Shulker::getHurtSound(const DamageSource* source)
{
    return NULL;
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
}

void Shulker::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
}

void Shulker::tick()
{
    mOldPeekAmount = mPeekAmount;
    Mob::tick();
}

void Shulker::move(MoverType type, double xa, double ya, double za, bool onGround)
{
    Entity::move(type, xa, ya, za, onGround);
}

void Shulker::setPos(double x, double y, double z)
{
    Entity::setPos(x, y, z);
}

double Shulker::getRidingHeight()
{
    return 0.3;
}

bool Shulker::teleportSomewhere()
{
    return true;
}

void Shulker::aiStep()
{
    Mob::aiStep();
}

void Shulker::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    LivingEntity::onSyncedDataUpdated(accessor);
}

bool Shulker::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;
    return LivingEntity::hurt(source, damage);
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
    if (mEntityData != NULL)
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

const void* Shulker::getDefaultLootTable()
{
    return NULL;
}

const DyeColor* Shulker::getColor()
{
    return DyeColor::byBlockData(mEntityData != NULL ? mEntityData->get(DATA_COLOR) : 0);
}

unsigned int Shulker::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    return Mob::GetInteractTooltip(holder);
}

unsigned long long Shulker::GetType()
{
    return 0;
}
