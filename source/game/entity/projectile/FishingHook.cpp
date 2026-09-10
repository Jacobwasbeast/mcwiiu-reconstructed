#include "game/entity/projectile/FishingHook.hpp"

#include <cmath>
#include <vector>

#include <boost/pointer_cast.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/LootTable.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EntityDataAccessor.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<int>* FishingHook::DATA_HOOKED_ENTITY = NULL;

FishingHook::FishingHook(Level* level, boost::shared_ptr<Player> owner)
    : Entity(level, true), mOwner(), mHookedIn()
{
    _init();
    init(owner);
    shoot();
}

FishingHook::FishingHook(Level* level, boost::shared_ptr<Player> owner, double x, double y, double z)
    : Entity(level, true), mOwner(), mHookedIn()
{
    _init();
    init(owner);
    setPos(x, y, z);
    mXo = mX;
    mYo = mY;
    mZo = mZ;
}

FishingHook::~FishingHook()
{
}

void FishingHook::bringInHookedEntity()
{
    if (mOwner)
    {
        mHookedIn->mXd += (mOwner->mX - mX) * 0.1;
        mHookedIn->mYd += (mOwner->mY - mY) * 0.1;
        mHookedIn->mZd += (mOwner->mZ - mZ) * 0.1;
        mHookedIn->mHurtMarked = true;
    }
}

int FishingHook::retrieve()
{
    if (mLevel->mIsClientSide || !mOwner)
    {
        return 0;
    }

    int result = 0;

    if (mHookedIn)
    {
        bringInHookedEntity();
        mLevel->broadcastEntityEvent(shared_from_this(), 31, 0);
        result = mHookedIn->GetType() == 0x800000AULL ? 3 : 5;
    }
    else if (mTicksCatchable > 0)
    {
        LootTableContext::Builder* builder =
            new LootTableContext::Builder(static_cast<ServerLevel*>(mLevel));
        builder->mLuck = (float)mLuck + mOwner->getLuck();

        LootTableContext context = builder->build();
        std::vector<not_null_ptr<ItemInstance> >* items =
            BuiltInLootTables::getLootTable(&BuiltInLootTables::FISHING)->getRandomItems(mRandom, context);

        for (std::vector<not_null_ptr<ItemInstance> >::iterator it = items->begin();
             it != items->end();
             ++it)
        {
            not_null_ptr<ItemInstance> item = *it;
            boost::shared_ptr<ItemEntity> itemEntity(new ItemEntity(mLevel, mX, mY, mZ, item));

            double dx = mOwner->mX - mX;
            double dy = mOwner->mY - mY;
            double dz = mOwner->mZ - mZ;
            double distance = std::sqrt(dx * dx + dy * dy + dz * dz);

            itemEntity->mXd = dx * 0.1;
            itemEntity->mYd = dy * 0.1 + std::sqrt(distance) * 0.08;
            itemEntity->mZd = dz * 0.1;
            mLevel->addEntity(itemEntity);

            boost::shared_ptr<ExperienceOrb> orb(new ExperienceOrb(mOwner->mLevel,
                                                                   mOwner->mX,
                                                                   mOwner->mY + 0.5,
                                                                   mOwner->mZ + 0.5,
                                                                   mRandom->nextInt(6) + 1));
            mOwner->mLevel->addEntity(orb);
        }

        delete items;
        delete builder;
        result = 1;
    }

    if (mInGround)
    {
        result = 2;
    }

    remove();
    return result;
}

void FishingHook::_init()
{
    fjDerivedCtorCalls();
    mInGround = false;
    mTicksInGround = 0;
    mOwner.reset();
    mTicksInAir = 0;
    mTicksCatchable = 0;
    mTicksCaughtDelay = 0;
    mTicksCatchableDelay = 0;
    mHookedIn.reset();
    setSize(0.25f, 0.25f);
    mAddedToChunk = true;
    mCurrentState = 0;
    mLuck = 0;
    mLureSpeed = 0;
}

void FishingHook::init(const boost::shared_ptr<Player>& owner)
{
    setSize(0.25f, 0.25f);
    mAddedToChunk = true;
    mOwner = owner;
}

void FishingHook::shoot()
{
    float xRot = Mth::clamp(mOwner->mXRotO + (mOwner->mXRot - mOwner->mXRotO), -89.9f, 89.9f);
    float yRot = mOwner->mYRotO + (mOwner->mYRot - mOwner->mYRotO);
    float cosYRot = Mth::cos(-yRot * 0.01745329238474369f - 3.1415927410125732f);
    float sinYRot = Mth::sin(-yRot * 0.01745329238474369f - 3.1415927410125732f);
    float negCosXRot = -Mth::cos(-xRot * 0.01745329238474369f);
    float sinXRot = Mth::sin(-xRot * 0.01745329238474369f);

    double x = mOwner->mXo + (mOwner->mX - mOwner->mXo) - (double)sinYRot * 0.3;
    double y = mOwner->mYo + (mOwner->mY - mOwner->mYo) + mOwner->getEyeHeight();
    double z = mOwner->mZo + (mOwner->mZ - mOwner->mZo) - (double)cosYRot * 0.3;
    moveTo(x, y, z, yRot, xRot);

    mXd = -(double)sinYRot;
    mYd = Mth::clamp(-(sinXRot / negCosXRot), -5.0f, 5.0f);
    mZd = -(double)cosYRot;

    double speed = std::sqrt(mXd * mXd + mYd * mYd + mZd * mZd);
    double spread = 0.6 / speed + 0.5;
    mXd *= mRandom->nextGaussian() * 0.0045 + spread;
    mYd *= mRandom->nextGaussian() * 0.0045 + spread;
    mZd *= mRandom->nextGaussian() * 0.0045 + spread;

    float horizontal = Mth::sqrt(mXd * mXd + mZd * mZd);
    mYRot = (float)(std::atan2(mXd, mZd) * 57.2957763671875);
    mXRot = (float)(std::atan2(mYd, (double)horizontal) * 57.2957763671875);
    mYRotO = mYRot;
    mXRotO = mXRot;

    mTicksCatchable = 0;
    mTicksCaughtDelay = 0;
    mTicksCatchableDelay = 0;
}

void FishingHook::setLureSpeed(int lureSpeed)
{
    mLureSpeed = lureSpeed;
}

void FishingHook::setLuck(int luck)
{
    mLuck = luck;
}

void FishingHook::staticCtor()
{
    DATA_HOOKED_ENTITY = SynchedEntityData::defineId<int>(0x88000006ULL, ENTITY_TYPE_ENTITY,
                                                          EntityDataSerializers::INT);
}

void FishingHook::defineSynchedData()
{
    mEntityData->createDateItem(DATA_HOOKED_ENTITY, 0);
}

bool FishingHook::canBeHooked(const boost::shared_ptr<Entity>& entity)
{
    return entity->isPickable() || entity->GetType() == 0x800000AULL;
}

bool FishingHook::makeStepSound()
{
    return false;
}

bool FishingHook::shouldRenderAtSqrDistance(double distanceSqr)
{
    return distanceSqr < 4096.0;
}

void FishingHook::handleEntityEvent(unsigned char event, int data)
{
    if (event == 31 && mHookedIn && Entity::instanceof(mHookedIn, ENTITY_TYPE_PLAYER) &&
        boost::static_pointer_cast<Player>(mHookedIn)->isLocalPlayer())
    {
        bringInHookedEntity();
    }

    Entity::handleEntityEvent(event, data);
}

bool FishingHook::shouldStopFishing()
{
    not_null_ptr<ItemInstance> mainHandItem = mOwner->getMainHandItem();
    not_null_ptr<ItemInstance> offhandItem = mOwner->getOffhandItem();
    bool mainHandIsRod = mainHandItem->getItem() == Items::fishingRod;
    bool offhandIsRod = offhandItem->getItem() == Items::fishingRod;

    if (!mOwner->mRemoved && mOwner->isAlive() && (mainHandIsRod || offhandIsRod) &&
        distanceToSqr(mOwner) <= 1024.0)
    {
        return false;
    }

    remove();
    return true;
}

void FishingHook::updateRotation()
{
    float horizontal = Mth::sqrt(mXd * mXd + mZd * mZd);
    mYRot = (float)(std::atan2(mXd, mZd) * 57.2957763671875);
    float xRot = (float)(std::atan2(mYd, (double)horizontal) * 57.2957763671875);

    while (xRot - mXRotO < -180.0f)
    {
        mXRotO -= 360.0f;
    }

    while (xRot - mXRotO >= 180.0f)
    {
        mXRotO += 360.0f;
    }

    while (mYRot - mYRotO < -180.0f)
    {
        mYRotO -= 360.0f;
    }

    while (mYRot - mYRotO >= 180.0f)
    {
        mYRotO += 360.0f;
    }

    mXRot = mXRotO + (xRot - mXRotO) * 0.20000000298023224f;
    mYRot = mYRotO + (mYRot - mYRotO) * 0.20000000298023224f;
}

void FishingHook::setHookedEntity()
{
    getEntityData()->set(DATA_HOOKED_ENTITY, mHookedIn->getId() + 1, true);
}

void FishingHook::checkCollision()
{
    Vec3* from = Vec3::newTemp(mX, mY, mZ);
    Vec3* to = Vec3::newTemp(mX + mXd, mY + mYd, mZ + mZd);
    HitResult* hitResult = mLevel->clip(from, to, false, true, false, false, NULL, false);

    from = Vec3::newTemp(mX, mY, mZ);
    to = Vec3::newTemp(mX + mXd, mY + mYd, mZ + mZd);

    if (hitResult != NULL)
    {
        to = Vec3::newTemp(hitResult->mLocation->x, hitResult->mLocation->y, hitResult->mLocation->z);
    }

    boost::shared_ptr<Entity> hitEntity;
    double nearest = 0.0;
    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntities(shared_from_this(), mBoundingBox->expand(mXd, mYd, mZd)->grow(1.0), NULL);

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
         it != entities->end();
         ++it)
    {
        boost::shared_ptr<Entity> entity = *it;

        if (canBeHooked(entity) && (entity != mOwner || mTicksInAir >= 5))
        {
            HitResult* entityHit = entity->mBoundingBox->grow(0.30000001192092896)->clip(from, to);

            if (entityHit != NULL)
            {
                double distance = from->distanceToSqr(entityHit->mLocation);

                if (distance < nearest || nearest == 0.0)
                {
                    hitEntity = entity;
                    nearest = distance;
                }
            }
        }
    }

    if (hitEntity)
    {
        hitResult = new HitResult(hitEntity);

        if (mOwner)
        {
            EntityDamageSource* damageSource = DamageSource::CreatePlayerAttack(mOwner);
            hitEntity->hurt(damageSource, 0.0f);
            delete damageSource;
        }
    }

    if (hitResult != NULL && hitResult->mType != HitResult::MISS)
    {
        if (hitResult->mType == HitResult::ENTITY)
        {
            mHookedIn = hitResult->mEntity;
            setHookedEntity();
        }
        else
        {
            mInGround = true;
        }
    }
}

void FishingHook::catchingFish(const BlockPos& pos)
{
    ServerLevel* serverLevel = static_cast<ServerLevel*>(mLevel);
    int speedBonus = 1;
    BlockPos abovePos = pos.above();

    if (mRandom->nextFloat() < 0.25f &&
        mLevel->getBiome(pos)->getBiomeType() == Biome::eBiomeType_Ocean)
    {
        speedBonus = 2;
    }

    if (mRandom->nextFloat() < 0.25f && mLevel->isRainingAt(abovePos))
    {
        speedBonus = 2;
    }

    if (mRandom->nextFloat() < 0.5f && !mLevel->canSeeSky(abovePos))
    {
        --speedBonus;
    }

    if (mTicksCatchable > 0)
    {
        --mTicksCatchable;

        if (mTicksCatchable > 0)
        {
            mYd -= 0.2 * (double)mRandom->nextFloat() * (double)mRandom->nextFloat();
        }
        else
        {
            mTicksCaughtDelay = 0;
            mTicksCatchableDelay = 0;
        }
    }
    else if (mTicksCatchableDelay > 0)
    {
        mTicksCatchableDelay -= speedBonus;

        if (mTicksCatchableDelay > 0)
        {
            mFishAngle = (float)mRandom->nextGaussian() * 4.0f + mFishAngle;
            float angle = mFishAngle * 0.01745329238474369f;
            float sinAngle = sinf(angle);
            float cosAngle = cosf(angle);
            double x = mX + (double)(sinAngle * (float)mTicksCatchableDelay) * 0.1;
            double y = std::floor(mBoundingBox->mMinY) + 1.0;
            double z = mZ + (double)(cosAngle * (float)mTicksCatchableDelay) * 0.1;
            Block* block = serverLevel->getBlockState(BlockPos(x, y - 1.0, z))->getBlock();

            if (block == Blocks::WATER || block == Blocks::FLOWING_WATER)
            {
                if (mRandom->nextFloat() < 0.15000000596046448f)
                {
                    serverLevel->sendParticles(PARTICLE_BUBBLE,
                                               x, y - 0.10000000149011612, z,
                                               1, (double)sinAngle, 0.1, (double)cosAngle, 0.0,
                                               arrayWithLength<int>(), 0.0, 0.0, 0.0);
                }

                float wakeX = cosAngle * 0.04f;
                float wakeZ = sinAngle * 0.04f;
                serverLevel->sendParticles(PARTICLE_WAKE,
                                           x, y, z,
                                           0, (double)wakeX, 0.01, (double)(-wakeZ), 1.0,
                                           arrayWithLength<int>(), 0.0, 0.0, 0.0);
                serverLevel->sendParticles(PARTICLE_WAKE,
                                           x, y, z,
                                           0, (double)(-wakeX), 0.01, (double)wakeZ, 1.0,
                                           arrayWithLength<int>(), 0.0, 0.0, 0.0);
            }
        }
        else
        {
            mYd = (double)(-0.4f * Mth::nextFloat(mRandom, 0.6f, 1.0f));
            playSound(SoundEvents::BOBBER_SPLASH, 0.25f,
                      (mRandom->nextFloat() - mRandom->nextFloat()) * 0.4f + 1.0f);

            double surfaceY = mBoundingBox->mMinY + 0.5;
            serverLevel->sendParticles(PARTICLE_BUBBLE,
                                       mX, surfaceY, mZ,
                                       (int)(mBbWidth * 20.0f + 1.0f),
                                       (double)mBbWidth, 0.0, (double)mBbWidth,
                                       0.20000000298023224,
                                       arrayWithLength<int>(), 0.0, 0.0, 0.0);
            serverLevel->sendParticles(PARTICLE_WAKE,
                                       mX, surfaceY, mZ,
                                       (int)(mBbWidth * 20.0f + 1.0f),
                                       (double)mBbWidth, 0.0, (double)mBbWidth,
                                       0.20000000298023224,
                                       arrayWithLength<int>(), 0.0, 0.0, 0.0);
            mTicksCatchable = Mth::nextInt(mRandom, 20, 40);
        }
    }
    else if (mTicksCaughtDelay > 0)
    {
        mTicksCaughtDelay -= speedBonus;

        float caughtDelay = (float)mTicksCaughtDelay;
        float chance = 0.15000000596046448f;

        if (caughtDelay < 20.0f)
        {
            chance = (20.0f - caughtDelay) * 0.05000000074505806f + chance;
        }
        else if (mTicksCaughtDelay < 40)
        {
            chance = (40.0f - caughtDelay) * 0.019999999552965164f + chance;
        }
        else if (mTicksCaughtDelay < 60)
        {
            chance = (60.0f - caughtDelay) * 0.009999999776482582f + chance;
        }

        if (mRandom->nextFloat() < chance)
        {
            float angle = Mth::nextFloat(mRandom, 0.0f, 360.0f) * 0.01745329238474369f;
            float distance = Mth::nextFloat(mRandom, 25.0f, 60.0f);
            double x = mX + (double)(Mth::sin(angle) * distance * 0.1f);
            double y = std::floor(mBoundingBox->mMinY) + 1.0;
            double z = mZ + (double)(Mth::cos(angle) * distance * 0.1f);
            Block* block = serverLevel->getBlockState(BlockPos((int)x, (int)y - 1, (int)z))->getBlock();

            if (block == Blocks::WATER || block == Blocks::FLOWING_WATER)
            {
                serverLevel->sendParticles(PARTICLE_SPLASH,
                                           x, y, z,
                                           mRandom->nextInt(2) + 2,
                                           0.10000000149011612, 0.0, 0.10000000149011612, 0.0,
                                           arrayWithLength<int>(), 0.0, 0.0, 0.0);
            }
        }

        if (mTicksCaughtDelay <= 0)
        {
            mFishAngle = Mth::nextFloat(mRandom, 0.0f, 360.0f);
            mTicksCatchableDelay = Mth::nextInt(mRandom, 20, 80);
        }
    }
    else
    {
        mTicksCaughtDelay = Mth::nextInt(mRandom, 100, 600);
        mTicksCaughtDelay -= mLureSpeed * 100;
    }
}

void FishingHook::lerpTo(double, double, double, float, float, int, bool)
{
}

void FishingHook::tick()
{
    PIXBeginNamedEvent(0.0f, "Fishing hook tick");

    Entity::tick();

    if (mOwner->mFishing != shared_from_this())
    {
        mOwner->mFishing = boost::static_pointer_cast<FishingHook>(shared_from_this());
    }

    if (!mOwner)
    {
        remove();
        return;
    }

    if (!mLevel->mIsClientSide && shouldStopFishing())
    {
        PIXEndNamedEvent();
        return;
    }

    if (mInGround)
    {
        ++mTicksInGround;

        if (mTicksInGround >= 1200)
        {
            remove();
            return;
        }
    }

    BlockPos pos(this);
    const BlockState* state = mLevel->getBlockState(pos);
    const BlockState* aboveState = mLevel->getBlockState(pos.above());
    float liquidHeight = 0.0f;

    if (state->getMaterial() == Material::WATER)
    {
        liquidHeight = LiquidBlock::getBlockLiquidHeight(state, mLevel, pos);
    }

    if (mCurrentState == 0)
    {
        if (mHookedIn)
        {
            mXd = 0.0;
            mYd = 0.0;
            mZd = 0.0;
            mCurrentState = 1;
            return;
        }

        if (liquidHeight > 0.0f)
        {
            double lift = mYd * 0.8 + 0.05;
            mXd *= 0.9;
            mYd = lift < 0.1365 ? lift : 0.1365;
            mZd *= 0.9;

            if (aboveState->getMaterial() != Material::WATER)
            {
                mCurrentState = 2;
                return;
            }
        }

        if (!mLevel->mIsClientSide)
        {
            checkCollision();
        }

        if (!mInGround && !mOnGround && !mHorizontalCollision)
        {
            ++mTicksInAir;
        }
        else
        {
            mTicksInAir = 0;
            mXd = 0.0;
            mYd = 0.0;
            mZd = 0.0;
        }
    }
    else if (mCurrentState == 1)
    {
        if (mHookedIn)
        {
            if (mHookedIn->mRemoved)
            {
                mHookedIn.reset();
                mCurrentState = 0;
            }
            else
            {
                mX = mHookedIn->mX;
                mY = mHookedIn->mBoundingBox->mMinY + (double)mHookedIn->mBbHeight * 0.8;
                mZ = mHookedIn->mZ;
                setPos(mX, mY, mZ);
            }
        }

        return;
    }
    else if (mCurrentState == 2)
    {
        mXd *= 0.9;
        mZd *= 0.9;

        double depth = mY + mYd - (double)pos.y - (double)liquidHeight;

        if (Mth::abs(depth) < 0.01)
        {
            depth += (double)((depth > 0.0) - (depth < 0.0)) * 0.1;
        }

        mYd -= depth * (double)mRandom->nextFloat() * 0.2;

        if (!mLevel->mIsClientSide && liquidHeight > 0.0f)
        {
            catchingFish(pos);
        }
    }

    if (state->getMaterial() != Material::WATER)
    {
        mYd -= 0.03;
    }

    move(eMoverType_Self, mXd, mYd, mZd, false);
    updateRotation();

    mXd *= 0.92;
    mYd *= 0.92;
    mZd *= 0.92;
    setPos(mX, mY, mZ);

    PIXEndNamedEvent();
}

void FishingHook::addAdditonalSaveData(CompoundTag*)
{
}

void FishingHook::readAdditionalSaveData(CompoundTag*)
{
}

void FishingHook::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (DATA_HOOKED_ENTITY->getId() == accessor->getId())
    {
        int hookedId = getEntityData()->get(DATA_HOOKED_ENTITY);

        if (hookedId > 0)
        {
            mHookedIn = mLevel->getEntity(hookedId - 1);
        }
        else
        {
            mHookedIn.reset();
        }
    }

    Entity::onSyncedDataUpdated(accessor);
}

boost::shared_ptr<Player> FishingHook::getOwner()
{
    return mOwner;
}

void FishingHook::remove()
{
    Entity::remove();

    if (mOwner)
    {
        mOwner->mFishing.reset();
    }
}

unsigned long long FishingHook::GetType()
{
    return 0x88000006ULL;
}

bool FishingHook::shouldTick()
{
    return true;
}
