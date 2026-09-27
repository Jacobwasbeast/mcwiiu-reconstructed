#include "game/core/ThrownTrident.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/LightningBolt.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<unsigned char>* ThrownTrident::ID_LOYALTY = NULL;
const EntityDataAccessor<bool>* ThrownTrident::ID_FOIL = NULL;

ThrownTrident::ThrownTrident(Level* level, boost::shared_ptr<LivingEntity> owner,
    not_null_ptr<ItemInstance> item)
    : AbstractRotatedItemProjectile(level, owner)
{
    _init();
    mTridentItem = item->copy();
    int loyalty = EnchantmentHelper::getEnchantmentLevel(Enchantments::LOYALTY, owner);
    if (loyalty > 0)
        mClampedToLevel = true;
    mEntityData->set(ID_LOYALTY, static_cast<unsigned char>(loyalty), true);
    mEntityData->set(ID_FOIL, mTridentItem->hasFoil(), true);
    mOwnerId = owner->getId();
}

ThrownTrident::ThrownTrident(Level* level)
    : AbstractRotatedItemProjectile(level)
{
    _init();
    mTridentItem = new ItemInstance(Items::trident);
}

ThrownTrident::ThrownTrident(Level* level, double x, double y, double z)
    : AbstractRotatedItemProjectile(level, x, y, z)
{
    _init();
    mTridentItem = new ItemInstance(Items::trident);
}

ThrownTrident::ThrownTrident(Level* level, not_null_ptr<ItemInstance> item,
    double x, double y, double z)
    : AbstractRotatedItemProjectile(level, x, y, z)
{
    _init();
    mTridentItem = item->copy();
}

ThrownTrident::~ThrownTrident()
{
}

void ThrownTrident::create(Level* level)
{
    new ThrownTrident(level);
}

void ThrownTrident::staticCtor()
{
    ID_LOYALTY = SynchedEntityData::defineId(EntityType::THROWN_TRIDENT,
        EntityType::THROWABLE_PROJECTILE, EntityDataSerializers::BYTE);
    ID_FOIL = SynchedEntityData::defineId(EntityType::THROWN_TRIDENT,
        EntityType::THROWABLE_PROJECTILE, EntityDataSerializers::BOOLEAN);
}

void ThrownTrident::_init()
{
    fjDerivedCtorCalls();
    mDealtDamage = false;
    mOwnerId = -1;
    mReturnTimer = 0;
}

void ThrownTrident::defineSynchedData()
{
    AbstractRotatedItemProjectile::defineSynchedData();
    mEntityData->createDateItem(ID_LOYALTY, static_cast<unsigned char>(0));
    mEntityData->createDateItem(ID_FOIL, false);
}

unsigned char ThrownTrident::getLoyaltyLevel()
{
    return mEntityData->get(ID_LOYALTY);
}

unsigned char ThrownTrident::isEnchanted()
{
    return mEntityData->get(ID_FOIL);
}

void ThrownTrident::tick()
{
    if (mInGroundTime > 4)
        mDealtDamage = true;

    if (mOwnerId >= 0 && !mOwner)
        mOwner = boost::dynamic_pointer_cast<LivingEntity>(mLevel->getEntity(mOwnerId));

    if ((mDealtDamage || isNoPhysics()) && mOwner && getLoyaltyLevel() > 0)
    {
        mClampedToLevel = true;
        setNoPhysics(true);
        Vec3* direction = Vec3::newTemp(mOwner->mX - mX,
            mOwner->mY + mOwner->getEyeHeight() - mY, mOwner->mZ - mZ);
        double scale = static_cast<double>(getLoyaltyLevel());
        mY += direction->y * 0.015 * scale;
        if (mLevel->mIsClientSide)
            mYo = mY;
        if (mReturnTimer == 0)
        {
            playSound(SoundEvents::TRIDENT_RETURN, 1.0f, 1.0f);
            mLevel->playSound(boost::shared_ptr<Player>(), BlockPos(mOwner), SoundEvents::TRIDENT_RETURN,
                              SoundSource::eSoundSource_Players, 0.2f, 1.0f, 16.0f);
        }
        direction->normalize();
        mXd += direction->x * 0.05 * scale - mXd * 0.05;
        mYd += direction->y * 0.05 * scale - mYd * 0.05;
        mZd += direction->z * 0.05 * scale - mZd * 0.05;
        ++mReturnTimer;
        if (!mLevel->mIsClientSide && !mOwner->isAlive())
        {
            spawnAtLocation(getPickupItem(), 0.1f);
            remove();
        }
    }

    AbstractRotatedItemProjectile::tick();
}

not_null_ptr<ItemInstance> ThrownTrident::getPickupItem()
{
    if (!mTridentItem->isEmpty())
        return mTridentItem;
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::trident));
}

void ThrownTrident::doPostHurtEffects(const boost::shared_ptr<LivingEntity>& entity)
{
    AbstractRotatedItemProjectile::doPostHurtEffects(entity);
}

boost::shared_ptr<Entity> ThrownTrident::findHitEntity(Vec3* from, Vec3* to)
{
    if (mDealtDamage)
        return boost::shared_ptr<Entity>();
    return AbstractRotatedItemProjectile::findHitEntity(from, to);
}

const SoundEvent* ThrownTrident::getOnHitSound() const
{
    return SoundEvents::TRIDENT_GROUND_IMPACT;
}

void ThrownTrident::onHitEntity(const HitResult* result)
{
    boost::shared_ptr<Entity> entity = result->mEntity;
    float damage = 8.0f;
    if ((entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        if (!mTridentItem->isEmpty())
        {
            boost::shared_ptr<LivingEntity> target =
                boost::static_pointer_cast<LivingEntity>(entity);
            damage += static_cast<float>(EnchantmentHelper::getDamageBonus(mTridentItem, target));
        }
    }
    boost::shared_ptr<ThrownTrident> trident =
        boost::static_pointer_cast<ThrownTrident>(shared_from_this());
    boost::shared_ptr<Entity> owner = mOwner
        ? boost::static_pointer_cast<Entity>(mOwner)
        : boost::static_pointer_cast<Entity>(trident);
    boost::shared_ptr<DamageSource> source = DamageSource::CreateTrident(trident, owner);
    mDealtDamage = true;
    if (entity->hurt(source.get(), damage))
    {
        if ((entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
        {
            boost::shared_ptr<LivingEntity> target =
                boost::static_pointer_cast<LivingEntity>(entity);
            if (mOwner &&
                (mOwner->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
            {
                EnchantmentHelper::doPostHurtEffects(target,
                    boost::static_pointer_cast<Entity>(mOwner));
                EnchantmentHelper::doPostDamageEffects(
                    boost::static_pointer_cast<LivingEntity>(mOwner), entity);
            }
            doPostHurtEffects(target);
        }
        playSound(SoundEvents::TRIDENT_PIERCE, 1.0f,
            1.2f / (mRandom->nextFloat() * 0.2f + 0.9f));
    }

    mXd *= -0.01;
    mYd *= -0.1;
    mZd *= -0.01;
    mHasImpulse = true;

    if ((entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY
        && mLevel->isThundering() && !mTridentItem->isEmpty()
        && EnchantmentHelper::hasChanneling(mTridentItem))
    {
        BlockPos pos(entity);
        if (mLevel->canSeeSky(pos))
        {
            playSound(SoundEvents::TRIDENT_THUNDER, 1.0f, 1.0f);
            boost::shared_ptr<LightningBolt> lightning(
                new LightningBolt(mLevel, pos.x, pos.y, pos.z, false));
            mLevel->addGlobalEntity(lightning);
        }
    }
}

void ThrownTrident::playerTouch(boost::shared_ptr<Player> player)
{
    if (mOwner && mOwner.get() != player.get())
        return;
    AbstractRotatedItemProjectile::playerTouch(player);
}

void ThrownTrident::outOfWorld()
{
    if (mClampedToLevel)
    {
        setPos(mX, mY, mZ);
        mInGround = true;
        return;
    }
    Entity::outOfWorld();
}

void ThrownTrident::readAdditionalSaveData(CompoundTag* tag)
{
    AbstractRotatedItemProjectile::readAdditionalSaveData(tag);
    mTridentItem = not_null_ptr<ItemInstance>(new ItemInstance(tag->getCompound(L"Trident")));
    mDealtDamage = tag->getBoolean(L"DealtDamage");
    if (tag->contains(L"OwnerId"))
        mOwnerId = tag->getInt(L"OwnerId");
    mEntityData->set(ID_LOYALTY,
        static_cast<unsigned char>(tag->getInt(L"LoyalLevel")), true);
}

void ThrownTrident::addAdditonalSaveData(CompoundTag* tag)
{
    AbstractRotatedItemProjectile::addAdditonalSaveData(tag);
    if (!mTridentItem->isEmpty())
    {
        CompoundTag* item = new CompoundTag();
        tag->put(L"Trident", mTridentItem->save(item));
    }
    tag->putBoolean(L"DealtDamage", mDealtDamage);
    if (mOwnerId >= 0)
        tag->putInt(L"OwnerId", mOwnerId);
    tag->putInt(L"LoyalLevel", getLoyaltyLevel());
}

void ThrownTrident::checkDespawn()
{
    if (mPickup == 1)
        return;
    AbstractRotatedItemProjectile::checkDespawn();
}

double ThrownTrident::getWaterInertia()
{
    return 0.9900000095367432;
}

bool ThrownTrident::shouldRender(double, double, double)
{
    return true;
}

bool ThrownTrident::shouldRenderAtSqrDistance(double)
{
    return true;
}

unsigned long long ThrownTrident::GetType()
{
    return EntityType::THROWN_TRIDENT;
}
