#include "game/world/generation/VillagerGolem.hpp"

#include <typeinfo>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/world/generation/DefendVillageTargetGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/MoveTowardsRestrictionGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/MoveThroughVillageGoal.hpp"
#include "game/entity/ai/OfferFlowerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"

const EntityDataAccessor<unsigned char>* VillagerGolem::DATA_PLAYER_CREATED_ID = NULL;

bool villagerGolemTargetPredicate(boost::shared_ptr<Entity> entity)
{
    return entity && (entity->GetType() & 0x0D000000ULL) == 0x0D000000ULL &&
           entity->GetType() != 0x5D540041ULL;
}

VillagerGolem::VillagerGolem(Level* level)
    : Golem(level), mVillageCheckTime(0), mVillage(), mAttackAnimationTick(0), mOfferFlowerTick(0)
{
    fjDerivedCtorCalls();
    setSize(1.4f, 2.7f);
}

VillagerGolem::~VillagerGolem()
{
}

void VillagerGolem::create(Level* level)
{
    new VillagerGolem(level);
}

void VillagerGolem::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x0D580002ULL);
}

void VillagerGolem::staticCtor()
{
    DATA_PLAYER_CREATED_ID = SynchedEntityData::defineId<unsigned char>(0x0D580002ULL, 0x0D580000ULL,
                                                                          EntityDataSerializers::BYTE);
}

void VillagerGolem::registerGoals()
{
    mGoalSelector.addGoal(1, new MeleeAttackGoal(this, 1.0, true), true);
    mGoalSelector.addGoal(3, new MoveThroughVillageGoal(this, 0.6, false), true);
    mGoalSelector.addGoal(4, new MoveTowardsRestrictionGoal(this, 1.0), true);
    mGoalSelector.addGoal(5, new OfferFlowerGoal(this), true);
    mGoalSelector.addGoal(6, new WaterAvoidingRandomStrollGoal(this, 0.6), true);
    mGoalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new DefendVillageTargetGoal(this), true);
    mTargetSelector.addGoal(2, new HurtByTargetGoal(this, false, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Player), true, false, true), true);
}

void VillagerGolem::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_PLAYER_CREATED_ID, static_cast<unsigned char>(0));
}

void VillagerGolem::customServerAiStep()
{
    if (--mVillageCheckTime < 1)
    {
        mVillageCheckTime = mRandom->nextInt(50) + 70;
        boost::shared_ptr<Villages> villages = mLevel->getVillages();
        boost::shared_ptr<Village> village = villages->getClosestVillage(BlockPos(this), 32);
        mVillage = village;
        if (!village)
            clearRestriction();
        else
        {
            const BlockPos* center = village->getCenter();
            restrictTo(*center, static_cast<int>(village->getRadius() * 0.6f));
        }
    }
    Mob::customServerAiStep();
}

void VillagerGolem::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(100.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
    getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE)->setBaseValue(1.0);
}

int VillagerGolem::decreaseAirSupply(int airSupply)
{
    return airSupply;
}

void VillagerGolem::doPush(boost::shared_ptr<Entity> entity)
{
    if (entity && (entity->GetType() & 0x40000000ULL) != 0 && entity->GetType() != 0x5D540041ULL &&
        mRandom->nextInt(20) == 0)
    {
        boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
        if (living)
            doEnchantDamageEffects(boost::dynamic_pointer_cast<LivingEntity>(shared_from_this()), living);
    }
    LivingEntity::doPush(entity);
}

void VillagerGolem::aiStep()
{
    Mob::aiStep();
    if (mAttackAnimationTick > 0)
        --mAttackAnimationTick;
    if (mOfferFlowerTick > 0)
        --mOfferFlowerTick;
    if (mRandom->nextInt(5) == 0 && mXd * mXd + mZd * mZd > 0.0)
    {
        BlockPos pos(static_cast<int>(mX), static_cast<int>(mY), static_cast<int>(mZ));
        const BlockState* state = mLevel->getBlockState(pos);
        if (state != NULL && state->getBlock() != NULL)
            mLevel->addParticle(ePARTICLE_BLOCK_CRACK, false, mX, mY + mEyeHeight, mZ, 0.0, 0.0, 0.0, arrayWithLength<int>(1, true));
    }
}

bool VillagerGolem::isPlayerCreated()
{
    return mEntityData->get(DATA_PLAYER_CREATED_ID) != 0;
}

bool VillagerGolem::canAttackType(unsigned long long type)
{
    if (isPlayerCreated() && (type & 0x0C800000ULL) == 0x0C800000ULL)
        return false;
    if (type == 0x5D540041ULL)
        return false;
    return Mob::canAttackType(type);
}

void VillagerGolem::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putBoolean(std::wstring(L"PlayerCreated"), isPlayerCreated());
}

void VillagerGolem::setPlayerCreated(bool playerCreated)
{
    unsigned char flags = mEntityData->get(DATA_PLAYER_CREATED_ID);
    mEntityData->set(DATA_PLAYER_CREATED_ID, playerCreated ? static_cast<unsigned char>(flags | 1) : static_cast<unsigned char>(flags & 0xfe), true);
}

void VillagerGolem::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    setPlayerCreated(tag->getBoolean(std::wstring(L"PlayerCreated")));
}

bool VillagerGolem::doHurtTarget(boost::shared_ptr<Entity> entity)
{
    mAttackAnimationTick = 10;
    boost::shared_ptr<LivingEntity> target = boost::dynamic_pointer_cast<LivingEntity>(entity);
    boost::shared_ptr<EntityDamageSource> source = DamageSource::CreateMobAttack(boost::dynamic_pointer_cast<LivingEntity>(shared_from_this()));
    bool hurt = target && target->hurt(source.get(), static_cast<float>(mRandom->nextInt(8) + 7));
    if (hurt)
        doEnchantDamageEffects(boost::dynamic_pointer_cast<LivingEntity>(shared_from_this()), entity);
    return hurt;
}

void VillagerGolem::handleEntityEvent(unsigned char event, int data)
{
    if (event == 4)
    {
        mAttackAnimationTick = 10;
        playSound(SoundEvents::IRONGOLEM_ATTACK, 1.0f, 1.0f);
    }
    else if (event == 11)
        mOfferFlowerTick = 400;
    else if (event == 34)
        mOfferFlowerTick = 0;
    else
        Mob::handleEntityEvent(event, data);
}

boost::shared_ptr<Village> VillagerGolem::getVillage()
{
    return mVillage.lock();
}

unsigned int VillagerGolem::getAttackAnimationTick()
{
    return mAttackAnimationTick;
}

void VillagerGolem::offerFlower(bool offer)
{
    mOfferFlowerTick = offer ? 400 : 0;
}

const SoundEvent* VillagerGolem::getHurtSound(const DamageSource*)
{
    return SoundEvents::IRONGOLEM_HURT;
}

const SoundEvent* VillagerGolem::getDeathSound()
{
    return SoundEvents::IRONGOLEM_DEATH;
}

void VillagerGolem::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::IRONGOLEM_STEP, 1.0f, 1.0f);
}

const void* VillagerGolem::getDefaultLootTable()
{
    return &BuiltInLootTables::IRON_GOLEM;
}

unsigned int VillagerGolem::getOfferFlowerTick()
{
    return mOfferFlowerTick;
}

void VillagerGolem::die(const DamageSource* source)
{
    boost::shared_ptr<Village> village = getVillage();
    if (!isPlayerCreated() && village)
        village->modifyStanding(getStringUUID(), -5);
    LivingEntity::die(source);
}

bool VillagerGolem::hurt(const DamageSource* source, float damage)
{
    if (isPlayerCreated())
    {
        boost::shared_ptr<Entity> attacker = source->getEntity();
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(attacker);
        if (player && !player->isAllowedToAttackPlayers())
            return false;
    }
    clearRestriction();
    mVillageCheckTime = 5;
    return LivingEntity::hurt(source, damage);
}

unsigned int VillagerGolem::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_pItemInstance->getItem() != NULL && Item::getId(tooltipData.m_pItemInstance->getItem()) == 0x1A4)
    {
        boost::shared_ptr<Village> village = getVillage();
        if (village && tooltipData.m_bAllowedToInteract)
            return 0x551269B5U;
    }
    return 0xFFFFFFFFU;
}

unsigned int VillagerGolem::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (isPlayerCreated() && !tooltipData.m_bAllowedToHurt)
        return 0xFFFFFFFFU;
    return 0xFDEF7118U;
}

unsigned long long VillagerGolem::GetType()
{
    return 0x0D580002ULL;
}
