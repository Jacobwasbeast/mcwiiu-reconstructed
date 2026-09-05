#include "game/core/EvocationIllager.hpp"
#include <typeinfo>
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/EvocationFangs.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/core/Sheep.hpp"
#include "game/core/Vex.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/RandomStrollGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"

EvocationIllager::EvocationIllager(Level* level) : SpellcasterIllager(level), mWololoTarget()
{
    fjDerivedCtorCalls();
    setSize(0.6f, 1.95f);
}
EvocationIllager::~EvocationIllager() {}
void EvocationIllager::registerGoals()
{
    Mob::registerGoals();
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, new AvoidEntityGoal(this, typeid(Mob), 8.0f, 0.6, 1.0), true);
    mGoalSelector.addGoal(4, new EvokerSummonSpellGoal(this), true);
    mGoalSelector.addGoal(5, new EvokerAttackSpellGoal(this), true);
    mGoalSelector.addGoal(6, new EvokerWololoSpellGoal(this), true);
    mGoalSelector.addGoal(8, new RandomStrollGoal(this, 0.6), true);
    mGoalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Mob), 8.0f, 1.0f), true);
    mGoalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f), true);
}
void EvocationIllager::registerAttributes() { Monster::registerAttributes(); }
void EvocationIllager::defineSynchedData() { SpellcasterIllager::defineSynchedData(); }
void EvocationIllager::addDataWalkers(DataFixerUpper* fixer) { Mob::addDataWalkers(fixer); }
void EvocationIllager::readAdditionalSaveData(CompoundTag* tag) { SpellcasterIllager::readAdditionalSaveData(tag); }
void EvocationIllager::addAdditonalSaveData(CompoundTag* tag)
{
    SpellcasterIllager::addAdditonalSaveData(tag);
    tag->putInt(L"SpellTicks", getSpellCastingTime());
}
const void* EvocationIllager::getDefaultLootTable() { return &BuiltInLootTables::EVOCATION_ILLAGER; }
void EvocationIllager::customServerAiStep() { SpellcasterIllager::customServerAiStep(); }
void EvocationIllager::tick() { SpellcasterIllager::tick(); }
bool EvocationIllager::isAlliedTo(boost::shared_ptr<Entity> entity)
{
    if (Entity::isAlliedTo(entity)) return true;
    if (entity && entity->GetType() == 0x5D540048ULL)
    {
        boost::shared_ptr<Vex> vex = boost::static_pointer_cast<Vex>(entity);
        return vex->getOwner() == getTarget();
    }
    return false;
}
const SoundEvent* EvocationIllager::getAmbientSound() { return SoundEvents::EVOCATION_ILLAGER_AMBIENT; }
const SoundEvent* EvocationIllager::getDeathSound() { return SoundEvents::EVOCATION_ILLAGER_DEATH; }
const SoundEvent* EvocationIllager::getHurtSound(const DamageSource*) { return SoundEvents::EVOCATION_ILLAGER_HURT; }
void EvocationIllager::setWololoTarget(const boost::shared_ptr<Sheep>& target) { mWololoTarget = target; }
boost::shared_ptr<Sheep> EvocationIllager::getWololoTarget() { return mWololoTarget; }
const SoundEvent* EvocationIllager::getCastingSoundEvent() { return SoundEvents::EVOCATION_ILLAGER_CAST_SPELL; }
void EvocationIllager::create(Level* level) { new EvocationIllager(level); }
unsigned long long EvocationIllager::GetType() { return 0x5D54184100000000ULL; }

EvocationIllager::EvokerAttackSpellGoal::EvokerAttackSpellGoal(EvocationIllager* evoker) : SpellcasterUseSpellGoal(evoker) {}
EvocationIllager::EvokerAttackSpellGoal::~EvokerAttackSpellGoal() {}
std::wstring EvocationIllager::EvokerAttackSpellGoal::getName() { return L"Evoker Attack Spell"; }
int EvocationIllager::EvokerAttackSpellGoal::getCastingTime() { return 40; }
int EvocationIllager::EvokerAttackSpellGoal::getCastingInterval() { return 100; }
const SoundEvent* EvocationIllager::EvokerAttackSpellGoal::getSpellPrepareSound() { return SoundEvents::EVOCATION_ILLAGER_PREPARE_ATTACK; }
SpellcasterIllager::IllagerSpell* EvocationIllager::EvokerAttackSpellGoal::getSpell() { return NULL; }
void EvocationIllager::EvokerAttackSpellGoal::createSpellEntity(double x, double y, double z, double yRot, float xRot, int warmupDelay)
{
    EvocationIllager* evoker = static_cast<EvocationIllager*>(mSpellcaster);
    if (evoker->mLevel != NULL) evoker->mLevel->addEntity(boost::shared_ptr<Entity>(new EvocationFangs(evoker->mLevel, x, y, z, static_cast<float>(yRot), warmupDelay, evoker->getTarget())));
}
void EvocationIllager::EvokerAttackSpellGoal::performSpellCasting()
{
    EvocationIllager* evoker = static_cast<EvocationIllager*>(mSpellcaster);
    boost::shared_ptr<LivingEntity> target = evoker->getTarget();
    if (target)
    {
        createSpellEntity(target->mX, target->mY, target->mZ, target->mYRot, target->mXRot, 0);
    }
}

EvocationIllager::EvokerSummonSpellGoal::EvokerSummonSpellGoal(EvocationIllager* evoker) : SpellcasterUseSpellGoal(evoker) {}
EvocationIllager::EvokerSummonSpellGoal::~EvokerSummonSpellGoal() {}
bool EvocationIllager::EvokerSummonSpellGoal::canUse() { return SpellcasterUseSpellGoal::canUse(); }
std::wstring EvocationIllager::EvokerSummonSpellGoal::getName() { return L"Evoker Summon Spell"; }
int EvocationIllager::EvokerSummonSpellGoal::getCastingTime() { return 100; }
int EvocationIllager::EvokerSummonSpellGoal::getCastingInterval() { return 340; }
void EvocationIllager::EvokerSummonSpellGoal::performSpellCasting()
{
    EvocationIllager* evoker = static_cast<EvocationIllager*>(mSpellcaster);
    if (evoker->mLevel == NULL) return;
    for (int i = 0; i < 3; ++i)
    {
        Vex* vex = new Vex(evoker->mLevel);
        vex->setPos(evoker->mX + i - 1, evoker->mY + 1.0, evoker->mZ + i - 1);
        evoker->mLevel->addEntity(boost::shared_ptr<Entity>(vex));
    }
}
const SoundEvent* EvocationIllager::EvokerSummonSpellGoal::getSpellPrepareSound() { return SoundEvents::EVOCATION_ILLAGER_PREPARE_SUMMON; }
SpellcasterIllager::IllagerSpell* EvocationIllager::EvokerSummonSpellGoal::getSpell() { return NULL; }

EvocationIllager::EvokerWololoSpellGoal::EvokerWololoSpellGoal(EvocationIllager* evoker) : SpellcasterUseSpellGoal(evoker) {}
EvocationIllager::EvokerWololoSpellGoal::~EvokerWololoSpellGoal() {}
void EvocationIllager::EvokerWololoSpellGoal::tick() { SpellcasterUseSpellGoal::tick(); }
bool EvocationIllager::EvokerWololoSpellGoal::canUse() { return SpellcasterUseSpellGoal::canUse(); }
bool EvocationIllager::EvokerWololoSpellGoal::canContinueToUse() { return static_cast<EvocationIllager*>(mSpellcaster)->getWololoTarget() && static_cast<EvocationIllager*>(mSpellcaster)->isCastingSpell(); }
void EvocationIllager::EvokerWololoSpellGoal::stop() { SpellcasterUseSpellGoal::stop(); static_cast<EvocationIllager*>(mSpellcaster)->setWololoTarget(boost::shared_ptr<Sheep>()); }
std::wstring EvocationIllager::EvokerWololoSpellGoal::getName() { return L"Wololo Spell"; }
void EvocationIllager::EvokerWololoSpellGoal::performSpellCasting()
{
    boost::shared_ptr<Sheep> sheep = static_cast<EvocationIllager*>(mSpellcaster)->getWololoTarget();
    if (sheep && sheep->getColor() != DyeColor::BLUE) sheep->setColor(DyeColor::BLUE);
}
int EvocationIllager::EvokerWololoSpellGoal::getCastWarmupTime() { return 40; }
int EvocationIllager::EvokerWololoSpellGoal::getCastingTime() { return 60; }
int EvocationIllager::EvokerWololoSpellGoal::getCastingInterval() { return 140; }
const SoundEvent* EvocationIllager::EvokerWololoSpellGoal::getSpellPrepareSound() { return SoundEvents::EVOCATION_ILLAGER_PREPARE_WOLOLO; }
SpellcasterIllager::IllagerSpell* EvocationIllager::EvokerWololoSpellGoal::getSpell() { return NULL; }
bool EvocationIllager::EvokerWololoSpellGoal::SheepSelector::apply(boost::shared_ptr<Entity> entity) const
{
    return boost::static_pointer_cast<Sheep>(entity)->getColor() == DyeColor::BROWN;
}
EvocationIllager::EvokerWololoSpellGoal::SheepSelector::~SheepSelector() {}
