#include "game/entity/projectile/Arrow.hpp"
#include "game/core/Difficulty.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/EntityType.hpp"

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
    if (tag->contains(L"enchantPower", Tag::TAG_ANY_NUMERIC))
        mEnchantPower = tag->getInt(L"enchantPower");
    if (tag->contains(L"knockback", Tag::TAG_ANY_NUMERIC))
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
    boost::shared_ptr<Entity> entity = result->mEntity;
    const double speed = Mth::sqrt(mZd * mZd + mXd * mXd + mYd * mYd);
    const float roll = mRandom->nextFloat(1.0f, 5.0f);
    const float gaussian = static_cast<float>(mRandom->nextGaussian());
    const int difficulty = mLevel->getDifficulty()->getId();
    float damage = static_cast<float>(
        roll + static_cast<float>(speed * 0.97f + (gaussian * 0.25f + static_cast<float>(difficulty) * 0.11f)));
    if (isCritArrow())
        damage = Mth::clamp(static_cast<float>(damage + mRandom->nextFloat(damage * 0.5f + 2.0f)), 9.0f, 10.0f);
    if (mEnchantPower > 0)
        damage = damage * (static_cast<float>(mEnchantPower) * 0.25f + 1.25f);
    int wholeDamage = static_cast<int>(damage);

    const bool fire = isOnFire() && entity->GetType() != EntityType::ENDER_MAN;
    boost::shared_ptr<Arrow> arrow = boost::static_pointer_cast<Arrow>(shared_from_this());
    DamageSource* source = DamageSource::CreateArrow(arrow, mOwner == NULL ? shared_from_this() : mOwner, fire);
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureArrowCanKnockback())
        wholeDamage = 0;

    bool hurt = entity->hurt(source, static_cast<float>(wholeDamage));
    delete source;
    if (!hurt)
    {
        mXd *= -0.1f;
        mYd *= -0.1f;
        mZd *= -0.1f;
        mFlightTime = 0;
        mYRot += 180.0f;
        mYRotO += 180.0f;
        setCritArrow(false);
        if (!mLevel->mIsClientSide && mZd * mZd + mXd * mXd + mYd * mYd < 0.001f)
        {
            if (mPickup == ePickup_Allowed)
                spawnAtLocation(getPickupItem(), 0.1f);
            remove();
        }
        return;
    }

    if ((entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(entity);
        if (!mLevel->mIsClientSide)
            living->setArrowCount(living->getArrowCount() + 1);
        if (mKnockback > 0)
        {
            const double horizontal = Mth::sqrt(mXd * mXd + mZd * mZd);
            if (horizontal > 0.0)
            {
                const double knockback = static_cast<double>(mKnockback);
                living->push(mXd * knockback * 0.6 / horizontal, 0.1, mZd * knockback * 0.6 / horizontal);
            }
        }
        if (mOwner != NULL && (mOwner->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
        {
            EnchantmentHelper::doPostHurtEffects(living, mOwner);
            EnchantmentHelper::doPostDamageEffects(boost::static_pointer_cast<LivingEntity>(mOwner), living);
        }
        doPostHurtEffects(living);
        if (mOwner != NULL && entity != mOwner && mOwner->GetType() == EntityType::SERVER_PLAYER)
        {
            boost::static_pointer_cast<ServerPlayer>(mOwner)->mConnection->send(
                boost::shared_ptr<Packet>(new ClientboundGameEventPacket(ClientboundGameEventPacket::SUCCESSFUL_ARROW_HIT, 0.0f)));
        }
    }

    playSound(SoundEvents::ARROW_HIT, 1.0f, 1.2f / (mRandom->nextFloat() * 0.2f + 0.9f));
    if (entity->GetType() != EntityType::ENDER_DRAGON)
        remove();
}
