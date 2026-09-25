#include "game/entity/mob/Mob.hpp"

#include <cmath>
#include <typeinfo>

#include "game/client/render/ParticleType.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/BodyControl.hpp"
#include "game/core/CensoredString.hpp"
#include "game/core/JumpControl.hpp"
#include "game/core/LootTable.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/core/LootTables.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Path.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/FloatTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/HangingEntity.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/core/Sensing.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/item/Items.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/core/Difficulty.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

namespace
{
not_null_ptr<ItemInstance> sMobEquipmentItems[4][5];
bool sMobEquipmentItemsInitialized = false;

void initMobEquipmentItems()
{
    if (sMobEquipmentItemsInitialized)
        return;
    sMobEquipmentItemsInitialized = true;

    Item* headItems[5] = {Items::leatherHelmet, Items::goldenHelmet, Items::chainmailHelmet,
                          Items::ironHelmet, Items::diamondHelmet};
    Item* chestItems[5] = {Items::leatherChestplate, Items::goldenChestplate,
                           Items::chainmailChestplate, Items::ironChestplate,
                           Items::diamondChestplate};
    Item* legsItems[5] = {Items::leatherLeggings, Items::goldenLeggings, Items::chainmailLeggings,
                          Items::ironLeggings, Items::diamondLeggings};
    Item* feetItems[5] = {Items::leatherBoots, Items::goldenBoots, Items::chainmailBoots,
                          Items::ironBoots, Items::diamondBoots};

    for (int tier = 0; tier < 5; ++tier)
    {
        sMobEquipmentItems[0][tier] = not_null_ptr<ItemInstance>(new ItemInstance(feetItems[tier]));
        sMobEquipmentItems[1][tier] = not_null_ptr<ItemInstance>(new ItemInstance(legsItems[tier]));
        sMobEquipmentItems[2][tier] = not_null_ptr<ItemInstance>(new ItemInstance(chestItems[tier]));
        sMobEquipmentItems[3][tier] = not_null_ptr<ItemInstance>(new ItemInstance(headItems[tier]));
    }
}
}

const EntityDataAccessor<unsigned char>* Mob::DATA_MOB_FLAGS_ID = NULL;

Mob::Mob(Level* level)
    : LivingEntity(level),
      mAmbientSoundTime(0),
      mExperienceReward(0),
      mLookControl(NULL),
      mMoveControl(NULL),
      mJumpControl(NULL),
      mBodyControl(NULL),
      mNavigation(NULL),
      mGoalSelector(),
      mTargetSelector(),
      mTarget(),
      mSensing(NULL),
      mHandItems(),
      mArmorItems(),
      mAdditionalItems(),
      mAdditionalDropChances(NULL),
      mAdditionalItemCount(0),
      mCanPickUpLoot(false),
      mPersistenceRequired(false),
      mPathfindingMalus(),
      mCustomName(),
      mMoveSpeed(0.0f),
      mLootTable(NULL),
      mLootTableSeed(0),
      mLeader(false),
      mFollowsLeader(false),
      mMaxFlockSize(1),
      mFlockSize(0)
{
    _init();
    registerAttributes();
    mLookControl = new LookControl(this);
    mMoveControl = new MoveControl(this);
    mJumpControl = new JumpControl(this);
    mSensing = new Sensing(this);
}

Mob::~Mob()
{
    delete mLookControl;
    delete mMoveControl;
    delete mJumpControl;
    delete mBodyControl;
    delete mNavigation;
    delete mSensing;
    delete[] mAdditionalDropChances;
}

unsigned long long Mob::GetType()
{
    return 0x000000000d000000ULL;
}

void Mob::staticCtor()
{
    DATA_MOB_FLAGS_ID = SynchedEntityData::defineId(ENTITY_TYPE_MOB, ENTITY_TYPE_LIVING_ENTITY,
                                                     EntityDataSerializers::BYTE);
}

void Mob::_init()
{
    mAmbientSoundTime = 0;
    mExperienceReward = 0;
    mTarget.reset();
    mMoveSpeed = 0.0f;
    mHandItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(2, ItemInstance::EMPTY);
    mArmorItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(4, ItemInstance::EMPTY);
    mAdditionalItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(5, ItemInstance::EMPTY);
    mAdditionalItemCount = 5;
    mAdditionalDropChances = new float[mAdditionalItemCount];
    for (unsigned int i = 0; i < mAdditionalItemCount; ++i)
        mAdditionalDropChances[i] = 0.0f;
    mHandDropChances[0] = 0.085f;
    mHandDropChances[1] = 0.085f;
    for (unsigned int i = 0; i < 4; ++i)
        mArmorDropChances[i] = 0.085f;
    mCanPickUpLoot = false;
    mPersistenceRequired = false;
    mCustomName.reset(new CensoredString());
    mLootTable = NULL;
    mLootTableSeed = 0;
    mLeader = false;
    mFollowsLeader = false;
    mMaxFlockSize = 1;
    mFlockSize = 0;
}

void Mob::registerAttributes()
{
    LivingEntity::registerAttributes();
    getAttributes()->registerAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(2.75);
}

void Mob::fjDerivedCtorCalls()
{
    LivingEntity::fjDerivedCtorCalls();
    mBodyControl = createBodyControl();
    mNavigation = createNavigation(mLevel);
    registerGoals();
}

void Mob::defineSynchedData()
{
    LivingEntity::defineSynchedData();
    getEntityData()->createDateItem(DATA_MOB_FLAGS_ID, (unsigned char)0);
}

void Mob::registerGoals()
{
}

PathNavigation* Mob::createNavigation(Level* level)
{
    return new GroundPathNavigation(this, level, true);
}

float Mob::getPathfindingMalus(const BlockPathTypes* type)
{
    boost::unordered_map<const BlockPathTypes*, float>::const_iterator it = mPathfindingMalus.find(type);
    if (it != mPathfindingMalus.end())
        return it->second;
    return type == NULL ? 0.0f : type->getMalus();
}

void Mob::setPathfindingMalus(const BlockPathTypes* type, float malus)
{
    mPathfindingMalus[type] = malus;
}

BodyControl* Mob::createBodyControl()
{
    return new BodyControl(this);
}

LookControl* Mob::getLookControl()
{
    return mLookControl;
}

MoveControl* Mob::getMoveControl()
{
    return mMoveControl;
}

JumpControl* Mob::getJumpControl()
{
    return mJumpControl;
}

PathNavigation* Mob::getNavigation()
{
    return mNavigation;
}

Sensing* Mob::getSensing()
{
    return mSensing;
}

boost::shared_ptr<LivingEntity> Mob::getTarget()
{
    return mTarget.lock();
}

void Mob::setTarget(boost::shared_ptr<LivingEntity> target)
{
    mTarget = target;
}

void Mob::setLevel(Level* level)
{
    Entity::setLevel(level);
    if (mNavigation != NULL)
        mNavigation->setLevel(level);
    mGoalSelector.setLevel(level);
    mTargetSelector.setLevel(level);
}

bool Mob::canAttackType(unsigned long long type)
{
    return type != 0x5D0000C1ULL;
}

void Mob::ate()
{
}

int Mob::getAmbientSoundInterval()
{
    return 80;
}

void Mob::playAmbientSound()
{
    const SoundEvent* sound = getAmbientSound();
    if (sound != NULL)
        playSound(sound, getSoundVolume(), getVoicePitch());
}

void Mob::baseTick()
{
    LivingEntity::baseTick();
    if (!isAlive())
        return;

    int roll = mRandom->nextInt(1000);
    int soundTime = mAmbientSoundTime;
    ++mAmbientSoundTime;
    if (roll < soundTime)
    {
        playAmbientSound();
        resetAmbientSoundTime();
    }
}

void Mob::playHurtSound(const DamageSource*)
{
    resetAmbientSoundTime();
}

void Mob::resetAmbientSoundTime()
{
    mAmbientSoundTime = -getAmbientSoundInterval();
}

int Mob::getExperienceReward(boost::shared_ptr<Player>)
{
    int reward = mExperienceReward;
    if (reward <= 0)
        return reward;

    const float dropChanceLimit = 1.0f;

    if (!mArmorItems[0]->isEmpty() && mArmorDropChances[0] <= dropChanceLimit)
        reward += mRandom->nextInt(3) + 1;
    if (!mArmorItems[1]->isEmpty() && mArmorDropChances[1] <= dropChanceLimit)
        reward += mRandom->nextInt(3) + 1;
    if (!mArmorItems[2]->isEmpty() && mArmorDropChances[2] <= dropChanceLimit)
        reward += mRandom->nextInt(3) + 1;
    if (!mArmorItems[3]->isEmpty() && mArmorDropChances[3] <= dropChanceLimit)
        reward += mRandom->nextInt(3) + 1;
    if (!mHandItems[0]->isEmpty() && mHandDropChances[0] <= dropChanceLimit)
        reward += mRandom->nextInt(3) + 1;
    if (!mHandItems[1]->isEmpty() && mHandDropChances[1] <= dropChanceLimit)
        reward += mRandom->nextInt(3) + 1;

    return reward;
}

void Mob::spawnAnim()
{
    if (!mLevel->mIsClientSide)
    {
        mLevel->broadcastEntityEvent(shared_from_this(), 20, 0);
        return;
    }

    for (int i = 0; i < 10; ++i)
    {
        double gx = mRandom->nextGaussian();
        double gy = mRandom->nextGaussian();
        double gz = mRandom->nextGaussian();
        double px = mX + (mRandom->nextFloat() * mBbWidth + mRandom->nextFloat() * mBbWidth) - mBbWidth;
        double py = mY + mRandom->nextFloat() * (double)mBbHeight;
        double pz = mZ + (mRandom->nextFloat() * mBbWidth + mRandom->nextFloat() * mBbWidth) - mBbWidth;
        mLevel->addParticle(PARTICLE_PORTAL, px, py, pz, gx * 0.02, gy * 0.02, gz * 0.02,
                            arrayWithLength<int>());

        gx = mRandom->nextGaussian();
        gy = mRandom->nextGaussian();
        gz = mRandom->nextGaussian();
        px = mX + (mRandom->nextFloat() * mBbWidth + mRandom->nextFloat() * mBbWidth) - mBbWidth;
        py = mY + mRandom->nextFloat() * (double)mBbHeight;
        pz = mZ + (mRandom->nextFloat() * mBbWidth + mRandom->nextFloat() * mBbWidth) - mBbWidth;
        mLevel->addParticle(PARTICLE_PORTAL, px, py, pz, gx * 0.02, gy * 0.02, gz * 0.02,
                            arrayWithLength<int>());
    }
}

void Mob::handleEntityEvent(unsigned char event, int data)
{
    if (event == 20)
    {
        spawnAnim();
        return;
    }
    LivingEntity::handleEntityEvent(event, data);
}

void Mob::setLeader(bool leader)
{
    mLeader = leader;
}

void Mob::tick()
{
    PIXBeginNamedEvent(4.5f, "Mob tick");
    LivingEntity::tick();
    if (!mLevel->mIsClientSide)
    {
        tickLeash();
        if (mTickCount % 5 == 0)
        {
            boost::shared_ptr<Entity> controllingPassenger = getControllingPassenger();
            bool hasNonMobPassenger = !Entity::instanceof(controllingPassenger, ENTITY_TYPE_MOB);
            boost::shared_ptr<Entity> vehicle = getVehicle();
            bool vehicleIsBoat = Entity::instanceof(vehicle, ENTITY_TYPE_BOAT);
            mGoalSelector.setControlFlag(1, hasNonMobPassenger);
            mGoalSelector.setControlFlag(4, hasNonMobPassenger && !vehicleIsBoat);
            mGoalSelector.setControlFlag(2, hasNonMobPassenger);
        }
        if (mLeader)
        {
            if (mRandom->nextInt(200) == 1)
            {
                AABB* searchBox = mBoundingBox->grow(2.5, 2.5, 2.5);
                std::vector<boost::shared_ptr<Entity> >* entities =
                    mLevel->getEntitiesOfClass(typeid(*this), searchBox);
                if (entities->size() < 2)
                {
                    setLeader(false);
                    mFlockSize = 0;
                }
                delete entities;
            }
        }
        if (isVehicle() && isUnderLiquid(Material::WATER))
            underwaterEjectRider();
    }
    PIXEndNamedEvent();
}

float Mob::tickHeadTurn(float, float headTurnDelta)
{
    if (mLookControl != NULL)
        mLookControl->tick();
    return headTurnDelta;
}

const SoundEvent* Mob::getAmbientSound()
{
    return NULL;
}

Item* Mob::getDeathLoot()
{
    return NULL;
}

void Mob::dropDeathLoot(bool, int lootingLevel)
{
    Item* item = getDeathLoot();
    if (item == NULL)
        return;
    int count = mRandom->nextInt(3);
    if (lootingLevel > 0)
        count += mRandom->nextInt(lootingLevel + 1);
    for (int i = 0; i < count; ++i)
        spawnAtLocation(item, 1);
}

void Mob::dropEquipment(bool hitByPlayer, int lootingLevel)
{
    for (int i = 0; i < 6; ++i)
    {
        const EquipmentSlot* slot = EquipmentSlot::SLOTS[i];
        not_null_ptr<ItemInstance> item = getItemSlot(slot);
        float chance = slot->getType() == EquipmentSlot::HAND
                           ? mHandDropChances[slot->getIndex()]
                           : mArmorDropChances[slot->getIndex()];
        if (item->isEmpty() || EnchantmentHelper::hasVanishingCurse(item))
            continue;
        if (!hitByPlayer && chance <= 0.2f)
            continue;
        if (mRandom->nextFloat() >= chance - lootingLevel * 0.01f)
            continue;
        if (chance > 0.2f && item->isDamageableItem())
        {
            int bound = item->getMaxDamage() - 3;
            if (bound < 1)
                bound = 1;
            int damage = mRandom->nextInt(bound);
            damage = mRandom->nextInt(damage + 1);
            item->setAuxValue(item->getMaxDamage() - damage);
        }
        spawnAtLocation(item, 0.0f);
    }
}

void Mob::addDataWalkers(DataFixerUpper* fixer, unsigned long long entityType)
{
    std::vector<std::wstring> itemNames;
    itemNames.push_back(L"ArmorItems");
    itemNames.push_back(L"HandItems");
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity,
                     (DataWalker*)new ItemListDataWalker(entityType, itemNames));
}

bool Mob::isLeftHanded()
{
    return (getEntityData()->get(DATA_MOB_FLAGS_ID) & 2) != 0;
}

void Mob::addAdditonalSaveData(CompoundTag* tag)
{
    LivingEntity::addAdditonalSaveData(tag);

    tag->putBoolean(L"CanPickUpLoot", canPickUpLoot());
    tag->putBoolean(L"PersistenceRequired", mPersistenceRequired);

    ListTag* armorItems = new ListTag(0);
    for (unsigned int i = 0; i < mArmorItems.size(); ++i)
    {
        CompoundTag* itemTag = new CompoundTag();
        if (!mArmorItems[i]->isEmpty())
            mArmorItems[i]->save(itemTag);
        armorItems->add(itemTag);
    }
    tag->put(L"ArmorItems", armorItems);

    ListTag* handItems = new ListTag(0);
    for (unsigned int i = 0; i < mHandItems.size(); ++i)
    {
        CompoundTag* itemTag = new CompoundTag();
        if (!mHandItems[i]->isEmpty())
            mHandItems[i]->save(itemTag);
        handItems->add(itemTag);
    }
    tag->put(L"HandItems", handItems);

    ListTag* armorDropChances = new ListTag(0);
    for (unsigned int i = 0; i < 4; ++i)
        armorDropChances->add(new FloatTag(mArmorDropChances[i]));
    tag->put(L"ArmorDropChances", armorDropChances);

    ListTag* handDropChances = new ListTag(0);
    for (unsigned int i = 0; i < 2; ++i)
        handDropChances->add(new FloatTag(mHandDropChances[i]));
    tag->put(L"HandDropChances", handDropChances);

    tag->putBoolean(L"Leashed", mLeashed);

    boost::shared_ptr<Entity> leashHolder = getLeashHolder();
    if (leashHolder)
    {
        CompoundTag* leashTag = new CompoundTag();
        unsigned long long holderType = leashHolder->GetType();

        if ((holderType & 0x0C000000ULL) == 0x0C000000ULL)
        {
            leashTag->putString(L"UUID", leashHolder->getUUID());
        }
        else if ((holderType & 0x0A080000ULL) == 0x0A080000ULL)
        {
            boost::shared_ptr<HangingEntity> hanging =
                boost::dynamic_pointer_cast<HangingEntity>(leashHolder);
            BlockPos pos = hanging->getPos();
            leashTag->putInt(L"X", pos.x);
            leashTag->putInt(L"Y", pos.y);
            leashTag->putInt(L"Z", pos.z);
        }

        tag->put(L"Leash", leashTag);
    }

    tag->putBoolean(L"LeftHanded", isLeftHanded());

    if (mLootTable != NULL)
    {
        tag->putString(L"DeathLootTable", mLootTable->toString());
        if (mLootTableSeed != 0)
            tag->putLong(L"DeathLootTableSeed", mLootTableSeed);
    }

    if (isNoAi())
        tag->putBoolean(L"NoAI", true);

}

void Mob::setPersistenceRequired(bool required)
{
    mPersistenceRequired = required;
}

bool Mob::isPersistenceRequired()
{
    return mPersistenceRequired;
}

void Mob::readAdditionalSaveData(CompoundTag* tag)
{
    LivingEntity::readAdditionalSaveData(tag);

    if (tag->contains(L"CanPickUpLoot", 1))
        setCanPickUpLoot(tag->getBoolean(L"CanPickUpLoot"));

    setPersistenceRequired(tag->getBoolean(L"PersistenceRequired"));

    if (tag->contains(L"ArmorItems", 9))
    {
        ListTag* items = tag->getList(L"ArmorItems", 10);
        for (int i = 0; i < 4; ++i)
            mArmorItems[i] = not_null_ptr<ItemInstance>(new ItemInstance(items->getCompound(i)));
    }

    if (tag->contains(L"HandItems", 9))
    {
        ListTag* items = tag->getList(L"HandItems", 10);
        for (int i = 0; i < 2; ++i)
            mHandItems[i] = not_null_ptr<ItemInstance>(new ItemInstance(items->getCompound(i)));
    }

    if (tag->contains(L"ArmorDropChances", 9))
    {
        ListTag* chances = tag->getList(L"ArmorDropChances", 5);
        for (int i = 0; i < chances->size(); ++i)
            mArmorDropChances[i] = chances->getFloat(i);
    }

    if (tag->contains(L"HandDropChances", 9))
    {
        ListTag* chances = tag->getList(L"HandDropChances", 5);
        for (int i = 0; i < chances->size(); ++i)
            mHandDropChances[i] = chances->getFloat(i);
    }

    mLeashed = tag->getBoolean(L"Leashed");
    if (mLeashed && tag->contains(L"Leash", 10))
    {
        delete mLeashInfoTag;
        mLeashInfoTag = tag->getCompound(L"Leash")->copy();
    }

    setLeftHanded(tag->getBoolean(L"LeftHanded"));

    if (tag->contains(L"DeathLootTable", 8))
    {
        mLootTable = new ResourceLocation(tag->getString(L"DeathLootTable"));
        mLootTableSeed = tag->getLong(L"DeathLootTableSeed");
    }

    setNoAi(tag->getBoolean(L"NoAI"));

}

ResourceLocation* Mob::getDefaultLootTable()
{
    return NULL;
}

ResourceLocation* Mob::getDefaultGearTable()
{
    return NULL;
}

void Mob::dropAllDeathLoot(bool hitByPlayer, int lootingLevel, const DamageSource* source)
{
    ResourceLocation* lootTableLocation = mLootTable;
    if (lootTableLocation == NULL)
    {
        lootTableLocation = getDefaultLootTable();
    }
    if (lootTableLocation == NULL)
    {
        LivingEntity::dropAllDeathLoot(hitByPlayer, lootingLevel, source);
        return;
    }

    ServerLevel* level = static_cast<ServerLevel*>(mLevel);
    LootTable* lootTable = level->getLootTables()->get(lootTableLocation);
    mLootTable = NULL;

    LootTableContext::Builder* builder = new LootTableContext::Builder(level);
    builder->mThisEntity = shared_from_this();
    builder->mDamageSource = source;
    if (hitByPlayer && mLastHurtByPlayer)
    {
        builder->mKillerEntity = mLastHurtByPlayer;
        builder->mLuck = (float)mLastHurtByPlayer->getLuck();
    }

    Random seededRandom(mLootTableSeed);
    LootTableContext context = builder->build();
    Random* random = mLootTableSeed == 0 ? mRandom : &seededRandom;

    std::vector<not_null_ptr<ItemInstance> >* items = lootTable->getRandomItems(random, context);
    for (std::vector<not_null_ptr<ItemInstance> >::iterator it = items->begin();
         it != items->end();
         ++it)
    {
        spawnAtLocation(*it, 0.0f);
    }

    dropEquipment(hitByPlayer, lootingLevel);
    delete items;
    delete builder;
}

void Mob::setZza(float zza)
{
    mZza = zza;
}

void Mob::setYya(float yya)
{
    mYya = yya;
}

void Mob::setXxa(float xxa)
{
    mXxa = xxa;
}

void Mob::setSpeed(float speed)
{
    LivingEntity::setSpeed(speed);
    mMoveSpeed = speed;
}

void Mob::aiStep()
{
    LivingEntity::aiStep();

    if (mLevel->mIsClientSide || !canPickUpLoot() || isNoAi())
        return;

    if (!mLevel->getGameRules()->getBoolean(1))
        return;

    AABB* searchBox = mBoundingBox->grow(1.0, 0.25, 1.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntitiesOfClass(typeid(ItemEntity), searchBox);
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
         it != entities->end();
         ++it)
    {
        boost::shared_ptr<ItemEntity> itemEntity = boost::static_pointer_cast<ItemEntity>(*it);
        if (itemEntity->mRemoved)
            continue;
        not_null_ptr<ItemInstance> item = itemEntity->getItem();
        if (item->isEmpty())
            continue;
        if (itemEntity->hasPickUpDelay())
            continue;
        pickUpItem(itemEntity);
    }
    delete entities;
}

const EquipmentSlot* Mob::getEquipmentSlotForItem(not_null_ptr<ItemInstance> item)
{
    Item* heldItem = item->getItem();
    if (heldItem == Item::byBlock(Blocks::pumpkin) || heldItem == Items::skull)
        return EquipmentSlot::HEAD;

    ArmorItem* armorItem = dynamic_cast<ArmorItem*>(heldItem);
    if (armorItem != NULL)
        return armorItem->getSlot();

    if (heldItem == Items::elytra)
        return EquipmentSlot::CHEST;

    return EquipmentSlot::MAINHAND;
}

void Mob::pickUpItem(boost::shared_ptr<ItemEntity> entity)
{
    not_null_ptr<ItemInstance> incoming = entity->getItem();

    const EquipmentSlot* slot = getEquipmentSlotForItem(incoming);

    not_null_ptr<ItemInstance> equipped = getItemSlot(slot);
    bool accept = equipped->isEmpty();
    if (!accept && equipped->sameItem(incoming)
        && equipped->getAuxValue() == incoming->getAuxValue()
        && (!incoming->hasTag() || equipped->hasTag()))
    {
        accept = equipped->getCount() < equipped->getMaxStackSize();
    }
    if (!accept)
        return;
    if (!canHoldItem(incoming))
        return;

    setItemSlot(slot, incoming);
    setPersistenceRequired(true);
    entity->remove();
}

bool Mob::canHoldItem(not_null_ptr<ItemInstance>)
{
    return true;
}

bool Mob::removeWhenFarAway()
{
    return !hasCustomName();
}

void Mob::checkDespawn()
{
    if (isPersistenceRequired())
        return;

    boost::shared_ptr<Player> player = mLevel->getNearestPlayer(shared_from_this(), 128.0, -1.0);
    if (!player)
        return;

    double distSq = distanceToSqr(player);
    if (removeWhenFarAway() && distSq > 16384.0)
    {
        remove();
        return;
    }

    const double closeDistSq = 1024.0;
    if (mNoActionTime >= 601 && mRandom->nextInt(800) == 0 && distSq > closeDistSq && removeWhenFarAway())
    {
        remove();
        return;
    }

    if (distSq < closeDistSq)
        mNoActionTime = 0;
}

void Mob::serverAiStep()
{
    PIXBeginNamedEvent(4.5f, "Tick target selector for %I64d", GetType());
    MemSect(0x33);
    ++mAmbientSoundTime;

    PIXBeginNamedEvent(4.5f, "Check despawn");
    checkDespawn();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(4.5f, "Tick sensing");
    mSensing->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(4.5f, "Tick target selector");
    mTargetSelector.tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(4.5f, "Tick goal selectors");
    mGoalSelector.tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(4.5f, "Tick navigation");
    mNavigation->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(4.5f, "Tick server ai mob step");
    customServerAiStep();
    PIXEndNamedEvent();

    if (isPassenger())
    {
        boost::shared_ptr<Entity> vehicle = getVehicle();
        if (Entity::instanceof(vehicle, ENTITY_TYPE_MOB))
        {
            boost::shared_ptr<Mob> vehicleMob = boost::static_pointer_cast<Mob>(getVehicle());

            PathNavigation* navigation = getNavigation();
            Path* currentPath = navigation->getPath();
            Path* clonedPath = currentPath != NULL ? currentPath->clone() : NULL;
            vehicleMob->getNavigation()->moveTo(clonedPath, 1.5);

            vehicleMob->getMoveControl()->copyFrom(getMoveControl());
        }
    }

    PIXBeginNamedEvent(4.5f, "Tick look");
    mLookControl->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(4.5f, "Tick move");
    mMoveControl->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(4.5f, "Tick jump");
    mJumpControl->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(4.5f, "Consider extra wandering");
    considerForExtraWandering(isDespawnProtected());
    PIXEndNamedEvent();

    MemSect(0);
    PIXEndNamedEvent();
}

void Mob::customServerAiStep()
{
}

int Mob::getMaxHeadXRot()
{
    return 40;
}

int Mob::getMaxHeadYRot()
{
    return 10;
}

float Mob::rotlerp(float from, float to, float max)
{
    float delta = to - from;
    if (delta > max)
        delta = max;
    if (delta < -max)
        delta = -max;
    return from + delta;
}

void Mob::lookAt(boost::shared_ptr<Entity> entity, float maxHeadXRot, float maxHeadYRot)
{
    if (!entity)
        return;

    double dx = entity->mX - mX;
    double dz = entity->mZ - mZ;
    double dy;

    if ((entity->GetType() & 0x0C000000ULL) == 0x0C000000ULL)
    {
        boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
        if (living)
            dy = (living->mY + living->getEyeHeight()) - (mY + getEyeHeight());
        else
            dy = entity->mY - mY;
    }
    else
    {
        dy = (entity->mBoundingBox->mMinY + entity->mBoundingBox->mMaxY) * 0.5 - (mY + getEyeHeight());
    }

    double dist = Mth::sqrt(dx * dx + dz * dz);
    const double radToDeg = 57.2957763671875;
    float targetBodyRot = (float)(atan2(dz, dx) * radToDeg) - 90.0f;
    float targetHeadRot = (float)(atan2(dy, dist) * radToDeg);
    mYBodyRot = rotlerp(mYBodyRot, targetBodyRot, maxHeadXRot);
    mYHeadRot = rotlerp(mYHeadRot, -targetHeadRot, maxHeadYRot);
}

bool Mob::checkSpawnRules()
{
    BlockPos below(BlockPos(this).below());
    const BlockState* state = mLevel->getBlockState(below);
    return state->isValidSpawn(shared_from_this());
}

bool Mob::checkSpawnObstruction()
{
    if (!mLevel->isUnobstructed(mBoundingBox, shared_from_this()))
        return false;
    return mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox, false, false, false)->empty() &&
           !mLevel->containsAnyLiquid_NoLoad(mBoundingBox);
}

double Mob::getSizeScale()
{
    return 1.0f;
}

double Mob::getHeadSizeScale()
{
    return 1.0f;
}

int Mob::getMaxSpawnClusterSize()
{
    return 4;
}

int Mob::getMaxFallDistance()
{
    if (!getVehicle())
        return 3;

    int fallDistance = (int)-(getMaxHealth() * 0.33f - getHealth());
    int difficultyAdjustment = 12 - mLevel->getDifficulty()->getId() * 4;
    fallDistance -= difficultyAdjustment;
    if (fallDistance < 0)
        fallDistance = 0;
    return fallDistance + 3;
}

std::vector<boost::shared_ptr<ItemInstance> >* Mob::getHandSlots()
{
    std::vector<boost::shared_ptr<ItemInstance> >* items =
        new std::vector<boost::shared_ptr<ItemInstance> >();
    items->push_back(mHandItems[0]);
    items->push_back(mHandItems[1]);
    return items;
}

std::vector<boost::shared_ptr<ItemInstance> >* Mob::getArmorSlots()
{
    std::vector<boost::shared_ptr<ItemInstance> >* items =
        new std::vector<boost::shared_ptr<ItemInstance> >();
    for (unsigned int i = 0; i < mArmorItems.size(); ++i)
        items->push_back(mArmorItems[i]);
    return items;
}

void Mob::setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item)
{
    if (slot == NULL || item.get() == NULL)
        return;
    playEquipSound(item);
    if (slot->getType() == EquipmentSlot::HAND && slot->getIndex() < 2)
        mHandItems[slot->getIndex()] = item;
    else if (slot->getType() == EquipmentSlot::ARMOR && slot->getIndex() < 4)
        mArmorItems[slot->getIndex()] = item;
}

not_null_ptr<ItemInstance> Mob::getItemSlot(const EquipmentSlot* slot)
{
    if (slot->getType() == EquipmentSlot::HAND)
        return mHandItems[slot->getIndex()];
    if (slot->getType() == EquipmentSlot::ARMOR)
        return mArmorItems[slot->getIndex()];
    return ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> Mob::getEquipmentForSlot(const EquipmentSlot* slot, int tier)
{
    initMobEquipmentItems();

    int slotIndex = -1;
    if (slot == EquipmentSlot::FEET)
        slotIndex = 0;
    else if (slot == EquipmentSlot::LEGS)
        slotIndex = 1;
    else if (slot == EquipmentSlot::CHEST)
        slotIndex = 2;
    else if (slot == EquipmentSlot::HEAD)
        slotIndex = 3;

    if (slotIndex < 0 || tier < 0 || tier > 4)
        return not_null_ptr<ItemInstance>();

    return sMobEquipmentItems[slotIndex][tier];
}

void Mob::populateDefaultEquipmentSlots(const DifficultyInstance& difficulty)
{
    ResourceLocation* gearTable = getDefaultGearTable();
    if (gearTable != NULL)
    {
        LootTable* lootTable = BuiltInLootTables::getLootTable(gearTable);
        if (lootTable == NULL)
            return;

        ServerLevel* serverLevel = static_cast<ServerLevel*>(mLevel);
        LootTableContext::Builder builder(serverLevel);
        builder.mThisEntity = shared_from_this();
        LootTableContext context = builder.build();

        std::vector<not_null_ptr<ItemInstance> >* items = lootTable->getRandomItems(mRandom, context);
        for (std::vector<not_null_ptr<ItemInstance> >::iterator it = items->begin();
             it != items->end();
             ++it)
        {
            not_null_ptr<ItemInstance> item = *it;
            if (item.get() == NULL)
                continue;

            Item* heldItem = item->getItem();
            ArmorItem* armorItem = dynamic_cast<ArmorItem*>(heldItem);
            if (armorItem != NULL)
            {
                setItemSlot(armorItem->getSlot(), item);
            }
            else if (heldItem != NULL && heldItem->allowOffhand())
            {
                setItemSlot(EquipmentSlot::OFFHAND, item);
            }
            else
            {
                setItemSlot(EquipmentSlot::MAINHAND, item);
            }
        }
        delete items;
        return;
    }

    float specialMultiplier = difficulty.getSpecialMultiplier();
    if (mRandom->nextFloat() >= 0.15f * specialMultiplier)
        return;

    int tier = mRandom->nextInt(2);
    if (mLevel->getDifficulty() == Difficulty::HARD)
    {
        if (mRandom->nextFloat() < 0.8999999761581421f)
            ++tier;
    }
    else if (mRandom->nextFloat() < 0.25f)
    {
        ++tier;
    }

    if (mRandom->nextFloat() < 0.095f)
        ++tier;
    if (mRandom->nextFloat() < 0.095f)
        ++tier;

    float stopChance = mLevel->getDifficulty() == Difficulty::HARD ? 0.8999999761581421f : 0.25f;
    bool firstArmorSlot = true;

    for (int i = 0; i < 6; ++i)
    {
        const EquipmentSlot* slot = EquipmentSlot::SLOTS[i];
        if (slot->getType() != EquipmentSlot::ARMOR)
            continue;

        if (!firstArmorSlot && mRandom->nextFloat() < stopChance)
            break;

        firstArmorSlot = false;

        not_null_ptr<ItemInstance> equipped = getItemSlot(slot);
        if (!equipped->isEmpty())
            continue;

        not_null_ptr<ItemInstance> equipment = getEquipmentForSlot(slot, tier);
        if (equipment.get() == NULL)
            continue;

        setItemSlot(slot, equipment);
    }
}

void Mob::populateDefaultEquipmentEnchantments(const DifficultyInstance& difficulty)
{
    float specialMultiplier = difficulty.getSpecialMultiplier();

    not_null_ptr<ItemInstance> mainHand = getMainHandItem();
    if (!mainHand->isEmpty() && mRandom->nextFloat() < 0.25f * specialMultiplier)
    {
        mainHand = EnchantmentHelper::enchantItem(
            mRandom, mainHand,
            (int)(specialMultiplier * (float)mRandom->nextInt(18) + 0.5f), false);
        setItemSlot(EquipmentSlot::MAINHAND, mainHand);
    }

    for (int i = 0; i < 6; ++i)
    {
        const EquipmentSlot* slot = EquipmentSlot::SLOTS[i];
        if (slot->getType() != EquipmentSlot::ARMOR)
            continue;

        not_null_ptr<ItemInstance> item = getItemSlot(slot);
        if (item->isEmpty())
            continue;

        if (mRandom->nextFloat() >= 0.5f * specialMultiplier)
            continue;

        item = EnchantmentHelper::enchantItem(
            mRandom, item,
            (int)(specialMultiplier * (float)mRandom->nextInt(18) + 0.5f), false);
        setItemSlot(slot, item);
    }
}

MobGroupData* Mob::finalizeMobSpawn(const DifficultyInstance&, MobGroupData* data, const std::wstring&)
{
    if (mRandom->nextFloat() < 0.05f)
        setLeftHanded(true);
    else
        setLeftHanded(false);
    return data;
}

void Mob::finalizeSpawnEggSpawn(int)
{
}

bool Mob::canBeControlledByRider()
{
    return false;
}

void Mob::setDropChance(const EquipmentSlot* slot, float chance)
{
    if (slot == NULL)
        return;
    if (slot->getType() == EquipmentSlot::HAND)
    {
        mHandDropChances[slot->getIndex()] = chance;
        return;
    }
    if (slot->getType() == EquipmentSlot::ARMOR)
        mArmorDropChances[slot->getIndex()] = chance;
}

bool Mob::canPickUpLoot()
{
    return mCanPickUpLoot;
}

void Mob::setCanPickUpLoot(bool canPickUp)
{
    mCanPickUpLoot = canPickUp;
}

bool Mob::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> heldItem = player->getItemInHand(hand);
    Item* heldItemType = heldItem->getItem();

    if (heldItemType == Items::lead && canBeLeashed(player) && player->isAllowedToAttackAnimals())
    {
        setLeashedTo(player, true);
        heldItem->shrink(1);
        return true;
    }

    if (mobInteract(player, hand))
        return true;

    return Entity::interact(player, hand);
}

bool Mob::mobInteract(const boost::shared_ptr<Player>&, InteractionHand::EInteractionHand)
{
    return false;
}

void Mob::underwaterEjectRider()
{
    if (breathesWater())
        return;
    ejectPassengers();
}

bool Mob::canBeLeashed(const boost::shared_ptr<Player>&)
{
    if (isLeashed())
        return false;
    return (GetType() & 0x40000000ULL) == 0;
}

bool Mob::startRiding(boost::shared_ptr<Entity> entity, bool force)
{
    return Entity::startRiding(entity, force);
}

unsigned int Mob::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (isLeashedToPlayer(holder.m_pPlayer))
        return 0x551269b5U;
    return Entity::GetInteractTooltip(holder);
}

bool Mob::shouldRender(double distance)
{
    AABB* box = mBoundingBox;
    int minX = Mth::floor(box->mMinX);
    int minY = Mth::floor(box->mMinY);
    int minZ = Mth::floor(box->mMinZ);
    int maxX = Mth::floor(box->mMaxX);
    int maxY = Mth::floor(box->mMaxY);
    int maxZ = Mth::floor(box->mMaxZ);

    if (!mLevel->reallyHasChunksAt(BlockPos(minX, minY, minZ), BlockPos(maxX, maxY, maxZ)))
        return false;

    return Entity::shouldRender(distance);
}

bool Mob::isValidSlotForItem(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item)
{
    const EquipmentSlot* itemSlot = getEquipmentSlotForItem(item);
    if (itemSlot == slot)
        return true;
    return (itemSlot == EquipmentSlot::MAINHAND && slot == EquipmentSlot::OFFHAND)
        || (itemSlot == EquipmentSlot::OFFHAND && slot == EquipmentSlot::MAINHAND);
}

bool Mob::setSlot(int slot, const not_null_ptr<ItemInstance>& item)
{
    const EquipmentSlot* equipmentSlot = NULL;
    if (slot == 98)
        equipmentSlot = EquipmentSlot::MAINHAND;
    else if (slot == 99)
        equipmentSlot = EquipmentSlot::OFFHAND;
    else
    {
        for (int i = 2; i < 6; ++i)
        {
            if (slot == EquipmentSlot::SLOTS[i]->getIndex() + 100)
            {
                equipmentSlot = EquipmentSlot::SLOTS[i];
                break;
            }
        }
    }
    if (equipmentSlot == NULL || item.get() == NULL)
        return false;
    if (!item->isEmpty() && !isValidSlotForItem(equipmentSlot, item))
        return false;
    setItemSlot(equipmentSlot, item);
    return true;
}

bool Mob::isControlledByLocalInstance()
{
    return canBeControlledByRider() && Entity::isControlledByLocalInstance();
}

bool Mob::isEffectiveAi()
{
    return LivingEntity::isEffectiveAi() && !isNoAi();
}

void Mob::setNoAi(bool noAi)
{
    unsigned char flags = getEntityData()->get(DATA_MOB_FLAGS_ID);
    getEntityData()->set(DATA_MOB_FLAGS_ID,
                         noAi ? (unsigned char)(flags | 1) : (unsigned char)(flags & 0xfe),
                         true);
}

void Mob::setLeftHanded(bool leftHanded)
{
    unsigned char flags = getEntityData()->get(DATA_MOB_FLAGS_ID);
    getEntityData()->set(DATA_MOB_FLAGS_ID,
                         leftHanded ? (unsigned char)(flags | 2) : (unsigned char)(flags & 0xfd),
                         true);
}

bool Mob::isNoAi()
{
    return (getEntityData()->get(DATA_MOB_FLAGS_ID) & 1) != 0;
}

const HumanoidArm* Mob::getMainArm()
{
    return isLeftHanded() ? HumanoidArm::LEFT : HumanoidArm::RIGHT;
}

std::wstring Mob::getName()
{
    return LivingEntity::getName();
}

unsigned int Mob::maxFlockSize()
{
    return mMaxFlockSize;
}

unsigned int Mob::shouldFollow()
{
    return mLeader && mFlockSize < maxFlockSize();
}

void Mob::setFollowsLeader(bool follows)
{
    mFollowsLeader = follows;
}

bool Mob::followsLeader()
{
    return mFollowsLeader;
}

bool Mob::isLeader()
{
    return mLeader;
}
