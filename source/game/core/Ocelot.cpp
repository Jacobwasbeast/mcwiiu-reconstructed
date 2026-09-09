#include "game/core/Ocelot.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowOwnerGoal.hpp"
#include "game/entity/ai/LeapAtTargetGoal.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/TemptGoal.hpp"
#include "game/entity/ai/NonTameRandomTargetGoal.hpp"
#include "game/world/block/OcelotSitOnBlockGoal.hpp"
#include "game/entity/ai/OcelotAttackGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/item/Items.hpp"
#include "game/world/generation/Player.hpp"
#include "game/entity/ai/SitGoal.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"

Ocelot::Ocelot(Level* level)
    : TamableAnimal(level), mTemptGoal(NULL), mSitOnBlockGoal(NULL), mCatType(0),
      mIsTempted(false), mSittingOnBlock(false)
{
    setSize(0.6f, 0.8f);
}

Ocelot::~Ocelot()
{
    delete mSitOnBlockGoal;
}

void Ocelot::create(Level* level)
{
    new Ocelot(level);
}

void Ocelot::setSittingOnBlock(bool sitting)
{
    mSittingOnBlock = sitting;
}

static Item* gOcelotFood[2] = { NULL, NULL };
static int gOcelotFoodAux[2] = { 0, 0 };

void Ocelot::staticCtor()
{
    SynchedEntityData::defineId<int>(0x0d6c0000ULL, 0x2d6c0001ULL, EntityDataSerializers::INT);
    SynchedEntityData::defineId<int>(0x0d6c0000ULL, 0x2d6c0001ULL, EntityDataSerializers::INT);
}
void Ocelot::registerGoals()
{
    delete mSitOnBlockGoal;
    mSitOnBlockGoal = new SitGoal(this);
    arrayWithLength<Item*> food(2, false);
    food.data[0] = gOcelotFood[0]; food.data[1] = gOcelotFood[1];
    arrayWithLength<int> aux(2, false);
    aux.data[0] = gOcelotFoodAux[0]; aux.data[1] = gOcelotFoodAux[1];
    mTemptGoal = new TemptGoal(this, 0.6, food, true, aux);
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, mSitOnBlockGoal, false);
    mGoalSelector.addGoal(3, mTemptGoal, false);
    mGoalSelector.addGoal(5, new FollowOwnerGoal(this, 1.0, 10.0f, 5.0f), true);
    mGoalSelector.addGoal(6, new OcelotSitOnBlockGoal(this, 0.8), true);
    mGoalSelector.addGoal(7, new LeapAtTargetGoal(this, 0.3f), true);
    mGoalSelector.addGoal(8, new OcelotAttackGoal(this), true);
    mGoalSelector.addGoal(9, new BreedGoal(this, 0.8), true);
    mGoalSelector.addGoal(10, new WaterAvoidingRandomStrollGoal(this, 0.8), true);
    mGoalSelector.addGoal(11, new LookAtPlayerGoal(this, typeid(Player), 10.0f), true);
}
void Ocelot::defineSynchedData() { TamableAnimal::defineSynchedData(); }

void Ocelot::SetIsTempted(bool tempted)
{
    mIsTempted = tempted;
}

void Ocelot::customServerAiStep()
{
    if (mIsTempted) setSittingOnBlock(false);
}

bool Ocelot::removeWhenFarAway()
{
    return Animal::removeWhenFarAway();
}

void Ocelot::registerAttributes()
{
    Mob::registerAttributes();
}

void Ocelot::causeFallDamage(float, float) { }

void Ocelot::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x2d6c0001ULL);
}

void Ocelot::addAdditonalSaveData(CompoundTag* tag)
{
    TamableAnimal::addAdditonalSaveData(tag);
    tag->putInt(L"CatType", getCatType());
}

void Ocelot::readAdditionalSaveData(CompoundTag* tag)
{
    TamableAnimal::readAdditionalSaveData(tag);
    if (tag->contains(L"CatType")) setCatType(tag->getInt(L"CatType"));
}

const SoundEvent* Ocelot::getAmbientSound()
{
    if (isInLove()) return SoundEvents::CAT_PURR;
    return getRandom()->nextInt(4) == 0 ? SoundEvents::CAT_PURREOW : SoundEvents::CAT_AMBIENT;
}

const SoundEvent* Ocelot::getHurtSound(const DamageSource*) { return SoundEvents::CAT_HURT; }
const SoundEvent* Ocelot::getDeathSound() { return SoundEvents::CAT_DEATH; }
float Ocelot::getSoundVolume() { return 0.4f; }

bool Ocelot::doHurtTarget(boost::shared_ptr<Entity> target)
{
    return Mob::doHurtTarget(target);
}

bool Ocelot::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source)) return false;
    if (getSitGoal() != NULL) getSitGoal()->wantToSit(false);
    return Animal::hurt(source, damage);
}

const void* Ocelot::getDefaultLootTable() { return &BuiltInLootTables::OCELOT; }

bool Ocelot::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    return AgableMob::mobInteract(player, hand);
}

boost::shared_ptr<AgableMob> Ocelot::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    Ocelot* child = new Ocelot(mLevel);
    child->setCatType(getCatType());
    return boost::shared_ptr<AgableMob>(child);
}

bool Ocelot::isFood(const not_null_ptr<ItemInstance>& item)
{
    return item != NULL && !item->isEmpty();
}

bool Ocelot::canMate(const boost::shared_ptr<Animal>& other)
{
    return Animal::canMate(other) && other != NULL;
}

int Ocelot::getCatType() { return mCatType; }
void Ocelot::setCatType(int type) { mCatType = type; }
bool Ocelot::checkSpawnRules() { return Animal::checkSpawnRules(); }
bool Ocelot::checkSpawnObstruction() { return Mob::checkSpawnObstruction(); }

std::wstring Ocelot::getName()
{
    if (hasCustomName()) return getCustomName();
    return std::wstring(L"entity.Cat.name");
}

MobGroupData* Ocelot::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                       const std::wstring& dataString)
{
    Mob::finalizeMobSpawn(difficulty, data, dataString);
    if (getRandom()->nextInt(7) == 0) setCatType(1 + getRandom()->nextInt(3));
    return data;
}

void Ocelot::positionRider(const boost::shared_ptr<Entity>& passenger)
{
    Entity::positionRider(passenger);
}

bool Ocelot::isSittingOnBlock() { return mSittingOnBlock; }
unsigned int Ocelot::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (!holder.m_bAllowedToHurt) return 0xffffffffU;
    return 0x82f8d90fU;
}
void Ocelot::reassessTameGoals() { TamableAnimal::reassessTameGoals(); }
unsigned long long Ocelot::GetType() { return 0x000000002D6C0001ULL; }
