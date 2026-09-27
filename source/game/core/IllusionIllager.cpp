#include "game/core/IllusionIllager.hpp"
#include "game/core/Mth.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/EntityType.hpp"
#include <cmath>
#include <typeinfo>
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AbstractIllager.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RangedBowAttackGoal.hpp"
#include "game/entity/ai/RandomStrollGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"

extern SpellcasterIllager::IllagerSpell* gSpell4;
extern SpellcasterIllager::IllagerSpell* gSpell5;

static ResourceLocation ILLUSION_ILLAGER_LOOT_TABLE(L"minecraft", L"entities/illusion_illager");

IllusionIllager::IllusionIllager(Level* level)
    : SpellcasterIllager(level), RangedAttackMob(), mIllusionLevel(0),
      mDisguisedPosition(Vec3::newPermanent(0.0, 0.0, 0.0)),
      mDisguisedPositionO(Vec3::newPermanent(0.0, 0.0, 0.0)),
      mDisguisedPosition2(Vec3::newPermanent(0.0, 0.0, 0.0)),
      mDisguisedPosition2O(Vec3::newPermanent(0.0, 0.0, 0.0)),
      mDisguisedPosition3(Vec3::newPermanent(0.0, 0.0, 0.0)),
      mDisguisedPosition3O(Vec3::newPermanent(0.0, 0.0, 0.0)),
      mDisguisedPosition4(Vec3::newPermanent(0.0, 0.0, 0.0)),
      mDisguisedPosition4O(Vec3::newPermanent(0.0, 0.0, 0.0))
{
    fjDerivedCtorCalls();
    setSize(0.6f, 1.95f);
}

IllusionIllager::~IllusionIllager()
{
    delete mDisguisedPosition;
    delete mDisguisedPositionO;
    delete mDisguisedPosition2;
    delete mDisguisedPosition2O;
    delete mDisguisedPosition3;
    delete mDisguisedPosition3O;
    delete mDisguisedPosition4;
    delete mDisguisedPosition4O;
}

void IllusionIllager::create(Level* level) { new IllusionIllager(level); }
bool IllusionIllager::isAggressive() { return getIllagerFlag(1); }
void IllusionIllager::setAggressive(bool aggressive) { setIllagerFlag(1, aggressive); }

void IllusionIllager::registerGoals()
{
    Mob::registerGoals();
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(4, new IllusionistMirrorSpellGoal(this), true);
    mGoalSelector.addGoal(5, new IllusionistBlindnessSpellGoal(this), true);
    mGoalSelector.addGoal(6, new RangedBowAttackGoal(this, 0.5, 20, 15.0f), true);
    mGoalSelector.addGoal(8, new RandomStrollGoal(this, 0.6), true);
    mGoalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f), true);
    mGoalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f), true);

    unsigned long long* hurtByTargetTypes = new unsigned long long[1];
    hurtByTargetTypes[0] = EntityType::ILLUSION_ILLAGER;
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true,
        arrayWithLength<unsigned long long>(hurtByTargetTypes, 1)), true);
    mTargetSelector.addGoal(2, (new NearestAttackableTargetGoal(this, typeid(Player), true, false, false))
        ->setUnseenMemoryTicks(300), true);
    mTargetSelector.addGoal(3, (new NearestAttackableTargetGoal(this, typeid(Villager), false, false, false))
        ->setUnseenMemoryTicks(300), true);
    mTargetSelector.addGoal(3, (new NearestAttackableTargetGoal(this, typeid(VillagerGolem), false, false, false))
        ->setUnseenMemoryTicks(300), true);
}

void IllusionIllager::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(18.0);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(32.0);
}

MobGroupData* IllusionIllager::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& dataString)
{
    setItemSlot(EquipmentSlot::MAINHAND, not_null_ptr<ItemInstance>(new ItemInstance(Items::bow)));
    return Mob::finalizeMobSpawn(difficulty, data, dataString);
}
void IllusionIllager::defineSynchedData() { SpellcasterIllager::defineSynchedData(); }
ResourceLocation* IllusionIllager::getDefaultLootTable() { return &ILLUSION_ILLAGER_LOOT_TABLE; }
AABB* IllusionIllager::getBoundingBoxForCulling() { return Entity::getBoundingBoxForCulling()->grow(3.0, 0.0, 3.0); }

void IllusionIllager::aiStep()
{
    Monster::aiStep();

    if (mLevel->mIsClientSide && isInvisible())
    {
        int nextLevel = mIllusionLevel - 1;
        if (nextLevel < 0)
        {
            nextLevel = 0;
        }

        if (mHurtTime == 1 || mTickCount % 1200 == 0)
        {
            mIllusionLevel = 3;

            int roll;
            double x;
            double y;
            double z;
            roll = mRandom->nextInt(6) - 4;
            y = roll > 0 ? (double)roll : 0.0;
            z = (-6.0f + (float)mRandom->nextInt(13)) * 0.5;
            x = (-6.0f + (float)mRandom->nextInt(13)) * 0.5;
            delete mDisguisedPosition;
            mDisguisedPosition = mDisguisedPosition3;
            mDisguisedPosition3 = Vec3::newPermanent(x, y, z);
            roll = mRandom->nextInt(6) - 4;
            y = roll > 0 ? (double)roll : 0.0;
            z = (-6.0f + (float)mRandom->nextInt(13)) * 0.5;
            x = (-6.0f + (float)mRandom->nextInt(13)) * 0.5;
            delete mDisguisedPositionO;
            mDisguisedPositionO = mDisguisedPosition3O;
            mDisguisedPosition3O = Vec3::newPermanent(x, y, z);
            roll = mRandom->nextInt(6) - 4;
            y = roll > 0 ? (double)roll : 0.0;
            z = (-6.0f + (float)mRandom->nextInt(13)) * 0.5;
            x = (-6.0f + (float)mRandom->nextInt(13)) * 0.5;
            delete mDisguisedPosition2;
            mDisguisedPosition2 = mDisguisedPosition4;
            mDisguisedPosition4 = Vec3::newPermanent(x, y, z);
            roll = mRandom->nextInt(6) - 4;
            y = roll > 0 ? (double)roll : 0.0;
            z = (-6.0f + (float)mRandom->nextInt(13)) * 0.5;
            x = (-6.0f + (float)mRandom->nextInt(13)) * 0.5;
            delete mDisguisedPosition2O;
            mDisguisedPosition2O = mDisguisedPosition4O;
            mDisguisedPosition4O = Vec3::newPermanent(x, y, z);

            for (int i = 0; i < 16; ++i)
            {
                mLevel->addParticle(PARTICLE_CLOUD, mX + (mRandom->nextDouble() - 0.5) * mBbWidth,
                                    mY + mRandom->nextDouble() * mBbHeight,
                                    mZ + (mRandom->nextDouble() - 0.5) * mBbWidth, 0.0, 0.0, 0.0,
                                    arrayWithLength<int>());
            }

            mLevel->playLocalSound(mX, mY, mZ, SoundEvents::ILLUSION_ILLAGER_MIRROR_MOVE, getSoundSource(), 1.0f, 1.0f,
                                   false, 16.0f);
        }
        else if (mHurtTime == mHurtDuration - 1)
        {
            mIllusionLevel = 3;
            delete mDisguisedPosition;
            mDisguisedPosition = mDisguisedPosition3;
            mDisguisedPosition3 = Vec3::newPermanent(0.0, 0.0, 0.0);
            delete mDisguisedPositionO;
            mDisguisedPositionO = mDisguisedPosition3O;
            mDisguisedPosition3O = Vec3::newPermanent(0.0, 0.0, 0.0);
            delete mDisguisedPosition2;
            mDisguisedPosition2 = mDisguisedPosition4;
            mDisguisedPosition4 = Vec3::newPermanent(0.0, 0.0, 0.0);
            delete mDisguisedPosition2O;
            mDisguisedPosition2O = mDisguisedPosition4O;
            mDisguisedPosition4O = Vec3::newPermanent(0.0, 0.0, 0.0);
        }
        else
        {
            mIllusionLevel = nextLevel;
        }
    }
}

bool IllusionIllager::isAlliedTo(boost::shared_ptr<Entity> entity)
{
    if (Entity::isAlliedTo(entity))
        return true;
    boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(entity);
    return living && living->getTeam() == NULL && getTeam() == NULL;
}
const SoundEvent* IllusionIllager::getAmbientSound() { return SoundEvents::ILLUSION_ILLAGER_AMBIENT; }
const SoundEvent* IllusionIllager::getDeathSound() { return SoundEvents::ILLUSION_ILLAGER_DEATH; }
const SoundEvent* IllusionIllager::getHurtSound(const DamageSource*) { return SoundEvents::ILLUSION_ILLAGER_HURT; }
const SoundEvent* IllusionIllager::getCastingSoundEvent() { return SoundEvents::ILLUSION_ILLAGER_CAST_SPELL; }
int IllusionIllager::getArmPose() { return isCastingSpell() ? 2 : (isAggressive() ? 3 : 0); }
unsigned long long IllusionIllager::GetType() { return EntityType::ILLUSION_ILLAGER; }

arrayWithLength<Vec3*> IllusionIllager::getIllusionOffsets(float partialTick)
{
    arrayWithLength<Vec3*> offsets(4, true);
    if (mIllusionLevel < 1)
    {
        offsets.data[0] = Vec3::newTemp(mDisguisedPosition2O);
        offsets.data[1] = Vec3::newTemp(mDisguisedPosition3O);
        offsets.data[2] = Vec3::newTemp(mDisguisedPosition4O);
        offsets.data[3] = Vec3::newTemp(mDisguisedPositionO);
        return offsets;
    }
    double factor = std::pow((static_cast<double>(mIllusionLevel) - partialTick) / 3.0, 0.25);
    double inverse = 1.0 - factor;
    offsets.data[0] = mDisguisedPosition2O->scale(inverse)->add(mDisguisedPosition->scale(factor));
    offsets.data[1] = mDisguisedPosition->scale(factor)->add(mDisguisedPosition3O->scale(inverse));
    offsets.data[2] = mDisguisedPosition2->scale(factor)->add(mDisguisedPosition4O->scale(inverse));
    offsets.data[3] = mDisguisedPosition3->scale(factor)->add(mDisguisedPositionO->scale(inverse));
    return offsets;
}

void IllusionIllager::performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor)
{
    boost::shared_ptr<TippableArrow> arrow = getArrow(distanceFactor);
    double dx = target->mX - mX;
    double dy = target->mBoundingBox->mMinY + target->mBbHeight / 3.0f - arrow->mY;
    double dz = target->mZ - mZ;
    double horizontal = Mth::sqrt(dx * dx + dz * dz);
    arrow->shoot(dx, dy + horizontal * 0.2f, dz, 1.6f, (float)(14 - mLevel->getDifficulty()->getId() * 4));
    playSound(SoundEvents::SKELETON_SHOOT, 1.0f, 1.0f / (getRandom()->nextFloat() * 0.4f + 0.8f));
    mLevel->addEntity(arrow);
}

boost::shared_ptr<TippableArrow> IllusionIllager::getArrow(float distanceFactor)
{
    boost::shared_ptr<LivingEntity> owner = boost::static_pointer_cast<LivingEntity>(shared_from_this());
    boost::shared_ptr<TippableArrow> arrow(new TippableArrow(mLevel, owner));
    arrow->setEnchantmentEffectsFromEntity(owner, distanceFactor);
    return arrow;
}

IllusionIllager::IllusionistBlindnessSpellGoal::IllusionistBlindnessSpellGoal(IllusionIllager* illager)
    : SpellcasterUseSpellGoal(illager), mTargetId(0) {}
IllusionIllager::IllusionistBlindnessSpellGoal::~IllusionistBlindnessSpellGoal() {}
bool IllusionIllager::IllusionistBlindnessSpellGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mSpellcaster->getTarget();
    if (!SpellcasterUseSpellGoal::canUse() || !target || target->getId() == mTargetId)
        return false;
    BlockPos pos(mSpellcaster);
    return mSpellcaster->mLevel->getCurrentDifficultyAt(pos).isHarderThan(
        static_cast<float>(Difficulty::HARD->getId()));
}
void IllusionIllager::IllusionistBlindnessSpellGoal::start()
{
    SpellcasterUseSpellGoal::start();
    mTargetId = mSpellcaster->getTarget()->getId();
}
void IllusionIllager::IllusionistBlindnessSpellGoal::performSpellCasting()
{
    boost::shared_ptr<LivingEntity> target = mSpellcaster->getTarget();
    if (target)
        target->addEffect(new MobEffectInstance(MobEffects::BLINDNESS, 400), boost::shared_ptr<Entity>());
}
int IllusionIllager::IllusionistBlindnessSpellGoal::getCastingTime() { return 20; }
int IllusionIllager::IllusionistBlindnessSpellGoal::getCastingInterval() { return 180; }
const SoundEvent* IllusionIllager::IllusionistBlindnessSpellGoal::getSpellPrepareSound() { return SoundEvents::ILLUSION_ILLAGER_PREPARE_BLINDNESS; }
SpellcasterIllager::IllagerSpell* IllusionIllager::IllusionistBlindnessSpellGoal::getSpell() { return gSpell5; }
std::wstring IllusionIllager::IllusionistBlindnessSpellGoal::getName() { return L"IllusionistBlindnessSpellGoal"; }

IllusionIllager::IllusionistMirrorSpellGoal::IllusionistMirrorSpellGoal(IllusionIllager* illager)
    : SpellcasterUseSpellGoal(illager) {}
IllusionIllager::IllusionistMirrorSpellGoal::~IllusionistMirrorSpellGoal() {}
bool IllusionIllager::IllusionistMirrorSpellGoal::canUse()
{
    return SpellcasterUseSpellGoal::canUse() && !mSpellcaster->hasEffect(MobEffects::INVISIBILITY);
}
void IllusionIllager::IllusionistMirrorSpellGoal::performSpellCasting()
{
    mSpellcaster->addEffect(new MobEffectInstance(MobEffects::INVISIBILITY, 1200), boost::shared_ptr<Entity>());
}
int IllusionIllager::IllusionistMirrorSpellGoal::getCastingTime() { return 20; }
int IllusionIllager::IllusionistMirrorSpellGoal::getCastingInterval() { return 340; }
const SoundEvent* IllusionIllager::IllusionistMirrorSpellGoal::getSpellPrepareSound() { return SoundEvents::ILLUSION_ILLAGER_PREPARE_MIRROR; }
SpellcasterIllager::IllagerSpell* IllusionIllager::IllusionistMirrorSpellGoal::getSpell() { return gSpell4; }
std::wstring IllusionIllager::IllusionistMirrorSpellGoal::getName() { return L"IllusionistMirrorSpellGoal"; }
