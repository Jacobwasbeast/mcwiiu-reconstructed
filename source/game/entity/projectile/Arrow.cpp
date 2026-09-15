#include "game/entity/projectile/Arrow.hpp"

#include "game/core/DamageSource.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"

Arrow::Arrow(Level* level)
    : AbstractRotatedItemProjectile(level)
{
    _init();
}

Arrow::Arrow(Level* level, boost::shared_ptr<LivingEntity> owner)
    : AbstractRotatedItemProjectile(level, owner)
{
    _init();
}

Arrow::Arrow(Level* level, double x, double y, double z)
    : AbstractRotatedItemProjectile(level, x, y, z)
{
    _init();
}

Arrow::~Arrow() = default;

void Arrow::addDataWalkers(DataFixerUpper*, unsigned long long)
{
}

void Arrow::addDataWalkers(DataFixerUpper*)
{
}

void Arrow::_init()
{
    mEnchantPower = 0;
    mKnockback = 0;
}

void Arrow::addAdditonalSaveData(CompoundTag* tag)
{
    AbstractRotatedItemProjectile::addAdditonalSaveData(tag);
    tag->putInt(L"enchantPower", mEnchantPower);
    tag->putInt(L"knockback", mKnockback);
}

void Arrow::readAdditionalSaveData(CompoundTag* tag)
{
    AbstractRotatedItemProjectile::readAdditionalSaveData(tag);
    if (tag->contains(L"enchantPower", 99))
        mEnchantPower = tag->getInt(L"enchantPower");
    if (tag->contains(L"knockback", 99))
        mKnockback = tag->getInt(L"knockback");
}

void Arrow::setKnockback(int knockback)
{
    mKnockback = knockback;
}

void Arrow::setEnchantPower(int power)
{
    mEnchantPower = power;
}

void Arrow::setEnchantmentEffectsFromEntity(const boost::shared_ptr<LivingEntity>& entity, float)
{
    int power = EnchantmentHelper::getEnchantmentLevel(Enchantments::POWER_ARROWS, entity);
    int punch = EnchantmentHelper::getEnchantmentLevel(Enchantments::PUNCH_ARROWS, entity);
    if (punch > 0)
        setKnockback(punch);
    if (power > 0)
        setEnchantPower(power);
    int flame = EnchantmentHelper::getEnchantmentLevel(Enchantments::FLAMING_ARROWS, entity);
    if (flame > 0)
        setSecondsOnFire(100);
}

void Arrow::onHitEntity(const HitResult* result)
{
    double speed = Mth::sqrt(mXd * mXd + mYd * mYd + mZd * mZd);
    float difficulty = static_cast<float>(mLevel->getDifficulty()->getId());
    float damage = static_cast<float>(mRandom->nextGaussian()) * 0.25f + (difficulty - 1.0f) * 0.11f;
    damage = static_cast<float>(speed) * 0.97f + damage;
    damage = mRandom->nextFloat(1.0f, 5.0f) + damage;
    if (isCritArrow())
        damage = Mth::clamp(damage + mRandom->nextFloat(damage * 0.5f + 2.0f), 9.0f, 10.0f);
    if (mEnchantPower > 0)
        damage = damage * (static_cast<float>(mEnchantPower) * 0.25f + 1.25f);

    boost::shared_ptr<Entity> owner = boost::static_pointer_cast<Entity>(mOwner);
    boost::shared_ptr<Arrow> arrow = boost::static_pointer_cast<Arrow>(shared_from_this());
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    const bool fire = miniGame != NULL && miniGame->HasFeatureArrowCanKnockback();
    boost::shared_ptr<DamageSource> source = DamageSource::CreateArrow(arrow, owner, fire);

    int wholeDamage = static_cast<int>(damage);
    if (fire)
        wholeDamage = 0;

    if (!result->mEntity->hurt(source.get(), static_cast<float>(wholeDamage)))
    {
        mXd *= -0.1;
        mYd *= -0.1;
        mZd *= -0.1;
        mYRot += 180.0f;
        mYRotO += 180.0f;
        mInGroundTime = 0;
        setCritArrow(false);
    }
    else
    {
        if ((result->mEntity->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
        {
            boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(result->mEntity);
            if (!mLevel->mIsClientSide)
                living->setArrowCount(living->getArrowCount() + 1);
        }
        if (mKnockback > 0)
        {
            double horizontal = Mth::sqrt(mXd * mXd + mYd * mYd);
            if (horizontal > 0.0)
            {
                result->mEntity->push(mXd * mKnockback * 0.6 / horizontal,
                    mYd * mKnockback * 0.6 / horizontal,
                    mZd * mKnockback * 0.6 / horizontal);
            }
        }
        if (mOwner)
        {
            EnchantmentHelper::doPostHurtEffects(
                boost::static_pointer_cast<LivingEntity>(result->mEntity), mOwner);
            EnchantmentHelper::doPostDamageEffects(
                boost::static_pointer_cast<LivingEntity>(mOwner), result->mEntity);
        }
    }

    remove();
}
