#include "game/entity/mob/Monster.hpp"

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

Monster::Monster(Level* level)
    : PathfinderMob(level)
{
    mAttackStrengthTicker = 5;
    mDeathScore |= 4;
}

Monster::~Monster()
{
}

unsigned long long Monster::GetType()
{
    return ENTITY_TYPE_MONSTER;
}

SoundSource::ESoundSource Monster::getSoundSource()
{
    return SoundSource::eSoundSource_Hostile;
}

void Monster::aiStep()
{
    updateSwingTime();
    if (getBrightness() > 0.5f)
        mDeathScore += 2;
    Mob::aiStep();
}

void Monster::tick()
{
    PIXBeginNamedEvent(4.5f, "Monster tick");
    Mob::tick();
    if (!mLevel->mIsClientSide && mLevel->getDifficulty() != Difficulty::PEACEFUL)
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        if (minecraft == NULL || !minecraft->isTutorial())
            checkDespawn();
    }
    PIXEndNamedEvent();
}

const SoundEvent* Monster::getSwimSound()
{
    return SoundEvents::HOSTILE_SWIM;
}

const SoundEvent* Monster::getSwimSplashSound()
{
    return SoundEvents::HOSTILE_SPLASH;
}

bool Monster::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;
    return LivingEntity::hurt(source, damage);
}

const SoundEvent* Monster::getHurtSound(const DamageSource*)
{
    return SoundEvents::GENERIC_HURT;
}

const SoundEvent* Monster::getDeathSound()
{
    return SoundEvents::GENERIC_DEATH;
}

const SoundEvent* Monster::getFallDamageSound(int distance)
{
    return distance > 4 ? SoundEvents::HOSTILE_BIG_FALL : SoundEvents::HOSTILE_SMALL_FALL;
}

bool Monster::doHurtTarget(boost::shared_ptr<Entity> target)
{
    boost::shared_ptr<LivingEntity> livingTarget = boost::static_pointer_cast<LivingEntity>(target);
    float damage = (float)getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue();
    int knockback = 0;
    if (livingTarget)
    {
        damage += (float)EnchantmentHelper::getDamageBonus(getMainHandItem(), livingTarget);
        knockback = EnchantmentHelper::getKnockbackBonus(boost::static_pointer_cast<LivingEntity>(shared_from_this()), livingTarget);
    }
    boost::shared_ptr<LivingEntity> attacker = boost::static_pointer_cast<LivingEntity>(shared_from_this());
    bool hurtTarget = target->hurt(static_cast<const DamageSource*>(DamageSource::CreateMobAttack(attacker).get()), damage);
    if (!hurtTarget)
        return false;
    if (knockback > 0 && livingTarget)
    {
        livingTarget->knockback(boost::static_pointer_cast<Entity>(shared_from_this()), knockback * 0.5f,
                                Mth::sin(mYRot * 0.01745329238474369f),
                                -Mth::cos(mYRot * 0.01745329238474369f));
        mXd *= 0.6;
        mZd *= 0.6;
    }
    int fireAspect = EnchantmentHelper::getFireAspect(attacker);
    if (fireAspect > 0)
        target->setSecondsOnFire(fireAspect * 4);
    doEnchantDamageEffects(attacker, target);
    return true;
}

double Monster::getWalkTargetValue(const BlockPos& pos)
{
    return (double)(127.0f - (float)mLevel->getBrightness(pos));
}

bool Monster::isDarkEnoughToSpawn()
{
    int y = (int)mY;
    if (mY < (double)y)
        --y;
    BlockPos pos(mX, (double)y, mZ);
    if (mRandom->nextInt(32) < mLevel->getBrightness((LightLayer::variety)15, pos))
        return false;
    int brightness = mLevel->getRawBrightness(pos);
    if (mLevel->isThundering())
    {
        int skyDarken = mLevel->getSkyDarken();
        mLevel->setSkyDarken(10);
        brightness = mLevel->getRawBrightness(pos);
        mLevel->setSkyDarken(skyDarken);
    }
    return brightness <= mRandom->nextInt(8);
}

bool Monster::checkSpawnRules()
{
    return mLevel->getDifficulty() != Difficulty::PEACEFUL &&
           isDarkEnoughToSpawn() && PathfinderMob::checkSpawnRules();
}

void Monster::registerAttributes()
{
    Mob::registerAttributes();
    getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_DAMAGE);
}

bool Monster::shouldDropExperience()
{
    return true;
}

bool Monster::isPreventingPlayerRest(boost::shared_ptr<Player>)
{
    return true;
}
