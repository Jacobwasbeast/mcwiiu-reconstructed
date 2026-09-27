#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/VindicationIllager.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/RandomStrollGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/SnowMan.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include <boost/assert.hpp>

VindicationIllager::VindicationIllager(Level* level) : AbstractIllager(level)
{
    fjDerivedCtorCalls();
    setSize(0.6f, 1.95f);
    mJohnny = false;
    mCategories |= EntityCategory::eEntityCategory_Humanoid | EntityCategory::eEntityCategory_Monster;
}
VindicationIllager::~VindicationIllager() = default;
VindicationIllager* VindicationIllager::create(Level* level) { return new VindicationIllager(level); }
void VindicationIllager::addDataWalkers(DataFixerUpper* fixer)
{ Mob::addDataWalkers(fixer, EntityType::VINDICATION_ILLAGER); }
void VindicationIllager::registerGoals()
{
    Mob::registerGoals();
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(4, new MeleeAttackGoal(this, 1.0, false), true);
    mGoalSelector.addGoal(8, new RandomStrollGoal(this, 0.6), true);
    mGoalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f), true);
    mGoalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f), true);
    arrayWithLength<unsigned long long> alertTypes(1, true);
    alertTypes.data[0] = EntityType::VINDICATION_ILLAGER;
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true, alertTypes), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(SnowMan), true, false, false), true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), true, false, false), true);
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
EMobType VindicationIllager::getMobType() { return MOB_TYPE_ILLAGER; }
ResourceLocation* VindicationIllager::getDefaultLootTable()
{ return &BuiltInLootTables::VINDICATION_ILLAGER; }
ResourceLocation* VindicationIllager::getDefaultGearTable()
{ return &BuiltInLootTables::VINDICATOR_GEAR; }
bool VindicationIllager::isAgressive() { return getIllagerFlag(1); }
void VindicationIllager::setAggressive(bool value) { setIllagerFlag(1, value); }
void VindicationIllager::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    if (mJohnny)
        tag->putBoolean(L"Johnny", true);
}
int VindicationIllager::getArmPose() { return isAgressive() ? ILLAGER_ARM_POSE_ATTACKING : ILLAGER_ARM_POSE_CROSSED; }
void VindicationIllager::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag->contains(L"Johnny", Tag::TAG_ANY_NUMERIC))
        mJohnny = tag->getBoolean(L"Johnny");
}
MobGroupData* VindicationIllager::finalizeMobSpawn(const DifficultyInstance& difficulty,
                                                  MobGroupData* data,
                                                  const std::wstring& reason)
{
    const std::wstring spawnReason;
    MobGroupData* result = Mob::finalizeMobSpawn(difficulty, data, spawnReason);
    populateDefaultEquipmentSlots(difficulty);
    populateDefaultEquipmentEnchantments(difficulty);
    return result;
}
void VindicationIllager::customServerAiStep()
{
    Mob::customServerAiStep();
    setAggressive(getTarget() != 0);
}
bool VindicationIllager::isAlliedTo(const boost::shared_ptr<Entity>& entity)
{
    if (Entity::isAlliedTo(entity))
        return true;
    if ((entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY) {
        boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(entity);
        if (living->getMobType() == MOB_TYPE_ILLAGER)
            return getTeam() == 0 && entity->getTeam() == 0;
    }
    return false;
}
void VindicationIllager::setCustomName(const std::wstring& name, const std::wstring& displayName, bool visible)
{
    Entity::setCustomName(name, displayName, visible);
    mJohnny = name == L"Johnny";
}
const SoundEvent* VindicationIllager::getAmbientSound() { return SoundEvents::VINDICATION_ILLAGER_AMBIENT; }
const SoundEvent* VindicationIllager::getDeathSound() { return SoundEvents::VINDICATION_ILLAGER_DEATH; }
const SoundEvent* VindicationIllager::getHurtSound(const DamageSource*) { return SoundEvents::VINDICATION_ILLAGER_HURT; }
unsigned long long VindicationIllager::GetType() { return EntityType::VINDICATION_ILLAGER; }

bool VindicationIllager::LIVING_ENTITY_SELECTOR::apply(boost::shared_ptr<Entity> entity) const
{
    if ((entity->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
        return false;
    boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(entity);
    return living->attackable();
}
VindicationIllager::VindicatorJohnnyAttackGoal::VindicatorJohnnyAttackGoal(VindicationIllager* mob)
    : NearestAttackableTargetGoal(mob, typeid(Entity), 0, true, false,
                                  new LIVING_ENTITY_SELECTOR(), false, true) {}
VindicationIllager::VindicatorJohnnyAttackGoal::~VindicatorJohnnyAttackGoal() = default;
bool VindicationIllager::VindicatorJohnnyAttackGoal::canUse()
{
    VindicationIllager* illager = static_cast<VindicationIllager*>(mMob);
    return illager->mJohnny && NearestAttackableTargetGoal::canUse();
}
