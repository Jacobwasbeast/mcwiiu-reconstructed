#include "game/entity/mob/LivingEntity.hpp"

#include <cmath>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/weak_ptr.hpp>

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/Abilities.hpp"
#include "game/core/Attribute.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/ModifiableAttributeMap.hpp"
#include "game/core/CombatRules.hpp"
#include "game/core/CombatTracker.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/Direction.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/InsomniaComponent.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/BubbleColumnSegment.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/core/Stat.hpp"
#include "game/core/Team.hpp"
#include "game/core/Wolf.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/ai/PushableBySelector.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/ElytraItem.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/FrostWalkerEnchantment.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/minigame/ThermalAreaRuleDefinition.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientboundAnimatePacket.hpp"
#include "game/network/ClientboundSetEquippedItemPacket.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/network/ClientboundTakeItemEntityPacket.hpp"
#include "game/network/Packet.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/LadderBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TrapDoorBlock.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/generation/PlayerTeam.hpp"

void MemSect(int section);
void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

const unsigned long long ENTITY_TYPE_EXPERIENCE_ORB = 0x0000000008000003ULL;
const unsigned long long ENTITY_TYPE_ITEM_ENTITY = 0x000000000800000AULL;
const unsigned long long ENTITY_TYPE_ABSTRACT_HORSE = 0x000000000D690000ULL;
const unsigned long long ENTITY_TYPE_SKELETON_HORSE = 0x000000002D690002ULL;
const unsigned long long ENTITY_TYPE_FLYING_MOB = 0x000000000D000080ULL;
const unsigned long long ENTITY_TYPE_WOLF = 0x000000000D6C0002ULL;
const unsigned long long ENTITY_TYPE_WITHER_BOSS = 0x000000005D540047ULL;
const unsigned long long ENTITY_TYPE_ARROW = 0x000000008A020000ULL;

const AttributeModifier* LivingEntity::SPEED_MODIFIER_SPRINTING;
const EntityDataAccessor<unsigned char>* LivingEntity::DATA_LIVING_ENTITY_FLAGS;
const EntityDataAccessor<float>* LivingEntity::DATA_HEALTH_ID;
const EntityDataAccessor<int>* LivingEntity::DATA_EFFECT_COLOR_ID;
const EntityDataAccessor<bool>* LivingEntity::DATA_EFFECT_AMBIENCE_ID;
const EntityDataAccessor<int>* LivingEntity::DATA_ARROW_COUNT_ID;

void LivingEntity::staticCtor()
{
    DATA_LIVING_ENTITY_FLAGS = SynchedEntityData::defineId(ENTITY_TYPE_LIVING_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::BYTE);
    DATA_HEALTH_ID = SynchedEntityData::defineId(ENTITY_TYPE_LIVING_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::FLOAT);
    DATA_EFFECT_COLOR_ID = SynchedEntityData::defineId(ENTITY_TYPE_LIVING_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::INT);
    DATA_EFFECT_AMBIENCE_ID = SynchedEntityData::defineId(ENTITY_TYPE_LIVING_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::BOOLEAN);
    DATA_ARROW_COUNT_ID = SynchedEntityData::defineId(ENTITY_TYPE_LIVING_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::INT);
}

LivingEntity::LivingEntity(Level* level)
    : Entity(level, true)
{
    MemSect(56);
    _init();
    MemSect(0);

    mBlocksBuilding = true;
    mRotA = (float)((Math::random() + 1.0) * 0.009999999776482582);
    setPos(mX, mY, mZ);
    mTimeOffs = (float)Math::random() * 12398.0f;
    mYRot = (float)(Math::random() * 3.1415927410125732 * 2.0);
    mYHeadRot = mYRot;
    mSwingCount = 0;
    mMaxUpStep = 0.6f;
}

LivingEntity::~LivingEntity()
{
    for (boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.begin();
         it != mActiveEffects.end();
         ++it)
    {
        delete it->second;
    }

    delete mAttributes;
    delete mCombatTracker;

    if (mLastDamageSource != NULL)
    {
        delete mLastDamageSource;
        mLastDamageSource = NULL;
    }

    delete mInsomniaComponent;
}

void LivingEntity::_init()
{
    mAttributes = NULL;
    mCombatTracker = new CombatTracker(this);
    mHandItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(2, ItemInstance::EMPTY);
    mArmorItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(4, ItemInstance::EMPTY);
    mSwinging = false;
    mSwingTime = 0;
    mRemoveArrowTime = 0;
    mHurtTime = 0;
    mHurtDuration = 0;
    mHurtDir = 0.0f;
    mDeathTime = 0;
    mOAttackAnim = 0.0f;
    mAttackAnim = 0.0f;
    mAnimationSpeedOld = 0.0f;
    mAnimationSpeed = 0.0f;
    mAnimationPosition = 0.0f;
    mInvulnerableDuration = 20;
    mOBob = 0.0f;
    mBob = 0.0f;
    mTimeOffs = 0.0f;
    mRotA = 0.0f;
    mYBodyRot = 0.0f;
    mYBodyRotO = 0.0f;
    mYHeadRot = 0.0f;
    mYHeadRotO = 0.0f;
    mFlyingSpeed = 0.019999999552965164f;
    mGlideDistance = 0.0;
    mThermalLiftSpeed = 0.0;
    mThermalLerping = false;
    mThermalLerpTicks = 0;
    mThermalTargetLiftSpeed = 0.0;
    mThermalLiftRising = true;
    mThermalCeiling = 0.0;
    mInThermalArea = false;
    mElytraSpeedBoostActive = false;
    mElytraSpeedBoost = 0.0;
    mThermalArea = NULL;
    mThermalAreaChanged = true;
    mElytraCollisionDistance = 0.0;
    mThermalAreasChecked = false;
    mTeam = NULL;
    mDeathMessage.assign(L"", 0);
    mHurtByPlayerOnPeaceful = false;
    mLastHurtByPlayer.reset();
    mLastHurtByPlayerTime = 0;
    mDead = false;
    mNoActionTime = 0;
    mORun = 0.0f;
    mRun = 0.0f;
    mAnimStep = 0.0f;
    mAnimStepO = 0.0f;
    mRotOffs = 0.0f;
    mDeathScore = 0;
    mLastHurt = 0.0f;
    mJumping = false;
    mInsomniaComponent = NULL;
    mXxa = 0.0f;
    mYya = 0.0f;
    mZza = 0.0f;
    mYRota = 0.0f;
    mLerpSteps = 0;
    mLerpX = 0.0;
    mLerpY = 0.0;
    mLerpZ = 0.0;
    mLerpYRot = 0.0;
    mLerpXRot = 0.0;
    mEffectsDirty = false;
    mLastHurtByMob.reset();
    mLastHurtByMobTimestamp = 0;
    mLastHurtMob.reset();
    mLastHurtMobTimestamp = 0;
    mSpeed = 0.0f;
    mMovementSpeedScale = 1.0f;
    mAttackStrengthCooldown = 60;
    mNoJumpDelay = 0;
    mUseItem = ItemInstance::EMPTY;
    mField0x504 = 0.0f;
    mUseItemRemaining = 0;
    mFallFlyTicks = 0;
    mField0x510 = 0;
    mAbsorptionAmount = 0.0f;
    mField0x518 = 0;
    mLastDamageSource = NULL;
    mLastDamageStamp = 0LL;
    mLastPos = BlockPos(0, 0, 0);
    mFallFlyingStateDisabled = false;
    mField0x538 = 0;
    mField0x53c = 0;
    mField0x540 = 0;
    mLightCacheTicks = 5;
    mLightSampleMinX = 0;
    mLightSampleMinY = 0;
    mLightSampleMinZ = 0;
    mLightSampleMaxX = 0;
    mLightSampleMaxY = 0;
    mLightSampleMaxZ = 0;
    mLightCacheValid = false;
    mLastLightColor = 0;
    mSwimSoundPlayed = false;
}

void LivingEntity::fjDerivedCtorCalls()
{
    Entity::fjDerivedCtorCalls();

    registerAttributes();
    setHealth(getMaxHealth());
}

void LivingEntity::ResetThermalVariables()
{
    mThermalLerping = false;
    mElytraSpeedBoost = 0.0;
    mThermalLerpTicks = 0;
    mThermalAreasChecked = true;
    mInThermalArea = false;
    mThermalLiftSpeed = 0.0;
    mZd = 0.0;
    mElytraProximityWarning = false;
    mXd = 0.0;
    mYd = 0.0;
    mElytraSpeedBoostActive = false;
    mFallDistance = 0.0f;
    SetLiftForceModifier(1.0);
    mGlideDistance = 0.0;
}

bool LivingEntity::isInterpolating() const
{
    return mLerpSteps != 0;
}

double LivingEntity::getLerpToYRot() const
{
    return mLerpYRot;
}

double LivingEntity::getLerpToXRot() const
{
    return mLerpXRot;
}

void LivingEntity::kill()
{
    hurt(DamageSource::OUT_OF_WORLD, 3.4028234663852886e+38f);
}

void LivingEntity::defineSynchedData()
{
    mEntityData->createDateItem(DATA_LIVING_ENTITY_FLAGS, (unsigned char)0);
    mEntityData->createDateItem(DATA_EFFECT_COLOR_ID, 0);
    mEntityData->createDateItem(DATA_EFFECT_AMBIENCE_ID, false);
    mEntityData->createDateItem(DATA_ARROW_COUNT_ID, 0);
    mEntityData->createDateItem(DATA_HEALTH_ID, 1.0f);
}

void LivingEntity::registerAttributes()
{
    getAttributes()->registerAttribute(SharedMonsterAttributes::MAX_HEALTH);
    getAttributes()->registerAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE);
    getAttributes()->registerAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
    getAttributes()->registerAttribute(SharedMonsterAttributes::ARMOR);
    getAttributes()->registerAttribute(SharedMonsterAttributes::ARMOR_TOUGHNESS);
}

void LivingEntity::checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos)
{
    if (!isInWater())
    {
        updateInWaterState();
    }

    if ((isInWater() || hasEffect(MobEffects::SLOW_FALLING)) && !isVehicle())
    {
        mFallDistance = 0.0f;
    }

    if (!mLevel->mIsClientSide && mFallDistance > 3.0f && onGround)
    {
        const BlockState* state = mLevel->getBlockState(mTickCache, pos);
        float excess = ceilf(mFallDistance - 3.0f);

        if (block->mMaterial != Material::AIR)
        {
            int count = (int)(150.0 * (double)Mth::clamp(excess / 15.0f + 0.20000000298023224f, 0.0f, 2.5f));
            arrayWithLength<int> particleData(1, false);
            particleData.data[0] = Block::getIdWithData(state);

            static_cast<ServerLevel*>(mLevel)->sendParticles(PARTICLE_BLOCK_DUST, mX, mY, mZ,
                                                             count, 0.0, 0.0, 0.0, 0.15000000596046448,
                                                             particleData, 0.0, 0.0, 0.0);
        }
    }

    Entity::checkFallDamage(y, onGround, block, pos);
}

InsomniaComponent* LivingEntity::getInsomniaComponent() const
{
    return mInsomniaComponent;
}

bool LivingEntity::isWaterMob()
{
    return false;
}

bool LivingEntity::breathesWater()
{
    return false;
}

bool LivingEntity::breathesAir()
{
    return true;
}

bool LivingEntity::breathesLava()
{
    return false;
}

bool LivingEntity::breathesSolids()
{
    return false;
}

int LivingEntity::GetSuffocateTime()
{
    return 0;
}

float LivingEntity::GetInhaleTime()
{
    return 0.0f;
}

int LivingEntity::GetAirRegenPerTick()
{
    float inhaleTime = GetInhaleTime();

    if (Mth::abs((double)inhaleTime) <= 9.999999747378752e-05)
    {
        return 0;
    }

    return (int)Math::round((float)getMaxAirSupply() / (inhaleTime * 20.0f));
}

bool LivingEntity::canBreathe()
{
    if (hasEffect(MobEffects::WATER_BREATHING) || hasEffect(MobEffects::CONDUIT_POWER))
    {
        return true;
    }

    BlockPos pos(mX, mY + (double)getEyeHeight(), mZ);
    const BlockState* state = mLevel->getBlockState(mTickCache, pos);
    Block* block = state->getBlock();
    Material* material = state->getMaterial();
    Material* fluidMaterial = mLevel->getExtraBlockState(pos)->getMaterial();

    if (block == Blocks::AIR || (!material->isSolid() && !material->isLiquid() && !fluidMaterial->isLiquid()))
    {
        return breathesAir();
    }

    if (block == Blocks::bubble_column_segment)
    {
        return breathesAir();
    }

    if (!material->isLiquid() && !fluidMaterial->isLiquid())
    {
        if (material != Material::LAVA && fluidMaterial != Material::LAVA)
        {
            if (state->isSolidBlockingBlock() && mLevel->isConsideredSolidBlock(pos) && isInWall())
            {
                return breathesSolids();
            }

            return breathesAir();
        }
    }
    else if (material != Material::LAVA && fluidMaterial != Material::LAVA)
    {
        if (!breathesWater() && isUnderLiquid(Material::WATER))
        {
            return false;
        }

        return true;
    }

    return breathesLava();
}

bool LivingEntity::generatesBubbles()
{
    return true;
}

void LivingEntity::baseTick()
{
    if (g_pApp->IsAppPaused())
    {
        return;
    }

    mOAttackAnim = mAttackAnim;

    if (isAlive() && isInWaterRainOrBubble())
    {
        clearFire();
    }

    Entity::baseTick();

    bool isPlayer = (GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER;

    if (isAlive())
    {
        if (isInWall())
        {
            hurt(DamageSource::IN_WALL, 1.0f);
        }
        else if (isPlayer && !mLevel->getWorldBorder()->isWithinBounds(mBoundingBox))
        {
            double distance = mLevel->getWorldBorder()->getDistanceToBorder(shared_from_this())
                              + mLevel->getWorldBorder()->getDamageSafeZone();

            if (distance < 0.0)
            {
                double damagePerBlock = mLevel->getWorldBorder()->getDamagePerBlock();

                if (damagePerBlock > 0.0)
                {
                    hurt(DamageSource::IN_WALL, Mth::clamp((float)floor(-distance * damagePerBlock), 1.0f, 3.4028234663852886e+38f));
                }
            }
        }
    }

    if (isFireImmune() || mLevel->mIsClientSide)
    {
        clearFire();
    }

    bool invulnerablePlayer = false;

    if (isPlayer)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(shared_from_this());

        if (player->mAbilities.mInvulnerable)
        {
            invulnerablePlayer = true;
        }
    }

    if (isAlive())
    {
        bool canBreatheNow = invulnerablePlayer || canBreathe();
        int airSupply = getAirSupply();
        int maxAirSupply = getMaxAirSupply();

        if (canBreatheNow)
        {
            if (airSupply < maxAirSupply)
            {
                airSupply = Mth::clamp(airSupply + GetAirRegenPerTick(), 0, maxAirSupply);
            }
        }
        else
        {
            if (isUnderLiquid(Material::WATER) && generatesBubbles() && canCreateParticles())
            {
                Vec3* eyePos = getAttachPos(ENTITY_LOCATION_EYES, 0.0f);
                mLevel->addParticle(PARTICLE_BUBBLE, eyePos->x, eyePos->y, eyePos->z, mXd, mYd, mZd,
                                    arrayWithLength<int>());
            }

            if (!mLevel->mIsClientSide || isPlayer)
            {
                airSupply = decreaseAirSupply(airSupply);
            }

            if (airSupply <= GetSuffocateTime())
            {
                if (isUnderLiquid(Material::WATER) && canCreateParticles() && isPlayer)
                {
                    for (int i = 0; i < 8; ++i)
                    {
                        mLevel->addParticle(PARTICLE_BUBBLE,
                                            mX + (double)(mRandom->nextFloat() - mRandom->nextFloat()),
                                            mY + (double)(mRandom->nextFloat() - mRandom->nextFloat()),
                                            mZ + (double)(mRandom->nextFloat() - mRandom->nextFloat()),
                                            mXd, mYd, mZd, arrayWithLength<int>());
                    }
                }

                airSupply = 0;
                hurt(DamageSource::DROWN, 2.0f);
            }

            if (!isPlayer && isUnderLiquid(Material::WATER) && canCreateParticles() && breathesAir()
                && airSupply < getMaxAirSupply())
            {
                for (int i = 0; i < 8; ++i)
                {
                    Vec3* eyePos = getAttachPos(ENTITY_LOCATION_EYES, 0.0f);
                    mLevel->addParticle(PARTICLE_BUBBLE,
                                        (double)(float)(eyePos->x + mRandom->nextGaussianFloat()),
                                        (double)(float)(eyePos->y + mRandom->nextGaussianFloat()),
                                        (double)(float)(eyePos->z + mRandom->nextGaussianFloat()),
                                        mXd, mYd, mZd, arrayWithLength<int>());
                }
            }

            if (!mLevel->mIsClientSide && isPassenger() && getVehicle()
                && (getVehicle()->GetType() & ENTITY_TYPE_LIVING_ENTITY) != ENTITY_TYPE_LIVING_ENTITY
                && getVehicle()->GetType() != ENTITY_TYPE_DOLPHIN
                && getVehicle()->GetType() != ENTITY_TYPE_SKELETON_HORSE)
            {
                stopRiding();
            }
        }

        if (Minecraft::InMiniGame(eMiniGameId_Battle, false) && isInvulnerable())
        {
            airSupply = 300;
        }

        setAirSupply(airSupply);
    }

    BlockPos pos(this);

    if (!pos.equals(mLastPos))
    {
        mLastPos = pos;
        onChangedBlock(pos);
    }

    mOBob = mBob;

    if (mHurtTime > 0)
    {
        --mHurtTime;
    }

    if (mInvulnerableTime > 0 && GetType() != ENTITY_TYPE_LOCAL_PLAYER)
    {
        --mInvulnerableTime;
    }

    if (getHealth() <= 0.0f)
    {
        tickDeath();
    }

    if (mLastHurtByPlayerTime > 0)
    {
        --mLastHurtByPlayerTime;
    }
    else
    {
        mLastHurtByPlayer.reset();
    }

    boost::shared_ptr<LivingEntity> lastHurtMob = getLastHurtMob();

    if (lastHurtMob && !lastHurtMob->isAlive())
    {
        mLastHurtMob.reset();
    }

    boost::shared_ptr<LivingEntity> lastHurtByMob = getLastHurtByMob();

    if (lastHurtByMob)
    {
        if (!lastHurtByMob->isAlive())
        {
            setLastHurtByMob(boost::shared_ptr<LivingEntity>());
        }
        else if (mTickCount - mLastHurtByMobTimestamp > 100)
        {
            setLastHurtByMob(boost::shared_ptr<LivingEntity>());
        }
    }

    tickEffects();

    mAnimStepO = mAnimStep;
    mYBodyRotO = mYBodyRot;
    mYHeadRotO = mYHeadRot;
    mYRotO = mYRot;
    mXRotO = mXRot;
}

void LivingEntity::onChangedBlock(const BlockPos& pos)
{
    boost::shared_ptr<LivingEntity> self = boost::static_pointer_cast<LivingEntity>(shared_from_this());
    int level = EnchantmentHelper::getEnchantmentLevel(Enchantments::FROST_WALKER, self);

    if (level > 0)
    {
        FrostWalkerEnchantment::onEntityMoved(self, mLevel, pos, level);
    }
}

bool LivingEntity::isBaby()
{
    return false;
}

void LivingEntity::tickDeath()
{
    ++mDeathTime;

    if (mDeathTime == 20)
    {
        if (!mLevel->mIsClientSide
            && (mLastHurtByPlayerTime > 0 || isAlwaysExperienceDropper())
            && shouldDropExperience()
            && mLevel->getGameRules()->getBoolean(4))
        {
            int reward = getExperienceReward(mLastHurtByPlayer);

            while (reward > 0)
            {
                int value = ExperienceOrb::getExperienceValue(reward);
                reward -= value;
                mLevel->addEntity(boost::shared_ptr<Entity>(new ExperienceOrb(mLevel, mX, mY, mZ, value)));
            }
        }

        remove();

        for (int i = 0; i < 20; ++i)
        {
            double xd = mRandom->nextGaussian() * 0.014;
            double yd = mRandom->nextGaussian() * 0.014;
            double zd = mRandom->nextGaussian() * 0.014;

            mLevel->addParticle(PARTICLE_EXPLODE,
                                mX + (double)(mRandom->nextFloat() * mBbWidth * 2.0f) - (double)mBbWidth,
                                mY + (double)(mRandom->nextFloat() * mBbHeight),
                                mZ + (double)(mRandom->nextFloat() * mBbWidth * 2.0f) - (double)mBbWidth,
                                xd, yd, zd, arrayWithLength<int>());
        }
    }
}

bool LivingEntity::shouldDropExperience()
{
    return !isBaby();
}

int LivingEntity::decreaseAirSupply(int airSupply)
{
    boost::shared_ptr<LivingEntity> self = boost::static_pointer_cast<LivingEntity>(shared_from_this());
    int respiration = EnchantmentHelper::getRespiration(self);

    if (respiration > 0 && mRandom->nextInt(respiration + 1) > 0)
    {
        return airSupply;
    }

    return airSupply - 1;
}

int LivingEntity::getExperienceReward(boost::shared_ptr<Player> player)
{
    return 0;
}

bool LivingEntity::isAlwaysExperienceDropper()
{
    return false;
}

void LivingEntity::stopRiding()
{
    boost::shared_ptr<Entity> vehicle = getVehicle();

    Entity::stopRiding();

    if (vehicle && vehicle != getVehicle())
    {
        if (!mLevel->mIsClientSide
            || (GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER
            || vehicle->GetType() != ENTITY_TYPE_BOAT)
        {
            findStandUpPosition(vehicle);
        }
    }
}

Random* LivingEntity::getRandom()
{
    return mRandom;
}

boost::shared_ptr<LivingEntity> LivingEntity::getLastHurtByMob()
{
    return mLastHurtByMob.lock();
}

int LivingEntity::getLastHurtByMobTimestamp()
{
    return mLastHurtByMobTimestamp;
}

void LivingEntity::setLastHurtByMob(boost::shared_ptr<LivingEntity> attacker)
{
    mLastHurtByMob = attacker;
    mLastHurtByMobTimestamp = mTickCount;
}

boost::shared_ptr<LivingEntity> LivingEntity::getLastHurtMob()
{
    return mLastHurtMob.lock();
}

int LivingEntity::getLastHurtMobTimestamp()
{
    return mLastHurtMobTimestamp;
}

void LivingEntity::setLastHurtMob(boost::shared_ptr<Entity> target)
{
    if ((target->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
    {
        mLastHurtMob = boost::static_pointer_cast<LivingEntity>(target);
    }
    else
    {
        mLastHurtMob.reset();
    }

    mLastHurtMobTimestamp = mTickCount;
}

int LivingEntity::getNoActionTime()
{
    return mNoActionTime;
}

void LivingEntity::playEquipSound(not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->isEmpty())
    {
        return;
    }

    if (GetType() == ENTITY_TYPE_ARMOR_STAND)
    {
        Item* item = itemInstance->getItem();
        ArmorItem* armorItem = dynamic_cast<ArmorItem*>(item);

        if (armorItem != NULL)
        {
            playSound(armorItem->getMaterial()->getEquipSound(), 1.0f, 1.0f);
            return;
        }

        const SoundEvent* sound = SoundEvents::ARMOR_EQUIP_GENERIC;

        if (item == Items::elytra)
        {
            sound = SoundEvents::ARMOR_EQUIP_ELYTRA;
        }

        playSound(sound, 1.0f, 1.0f);
        return;
    }

    if (itemInstance->getItem() == Items::elytra)
    {
        playSound(SoundEvents::ARMOR_EQUIP_ELYTRA, 1.0f, 1.0f);
    }
}

void LivingEntity::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putFloat(std::wstring(L"Health"), getHealth());
    tag->putShort(std::wstring(L"HurtTime"), (short)mHurtTime);
    tag->putShort(std::wstring(L"HurtByTimestamp"), (short)mLastHurtByMobTimestamp);
    tag->putShort(std::wstring(L"DeathTime"), (short)mDeathTime);
    tag->putFloat(std::wstring(L"AbsorptionAmount"), getAbsorptionAmount());

    for (int i = 0; i < 6; ++i)
    {
        const EquipmentSlot* slot = EquipmentSlot::SLOTS[i];
        not_null_ptr<ItemInstance> itemInstance = getItemSlot(slot);

        if (!itemInstance->isEmpty())
        {
            getAttributes()->removeAttributeModifiers(itemInstance->getAttributeModifiers(slot));
        }
    }

    tag->put(std::wstring(L"Attributes"), SharedMonsterAttributes::saveAttributes(getAttributes()));

    for (int i = 0; i < 6; ++i)
    {
        const EquipmentSlot* slot = EquipmentSlot::SLOTS[i];
        not_null_ptr<ItemInstance> itemInstance = getItemSlot(slot);

        if (!itemInstance->isEmpty())
        {
            getAttributes()->addAttributeModifiers(itemInstance->getAttributeModifiers(slot));
        }
    }

    if (!mActiveEffects.empty())
    {
        ListTag* effects = new ListTag(0);

        for (boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.begin();
             it != mActiveEffects.end();
             ++it)
        {
            effects->add(it->second->save(new CompoundTag()));
        }

        tag->put(std::wstring(L"ActiveEffects"), effects);
    }

    tag->putBoolean(std::wstring(L"FallFlying"), isFallFlying());

    if (mInsomniaComponent != NULL)
    {
        mInsomniaComponent->addAdditionalSaveData(tag);
    }
}

void LivingEntity::readAdditionalSaveData(CompoundTag* tag)
{
    setAbsorptionAmount(tag->getFloat(std::wstring(L"AbsorptionAmount")));

    if (tag->contains(std::wstring(L"Attributes"), 9) && mLevel != NULL && !mLevel->mIsClientSide)
    {
        SharedMonsterAttributes::loadAttributes(getAttributes(), tag->getList(std::wstring(L"Attributes"), 10));
    }

    if (tag->contains(std::wstring(L"ActiveEffects"), 9))
    {
        ListTag* effects = tag->getList(std::wstring(L"ActiveEffects"), 10);

        for (int i = 0; i < effects->size(); ++i)
        {
            MobEffectInstance* effectInstance = MobEffectInstance::load(effects->getCompound(i));

            if (effectInstance != NULL)
            {
                mActiveEffects[effectInstance->getEffect()] = effectInstance;
            }
        }
    }

    if (tag->contains(std::wstring(L"Health"), 99))
    {
        setHealth(tag->getFloat(std::wstring(L"Health")));
    }

    mHurtTime = tag->getShort(std::wstring(L"HurtTime"));
    mDeathTime = tag->getShort(std::wstring(L"DeathTime"));
    mLastHurtByMobTimestamp = tag->getInt(std::wstring(L"HurtByTimestamp"));

    if (tag->contains(std::wstring(L"Team"), 8))
    {
        std::wstring teamName = tag->getString(std::wstring(L"Team"));
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(shared_from_this());

        mLevel->GetScoreboard()->addPlayerToTeam(player->mXuid,
                                                 mLevel->GetScoreboard()->getPlayerTeam(teamName));
    }

    if (tag->getBoolean(std::wstring(L"FallFlying")))
    {
        setSharedFlag(9, true);
    }

    if (mInsomniaComponent != NULL)
    {
        mInsomniaComponent->readAdditionalSaveData(tag);
    }
}

void LivingEntity::tickEffects()
{
    boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.begin();

    while (it != mActiveEffects.end())
    {
        MobEffectInstance* effectInstance = it->second;

        if (!effectInstance->tick(boost::static_pointer_cast<LivingEntity>(shared_from_this())))
        {
            if (mLevel->mIsClientSide)
            {
                ++it;
            }
            else
            {
                it = mActiveEffects.erase(it);
                onEffectRemoved(effectInstance);
                delete effectInstance;
            }
        }
        else
        {
            if (effectInstance->getDuration() % 600 == 0)
            {
                onEffectUpdated(effectInstance, false);
            }

            ++it;
        }
    }

    if (mEffectsDirty)
    {
        if (!mLevel->mIsClientSide)
        {
            updateInvisibilityStatus();
        }

        mEffectsDirty = false;
    }

    int colour = mEntityData->get(DATA_EFFECT_COLOR_ID);
    bool ambient = mEntityData->get(DATA_EFFECT_AMBIENCE_ID);

    if (colour > 0)
    {
        bool spawn;

        if (isInvisible())
        {
            spawn = mRandom->nextInt(15) == 0;
        }
        else
        {
            spawn = mRandom->nextBoolean();
        }

        if (ambient)
        {
            spawn = spawn && mRandom->nextInt(5) == 0;
        }

        if (spawn)
        {
            double red = (double)((colour >> 16) & 255) / 255.0;
            double green = (double)((colour >> 8) & 255) / 255.0;
            double blue = (double)(colour & 255) / 255.0;

            mLevel->addParticle(ambient ? PARTICLE_MOB_SPELL_AMBIENT : PARTICLE_MOB_SPELL,
                                mX + (mRandom->nextDouble() - 0.5) * (double)mBbWidth,
                                mY + mRandom->nextDouble() * (double)mBbHeight,
                                mZ + (mRandom->nextDouble() - 0.5) * (double)mBbWidth,
                                red, green, blue, arrayWithLength<int>());
        }
    }
}

bool LivingEntity::areAllEffectsAmbient(std::vector<MobEffectInstance*>* effects)
{
    for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
    {
        if (!(*it)->isAmbient())
        {
            return false;
        }
    }

    return true;
}

void LivingEntity::updateInvisibilityStatus()
{
    if (mActiveEffects.empty())
    {
        mEntityData->set(DATA_EFFECT_AMBIENCE_ID, false, true);
        mEntityData->set(DATA_EFFECT_COLOR_ID, 0, true);
        setInvisible(false);
        setWeakened(false);
        return;
    }

    std::vector<MobEffectInstance*> effects;

    for (boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.begin();
         it != mActiveEffects.end();
         ++it)
    {
        effects.push_back(it->second);
    }

    mEntityData->set(DATA_EFFECT_AMBIENCE_ID, areAllEffectsAmbient(&effects), true);
    mEntityData->set(DATA_EFFECT_COLOR_ID, PotionUtils::getColor(&effects), true);

    setInvisible(hasEffect(MobEffects::INVISIBILITY));
    setWeakened(hasEffect(MobEffects::WEAKNESS));
}

void LivingEntity::removeEffectParticles()
{
    mEntityData->set(DATA_EFFECT_AMBIENCE_ID, false, true);
    mEntityData->set(DATA_EFFECT_COLOR_ID, 0, true);
}

void LivingEntity::removeAllEffects()
{
    if (mLevel->mIsClientSide)
    {
        return;
    }

    boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.begin();

    while (it != mActiveEffects.end())
    {
        MobEffectInstance* effectInstance = it->second;
        it = mActiveEffects.erase(it);
        onEffectRemoved(effectInstance);
        delete effectInstance;
    }
}

std::vector<MobEffectInstance*>* LivingEntity::getActiveEffects()
{
    std::vector<MobEffectInstance*>* effects = new std::vector<MobEffectInstance*>();

    for (boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.begin();
         it != mActiveEffects.end();
         ++it)
    {
        effects->push_back(it->second);
    }

    return effects;
}

bool LivingEntity::hasEffect(MobEffect* effect)
{
    return mActiveEffects.find(effect) != mActiveEffects.end();
}

MobEffectInstance* LivingEntity::getEffect(MobEffect* effect)
{
    boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.find(effect);

    if (it == mActiveEffects.end())
    {
        return NULL;
    }

    return it->second;
}

void LivingEntity::addEffect(MobEffectInstance* effectInstance, const boost::shared_ptr<Entity>& source)
{
    if (!canBeAffected(effectInstance))
    {
        return;
    }

    if (mActiveEffects.find(effectInstance->getEffect()) == mActiveEffects.end())
    {
        mActiveEffects[effectInstance->getEffect()] = effectInstance;
        onEffectAdded(effectInstance);
    }
    else
    {
        MobEffectInstance* existing = mActiveEffects[effectInstance->getEffect()];
        existing->update(effectInstance);
        onEffectUpdated(existing, true);
    }
}

void LivingEntity::addEffectNoUpdate(MobEffectInstance* effectInstance)
{
    if (!canBeAffected(effectInstance))
    {
        return;
    }

    if (mActiveEffects.find(effectInstance->getEffect()) != mActiveEffects.end())
    {
        mActiveEffects[effectInstance->getEffect()] = effectInstance;
        onEffectAdded(effectInstance);
    }
    else
    {
        mActiveEffects[effectInstance->getEffect()]->update(effectInstance);
    }
}

bool LivingEntity::canBeAffected(MobEffectInstance* effectInstance)
{
    if (getMobType() == MOB_TYPE_UNDEAD
        && (effectInstance->getEffect() == MobEffects::REGENERATION
            || effectInstance->getEffect() == MobEffects::POISON))
    {
        return false;
    }

    return true;
}

bool LivingEntity::isInvertedHealAndHarm()
{
    return getMobType() == MOB_TYPE_UNDEAD;
}

MobEffectInstance* LivingEntity::removeEffectNoUpdate(MobEffect* effect)
{
    boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.find(effect);

    if (it == mActiveEffects.end())
    {
        return NULL;
    }

    MobEffectInstance* effectInstance = it->second;
    mActiveEffects.erase(it);

    return effectInstance;
}

void LivingEntity::removeEffect(MobEffect* effect)
{
    boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.find(effect);

    if (it == mActiveEffects.end())
    {
        return;
    }

    MobEffectInstance* effectInstance = it->second;

    if (effectInstance != NULL)
    {
        onEffectRemoved(effectInstance);
        delete effectInstance;
    }

    mActiveEffects.erase(it);
}

void LivingEntity::onEffectAdded(MobEffectInstance* effectInstance)
{
    mEffectsDirty = true;

    if (!mLevel->mIsClientSide)
    {
        MobEffect* effect = effectInstance->getEffect();
        effect->addAttributeModifiers(boost::static_pointer_cast<LivingEntity>(shared_from_this()),
                                      getAttributes(), effectInstance->getAmplifier());
    }
}

void LivingEntity::onEffectUpdated(MobEffectInstance* effectInstance, bool reapply)
{
    mEffectsDirty = true;

    if (reapply && !mLevel->mIsClientSide)
    {
        MobEffect* effect = effectInstance->getEffect();
        effect->removeAttributeModifiers(boost::static_pointer_cast<LivingEntity>(shared_from_this()),
                                         getAttributes(), effectInstance->getAmplifier());
        effect->addAttributeModifiers(boost::static_pointer_cast<LivingEntity>(shared_from_this()),
                                      getAttributes(), effectInstance->getAmplifier());
    }
}

void LivingEntity::onEffectRemoved(MobEffectInstance* effectInstance)
{
    mEffectsDirty = true;

    if (!mLevel->mIsClientSide)
    {
        MobEffect* effect = effectInstance->getEffect();
        effect->removeAttributeModifiers(boost::static_pointer_cast<LivingEntity>(shared_from_this()),
                                         getAttributes(), effectInstance->getAmplifier());
    }
}

void LivingEntity::heal(float amount)
{
    float health = getHealth();

    if (health > 0.0f)
    {
        setHealth(health + amount);
    }
}

float LivingEntity::getHealth()
{
    return mEntityData->get(DATA_HEALTH_ID);
}

void LivingEntity::setHealth(float health)
{
    mEntityData->set(DATA_HEALTH_ID, Mth::clamp(health, 0.0f, getMaxHealth()), true);
}


bool LivingEntity::hurt(const DamageSource* source, float damage)
{
    if ((isInvulnerable() && !source->isBypassInvul())
        || g_pApp->IsAppPaused()
        || (mLevel->mIsClientSide && dynamic_cast<const EntityDamageSource*>(source) == NULL))
    {
        return false;
    }

    mNoActionTime = 0;

    if (getHealth() <= 0.0f)
    {
        return false;
    }

    if (source->isFire() && hasEffect(MobEffects::FIRE_RESISTANCE))
    {
        if ((GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER && source == DamageSource::LAVA)
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(shared_from_this());
            player->awardStat(GenericStats::stayinFrosty(), GenericStats::param_stayinFrosty());
        }

        return false;
    }

    float initialDamage = damage;

    if (source == DamageSource::ANVIL || source == DamageSource::FALLING_BLOCK)
    {
        not_null_ptr<ItemInstance> helmet = getItemSlot(EquipmentSlot::HEAD);

        if (!helmet->isEmpty())
        {
            helmet->hurtAndBreak((int)(damage * 4.0f + mRandom->nextFloat() * damage * 2.0f),
                                 boost::static_pointer_cast<LivingEntity>(shared_from_this()));
            damage *= 0.75f;
        }
    }

    bool blocked = false;

    if (damage > 0.0f && isDamageSourceBlocked(source))
    {
        hurtCurrentlyUsedShield(damage);
        damage = 0.0f;
        blocked = true;
    }

    mAnimationSpeed = 1.5f;

    bool freshHurt = true;

    if (!source->isBypassGraceInvul() && (float)mInvulnerableTime > (float)mInvulnerableDuration * 0.5f)
    {
        if (damage <= mLastHurt)
        {
            return false;
        }

        if (!mLevel->mIsClientSide)
        {
            actuallyHurt(source, damage - mLastHurt);
        }

        mLastHurt = damage;
        mHurtDir = 0.0f;
        freshHurt = false;
    }
    else
    {
        mLastHurt = damage;
        mInvulnerableTime = mInvulnerableDuration;

        if (!mLevel->mIsClientSide)
        {
            actuallyHurt(source, damage);
        }

        mHurtDuration = 10;
        mHurtTime = 10;
        mHurtDir = 0.0f;
    }

    boost::shared_ptr<Entity> attacker = source->getEntity();

    if (attacker)
    {
        if ((attacker->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
        {
            mHurtByPlayerOnPeaceful = false;
            setLastHurtByMob(boost::static_pointer_cast<LivingEntity>(attacker));
        }

        if ((attacker->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        {
            if (mLevel->getDifficulty() == Difficulty::PEACEFUL)
            {
                mHurtByPlayerOnPeaceful = true;
            }

            mLastHurtByPlayerTime = 100;
            mLastHurtByPlayer = boost::static_pointer_cast<Player>(attacker);
        }
        else if (attacker->GetType() == ENTITY_TYPE_WOLF)
        {
            boost::shared_ptr<Wolf> wolf = boost::static_pointer_cast<Wolf>(attacker);

            if (wolf->isTame())
            {
                mLastHurtByPlayerTime = 100;
                mLastHurtByPlayer.reset();
            }
        }
    }

    if (source->isFireProjectile())
    {
        setSecondsOnFire(5);
    }

    if (mLevel->mIsClientSide)
    {
        return false;
    }

    if (freshHurt)
    {
        if (blocked)
        {
            mLevel->broadcastEntityEvent(shared_from_this(), 29, 0);
        }
        else if (dynamic_cast<const EntityDamageSource*>(source) != NULL
                 && static_cast<const EntityDamageSource*>(source)->isThorns())
        {
            mLevel->broadcastEntityEvent(shared_from_this(), 33, 0);
        }
        else
        {
            unsigned char event = 2;

            if (source == DamageSource::DROWN)
            {
                event = 36;
            }
            else if (source->isFire())
            {
                event = 37;
            }

            mLevel->broadcastEntityEvent(shared_from_this(), event, 0);
        }

        if (source != DamageSource::DROWN && (!blocked || damage > 0.0f))
        {
            markHurt();
        }

        if (attacker)
        {
            double dx = attacker->mX - mX;
            double dz = attacker->mZ - mZ;

            while (dx * dx + dz * dz < 0.0001)
            {
                dx = (Math::random() - Math::random()) * 0.01;
                dz = (Math::random() - Math::random()) * 0.01;
            }

            mHurtDir = (float)(atan2(dz, dx) * 180.0 / 3.1415927410125732 - (double)mYRot);
            knockback(attacker, 0.4f, dx, dz);
        }
        else
        {
            mHurtDir = (float)((int)(Math::random() * 2.0) * 180);
        }
    }

    MemSect(31);

    if (getHealth() <= 0.0f)
    {
        if (!checkTotemDeathProtection(source))
        {
            const SoundEvent* deathSound = getDeathSound();

            if (freshHurt && deathSound != NULL)
            {
                playSound(deathSound, getSoundVolume(), getVoicePitch());
            }

            die(source);
        }
    }
    else if (freshHurt)
    {
        const SoundEvent* hurtSound = getHurtSound(source);

        if (hurtSound != NULL)
        {
            playSound(hurtSound, getSoundVolume(), getVoicePitch());
        }
    }

    MemSect(0);

    bool damaged = !blocked || damage > 0.0f;

    if (damaged)
    {
        if (mLastDamageSource != NULL)
        {
            delete mLastDamageSource;
        }

        mLastDamageSource = new DamageSource(*source);
        mLastDamageStamp = mLevel->getGameTime();
    }

    return damaged;
}

const DamageSource* LivingEntity::getLastDamageSource()
{
    if (mLevel->getGameTime() - mLastDamageStamp > 40LL)
    {
        delete mLastDamageSource;
        mLastDamageSource = NULL;
    }

    return mLastDamageSource;
}

void LivingEntity::blockUsingShield(const boost::shared_ptr<LivingEntity>& attacker)
{
    attacker->knockback(shared_from_this(), 0.5f, mX - attacker->mX, mZ - attacker->mZ);
}

void LivingEntity::playHurtSound(const DamageSource* source)
{
    const SoundEvent* sound = getHurtSound(source);

    if (sound != NULL)
    {
        playSound(sound, getSoundVolume(), getVoicePitch());
    }
}

bool LivingEntity::checkTotemDeathProtection(const DamageSource* source)
{
    if (source->isBypassInvul())
    {
        return false;
    }

    not_null_ptr<ItemInstance> totem = ItemInstance::EMPTY;

    for (int hand = 0; hand < 2; ++hand)
    {
        not_null_ptr<ItemInstance> held = getItemInHand((InteractionHand::EInteractionHand)hand);

        if (held->getItem() == Items::totemOfUndying)
        {
            totem = held->copy();
            held->shrink(1);
            break;
        }
    }

    if (!totem->isEmpty())
    {
        setHealth(1.0f);
        removeAllEffects();
        addEffect(new MobEffectInstance(MobEffects::REGENERATION, 800, 1), boost::shared_ptr<Entity>());
        addEffect(new MobEffectInstance(MobEffects::ABSORPTION, 100, 1), boost::shared_ptr<Entity>());
        addEffect(new MobEffectInstance(MobEffects::FIRE_RESISTANCE, 800, 1), boost::shared_ptr<Entity>());
        mLevel->broadcastEntityEvent(shared_from_this(), 35, 0);
    }

    return !totem->isEmpty();
}

bool LivingEntity::isDamageSourceBlocked(const DamageSource* source)
{
    return !source->isBypassArmor() && isBlocking();
}

void LivingEntity::breakItem(not_null_ptr<ItemInstance> itemInstance)
{
    if (!mLevel->mIsClientSide)
    {
        mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::ITEM_BREAK,
                          getSoundSource(), 0.800000011920929f,
                          (float)(mLevel->mRandom->nextFloat() * 0.4000000059604645 + 0.800000011920929), 16.0f);
    }

    for (int i = 0; i < 5; ++i)
    {
        Vec3* velocity = Vec3::newTemp((mRandom->nextFloat() - 0.5) * 0.1, Math::random() * 0.1 + 0.1, 0.0);
        velocity = velocity->xRot(-(mXRot * 0.01745329238474369f));
        velocity = velocity->yRot(-(mYRot * 0.01745329238474369f));

        Vec3* offset = Vec3::newTemp((mRandom->nextFloat() - 0.5) * 0.3, -mRandom->nextFloat() * 0.6 - 0.3, 0.6);
        offset = offset->xRot(-(mXRot * 0.01745329238474369f));
        offset = offset->yRot(-(mYRot * 0.01745329238474369f));
        offset = offset->add(mX, mY + (double)getEyeHeight(), mZ);

        arrayWithLength<int> particleData(1, false);
        particleData.data[0] = Item::VALIDgetId(itemInstance->getItem());

        mLevel->addParticle(PARTICLE_ICON_CRACK, offset->x, offset->y, offset->z,
                            velocity->x, velocity->y + 0.05, velocity->z, particleData);
    }
}

void LivingEntity::die(const DamageSource* source)
{
    if (mDead)
    {
        return;
    }

    boost::shared_ptr<Entity> killer = source->getEntity();
    boost::shared_ptr<LivingEntity> credit = getKillCredit();

    if (mDeathScore >= 0 && credit)
    {
        credit->awardKillScore(shared_from_this(), mDeathScore, source);
    }

    if (killer)
    {
        killer->killed(boost::dynamic_pointer_cast<LivingEntity>(shared_from_this()));
    }

    mDead = true;
    getCombatTracker()->recheckStatus();

    if (!mLevel->mIsClientSide)
    {
        int looting = 0;
        boost::shared_ptr<Player> player;

        if (killer && (killer->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        {
            player = boost::dynamic_pointer_cast<Player>(killer);
            looting = EnchantmentHelper::getMobLooting(boost::dynamic_pointer_cast<LivingEntity>(player));
        }

        if (shouldDropExperience() && mLevel->getGameRules()->getBoolean(4))
        {
            dropAllDeathLoot(mLastHurtByPlayerTime > 0, looting, source);
        }

        if (player)
        {
            player->awardStat(gStats->get_killMob(),
                              GenericStats::param_mobKill(player,
                                                          boost::dynamic_pointer_cast<Mob>(shared_from_this()),
                                                          source));
        }
    }

    mLevel->broadcastEntityEvent(shared_from_this(), 3, 0);
}

void LivingEntity::dropEquipment(bool hitByPlayer, int lootingLevel)
{
}

void LivingEntity::knockback(boost::shared_ptr<Entity> attacker, float strength, double x, double z)
{
    AttributeInstance* resistance = getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE);

    if (mRandom->nextDouble() < resistance->getValue())
    {
        return;
    }

    mHasImpulse = true;

    double horizontal = Mth::sqrt(x * x + z * z);

    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureReducedKnockback())
    {
        strength = 0.10000000149011612f;
    }

    mXd = mXd * 0.5 - (x / horizontal) * (double)strength;
    mYd = mYd * 0.5;
    mZd = mZd * 0.5 - (z / horizontal) * (double)strength;

    if (mOnGround)
    {
        double lift = mYd + (double)strength;

        if (lift > 0.4000000059604645)
        {
            lift = 0.4000000059604645;
        }

        mYd = lift;
    }
}

const SoundEvent* LivingEntity::getHurtSound(const DamageSource* source)
{
    return SoundEvents::GENERIC_HURT;
}

const SoundEvent* LivingEntity::getDeathSound()
{
    return SoundEvents::GENERIC_DEATH;
}

const SoundEvent* LivingEntity::getFallDamageSound(int distance)
{
    if (distance > 4)
    {
        return SoundEvents::GENERIC_BIG_FALL;
    }

    return SoundEvents::GENERIC_SMALL_FALL;
}

void LivingEntity::dropAllDeathLoot(bool hitByPlayer, int lootingLevel, const DamageSource* source)
{
    dropDeathLoot(mLastHurtByPlayerTime > 0, lootingLevel);
    dropEquipment(mLastHurtByPlayerTime > 0, lootingLevel);
}

void LivingEntity::dropDeathLoot(bool hitByPlayer, int lootingLevel)
{
}

bool LivingEntity::trapdoorUsableAsLadder(const BlockPos& pos, const BlockState* state)
{
    if (!state->getValue(TrapDoorBlock::OPEN))
    {
        return false;
    }

    const BlockState* below = mLevel->getBlockState(mTickCache, pos.below());

    if (below->getBlock() != Blocks::ladder)
    {
        return false;
    }

    return below->getValue(LadderBlock::FACING) == state->getValue(TrapDoorBlock::FACING);
}

bool LivingEntity::onLadder()
{
    int x = Mth::floor(mX);
    int y = Mth::floor(mBoundingBox->mMinY);
    int z = Mth::floor(mZ);

    if ((GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER
        && boost::static_pointer_cast<Player>(shared_from_this())->isSpectator())
    {
        return false;
    }

    BlockPos pos(x, y, z);
    const BlockState* state = mLevel->getBlockState(mTickCache, pos);
    Block* block = state->getBlock();

    if (block == Blocks::ladder || block == Blocks::vine)
    {
        return true;
    }

    if (dynamic_cast<const TrapDoorBlock*>(block) != NULL && trapdoorUsableAsLadder(pos, state))
    {
        return true;
    }

    return false;
}

bool LivingEntity::isShootable()
{
    return true;
}

bool LivingEntity::isAlive()
{
    return !mRemoved && getHealth() > 0.0f;
}

void LivingEntity::causeFallDamage(float fallDistance, float damageMultiplier)
{
    Entity::causeFallDamage(fallDistance, damageMultiplier);

    int damage;

    if (hasEffect(MobEffects::JUMP))
    {
        int amplifier = getEffect(MobEffects::JUMP)->getAmplifier();
        damage = (int)ceilf((fallDistance - 3.0f - (float)((double)amplifier + 1.0)) * damageMultiplier);
    }
    else
    {
        damage = (int)ceilf((fallDistance - 3.0f - 0.0f) * damageMultiplier);
    }

    if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && !TakeGlideCollisionDamage())
    {
        return;
    }

    if (damage < 1)
    {
        return;
    }

    playSound(getFallDamageSound(damage), 1.0f, 1.0f);
    hurt(DamageSource::FALL, (float)damage);

    BlockPos pos(Mth::floor(mX), Mth::floor(mY - 0.20000000298023224), Mth::floor(mZ));
    const BlockState* state = mLevel->getBlockState(mTickCache, pos);

    if (state->getMaterial() != Material::AIR)
    {
        const SoundType* soundType = state->getBlock()->getSoundType();

        MemSect(31);
        playSound(soundType->getFallSound(), soundType->getVolume() * 0.5f, soundType->getPitch() * 0.75f);
        MemSect(0);
    }
}

void LivingEntity::animateHurt()
{
    mHurtDuration = 10;
    mHurtTime = 10;
    mHurtDir = 0.0f;
}

int LivingEntity::getArmorValue()
{
    return Mth::floor(getAttribute(SharedMonsterAttributes::ARMOR)->getValue());
}

void LivingEntity::hurtArmor(float damage)
{
}

void LivingEntity::hurtCurrentlyUsedShield(float damage)
{
}

float LivingEntity::getDamageAfterArmorAbsorb(const DamageSource* source, float damage)
{
    if (source->isBypassArmor())
    {
        return damage;
    }

    hurtArmor(damage);

    return CombatRules::getDamageAfterAbsorb(damage, (float)getArmorValue(),
                                             (float)getAttribute(SharedMonsterAttributes::ARMOR_TOUGHNESS)->getValue());
}

float LivingEntity::getDamageAfterMagicAbsorb(const DamageSource* source, float damage)
{
    if (source->isBypassMagic())
    {
        return damage;
    }

    if (hasEffect(MobEffects::DAMAGE_RESISTANCE)
        && source != DamageSource::OUT_OF_WORLD
        && source != DamageSource::KILL)
    {
        int amplifier = getEffect(MobEffects::DAMAGE_RESISTANCE)->getAmplifier();
        damage = damage * (float)(25 - (amplifier * 5 + 5)) / 25.0f;
    }

    if (damage <= 0.0f)
    {
        return 0.0f;
    }

    int protection = EnchantmentHelper::getDamageProtection(mArmorItems, source);

    if (protection > 0)
    {
        damage = CombatRules::getDamageAfterMagicAbsorb(damage, (float)protection);
    }

    return damage;
}

void LivingEntity::actuallyHurt(const DamageSource* source, float damage)
{
    if (isInvulnerable() && !source->isBypassInvul())
    {
        return;
    }

    damage = getDamageAfterArmorAbsorb(source, damage);
    damage = getDamageAfterMagicAbsorb(source, damage);

    float absorbed = damage - getAbsorptionAmount();

    if (absorbed < 0.0f)
    {
        absorbed = 0.0f;
    }

    setAbsorptionAmount(getAbsorptionAmount() - (damage - absorbed));

    if (absorbed != 0.0f)
    {
        float health = getHealth();
        setHealth(health - absorbed);
        getCombatTracker()->recordDamage(source, health, absorbed);
        setAbsorptionAmount(getAbsorptionAmount() - absorbed);
    }
}

void LivingEntity::spawnEatParticles(const ItemInstance& itemInstance, int count)
{
    for (int i = 0; i < count; ++i)
    {
        Vec3* velocity = Vec3::newTemp((mRandom->nextDouble() - 0.5) * 0.1, mRandom->nextDouble() * 0.1 + 0.1, 0.0);
        velocity = velocity->xRot(-(mXRot * 0.01745329238474369f));
        velocity = velocity->yRot(-(mYBodyRot * 0.01745329238474369f));
        velocity->y = velocity->y + 0.05;

        Vec3* offset = Vec3::newTemp((mRandom->nextDouble() - 0.5) * 0.3, -mRandom->nextDouble() * 0.6 - 0.3, 0.6);
        offset = offset->xRot(-(mXRot * 0.01745329238474369f));
        offset = offset->yRot(-(mYBodyRot * 0.01745329238474369f));
        offset = offset->add(mX, mY + (double)getEyeHeight(), mZ);

        arrayWithLength<int> particleData(1, false);
        particleData.data[0] = Item::getId(itemInstance.getItem());

        mLevel->addParticle(PARTICLE_ICON_CRACK, offset->x, offset->y, offset->z,
                            velocity->x, velocity->y, velocity->z, particleData);
    }
}

CombatTracker* LivingEntity::getCombatTracker()
{
    return mCombatTracker;
}

boost::shared_ptr<LivingEntity> LivingEntity::getKillCredit()
{
    boost::shared_ptr<LivingEntity> killer = mCombatTracker->getKiller();

    if (killer)
    {
        return killer;
    }

    if (mLastHurtByPlayer)
    {
        return boost::static_pointer_cast<LivingEntity>(mLastHurtByPlayer);
    }

    return mLastHurtByMob.lock();
}

float LivingEntity::getMaxHealth()
{
    return (float)getAttribute(SharedMonsterAttributes::MAX_HEALTH)->getValue();
}

int LivingEntity::getArrowCount()
{
    return mEntityData->get(DATA_ARROW_COUNT_ID);
}

void LivingEntity::setArrowCount(int arrowCount)
{
    mEntityData->set(DATA_ARROW_COUNT_ID, arrowCount, true);
}

int LivingEntity::getCurrentSwingDuration()
{
    int amplification = 0;

    if (hasEffect(MobEffects::DIG_SPEED))
    {
        amplification = getEffect(MobEffects::DIG_SPEED)->getAmplifier();
    }

    if (hasEffect(MobEffects::CONDUIT_POWER))
    {
        int conduit = getEffect(MobEffects::CONDUIT_POWER)->getAmplifier() + 1;

        if (amplification < conduit)
        {
            amplification = conduit;
        }
    }

    if (amplification > 0)
    {
        return 5 - amplification;
    }

    if (hasEffect(MobEffects::DIG_SLOWDOWN))
    {
        return getEffect(MobEffects::DIG_SLOWDOWN)->getAmplifier() * 2 + 8;
    }

    return 6;
}

bool LivingEntity::swing(InteractionHand::EInteractionHand hand)
{
    int swingTime = mSwingTime;
    bool wasSwinging = mSwinging;

    if (wasSwinging && swingTime >= 0 && swingTime < getCurrentSwingDuration() / 2)
    {
        return false;
    }

    mSwingTime = -1;

    if (wasSwinging)
    {
        if (swingTime != -1)
        {
            ++mSwingCount;
        }
    }
    else
    {
        mSwingCount = 0;
    }

    mSwingingArm = hand;
    mSwinging = true;

    ServerLevel* serverLevel = dynamic_cast<ServerLevel*>(mLevel);

    if (serverLevel != NULL)
    {
        serverLevel->getTracker()->broadcast(shared_from_this(),
                                             boost::shared_ptr<Packet>(new ClientboundAnimatePacket(shared_from_this(),
                                                                                                    hand == InteractionHand::eInteractionHand_MainHand ? 0 : 3)));
    }

    return true;
}

void LivingEntity::handleEntityEvent(unsigned char event, int data)
{
    if (event == 3)
    {
        MemSect(31);

        const SoundEvent* deathSound = getDeathSound();

        if (deathSound != NULL)
        {
            playSound(deathSound, getSoundVolume(),
                      (mRandom->nextFloat() - mRandom->nextFloat()) * 0.20000000298023224f + 1.0f);
        }

        MemSect(0);

        setHealth(0.0f);
        die(DamageSource::GENERIC);
        return;
    }

    if (event == 30)
    {
        playSound(SoundEvents::SHIELD_BREAK, 0.800000011920929f,
                  (float)(mLevel->mRandom->nextFloat() * 0.4000000059604645 + 0.800000011920929));
        return;
    }

    if (event != 2 && event != 29 && event != 33 && event != 36 && event != 37)
    {
        Entity::handleEntityEvent(event, data);
        return;
    }

    mHurtTime = 10;
    mAnimationSpeed = 1.5f;
    mHurtDir = 0.0f;
    mInvulnerableTime = mInvulnerableDuration;
    mHurtDuration = 10;

    if (event == 33)
    {
        playSound(SoundEvents::ENCHANT_THORNS_HIT, getSoundVolume(),
                  (mRandom->nextFloat() - mRandom->nextFloat()) * 0.20000000298023224f + 1.0f);
    }
    else if (event == 29)
    {
        playSound(SoundEvents::SHIELD_BLOCK, 1.0f,
                  (float)(mLevel->mRandom->nextFloat() * 0.4000000059604645 + 0.800000011920929));
    }

    const DamageSource* source = DamageSource::GENERIC;

    if (event == 37)
    {
        source = DamageSource::ON_FIRE;
    }
    else if (event == 36)
    {
        source = DamageSource::DROWN;
    }

    MemSect(31);

    const SoundEvent* hurtSound = getHurtSound(source);

    if (hurtSound != NULL)
    {
        playSound(hurtSound, getSoundVolume(),
                  (mRandom->nextFloat() - mRandom->nextFloat()) * 0.20000000298023224f + 1.0f);
    }

    MemSect(0);

    hurt(DamageSource::GENERIC, 0.0f);
}

void LivingEntity::outOfWorld()
{
    hurt(DamageSource::OUT_OF_WORLD, 4.0f);
}

void LivingEntity::updateSwingTime()
{
    int duration = getCurrentSwingDuration();

    if (mSwinging)
    {
        int swingTime = mSwingTime + 1;

        if (swingTime < duration)
        {
            mSwingTime = swingTime;
        }
        else
        {
            mSwingTime = 0;
            mSwinging = false;
        }
    }
    else
    {
        mSwingTime = 0;
    }

    mAttackAnim = (float)mSwingTime / (float)duration;
}

AttributeInstance* LivingEntity::getAttribute(Attribute* attribute)
{
    return getAttributes()->getInstance(attribute);
}

BaseAttributeMap* LivingEntity::getAttributes()
{
    if (mAttributes == NULL)
    {
        mAttributes = new ModifiableAttributeMap();
    }

    return mAttributes;
}

EMobType LivingEntity::getMobType()
{
    return MOB_TYPE_UNDEFINED;
}

not_null_ptr<ItemInstance> LivingEntity::getMainHandItem()
{
    return getItemSlot(EquipmentSlot::MAINHAND);
}

not_null_ptr<ItemInstance> LivingEntity::getOffhandItem()
{
    return getItemSlot(EquipmentSlot::OFFHAND);
}

not_null_ptr<ItemInstance> LivingEntity::getItemInHand(InteractionHand::EInteractionHand hand)
{
    if (hand == InteractionHand::eInteractionHand_MainHand)
    {
        return getItemSlot(EquipmentSlot::MAINHAND);
    }

    if (hand == InteractionHand::eInteractionHand_OffHand)
    {
        return getItemSlot(EquipmentSlot::OFFHAND);
    }

    return ItemInstance::EMPTY;
}

void LivingEntity::setItemInHand(InteractionHand::EInteractionHand hand, not_null_ptr<ItemInstance> itemInstance)
{
    if (hand == InteractionHand::eInteractionHand_MainHand)
    {
        setItemSlot(EquipmentSlot::MAINHAND, itemInstance);
    }
    else if (hand == InteractionHand::eInteractionHand_OffHand)
    {
        setItemSlot(EquipmentSlot::OFFHAND, itemInstance);
    }
}

bool LivingEntity::hasItemInSlot(const EquipmentSlot* slot)
{
    return !getItemSlot(slot)->isEmpty();
}

void LivingEntity::setSprinting(bool sprinting)
{
    Entity::setSprinting(sprinting);

    AttributeInstance* speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);

    if (speed->getModifier(eMODIFIER_ID_SprintingSpeedBoost) != NULL)
    {
        speed->removeModifier(eMODIFIER_ID_SprintingSpeedBoost);
    }

    if (sprinting)
    {
        speed->addModifier(new AttributeModifier(*SPEED_MODIFIER_SPRINTING));
    }
}

int LivingEntity::getItemInHandIcon(not_null_ptr<ItemInstance> itemInstance, int frame)
{
    Item* item = itemInstance->getItem();

    return (int)item->getLayerIcon(itemInstance->getAuxValue(), frame, itemInstance);
}

float LivingEntity::getSoundVolume()
{
    return 1.0f;
}

float LivingEntity::getVoicePitch()
{
    if (isBaby())
    {
        return (mRandom->nextFloat() - mRandom->nextFloat()) * 0.20000000298023224f + 1.5f;
    }

    return (mRandom->nextFloat() - mRandom->nextFloat()) * 0.20000000298023224f + 1.0f;
}

bool LivingEntity::isImmobile()
{
    return getHealth() <= 0.0f;
}


void LivingEntity::findStandUpPosition(boost::shared_ptr<Entity> vehicle)
{
    if (vehicle->GetType() != ENTITY_TYPE_BOAT
        && (vehicle->GetType() & ENTITY_TYPE_ABSTRACT_HORSE) != ENTITY_TYPE_ABSTRACT_HORSE)
    {
        double standX = vehicle->mX;
        double standY = vehicle->mBoundingBox->mMinY + (double)vehicle->mBbHeight;
        double standZ = vehicle->mZ;

        const Direction* forward = vehicle->getMotionDirection();

        if (forward != NULL)
        {
            const Direction* side = forward->getClockWise();
            const int offsets[9][2] = { { 0, 0 }, { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 },
                                        { -1, -1 }, { 1, -1 }, { -1, 1 }, { 1, 1 } };

            double baseX = floor(mX) + 0.5;
            double baseZ = floor(mZ) + 0.5;
            double halfWidth = (mBoundingBox->mMaxX - mBoundingBox->mMinX) * 0.5;
            double halfDepth = (mBoundingBox->mMaxZ - mBoundingBox->mMinZ) * 0.5;

            AABB* box = AABB::newPermanent(baseX - halfWidth, vehicle->mBoundingBox->mMinY,
                                           baseZ - halfDepth, baseX + halfWidth,
                                           floor(vehicle->mBoundingBox->mMinY) + (double)mBbHeight,
                                           baseZ + halfDepth);

            for (int i = 0; i < 9; ++i)
            {
                double dx = (double)(forward->getStepX() * offsets[i][0] + side->getStepX() * offsets[i][1]);
                double dz = (double)(forward->getStepZ() * offsets[i][0] + side->getStepZ() * offsets[i][1]);

                if (dx == 0.0 && dz == 0.0)
                {
                    continue;
                }

                int blockX = (int)(mX + dx);
                int blockZ = (int)(mZ + dz);

                if (mLevel->hasBlockCubes(box->move(dx, 0.0, dz)))
                {
                    continue;
                }

                BlockPos here(blockX, (int)mY, blockZ);

                if (mLevel->getBlockState(mTickCache, here)->isTopSolidBlocking())
                {
                    teleportTo(mX + dx, floor(mY) + 1.0, mZ + dz);
                    return;
                }

                BlockPos below(blockX, (int)mY - 1, blockZ);

                if (mLevel->getBlockState(mTickCache, below)->isTopSolidBlocking()
                    || mLevel->getBlockState(mTickCache, below)->getMaterial() == Material::WATER)
                {
                    standX = mX + dx;
                    standY = floor(mY) + 1.0;
                    standZ = mZ + dz;
                }
            }
        }

        teleportTo(standX, standY, standZ);
        return;
    }

    double distance = (double)(mBbWidth * 0.5f + vehicle->mBbWidth * 0.5f) + 0.4;
    float angle;

    if (vehicle->GetType() == ENTITY_TYPE_BOAT)
    {
        angle = 0.0f;
    }
    else
    {
        angle = 1.5707963705062866f * (getMainArm() == HumanoidArm::LEFT ? -1.0f : 1.0f);
    }

    double dirX = -(double)Mth::sin(-(mYRot * 0.01745329238474369f) - 3.1415927410125732f + angle);
    double dirZ = -(double)Mth::cos(-(mYRot * 0.01745329238474369f) - 3.1415927410125732f + angle);
    double scale = Mth::abs(dirX) - Mth::abs(dirZ) < 0.0 ? Mth::abs(dirZ) : Mth::abs(dirX);

    distance = distance / scale;

    double newX = mX + dirX * distance;
    double newZ = mZ + dirZ * distance;

    setPos(newX, vehicle->mY + (double)vehicle->mBbHeight + 0.001, newZ);

    if (mLevel->hasBlockCubes(mBoundingBox))
    {
        setPos(newX, vehicle->mY + (double)vehicle->mBbHeight + 1.001, newZ);

        if (mLevel->hasBlockCubes(mBoundingBox))
        {
            setPos(vehicle->mX, vehicle->mY + (double)mBbHeight + 0.001, vehicle->mZ);
        }
    }
}

bool LivingEntity::shouldShowName()
{
    return isCustomNameVisible();
}

float LivingEntity::getJumpPower()
{
    return 0.41999998688697815f;
}

void LivingEntity::jumpFromGround()
{
    mYd = (double)getJumpPower();

    if (hasEffect(MobEffects::JUMP))
    {
        mYd = mYd + (double)((float)(getEffect(MobEffects::JUMP)->getAmplifier() + 1) * 0.10000000149011612f);
    }

    if (isSprinting())
    {
        float radians = mYRot * 0.01745329238474369f;
        mXd = mXd - (double)(Mth::sin(radians) * 0.20000000298023224f);
        mZd = mZd + (double)(Mth::cos(radians) * 0.20000000298023224f);
    }

    mHasImpulse = true;
}

void LivingEntity::jumpInWater()
{
    mYd = mYd + 0.03999999910593033;
}

void LivingEntity::goDownInWater()
{
    mYd = mYd - 0.0030468758195638657;
}

void LivingEntity::jumpInLava()
{
    mYd = mYd + 0.0030468758195638657;
}

float LivingEntity::getWaterSlowDown()
{
    return 0.800000011920929f;
}

float LivingEntity::getFrictionForTravel()
{
    MutableBlockPos pos(Mth::floor(mX), Mth::floor(mBoundingBox->mMinY - 1.0), Mth::floor(mZ));

    float friction = 0.9100000262260437f;

    if (!mOnGround)
    {
        return friction;
    }

    Block* block = mLevel->getBlockState(mTickCache, pos)->getBlock();

    if (IsCreativeFlying()
        && (block == Blocks::ice || block == Blocks::packed_ice || block == Blocks::frosted_ice
            || block == Blocks::blue_ice || block == Blocks::slime))
    {
        return Blocks::AIR->mFriction * friction;
    }

    return mLevel->getBlockState(mTickCache, pos)->getBlock()->mFriction * friction;
}

bool LivingEntity::shouldApplyWaterGravity()
{
    if ((GetType() & ENTITY_TYPE_VILLAGER) != ENTITY_TYPE_VILLAGER
        && GetType() != ENTITY_TYPE_DOLPHIN
        && GetType() != ENTITY_TYPE_RABBIT)
    {
        return true;
    }

    return isImmersedInWater();
}

void LivingEntity::activateElytraSpeedBoost(double boost)
{
    mElytraSpeedBoost = boost;
    mElytraSpeedBoostActive = true;
}

void LivingEntity::CheckThermalAreas()
{
    SetLiftForceModifier(1.0);

    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();

    if (options == NULL)
    {
        return;
    }

    LevelRuleset* ruleset = options->getRequiredGameRules();

    if (g_pApp->GetFirstMiniGameType() != eMiniGameId_Glide || ruleset == NULL)
    {
        return;
    }

    int boostMod = 0;
    int boostDirection = 0;

    std::vector<ThermalAreaRuleDefinition*> areas;
    ruleset->getThermalAreas(&areas);

    ThermalAreaRuleDefinition* area = NULL;
    bool inside = false;

    for (std::vector<ThermalAreaRuleDefinition*>::iterator it = areas.begin(); it != areas.end(); ++it)
    {
        if (!(*it)->isEnabled())
        {
            continue;
        }

        if (!(*it)->getConditionsMet(shared_from_this()))
        {
            continue;
        }

        AABB* areaBounds = (*it)->getArea();
        if (!getBoundingBoxForCulling()->intersects(areaBounds->mMinX, areaBounds->mMinY, areaBounds->mMinZ,
                                                     areaBounds->mMaxX, areaBounds->mMaxY, areaBounds->mMaxZ))
        {
            continue;
        }

        area = *it;
        inside = true;
        break;
    }

    if (inside && !mThermalAreasChecked)
    {
        if (mThermalAreaChanged)
        {
            mThermalAreaChanged = false;

            if (!(area->isEnabled() && area->getElytraSpeedBoost() != 0.0))
            {
                if (mLevel->mIsClientSide)
                {
                    if (GetType() == ENTITY_TYPE_LOCAL_PLAYER)
                    {
                        Minecraft::GetInstance()->mSoundEngine->playUI(SoundEvents::MG03_THERMAL_BOOST, 1.0f, 1.0f);
                    }
                }
                else if (GetType() == ENTITY_TYPE_PLAYER)
                {
                    boost::shared_ptr<Packet> packet(new ClientboundSoundPacket(
                        ClientboundSoundPacket::eSoundInstances_Default, SoundEvents::MG03_THERMAL_BOOST,
                        1.0f, 1.0f, false, getId()));

                    gMinecraftServer->getPlayers()->broadcast(boost::static_pointer_cast<ServerPlayer>(shared_from_this()),
                                                              mX, mY, mZ, 30.0, mDimension, packet);
                }
            }
        }

        if (area->isEnabled())
        {
            if (!mInThermalArea)
            {
                if (area->getLiftSpeed() != 0.0)
                {
                    mThermalLerpTicks = 0;
                    mThermalLerping = true;
                }

                mThermalTargetLiftSpeed = area->getLiftSpeed();
            }

            mInThermalArea = true;
            mThermalCeiling = area->getMaxHeight();
            mThermalLiftRising = area->getLiftSpeed() >= 0.0;

            if (area->getElytraSpeedBoost() != 0.0)
            {
                activateElytraSpeedBoost(area->getElytraSpeedBoost());
                area->setBoostMods(boostMod, boostDirection);
            }
        }

        mThermalArea = area;
    }

    if (mThermalAreasChecked)
    {
        mThermalAreasChecked = inside;
        return;
    }

    if (!inside && mThermalArea != NULL)
    {
        mThermalAreaChanged = true;

        if (!mLevel->mIsClientSide)
        {
            mThermalArea->updateUseCount();
            mThermalArea = NULL;
        }
    }

    if (mInThermalArea)
    {
        if (mThermalCeiling <= mY || mVerticalCollision)
        {
            mInThermalArea = false;
        }
        else if (mThermalLerping)
        {
            if (mThermalLiftSpeed == 0.0 && mYd != 0.0)
            {
                mThermalLiftSpeed = mYd;
            }

            if (mThermalLiftRising)
            {
                if (mThermalTargetLiftSpeed <= mThermalLiftSpeed)
                {
                    mThermalLiftSpeed = mThermalTargetLiftSpeed;
                }
                else
                {
                    mThermalLiftSpeed = mThermalLiftSpeed
                                        + ((mThermalTargetLiftSpeed - mThermalLiftSpeed) / mThermalTargetLiftSpeed) * 0.1;
                }
            }
            else if (mThermalLiftSpeed <= mThermalTargetLiftSpeed)
            {
                mThermalLiftSpeed = mThermalTargetLiftSpeed;
            }
            else
            {
                mThermalLiftSpeed = -(((mThermalTargetLiftSpeed - mThermalLiftSpeed) / mThermalTargetLiftSpeed) * 0.1
                                      - mThermalLiftSpeed);
            }

            if (mThermalLerpTicks + 1 < 21)
            {
                ++mThermalLerpTicks;
            }
            else
            {
                mThermalLerping = false;
                mThermalLerpTicks = 0;
            }
        }
    }
    else if (mThermalLiftRising)
    {
        if (mThermalLiftSpeed <= 0.0)
        {
            mThermalLiftSpeed = 0.0;
        }
        else
        {
            mThermalLiftSpeed = mThermalLiftSpeed - 0.0010000000474974513;
        }
    }
    else if (mThermalLiftSpeed < 0.0)
    {
        mThermalLiftSpeed = mThermalLiftSpeed + 0.0010000000474974513;
    }
    else
    {
        mThermalLiftSpeed = 0.0;
    }

    if (mElytraSpeedBoostActive)
    {
        double speed = Mth::sqrt(mXd * mXd + mZd * mZd);

        if (speed >= mElytraSpeedBoost)
        {
            mXd = mXd * 0.9990000128746033;
            mZd = mZd * 0.9990000128746033;
            return;
        }

        double newXd = mXd + Mth::abs(mXd) * 0.1 * (double)boostMod;
        double newZd = mZd + Mth::abs(mZd) * 0.1 * (double)boostDirection;

        mXd = newXd;
        mZd = newZd;

        if (newXd * (double)boostMod < 0.0 || newZd * (double)boostDirection < 0.0)
        {
            mElytraSpeedBoostActive = false;
        }
    }
}

void LivingEntity::fallFlyingTravel(double& xd, double& yd, double& zd, Vec3* lookAngle,
                                    float& xRot, float& liftFactor, double& horizontalSpeed,
                                    double lookHorizontal)
{
    if (yd > -0.5)
    {
        liftFactor = 1.0f;
    }

    float pitch = xRot * 0.01745329238474369f;
    double lookLength = sqrt(lookAngle->x * lookAngle->x + lookAngle->z * lookAngle->z);
    horizontalSpeed = sqrt(xd * xd + zd * zd);

    double length = (double)lookAngle->length();
    float cosPitch = cosf(pitch);
    double lift = Mth::clamp(length / 0.4, 0.0, 1.0);

    lift = (double)((float)((double)cosPitch * (double)(float)((double)cosPitch * lift)) * (float)lookHorizontal);

    double newYd = yd + lift * 0.75 - 0.05;

    if (newYd < 0.0 && lookLength > 0.0)
    {
        double redirect = newYd * -0.1 * lift;
        yd = newYd + redirect;
        xd = xd + (lookAngle->x * redirect) / lookLength;
        zd = zd + (lookAngle->z * redirect) / lookLength;
    }
    else
    {
        yd = newYd;
    }

    if (pitch < 0.0f)
    {
        double dive = horizontalSpeed * -(double)sinf(pitch) * 0.04;
        yd = yd + dive * 3.2;
        xd = xd - (lookAngle->x * dive) / lookLength;
        zd = zd - (lookAngle->z * dive) / lookLength;
    }

    xd = xd + ((lookAngle->x / lookLength) * horizontalSpeed - xd) * 0.1;
    zd = zd + ((lookAngle->z / lookLength) * horizontalSpeed - zd) * 0.1;

    xd = xd * 0.9900000095367432;
    yd = yd * 0.9800000190734863;
    zd = zd * 0.9900000095367432;
}

void LivingEntity::travel(float xxa, float yya, float zza)
{
    boost::shared_ptr<Player> player;

    if ((GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        player = boost::static_pointer_cast<Player>(shared_from_this());
    }

    if (GetType() == ENTITY_TYPE_SKELETON_HORSE && hasPassenger(ENTITY_TYPE_PLAYER))
    {
        return;
    }

    if (PositionLocked())
    {
        mXd = 0.0;
        mZd = 0.0;

        if (Minecraft::InMiniGame(eMiniGameId_Tumble, false) || Minecraft::InMiniGame(eMiniGameId_Glide, false))
        {
            mYd = 0.0;
        }
    }

    if (!isEffectiveAi() && !isControlledByLocalInstance())
    {
        return;
    }

    if ((isInWater() && (!player || !player->mAbilities.mFlying))
        || (isInLava() && (!player || !player->mAbilities.mFlying)))
    {
        return;
    }

    if (isFallFlying() && !PositionLocked() && !g_pApp->IsAppPaused())
    {
        return;
    }

    MutableBlockPos groundPos(Mth::floor(mX), Mth::floor(mBoundingBox->mMinY - 1.0), Mth::floor(mZ));

    float friction = getFrictionForTravel();
    float acceleration = 0.16277135908603668f / (friction * friction * friction);

    PIXBeginNamedEvent(0.0f, "Update speed");

    float speed;

    if (mOnGround)
    {
        if (GetType() == ENTITY_TYPE_WITHER_BOSS)
        {
            speed = mFlyingSpeed * acceleration;
        }
        else
        {
            speed = mFlyingSpeed;
        }
    }
    else
    {
        speed = (float)(getSpeed() * (double)acceleration);
    }

    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "moveRelative");
    moveRelative(xxa, yya, zza, speed);
    PIXEndNamedEvent();

    friction = getFrictionForTravel();

    if (onLadder())
    {
        mXd = (double)Mth::clamp((float)mXd, -0.15000000596046448f, 0.15000000596046448f);
        mZd = (double)Mth::clamp((float)mZd, -0.15000000596046448f, 0.15000000596046448f);
        mFallDistance = 0.0f;

        if (mYd < -0.15000000596046448)
        {
            mYd = -0.15000000596046448;
        }

        if (isSneaking() && (GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER && mYd < 0.0)
        {
            mYd = 0.0;
        }
    }

    PIXBeginNamedEvent(0.0f, "move");
    move(eMoverType_Self, mXd, mYd, mZd, false);
    PIXEndNamedEvent();

    if (mHorizontalCollision && onLadder())
    {
        mYd = 0.20000000298023224;
    }

    PIXBeginNamedEvent(0.0f, "Update y delta");

    bool levitating = hasEffect(MobEffects::LEVITATION);
    bool loadedChunk = true;

    if (!levitating)
    {
        BlockPos here(Mth::floor(mX), 0, Mth::floor(mZ));

        if (mLevel->mIsClientSide && !CGameNetworkManager::sInstance.IsHost())
        {
            loadedChunk = false;
        }

        if (mLevel->hasChunkAt(here) && mLevel->getChunkAt(here)->isTerrainPopulated())
        {
            loadedChunk = true;
        }
    }

    bool lockVertical = PositionLocked()
                        && (Minecraft::InMiniGame(eMiniGameId_Tumble, false)
                            || Minecraft::InMiniGame(eMiniGameId_Glide, false));

    if ((!levitating && !loadedChunk) || lockVertical)
    {
        mYd = 0.0;
    }
    else if (levitating)
    {
        mYd = mYd + (0.05 * (double)(getEffect(MobEffects::LEVITATION)->getAmplifier() + 1) - mYd) * 0.2;
    }
    else if (isNoGravity())
    {
        if (mYd >= 0.0)
        {
            mYd = mYd + 0.0;
        }
        else if (hasEffect(MobEffects::SLOW_FALLING))
        {
            mYd = mYd + 0.0;
        }
        else
        {
            mYd = mYd - 0.0784000015258789;
            mFallDistance = 0.0f;
        }
    }

    mYd = mYd * 0.9800000190734863;
    mXd = mXd * (double)friction;
    mZd = mZd * (double)friction;

    PIXEndNamedEvent();
}

void LivingEntity::updateModelAnimation()
{
    double dx = mX - mXo;
    double dz = mZ - mZo;

    mAnimationSpeedOld = mAnimationSpeed;

    float target;

    if ((GetType() & ENTITY_TYPE_FLYING_MOB) == ENTITY_TYPE_FLYING_MOB)
    {
        double dy = mY - mYo;
        target = (float)(Mth::sqrt(dx * dx + dy * dy + dz * dz) * 4.0);
    }
    else
    {
        target = (float)(Mth::sqrt(dx * dx + dz * dz) * 4.0);
    }

    if (target > 1.0f)
    {
        target = 1.0f;
    }

    mAnimationSpeed = mAnimationSpeed + (target - mAnimationSpeed) * 0.4000000059604645f;
    mAnimationPosition = mAnimationPosition + mAnimationSpeed;
}

void LivingEntity::takeFallFlyingCollisionDamage()
{
    if (mLevel->mIsClientSide)
    {
        return;
    }

    double xd = mXd;
    double yd = mYd;
    double zd = mZd;
    float xRot = mXRot;
    float liftFactor = mFallDistance;
    double horizontalSpeed = 0.0;

    fallFlyingTravel(xd, yd, zd, getLookAngle(), xRot, liftFactor, horizontalSpeed,
                     GetLiftForceModifier());

    if (mHorizontalCollision)
    {
        double speed = sqrt(xd * xd + zd * zd);
        double damage = (horizontalSpeed - speed) * 10.0 - 3.0;

        if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && TakeGlideCollisionDamage())
        {
            damage = 1.0;
        }

        if (hurt(DamageSource::FALL, (float)damage))
        {
            playSound(getFallDamageSound((int)damage), 1.0f, 1.0f);
        }
    }
    else if (mVerticalCollision && Minecraft::InMiniGame(eMiniGameId_Glide, true)
             && TakeGlideCollisionDamage() && hurt(DamageSource::FALL, 1.0f))
    {
        playSound(getFallDamageSound(1), 1.0f, 1.0f);
    }

    if (mOnGround)
    {
        if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && TakeGlideCollisionDamage())
        {
            float health = getHealth();

            if (hurt(DamageSource::FALL, health + 1.0f))
            {
                playSound(getFallDamageSound((int)(health + 1.0f)), 1.0f, 1.0f);
            }
        }

        setSharedFlag(9, false);
    }
}

int LivingEntity::getLightColor()
{
    AABB* box = mBoundingBox;

    double minX = box->mMinX;
    double minY = box->mMinY;
    double minZ = box->mMinZ;
    double maxX = box->mMaxX;
    double maxY = box->mMaxY;
    double maxZ = box->mMaxZ;

    bool riding = false;

    if (isPassenger() && getVehicle())
    {
        if ((getVehicle()->GetType() & ENTITY_TYPE_MINECART) != ENTITY_TYPE_MINECART
            && getVehicle()->GetType() == ENTITY_TYPE_BOAT)
        {
            riding = true;
        }
    }

    int minBlockX = mLightSampleMinX;
    int minBlockY = mLightSampleMinY;
    int minBlockZ = mLightSampleMinZ;
    int maxBlockX = mLightSampleMaxX;
    int maxBlockY = mLightSampleMaxY;
    int maxBlockZ = mLightSampleMaxZ;

    if (riding)
    {
        double liftedMinY = minY + 1.0;

        if (maxY <= liftedMinY)
        {
            maxY = liftedMinY + 0.1;
        }

        minBlockX = Mth::floor(minX);
        minBlockY = Mth::floor(liftedMinY);
        minBlockZ = Mth::floor(minZ);
        maxBlockX = Mth::floor(maxX);
        maxBlockY = Mth::floor(maxY);
        maxBlockZ = Mth::floor(maxZ);
    }

    int sampleCount = ((maxBlockX - minBlockX) + 1) * ((maxBlockY - minBlockY) + 1) * ((maxBlockZ - minBlockZ) + 1);

    int* samples = mLightSamples;

    if (sampleCount > 32)
    {
        samples = new int[sampleCount];
    }

    bool resample = true;

    if (mLightCacheTicks >= 0)
    {
        if (mLightCacheTicks < 1)
        {
            int colour = Entity::getLightColor();

            if (colour != mLastLightColor)
            {
                mLastLightColor = colour;
                ++mLightCacheTicks;
            }

            if (mLightCacheTicks < 1)
            {
                resample = false;
            }
        }

        if (resample)
        {
            --mLightCacheTicks;
        }
    }
    else
    {
        resample = false;
    }

    if (resample)
    {
        int index = 0;

        for (int x = minBlockX; x <= maxBlockX; ++x)
        {
            for (int y = minBlockY; y <= maxBlockY; ++y)
            {
                for (int z = minBlockZ; z <= maxBlockZ; ++z)
                {
                    samples[index] = mLevel->getLightColor(BlockPos(x, y, z), 0, -1);
                    ++index;
                }
            }
        }
    }

    float blockLight = 0.0f;
    float skyLight = 0.0f;
    int index = 0;

    for (int x = minBlockX; x <= maxBlockX; ++x)
    {
        for (int y = minBlockY; y <= maxBlockY; ++y)
        {
            for (int z = minBlockZ; z <= maxBlockZ; ++z)
            {
                int sample = samples[index];
                ++index;

                blockLight = blockLight + (float)(sample & 255);
                skyLight = skyLight + (float)((sample >> 16) & 255);
            }
        }
    }

    if (sampleCount > 32)
    {
        delete[] samples;
    }

    if (blockLight > 240.0f)
    {
        blockLight = 240.0f;
    }

    if (skyLight > 240.0f)
    {
        skyLight = 240.0f;
    }

    return ((int)skyLight << 16) | (int)blockLight;
}

void LivingEntity::setBoundingBox(AABB* boundingBox)
{
    int minX = Mth::floor(boundingBox->mMinX);
    int minY = Mth::floor(boundingBox->mMinY);
    int minZ = Mth::floor(boundingBox->mMinZ);
    int maxX = Mth::floor(boundingBox->mMaxX);
    int maxY = Mth::floor(boundingBox->mMaxY);
    int maxZ = Mth::floor(boundingBox->mMaxZ);

    bool changed = minX != mLightSampleMinX || maxX != mLightSampleMaxX || minY != mLightSampleMinY
                   || maxY != mLightSampleMaxY || minZ != mLightSampleMinZ || maxZ != mLightSampleMaxZ;

    mLightSampleMinX = minX;
    mLightSampleMaxX = maxX;
    mLightSampleMinY = minY;
    mLightSampleMaxY = maxY;
    mLightSampleMinZ = minZ;
    mLightSampleMaxZ = maxZ;

    if (changed)
    {
        ++mLightCacheTicks;
    }

    Entity::setBoundingBox(boundingBox);
}

float LivingEntity::getSpeed()
{
    return mSpeed;
}

void LivingEntity::setSpeed(float speed)
{
    mSpeed = speed;
}

bool LivingEntity::doHurtTarget(boost::shared_ptr<Entity> target)
{
    setLastHurtMob(target);

    return false;
}

bool LivingEntity::isSleeping()
{
    return false;
}


void LivingEntity::updateEquipmentSlots()
{
    for (int i = 0; i < 6; ++i)
    {
        const EquipmentSlot* slot = EquipmentSlot::SLOTS[i];
        not_null_ptr<ItemInstance> previous;

        if (slot->getType() == EquipmentSlot::HAND)
        {
            previous = mHandItems[slot->getIndex()];
        }
        else
        {
            previous = mArmorItems[slot->getIndex()];
        }

        not_null_ptr<ItemInstance> current = getItemSlot(slot);

        if (ItemInstance::matches(current, previous, true))
        {
            continue;
        }

        static_cast<ServerLevel*>(mLevel)->getTracker()->broadcast(shared_from_this(),
                                                                   boost::shared_ptr<Packet>(new ClientboundSetEquippedItemPacket(getId(), slot, current)));

        if (!previous->isEmpty())
        {
            getAttributes()->removeAttributeModifiers(previous->getAttributeModifiers(slot));
        }

        if (!current->isEmpty())
        {
            getAttributes()->addAttributeModifiers(current->getAttributeModifiers(slot));
        }

        if (slot->getType() == EquipmentSlot::HAND)
        {
            mHandItems[slot->getIndex()] = current->isEmpty() ? ItemInstance::EMPTY : current->copy();
        }
        else
        {
            mArmorItems[slot->getIndex()] = current->isEmpty() ? ItemInstance::EMPTY : current->copy();
        }
    }
}

void LivingEntity::tick()
{
    PIXBeginNamedEvent(0.0f, "Living entity tick");

    mTickCache.buildCache(mLevel, AABB::newTemp(mBoundingBox));

    Entity::tick();

    updatingUsingItem();

    if (!mLevel->mIsClientSide)
    {
        int arrows = getArrowCount();

        if (arrows > 0)
        {
            if (mRemoveArrowTime < 1)
            {
                mRemoveArrowTime = 600 - arrows * 20;
            }

            --mRemoveArrowTime;

            if (mRemoveArrowTime < 1)
            {
                setArrowCount(arrows - 1);
            }
        }

        updateEquipmentSlots();

        if (mTickCount % 20 == 0)
        {
            PIXBeginNamedEvent(0.0f, "Recheck combat tracker status");
            getCombatTracker()->recheckStatus();
            PIXEndNamedEvent();
        }

        if (!mGlowing)
        {
            bool glowing = hasEffect(MobEffects::GLOWING);

            if (getSharedFlag(8) != glowing)
            {
                setSharedFlag(8, glowing);
            }
        }
    }

    if (mInsomniaComponent != NULL)
    {
        mInsomniaComponent->tick();
    }

    BlockPos pos(mX + (double)(mBbWidth * 0.5f), mY, mZ + (double)(mBbWidth * 0.5f));
    const BlockState* state = mLevel->getBlockState(mTickCache, pos);

    if (Block::getId(state->getBlock()) == 272)
    {
        if (!mSwimSoundPlayed)
        {
            if (BubbleColumnSegment::getFlowDownward(state))
            {
                mLevel->playLocalSound(mX, mY, mZ, SoundEvents::BUBBLE_COLUMN_UPWARDS_INSIDE,
                                       getSoundSource(), 1.0f, 1.0f, false, 16.0f);
            }
            else
            {
                mLevel->playLocalSound(mX, mY, mZ, SoundEvents::BUBBLE_COLUMN_WHIRLPOOL_INSIDE,
                                       getSoundSource(), 1.0f, 1.0f, false, 16.0f);
            }
        }

        mSwimSoundPlayed = true;
    }
    else
    {
        mSwimSoundPlayed = false;
    }

    PIXBeginNamedEvent(0.0f, "aiStep");
    aiStep();
    PIXEndNamedEvent();

    double dx = mX - mXo;
    double dz = mZ - mZo;
    float travelled = (float)(dx * dx + dz * dz);
    float bodyRot = mYBodyRot;
    float headTurn = 0.0f;

    mORun = mRun;

    if (travelled > 0.0025000000745058060f)
    {
        travelled = Mth::sqrt(travelled) * 3.0f;

        float wanted = (float)atan2(dz, dx) * 57.2957763671875f - 90.0f;
        float diff = Mth::abs(Mth::wrapDegrees(mYRot) - wanted);

        if (diff > 5.0f && diff < 175.0f)
        {
            bodyRot = wanted - 180.0f;
        }
        else
        {
            bodyRot = wanted;
        }
    }
    else
    {
        travelled = 0.0f;
    }

    if (mAttackAnim > 0.0f)
    {
        bodyRot = mYRot;
    }

    if (!mOnGround)
    {
        travelled = 0.0f;
    }

    mRun = mRun + (travelled - mRun) * 0.30000001192092896f;

    headTurn = tickHeadTurn(bodyRot, headTurn);

    while (mYRot - mYRotO < -180.0f)
    {
        mYRotO = mYRotO - 360.0f;
    }

    while (mYRot - mYRotO >= 180.0f)
    {
        mYRotO = mYRotO + 360.0f;
    }

    while (mYBodyRot - mYBodyRotO < -180.0f)
    {
        mYBodyRotO = mYBodyRotO - 360.0f;
    }

    while (mYBodyRot - mYBodyRotO >= 180.0f)
    {
        mYBodyRotO = mYBodyRotO + 360.0f;
    }

    while (mXRot - mXRotO < -180.0f)
    {
        mXRotO = mXRotO - 360.0f;
    }

    while (mXRot - mXRotO >= 180.0f)
    {
        mXRotO = mXRotO + 360.0f;
    }

    while (mYHeadRot - mYHeadRotO < -180.0f)
    {
        mYHeadRotO = mYHeadRotO - 360.0f;
    }

    while (mYHeadRot - mYHeadRotO >= 180.0f)
    {
        mYHeadRotO = mYHeadRotO + 360.0f;
    }

    mAnimStep = mAnimStep + headTurn;

    if (isFallFlying())
    {
        ++mFallFlyTicks;
    }
    else
    {
        mFallFlyTicks = 0;
    }

    PIXEndNamedEvent();
}

float LivingEntity::tickHeadTurn(float yBodyRot, float headTurnDelta)
{
    mYBodyRot = mYBodyRot + Mth::wrapDegrees(yBodyRot - mYBodyRot) * 0.30000001192092896f;

    float diff = Mth::wrapDegrees(mYRot - mYBodyRot);
    bool turnedAround = !(diff >= -75.0f && diff < 90.0f);

    float clamped = diff;

    if (clamped < -50.0f)
    {
        clamped = -50.0f;
    }
    else if (clamped > 50.0f)
    {
        clamped = 50.0f;
    }

    mYBodyRot = mYRot - clamped;

    if (clamped * clamped > 2500.0f)
    {
        mYBodyRot = mYBodyRot + clamped * 0.20000000298023224f;
    }

    if (turnedAround)
    {
        headTurnDelta = -headTurnDelta;
    }

    return headTurnDelta;
}

void LivingEntity::updateFallFlying()
{
    bool flying = getSharedFlag(9);

    bool blocked = false;

    if (isInWater() && g_pApp->GetFirstMiniGameType() != eMiniGameId_Glide)
    {
        blocked = true;
    }

    if (!flying || mOnGround || blocked || isPassenger())
    {
        flying = false;
    }
    else
    {
        not_null_ptr<ItemInstance> chest = getItemSlot(EquipmentSlot::CHEST);

        flying = false;

        if (chest->getItem() == Items::elytra && ElytraItem::isFlyEnabled(chest))
        {
            flying = true;

            if ((mTickCount & 19) == 0)
            {
                chest->hurtAndBreak(1, boost::static_pointer_cast<LivingEntity>(shared_from_this()));
            }
        }
    }

    if (!mLevel->mIsClientSide)
    {
        setSharedFlag(9, flying);
    }
}

void LivingEntity::aiStep()
{
    if (mNoJumpDelay > 0)
    {
        --mNoJumpDelay;
    }

    if (isControlledByLocalInstance())
    {
        mLerpSteps = 0;
        absMoveTo(mX, mY, mZ, mYRot, mXRot);
    }

    if (mLerpSteps > 0)
    {
        double lerpX = mX + (mLerpX - mX) / (double)mLerpSteps;
        double lerpY = mY + (mLerpY - mY) / (double)mLerpSteps;
        double lerpZ = mZ + (mLerpZ - mZ) / (double)mLerpSteps;

        mYRot = mYRot + (float)(Mth::wrapDegrees(mLerpYRot - (double)mYRot) / (double)mLerpSteps);
        mXRot = mXRot + (float)((mLerpXRot - (double)mXRot) / (double)mLerpSteps);

        --mLerpSteps;

        setPos(lerpX, lerpY, lerpZ);
        setRot(mYRot, mXRot);
    }
    else if (!isEffectiveAi())
    {
        mXd = mXd * 0.9800000190734863;
        mYd = mYd * 0.9800000190734863;
        mZd = mZd * 0.9800000190734863;
    }

    if (Mth::abs((float)mXd) < 0.003000000026077032f)
    {
        mXd = 0.0;
    }

    if (Mth::abs((float)mYd) < 0.003000000026077032f)
    {
        mYd = 0.0;
    }

    if (Mth::abs((float)mZd) < 0.003000000026077032f)
    {
        mZd = 0.0;
    }

    PIXBeginNamedEvent(0.0f, "ai");

    if (isImmobile())
    {
        mJumping = false;
        mXxa = 0.0f;
        mZza = 0.0f;
        mYRota = 0.0f;
    }
    else if (isEffectiveAi())
    {
        serverAiStep();
    }

    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "jump");

    if (mJumping)
    {
        if (isInWater())
        {
            jumpInWater();
        }
        else if (isInLava())
        {
            jumpInLava();
        }
        else if (mOnGround && mNoJumpDelay == 0)
        {
            jumpFromGround();
            mNoJumpDelay = 10;
        }
    }
    else
    {
        mNoJumpDelay = 0;
    }

    PIXEndNamedEvent();

    mXxa = mXxa * 0.9800000190734863f;
    mZza = mZza * 0.9800000190734863f;
    mYRota = mYRota * 0.8999999761581421f;

    PIXBeginNamedEvent(0.0f, "travel");
    travel(mXxa, mYya, mZza);
    PIXEndNamedEvent();

    if (!g_pApp->IsAppPaused())
    {
        updateFallFlying();
    }

    PIXBeginNamedEvent(0.0f, "push entities");

    if (!Minecraft::InMiniGame(eMiniGameId_Glide, false))
    {
        pushEntities();
    }

    PIXEndNamedEvent();
}

void LivingEntity::serverAiStep()
{
}

void LivingEntity::pushEntities()
{
    PushableBySelector selector(shared_from_this());
    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntities(shared_from_this(), mBoundingBox->grow(0.20000000298023224, 0.0, 0.20000000298023224),
                            &selector, NULL);

    if (entities == NULL || entities->empty())
    {
        return;
    }

    int maxCramming = mLevel->getGameRules()->getInt(17);

    if (maxCramming > 0 && entities->size() >= (unsigned int)maxCramming && mRandom->nextInt(4) == 0)
    {
        int passengers = 0;

        for (unsigned int i = 0; i < entities->size(); ++i)
        {
            if (!entities->at(i)->isPassenger())
            {
                ++passengers;
            }
        }

        if (passengers > maxCramming - 1)
        {
            hurt(DamageSource::CRAMMING, 6.0f);
        }
    }

    for (unsigned int i = 0; i < entities->size(); ++i)
    {
        entities->at(i)->push(shared_from_this());
    }
}

void LivingEntity::doPush(boost::shared_ptr<Entity> entity)
{
    entity->push(shared_from_this());
}

void LivingEntity::rideTick()
{
    Entity::rideTick();

    mORun = mRun;
    mRun = 0.0f;
    mFallDistance = 0.0f;
}

void LivingEntity::lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport)
{
    mLerpX = x;
    mLerpY = y;
    mLerpZ = z;
    mLerpYRot = (double)yRot;
    mLerpXRot = (double)xRot;
    mLerpSteps = steps;
}

void LivingEntity::setJumping(bool jumping)
{
    mJumping = jumping;
}

void LivingEntity::take(boost::shared_ptr<Entity> entity, int count)
{
    if (entity->mRemoved || mLevel->mIsClientSide)
    {
        return;
    }

    if (entity->GetType() != ENTITY_TYPE_ITEM_ENTITY
        && (entity->GetType() & ENTITY_TYPE_ARROW) != ENTITY_TYPE_ARROW
        && entity->GetType() != ENTITY_TYPE_EXPERIENCE_ORB)
    {
        return;
    }

    static_cast<ServerLevel*>(mLevel)->getTracker()->broadcast(entity,
                                                               boost::shared_ptr<Packet>(new ClientboundTakeItemEntityPacket(entity->getId(), getId(), count)));
}

bool LivingEntity::canSee(boost::shared_ptr<Entity> entity)
{
    Vec3* from = Vec3::newTemp(mX, mY + (double)getEyeHeight(), mZ);
    Vec3* to = Vec3::newTemp(entity->mX, entity->mY + (double)entity->getEyeHeight(), entity->mZ);

    HitResult* hit = mLevel->clip(from, to);

    if (hit != NULL)
    {
        delete hit;
        return false;
    }

    return true;
}

Vec3* LivingEntity::getViewVector(float partialTicks)
{
    if (partialTicks == 1.0f)
    {
        return calculateViewVector(mXRot, mYHeadRot);
    }

    return calculateViewVector(mXRotO + (mXRot - mXRotO) * partialTicks,
                               mYHeadRotO + (mYHeadRot - mYHeadRotO) * partialTicks);
}

float LivingEntity::getAttackAnim(float partialTicks)
{
    float delta = mAttackAnim - mOAttackAnim;

    if (delta < 0.0f)
    {
        delta = delta + 1.0f;
    }

    return mOAttackAnim + delta * partialTicks;
}

AABB* LivingEntity::getSweptVolume()
{
    return AABB::newTemp(mX, mY, mZ, mXOld, mYOld, mZOld);
}

bool LivingEntity::isEffectiveAi()
{
    return !mLevel->mIsClientSide;
}

bool LivingEntity::isPickable()
{
    return !mRemoved;
}

bool LivingEntity::isPushable(boost::shared_ptr<Entity> entity)
{
    return isAlive() && !onLadder();
}

void LivingEntity::markHurt()
{
    mHurtMarked = getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE)->getValue() <= mRandom->nextDouble();
}

float LivingEntity::getYHeadRot()
{
    return mYHeadRot;
}

void LivingEntity::setYHeadRot(float yHeadRot)
{
    mYHeadRot = yHeadRot;
}

void LivingEntity::setYBodyRot(float yBodyRot)
{
    mYBodyRot = yBodyRot;
}

float LivingEntity::getAbsorptionAmount()
{
    return mAbsorptionAmount;
}

void LivingEntity::setAbsorptionAmount(float absorptionAmount)
{
    if (absorptionAmount < 0.0f)
    {
        absorptionAmount = 0.0f;
    }

    mAbsorptionAmount = absorptionAmount;
}

void LivingEntity::onEnterCombat()
{
}

void LivingEntity::onLeaveCombat()
{
}

void LivingEntity::updateEffectVisibility()
{
    mEffectsDirty = true;
}

bool LivingEntity::isUsingItem()
{
    return (mEntityData->get(DATA_LIVING_ENTITY_FLAGS) & 1) != 0;
}

InteractionHand::EInteractionHand LivingEntity::getUsedItemHand()
{
    return (mEntityData->get(DATA_LIVING_ENTITY_FLAGS) & 2) != 0
               ? InteractionHand::eInteractionHand_OffHand
               : InteractionHand::eInteractionHand_MainHand;
}

int LivingEntity::getUseItemRemainingTicks()
{
    return mUseItemRemaining;
}

void LivingEntity::updatingUsingItem()
{
    if (!isUsingItem())
    {
        return;
    }

    if (getItemInHand(getUsedItemHand()) == mUseItem)
    {
        int remaining = getUseItemRemainingTicks();

        if (remaining < 26 && (remaining & 3) == 0)
        {
            spawnItemUseParticles(mUseItem, 5);
        }

        --mUseItemRemaining;

        if (mUseItemRemaining == 0 && !mLevel->mIsClientSide)
        {
            completeUsingItem();
        }
    }
    else
    {
        stopUsingItem();
    }
}

void LivingEntity::startUsingItem(InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = getItemInHand(hand);

    if (itemInstance->isEmpty() || isUsingItem())
    {
        return;
    }

    mUseItem = itemInstance;
    mUseItemRemaining = itemInstance->getUseDuration();

    if (!mLevel->mIsClientSide)
    {
        mEntityData->set(DATA_LIVING_ENTITY_FLAGS,
                         (unsigned char)(hand == InteractionHand::eInteractionHand_OffHand ? 3 : 1),
                         true);
    }
}

void LivingEntity::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    Entity::onSyncedDataUpdated(accessor);

    if (DATA_LIVING_ENTITY_FLAGS != accessor)
    {
        return;
    }

    if (mLevel->mIsClientSide)
    {
        if (isUsingItem() && mUseItem->isEmpty())
        {
            mUseItem = getItemInHand(getUsedItemHand());

            if (!mUseItem->isEmpty())
            {
                mUseItemRemaining = mUseItem->getUseDuration();
            }

            return;
        }
    }

    if (!isUsingItem() && !mUseItem->isEmpty())
    {
        mUseItem = ItemInstance::EMPTY;
        mUseItemRemaining = 0;
    }
}

void LivingEntity::spawnItemUseParticles(not_null_ptr<ItemInstance> itemInstance, int count)
{
    if (itemInstance->isEmpty() || !isUsingItem())
    {
        return;
    }

    if (itemInstance->getUseAnimation() == 2)
    {
        playSound(SoundEvents::GENERIC_DRINK, 0.5f,
                  (float)(mRandom->nextFloat() * 0.10000000149011612 + 0.8999999761581421));
    }

    if (itemInstance->getUseAnimation() != 1)
    {
        return;
    }

    for (int i = 0; i < count; ++i)
    {
        Vec3* velocity = Vec3::newTemp((mRandom->nextFloat() - 0.5) * 0.1, Math::random() * 0.1 + 0.1, 0.0);
        velocity = velocity->xRot(-(mXRot * 0.01745329238474369f));
        velocity = velocity->yRot(-(mYRot * 0.01745329238474369f));

        Vec3* offset = Vec3::newTemp((mRandom->nextFloat() - 0.5) * 0.3, -mRandom->nextFloat() * 0.6 - 0.3, 0.6);
        offset = offset->xRot(-(mXRot * 0.01745329238474369f));
        offset = offset->yRot(-(mYRot * 0.01745329238474369f));
        offset = offset->add(mX, mY + (double)getEyeHeight(), mZ);

        if (itemInstance->isStackedByData())
        {
            arrayWithLength<int> particleData(2, false);
            particleData.data[0] = Item::getId(itemInstance->getItem());
            particleData.data[1] = itemInstance->getAuxValue();

            mLevel->addParticle(PARTICLE_ICON_CRACK, offset->x, offset->y, offset->z,
                                velocity->x, velocity->y + 0.05, velocity->z, particleData);
        }
        else
        {
            arrayWithLength<int> particleData(1, false);
            particleData.data[0] = Item::getId(itemInstance->getItem());

            mLevel->addParticle(PARTICLE_ICON_CRACK, offset->x, offset->y, offset->z,
                                velocity->x, velocity->y + 0.05, velocity->z, particleData);
        }
    }

    playSound(SoundEvents::GENERIC_EAT, (float)(0.5 + 0.5 * (double)mRandom->nextInt(2)),
              (mRandom->nextFloat() - mRandom->nextFloat()) * 0.20000000298023224f + 1.0f);
}

void LivingEntity::completeUsingItem()
{
    if (mUseItem->isEmpty() || !isUsingItem())
    {
        return;
    }

    spawnItemUseParticles(mUseItem, 16);

    mUseItem->finishUsingItem(mLevel, boost::dynamic_pointer_cast<Player>(shared_from_this()));

    getUsedItemHand();
    stopUsingItem();
}

int LivingEntity::getTicksUsingItem()
{
    if (!isUsingItem() || mUseItem->isEmpty())
    {
        return 0;
    }

    return mUseItem->getUseDuration() - getUseItemRemainingTicks();
}

void LivingEntity::releaseUsingItem()
{
    releaseUsingItem(getUseItemRemainingTicks());
}

void LivingEntity::releaseUsingItem(int useDuration)
{
    ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();

    if (!mUseItem->isEmpty() && (gameMode == NULL || gameMode->IsValidStateForItemUse()))
    {
        mUseItem->releaseUsing(mLevel, boost::static_pointer_cast<LivingEntity>(shared_from_this()), useDuration);
    }

    stopUsingItem();
}

void LivingEntity::stopUsingItem()
{
    if (!mLevel->mIsClientSide)
    {
        mEntityData->set(DATA_LIVING_ENTITY_FLAGS, (unsigned char)0, true);
    }

    mUseItem = ItemInstance::EMPTY;
    mUseItemRemaining = 0;
}

bool LivingEntity::isBlocking()
{
    if (!isUsingItem() || mUseItem->isEmpty())
    {
        return false;
    }

    return mUseItem->getUseAnimation() == 3;
}

bool LivingEntity::isFallFlying()
{
    return getSharedFlag(9);
}

int LivingEntity::getFallFlyingTicks()
{
    return mFallFlyTicks;
}

bool LivingEntity::getElytraProximityWarning()
{
    return mElytraProximityWarning;
}

double LivingEntity::getElytraCollisionDistance()
{
    return mElytraCollisionDistance;
}

not_null_ptr<ItemInstance> LivingEntity::getUseItem()
{
    return mUseItem;
}

bool LivingEntity::randomTeleport(double x, double y, double z)
{
    double oldX = mX;
    double oldY = mY;
    double oldZ = mZ;

    mX = x;
    mY = y;
    mZ = z;

    BlockPos pos(this);
    Random* random = getRandom();

    bool teleported = false;

    if (mLevel->hasChunkAt(pos) && pos.y > 0)
    {
        while (!mLevel->getBlockState(mTickCache, pos.below())->getMaterial()->blocksMotion())
        {
            mY = mY - 1.0;

            if (pos.y < 1)
            {
                break;
            }
        }

        setPos(mX, mY, mZ);

        std::vector<AABB*>* collisions = mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox, false, false, false);

        if (collisions->empty() && !mLevel->containsAnyLiquid(mBoundingBox))
        {
            teleported = true;
        }
    }

    if (!teleported)
    {
        setPos(oldX, oldY, oldZ);
        return false;
    }

    teleportTo(mX, mY, mZ);

    for (int i = 0; i < 128; ++i)
    {
        double progress = (double)i / 127.0;

        float xd = (random->nextFloat() - 0.5f) * 0.20000000298023224f;
        float yd = (random->nextFloat() - 0.5f) * 0.20000000298023224f;
        float zd = (random->nextFloat() - 0.5f) * 0.20000000298023224f;

        double px = oldX + (mX - oldX) * progress + (random->nextDouble() - 0.5) * (double)mBbWidth * 2.0;
        double py = oldY + (mY - oldY) * progress + random->nextDouble() * (double)mBbHeight;
        double pz = oldZ + (mZ - oldZ) * progress + (random->nextDouble() - 0.5) * (double)mBbWidth * 2.0;

        mLevel->addParticle(PARTICLE_PORTAL, px, py, pz, (double)xd, (double)yd, (double)zd,
                            arrayWithLength<int>());
    }

    return true;
}

bool LivingEntity::isAffectedByPotions()
{
    return true;
}

bool LivingEntity::attackable()
{
    return true;
}

void LivingEntity::setRecordPlayingNearby(const BlockPos& pos, bool playing)
{
}

Team* LivingEntity::getTeam()
{
    if (GetType() == ENTITY_TYPE_PLAYER)
    {
        return mLevel->GetScoreboard()->getPlayersTeam(static_cast<Player*>(this)->mXuid);
    }

    return NULL;
}

void LivingEntity::disableFallFlyingState(bool disabled)
{
    mFallFlyingStateDisabled = disabled;
}

bool LivingEntity::isFallFlyingStateDisabled()
{
    return mFallFlyingStateDisabled;
}

void LivingEntity::stopCurrentLerp()
{
    Entity::stopCurrentLerp();

    mYHeadRotO = mYHeadRot;
    mYBodyRotO = mYBodyRot;
    mAnimationSpeed = 0.0f;
    mAnimationSpeedOld = 0.0f;
}

bool LivingEntity::IsCreativeFlying()
{
    return false;
}

unsigned long long LivingEntity::GetType()
{
    return ENTITY_TYPE_LIVING_ENTITY;
}
