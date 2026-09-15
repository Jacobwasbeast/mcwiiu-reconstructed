#include "game/entity/mob/Zombie.hpp"

#include <boost/shared_ptr.hpp>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/Attribute.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Creeper.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/Mth.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/FindMountGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/MoveTowardsRestrictionGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/SinkInWaterRandomStrollGoal.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/core/Drowned.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MoveThroughVillageGoal.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/ZombieVillager.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

static const EntityDataAccessor<bool>* gZombieBabyId = NULL;
static const EntityDataAccessor<bool>* gZombieAggressiveId = NULL;
static const EntityDataAccessor<bool>* gZombieJockeyId = NULL;
static const EntityDataAccessor<bool>* gZombieConvertingId = NULL;

Zombie::ZombieGroupData::ZombieGroupData(bool canBreakDoors, bool isBaby, bool isJockey)
    : mCanBreakDoors(canBreakDoors), mIsBaby(isBaby), mIsJockey(isJockey)
{
}

Zombie::ZombieGroupData::~ZombieGroupData()
{
}

Zombie::Zombie(Level* level)
    : Monster(level), mBreakDoorGoal(NULL), mCanBreakDoors(false), mBaseWidth(0.6f),
      mBaseHeight(1.95f), mIsJockey(false), mConversionTime(0), mWaterConversionTime(0)
{
    fjDerivedCtorCalls();
    _init();
}

Zombie::Zombie(Level* level, bool initializeGoals)
    : Monster(level), mBreakDoorGoal(NULL), mCanBreakDoors(false), mBaseWidth(0.6f),
      mBaseHeight(1.95f), mIsJockey(false), mConversionTime(0), mWaterConversionTime(0)
{
    _init();
    if (initializeGoals)
        fjDerivedCtorCalls();
}

Zombie::~Zombie()
{
    delete mBreakDoorGoal;
}

void Zombie::create(Level* level)
{
    new Zombie(level);
}

void Zombie::staticCtor()
{
    gZombieBabyId = SynchedEntityData::defineId<bool>(0x5D550040ULL, 0x4D540000ULL, EntityDataSerializers::BOOLEAN);
    gZombieAggressiveId = SynchedEntityData::defineId<bool>(0x5D550040ULL, 0x4D540000ULL, EntityDataSerializers::BOOLEAN);
    gZombieConvertingId = SynchedEntityData::defineId<bool>(0x5D550040ULL, 0x4D540000ULL, EntityDataSerializers::BOOLEAN);
}

void Zombie::_init()
{
    mCanBreakDoors = false;
    mBaseWidth = 0.6f;
    mBaseHeight = 1.95f;
    mIsJockey = false;
    mConversionTime = 0;
    mWaterConversionTime = 0;
}

void Zombie::registerGoals()
{
    mGoalSelector.addGoal(2, new MeleeAttackGoal(this, 1.0, false), true);
    mGoalSelector.addGoal(5, new MoveTowardsRestrictionGoal(this, 1.0), true);
    mGoalSelector.addGoal(7, new SinkInWaterRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
    addBehaviourGoals();
}

void Zombie::addBehaviourGoals()
{
    mGoalSelector.addGoal(6, new MoveThroughVillageGoal(this, 1.0, false), true);
    mGoalSelector.addGoal(1, new FindMountGoal(*this, 16.0f, -1, false, false, -1.0f), true);
    arrayWithLength<unsigned long long> entityTypes(1, true);
    entityTypes.data[0] = 0x5d550041ULL;
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true, entityTypes), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Drowned), false, false, false), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(AbstractSkeleton), true, false, false), true);
    mTargetSelector.addGoal(5, new NearestAttackableTargetGoal(this, typeid(LivingEntity), 10, true, false, NULL, false, false), true);
}

void Zombie::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.23);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(3.0);
    getAttribute(SharedMonsterAttributes::ATTACK_SPEED)->setBaseValue(2.0);
    getAttributes()->getInstance(eATTRIBUTE_ID_ZombieSpawnReinforcements)
        ->setBaseValue(mRandom->nextDouble() * 0.145);
}

void Zombie::defineSynchedData()
{
    Monster::defineSynchedData();
    if (mEntityData && gZombieBabyId)
        mEntityData->createDateItem(gZombieBabyId, false);
    if (mEntityData && gZombieAggressiveId)
        mEntityData->createDateItem(gZombieAggressiveId, false);
    if (mEntityData && gZombieJockeyId)
        mEntityData->createDateItem(gZombieJockeyId, false);
    if (mEntityData && gZombieConvertingId)
        mEntityData->createDateItem(gZombieConvertingId, false);
}

void Zombie::setAggressive(bool aggressive)
{
    getEntityData()->set(gZombieAggressiveId, aggressive, true);
}

bool Zombie::isAggressive()
{
    return getEntityData()->get(gZombieAggressiveId);
}

bool Zombie::canBreakDoors()
{
    return mCanBreakDoors;
}

void Zombie::setCanBreakDoors(bool canBreakDoors)
{
    if (mCanBreakDoors != canBreakDoors)
    {
        mCanBreakDoors = canBreakDoors;
        static_cast<GroundPathNavigation*>(getNavigation())->setCanOpenDoors(canBreakDoors);
        if (canBreakDoors)
            mGoalSelector.addGoal(1, mBreakDoorGoal, false);
        else
            mGoalSelector.removeGoal(mBreakDoorGoal);
    }
}

bool Zombie::isBaby()
{
    return getEntityData()->get(gZombieBabyId);
}

bool Zombie::isJockey()
{
    return mIsJockey;
}

int Zombie::getExperienceReward(boost::shared_ptr<Player> player)
{
    int reward = Monster::getExperienceReward(player);
    if (isBaby())
        reward = (int)((float)reward * 2.5f);
    return reward;
}

void Zombie::setBaby(bool baby)
{
    getEntityData()->set(gZombieBabyId, baby, true);
    internalSetSize(baby ? 0.5f : 1.0f);
}

void Zombie::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (gZombieBabyId && accessor && accessor->getId() == gZombieBabyId->getId())
        setSize(0.6f, 1.95f);
    LivingEntity::onSyncedDataUpdated(accessor);
}

void Zombie::aiStep()
{
    if (!mLevel->mIsClientSide && mLevel->isDay() && isSunSensitive() && getBrightness() > 0.5f &&
        mRandom->nextFloat() * 30.0f < (getBrightness() - 0.4f) * 2.0f)
    {
        BlockPos pos(mX, mY + getRidingHeight(), mZ);
        if (mLevel->canSeeSky(pos))
        {
            not_null_ptr<ItemInstance> helmet = getItemSlot(EquipmentSlot::HEAD);
            if (helmet->isEmpty())
                setSecondsOnFire(8);
            else if (helmet->isDamageableItem())
            {
                helmet->setAuxValue(helmet->getDamageValue() + mRandom->nextInt(2));
                if (helmet->getDamageValue() >= helmet->getMaxDamage())
                    setItemSlot(EquipmentSlot::HEAD, not_null_ptr<ItemInstance>(new ItemInstance()));
            }
        }
    }
    Monster::aiStep();
    if (isBaby())
        setSpeed(0.5f);
    else
        setSpeed(1.0f);
}

bool Zombie::hurt(const DamageSource* source, float damage)
{
    if (!Monster::hurt(source, damage))
        return false;

    boost::shared_ptr<LivingEntity> attacker = getTarget();
    if (!attacker)
    {
        boost::shared_ptr<Entity> entity = source->getEntity();
        if (Entity::instanceof(entity, ENTITY_TYPE_LIVING_ENTITY))
            attacker = boost::static_pointer_cast<LivingEntity>(entity);
    }

    if (attacker && mLevel->getDifficulty() == Difficulty::NORMAL
        && mRandom->nextFloat() < (float)getAttributes()->getInstance(eATTRIBUTE_ID_ZombieSpawnReinforcements)->getValue()
        && mLevel->getGameRules()->getBoolean(3))
    {
        int blockX = Mth::floor(mX);
        int blockY = Mth::floor(mY);
        int blockZ = Mth::floor(mZ);
        if ((double)blockX - 176.0 > mX)
            --blockX;
        if ((double)blockY - 176.0 > mY)
            --blockY;
        if ((double)blockZ - 176.0 > mZ)
            --blockZ;

        boost::shared_ptr<Zombie> reinforcement(new Zombie(mLevel));
        for (int attempt = 0; attempt < 50; ++attempt)
        {
            int offsetX = Mth::nextInt(mRandom, 7, 40) * Mth::nextInt(mRandom, -1, 1);
            int offsetY = Mth::nextInt(mRandom, 7, 40) * Mth::nextInt(mRandom, -1, 1);
            int offsetZ = Mth::nextInt(mRandom, 7, 40) * Mth::nextInt(mRandom, -1, 1);
            int spawnX = blockX + offsetX;
            int spawnY = blockY + offsetY;
            int spawnZ = blockZ + offsetZ;
            BlockPos groundPos(spawnX, spawnY - 1, spawnZ);
            BlockPos spawnPos(spawnX, spawnY, spawnZ);
            if (!mLevel->getBlockState(groundPos)->isTopSolidBlocking())
                continue;
            if (mLevel->getRawBrightness(spawnPos) > 9)
                continue;
            reinforcement->setPos((double)spawnX, (double)spawnY, (double)spawnZ);
            if (mLevel->hasNearPlayer((double)spawnX, (double)spawnY, (double)spawnZ, 7.0))
                continue;
            if (!mLevel->isUnobstructed(reinforcement->mBoundingBox, reinforcement))
                continue;
            std::vector<AABB*>* collisions =
                mLevel->getCollisionAABBs(reinforcement, reinforcement->mBoundingBox, false, false, false);
            bool noCollisions = collisions->empty();
            delete collisions;
            if (!noCollisions)
                continue;
            if (mLevel->containsAnyLiquid(reinforcement->mBoundingBox))
                continue;

            mLevel->addEntity(reinforcement);
            reinforcement->setTarget(attacker);
            reinforcement->finalizeMobSpawn(
                mLevel->getCurrentDifficultyAt(BlockPos(reinforcement)), NULL, std::wstring());
            AttributeInstance* spawnChance =
                getAttributes()->getInstance(eATTRIBUTE_ID_ZombieSpawnReinforcements);
            AttributeInstance* reinforcementSpawnChance =
                reinforcement->getAttributes()->getInstance(eATTRIBUTE_ID_ZombieSpawnReinforcements);
            spawnChance->addModifier(new AttributeModifier(-0.05, 0));
            reinforcementSpawnChance->addModifier(new AttributeModifier(-0.05, 0));
            break;
        }
    }

    return true;
}

bool Zombie::doHurtTarget(boost::shared_ptr<Entity> target)
{
    bool hurt = Monster::doHurtTarget(target);
    if (hurt)
    {
        float difficulty = mLevel->getCurrentDifficultyAt(BlockPos(this)).getEffectiveDifficulty();
        if (getMainHandItem()->isEmpty() && isOnFire() && mRandom->nextFloat() < difficulty * 0.3f)
        {
            target->setSecondsOnFire((int)difficulty << 1);
        }
    }
    return hurt;
}

bool Zombie::isSunSensitive() const
{
    return true;
}

const SoundEvent* Zombie::getAmbientSound()
{
    return SoundEvents::ZOMBIE_AMBIENT;
}

const SoundEvent* Zombie::getHurtSound(const DamageSource*)
{
    return SoundEvents::ZOMBIE_HURT;
}

const SoundEvent* Zombie::getDeathSound()
{
    return SoundEvents::ZOMBIE_DEATH;
}

const SoundEvent* Zombie::getStepSound()
{
    return SoundEvents::ZOMBIE_STEP;
}

void Zombie::playStepSound(const BlockPos&, Block*)
{
    playSound(getStepSound(), 0.15f, 1.0f);
}

EMobType Zombie::getMobType()
{
    return MOB_TYPE_UNDEAD;
}

ResourceLocation* Zombie::getDefaultLootTable()
{
    return &BuiltInLootTables::ZOMBIE_EQUIPMENT;
}

ResourceLocation* Zombie::getDefaultGearTable()
{
    return &BuiltInLootTables::ZOMBIE_EQUIPMENT;
}

void Zombie::populateDefaultEquipmentSlots(const DifficultyInstance& difficulty)
{
    Mob::populateDefaultEquipmentSlots(difficulty);
}

void Zombie::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x5d550040ULL);
}

void Zombie::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    if (isBaby())
        tag->putBoolean(std::wstring(L"IsBaby"), true);
    tag->putBoolean(std::wstring(L"CanBreakDoors"), canBreakDoors());
    tag->putBoolean(std::wstring(L"IsJockey"), mIsJockey);
    tag->putInt(std::wstring(L"InWaterTime"), mWaterConversionTime);
    tag->putInt(std::wstring(L"DrownedConversionTime"), mConversionTime);
}

void Zombie::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag->getBoolean(std::wstring(L"IsBaby")))
        setBaby(true);
    setCanBreakDoors(tag->getBoolean(std::wstring(L"CanBreakDoors")));
    mIsJockey = tag->getBoolean(std::wstring(L"IsJockey"));
    mConversionTime = tag->getInt(std::wstring(L"DrownedConversionTime"));
    getEntityData()->set(gZombieConvertingId, mConversionTime != 0, true);
    mWaterConversionTime = tag->getInt(std::wstring(L"InWaterTime"));
}

void Zombie::killed(boost::shared_ptr<LivingEntity> entity)
{
    Entity::killed(entity);

    if (mLevel->getDifficulty() == Difficulty::NORMAL || mLevel->getDifficulty() == Difficulty::HARD)
    {
        if (entity->GetType() == 0x0D600001ULL && mLevel->canCreateMore(0x5D550040ULL, 0) &&
            (mLevel->getDifficulty() == Difficulty::HARD || !mRandom->nextBoolean()))
        {
            boost::shared_ptr<Villager> villager = boost::static_pointer_cast<Villager>(entity);
            boost::shared_ptr<ZombieVillager> zombieVillager(new ZombieVillager(mLevel));

            zombieVillager->copyPosition(villager);
            mLevel->removeEntity(villager);

            DifficultyInstance difficulty = mLevel->getCurrentDifficultyAt(BlockPos(villager));
            ZombieGroupData* group = new ZombieGroupData(false, false, true);
            zombieVillager->finalizeMobSpawn(difficulty, group, std::wstring());

            zombieVillager->setProfession(villager->getProfession());
            zombieVillager->setBaby(villager->isBaby());
            zombieVillager->setNoAi(villager->isNoAi());
            if (villager->hasCustomName())
            {
                zombieVillager->setCustomName(villager->getCustomName(), villager->getOwnerUUID(),
                                               wasNamedByRestrictedPlayer());
                zombieVillager->setCustomNameVisible(villager->isCustomNameVisible());
            }

            mLevel->addEntity(zombieVillager);
            mLevel->levelEvent(boost::shared_ptr<Player>(), 0x402, BlockPos(this), 0);
        }
    }
}

double Zombie::getEyeHeight()
{
    return isBaby() ? 0.93 : 1.74;
}

void Zombie::positionRider(const boost::shared_ptr<Entity>& entity)
{
    if (!hasPassenger(entity))
        return;

    if (RiderPositionOverrides::OverridePosition(shared_from_this(), entity))
        return;

    float angle = mYRot * 0.01745329238474369f;
    double sin = Mth::sin(angle);
    double cos = Mth::cos(angle);
    entity->setPos(mX + 0.35 * sin, mY + 1.3, mZ - 0.35 * cos);
}

bool Zombie::breathesWater()
{
    return true;
}

bool Zombie::canHoldItem(not_null_ptr<ItemInstance> item)
{
    return !item->isEmpty() && !isPassenger();
}

MobGroupData* Zombie::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                       const std::wstring& dataString)
{
    data = Mob::finalizeMobSpawn(difficulty, data, std::wstring());

    float specialMultiplier = difficulty.getSpecialMultiplier();
    setCanPickUpLoot(mRandom->nextFloat() < specialMultiplier * 0.55f);
    setCanBreakDoors(mRandom->nextFloat() < specialMultiplier * 0.1f);

    populateDefaultEquipmentSlots(difficulty);
    populateDefaultEquipmentEnchantments(difficulty);

    return data;
}

bool Zombie::isConverting()
{
    return getEntityData()->get(gZombieConvertingId);
}

void Zombie::startConverting(int conversionTime)
{
    mConversionTime = conversionTime;
    getEntityData()->set(gZombieConvertingId, true, true);
    mLevel->broadcastEntityEvent(shared_from_this(), 0x10, 0);
}

void Zombie::waterCheck()
{
    if (isInWater())
        ++mWaterConversionTime;
    else
        mWaterConversionTime = 0;
    if (!isConverting() && mWaterConversionTime > 599)
        startConverting(300);
}

void Zombie::tick()
{
    waterCheck();
    Monster::tick();
}

void Zombie::setSize(float width, float height)
{
    bool initialized = mBaseWidth > 0.0f && mBaseHeight > 0.0f;
    mBaseWidth = width;
    mBaseHeight = height;
    if (!initialized)
        internalSetSize(1.0f);
}

void Zombie::internalSetSize(float scale)
{
    Entity::setSize(mBaseWidth * scale, mBaseHeight * scale);
}

not_null_ptr<ItemInstance> Zombie::getSkull()
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 2));
}

double Zombie::getRidingHeight()
{
    return isBaby() ? 0.6 : 1.0;
}

void Zombie::die(const DamageSource* source)
{
    LivingEntity::die(source);

    boost::shared_ptr<Entity> entity = source->getEntity();
    if (!entity || entity->GetType() != 0x5D540041ULL)
        return;

    boost::shared_ptr<Creeper> creeper = boost::static_pointer_cast<Creeper>(entity);
    if (!creeper->isPowered() || !creeper->canDropMobsSkull())
        return;

    creeper->increaseDroppedSkulls();
    not_null_ptr<ItemInstance> skull = getSkull();
    if (!skull->isEmpty())
        spawnAtLocation(skull, 1.65f);
}

bool Zombie::startRiding(boost::shared_ptr<Entity> vehicle, bool force)
{
    return Entity::startRiding(vehicle, force);
}

void Zombie::finishConversion()
{
    boost::shared_ptr<Drowned> drowned(new Drowned(mLevel));
    drowned->copyPosition(shared_from_this());

    if (isBaby())
    {
        ZombieGroupData* group = new ZombieGroupData(true, isJockey(), false);
        DifficultyInstance difficulty = mLevel->getCurrentDifficultyAt(BlockPos(drowned));
        drowned->finalizeMobSpawn(difficulty, group, std::wstring());
    }
    else
    {
        ZombieGroupData* group = new ZombieGroupData(false, false, false);
        DifficultyInstance difficulty = mLevel->getCurrentDifficultyAt(BlockPos(drowned));
        drowned->finalizeMobSpawn(difficulty, group, std::wstring());
    }

    playSound(SoundEvents::DROWNED_CONVERTED, 1.0f, 1.0f);
    mLevel->removeEntity(shared_from_this());
    dropEquipment(true, 0);
    drowned->setNoAi(isNoAi());
    drowned->setCanPickUpLoot(canPickUpLoot());
    drowned->setCanBreakDoors(canBreakDoors());
    mLevel->addEntity(drowned);
}

unsigned long long Zombie::GetType()
{
    return 0x5D550040ULL;
}
