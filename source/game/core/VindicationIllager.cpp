#include "game/core/VindicationIllager.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/RandomStrollGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/SnowMan.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"

VindicationIllager::VindicationIllager(Level* level)
    : AbstractIllager(level)
{
    fjDerivedCtorCalls();
    setSize(0.6f, 1.95f);
    mJohnny = false;
    mCategories |= 0xc;
}

VindicationIllager::~VindicationIllager()
{
}

void VindicationIllager::create(Level* level)
{
    new VindicationIllager(level);
}

void VindicationIllager::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x5d541041ULL);
}

void VindicationIllager::registerGoals()
{
    Mob::registerGoals();
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(4, new MeleeAttackGoal(this, 1.0, false), true);
    mGoalSelector.addGoal(8, new RandomStrollGoal(this, 0.6), true);
    mGoalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f), true);
    mGoalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f), true);

    arrayWithLength<unsigned long long> entityTypes(1, true);
    entityTypes.data[0] = 0x5d541041ULL;
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true, entityTypes), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), true, false, false), true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(SnowMan), true, false, false), true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), true, false, false), true);
    mTargetSelector.addGoal(4, new VindicatorJohnnyAttackGoal(this), true);
}

void VindicationIllager::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.35);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(12.0);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(24.0);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(5.0);
}

void VindicationIllager::defineSynchedData()
{
    Mob::defineSynchedData();
    getEntityData()->createDateItem(DATA_ILLAGER_FLAGS, (unsigned char)0);
}

EMobType VindicationIllager::getMobType()
{
    return MOB_TYPE_ILLAGER;
}

ResourceLocation* VindicationIllager::getDefaultLootTable()
{
    return &BuiltInLootTables::VINDICATION_ILLAGER;
}

ResourceLocation* VindicationIllager::getDefaultGearTable()
{
    return &BuiltInLootTables::VINDICATOR_GEAR;
}

bool VindicationIllager::isAgressive()
{
    return getIllagerFlag(1);
}

void VindicationIllager::setAggressive(bool aggressive)
{
    setIllagerFlag(1, aggressive);
}

void VindicationIllager::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    if (mJohnny)
        tag->putBoolean(L"Johnny", true);
}

int VindicationIllager::getArmPose()
{
    return isAgressive();
}

void VindicationIllager::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag->contains(L"Johnny"))
        mJohnny = tag->getBoolean(L"Johnny");
}

MobGroupData* VindicationIllager::finalizeMobSpawn(const DifficultyInstance& difficulty,
                                                    MobGroupData* data,
                                                    const std::wstring&)
{
    MobGroupData* result = Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
    populateDefaultEquipmentSlots(difficulty);
    populateDefaultEquipmentEnchantments(difficulty);
    return result;
}

void VindicationIllager::customServerAiStep()
{
    Mob::customServerAiStep();
    setAggressive(!!getTarget());
}

bool VindicationIllager::isAlliedTo(boost::shared_ptr<Entity> entity)
{
    if (Entity::isAlliedTo(entity))
        return true;

    if ((entity->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY
        && boost::static_pointer_cast<LivingEntity>(entity)->getMobType() == MOB_TYPE_ILLAGER)
    {
        return getTeam() == NULL && entity->getTeam() == NULL;
    }

    return false;
}

void VindicationIllager::setCustomName(const std::wstring& name,
                                       const std::wstring& ownerUUID,
                                       bool namedByRestrictedPlayer)
{
    Entity::setCustomName(name, ownerUUID, namedByRestrictedPlayer);
    mJohnny = name == L"Johnny";
}

const SoundEvent* VindicationIllager::getAmbientSound()
{
    return SoundEvents::VINDICATION_ILLAGER_AMBIENT;
}

const SoundEvent* VindicationIllager::getDeathSound()
{
    return SoundEvents::VINDICATION_ILLAGER_DEATH;
}

const SoundEvent* VindicationIllager::getHurtSound(const DamageSource*)
{
    return SoundEvents::VINDICATION_ILLAGER_HURT;
}

unsigned long long VindicationIllager::GetType()
{
    return 0x5d541041ULL;
}

VindicationIllager::LIVING_ENTITY_SELECTOR::~LIVING_ENTITY_SELECTOR()
{
}

bool VindicationIllager::LIVING_ENTITY_SELECTOR::apply(boost::shared_ptr<Entity> entity) const
{
    return (entity->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY
        && boost::static_pointer_cast<LivingEntity>(entity)->isAlive();
}

VindicationIllager::VindicatorJohnnyAttackGoal::VindicatorJohnnyAttackGoal(VindicationIllager* illager)
    : NearestAttackableTargetGoal(illager, typeid(LivingEntity), 0, true, true,
                                  new VindicationIllager::LIVING_ENTITY_SELECTOR(), false, true)
{
}

VindicationIllager::VindicatorJohnnyAttackGoal::~VindicatorJohnnyAttackGoal()
{
}

bool VindicationIllager::VindicatorJohnnyAttackGoal::canUse()
{
    return static_cast<VindicationIllager*>(mMob)->mJohnny
        && NearestAttackableTargetGoal::canUse();
}
