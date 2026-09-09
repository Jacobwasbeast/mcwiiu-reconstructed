#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/core/FoodData.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/GenericStats.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/PlayerEnderChestContainer.hpp"
#include "game/client/render/PlayerModelPart.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemCooldowns.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/InsomniaComponent.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/CameraController.hpp"
#include "game/core/LockCode.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Team.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/item/Items.hpp"
#include "game/item/Item.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/world/block/Block.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/Random.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include <boost/functional/hash.hpp>
#include <climits>
#include <cmath>
#include <iomanip>
#include <sstream>

const EntityDataAccessor<float>* Player::DATA_PLAYER_ABSORPTION_ID = NULL;
const EntityDataAccessor<int>* Player::DATA_SCORE_ID = NULL;
const EntityDataAccessor<unsigned char>* Player::DATA_PLAYER_MODE_CUSTOMISATION = NULL;
const EntityDataAccessor<unsigned char>* Player::DATA_MAIN_HAND = NULL;
const EntityDataAccessor<CompoundTag*>* Player::DATA_SHOULDER_LEFT = NULL;
const EntityDataAccessor<CompoundTag*>* Player::DATA_SHOULDER_RIGHT = NULL;

Player::ChatVisiblity::ChatVisiblity(int id) : mId(id)
{
}

Player::PreventsMeFromSleeping::PreventsMeFromSleeping(boost::shared_ptr<Player> player) : mPlayer(player)
{
}

Player::PreventsMeFromSleeping::~PreventsMeFromSleeping()
{
}

bool Player::PreventsMeFromSleeping::apply(boost::shared_ptr<Entity> entity) const
{
    return entity.get() != mPlayer.get();
}

Player::Player(Level* level, const std::wstring& name) : LivingEntity(level), mAbilities(), mField0x738(NULL), m_0x784(0), m_0x798(0), mSuid(StatsUID::EMPTY_SUID())
{
    mName = name;
    mIsGuest = false;
}

StatsUID Player::getSUID()
{
    if (mSuid == StatsUID::EMPTY_SUID()) mSuid = StatsUID(m_0x784);
    return mSuid;
}

Player::~Player()
{
    delete mField0x738;
}

bool Player::shouldTick()
{
    return true;
}

void Player::registerAttributes()
{
    LivingEntity::registerAttributes();
    getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(1.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.10000000149011612);
    getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_SPEED);
    getAttributes()->registerAttribute(SharedMonsterAttributes::LUCK);
}

void Player::staticCtor()
{
    DATA_PLAYER_ABSORPTION_ID = SynchedEntityData::defineId(ENTITY_TYPE_PLAYER, ENTITY_TYPE_LIVING_ENTITY, EntityDataSerializers::FLOAT);
    DATA_SCORE_ID = SynchedEntityData::defineId(ENTITY_TYPE_PLAYER, ENTITY_TYPE_LIVING_ENTITY, EntityDataSerializers::INT);
    DATA_PLAYER_MODE_CUSTOMISATION = SynchedEntityData::defineId(ENTITY_TYPE_PLAYER, ENTITY_TYPE_LIVING_ENTITY, EntityDataSerializers::BYTE);
    DATA_MAIN_HAND = SynchedEntityData::defineId(ENTITY_TYPE_PLAYER, ENTITY_TYPE_LIVING_ENTITY, EntityDataSerializers::BYTE);
    DATA_SHOULDER_LEFT = SynchedEntityData::defineId(ENTITY_TYPE_PLAYER, ENTITY_TYPE_LIVING_ENTITY, EntityDataSerializers::COMPOUND_TAG);
    DATA_SHOULDER_RIGHT = SynchedEntityData::defineId(ENTITY_TYPE_PLAYER, ENTITY_TYPE_LIVING_ENTITY, EntityDataSerializers::COMPOUND_TAG);
}

void Player::defineSynchedData()
{
    LivingEntity::defineSynchedData();
    mEntityData->createDateItem(DATA_PLAYER_ABSORPTION_ID, 0.0f);
    mEntityData->createDateItem(DATA_SCORE_ID, 0);
    mEntityData->createDateItem(DATA_PLAYER_MODE_CUSTOMISATION, static_cast<unsigned char>(0));
    mEntityData->createDateItem(DATA_MAIN_HAND, static_cast<unsigned char>(1));
    mEntityData->createDateItem(DATA_SHOULDER_LEFT, new CompoundTag());
    mEntityData->createDateItem(DATA_SHOULDER_RIGHT, new CompoundTag());
}

bool Player::shouldRenderAtSqrDistance(double distance)
{
    double range = mBoundingBox->getSize() * 10.0;
    if (range != range) range = 1.0;
    range *= 64.0 * getViewScale();
    return distance < range * range;
}

bool Player::checkBedExists()
{
    return mField0x6c4 != NULL && mLevel->getBlockState(*mField0x6c4)->getBlock() == Blocks::bed;
}

void Player::moveCloak()
{
    double cloakX = mX - mChasingPosX;
    double cloakY = mY - mChasingPosY;
    double cloakZ = mZ - mChasingPosZ;

    if (cloakX > 10.0 || cloakX < -10.0) mChasingPosX = mX;
    if (cloakY > 10.0 || cloakY < -10.0) mChasingPosY = mY;
    if (cloakZ > 10.0 || cloakZ < -10.0) mChasingPosZ = mZ;

    mPrevChasingPosX = mChasingPosX;
    mPrevChasingPosY = mChasingPosY;
    mPrevChasingPosZ = mChasingPosZ;
    mChasingPosX += cloakX * 0.25;
    mChasingPosY += cloakY * 0.25;
    mChasingPosZ += cloakZ * 0.25;
}

void Player::SetNormalSize()
{
    mField0x828 = isSpectator();
}

void Player::resetPos()
{
    SetNormalSize();
    Entity::resetPos();
    setHealth(getMaxHealth());
    mDeathTime = 0;
}

void Player::handleEntityEvent(uint8_t event, int32_t data)
{
    if (event == 9) {
        completeUsingItem();
    } else if (event == 22) {
        mField0x760 = 1;
    } else if (event == 23) {
        mField0x760 = 0;
    } else {
        LivingEntity::handleEntityEvent(event, data);
    }
}

uint32_t Player::canHarmPlayer(boost::shared_ptr<Player> player)
{
    Team* team = getTeam();
    Team* otherTeam = player->getTeam();
    if (team != NULL && team->isAlliedTo(otherTeam)) return team->isAllowFriendlyFire();
    return 1;
}

uint32_t Player::getScore()
{
    return mEntityData->get(DATA_SCORE_ID);
}

void Player::setScore(int32_t score)
{
    mEntityData->set(DATA_SCORE_ID, score, true);
}

void Player::increaseScore(int32_t amount)
{
    setScore(static_cast<int32_t>(getScore()) + amount);
}

CompoundTag* Player::getShoulderEntityLeft()
{
    return mEntityData->get(DATA_SHOULDER_LEFT);
}

CompoundTag* Player::getShoulderEntityRight()
{
    return mEntityData->get(DATA_SHOULDER_RIGHT);
}

void Player::setShoulderEntityLeft(CompoundTag* tag)
{
    mEntityData->set(DATA_SHOULDER_LEFT, tag, true);
}

void Player::setShoulderEntityRight(CompoundTag* tag)
{
    mEntityData->set(DATA_SHOULDER_RIGHT, tag, true);
}

uint32_t Player::setEntityOnShoulder(CompoundTag* tag)
{
    if (isPassenger() || !mOnGround || isInWater()) return 0;
    if (getShoulderEntityLeft()->isEmpty()) {
        setShoulderEntityLeft(tag);
        return 1;
    }
    if (getShoulderEntityRight()->isEmpty()) {
        setShoulderEntityRight(tag);
        return 1;
    }
    delete tag;
    return 0;
}

bool Player::canHarmPlayer(std::wstring)
{
    return isAllowedToAttackPlayers();
}

Player::EPlayerGamePrivileges Player::getPlayerGamePrivilege(uint32_t privileges, EPlayerGamePrivileges privilege)
{
    if (privilege == ePlayerGamePrivilege_All) return static_cast<EPlayerGamePrivileges>(privileges);
    if (privilege > 0x1f) return static_cast<EPlayerGamePrivileges>(0);
    return static_cast<EPlayerGamePrivileges>(privileges & (1u << (privilege & 0x3f)));
}

Player::EPlayerGamePrivileges Player::getPlayerGamePrivilege(EPlayerGamePrivileges privilege)
{
    if (privilege == ePlayerGamePrivilege_All) return static_cast<EPlayerGamePrivileges>(mField0x7fc);
    if (privilege < 0x20) return static_cast<EPlayerGamePrivileges>(mField0x7fc & (1u << (privilege & 0x3f)));
    return static_cast<EPlayerGamePrivileges>(0);
}

uint32_t Player::hasInvulnerablePrivilege()
{
    return g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x12)) != 0 && getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(8)) != 0;
}

uint32_t Player::isAllowedToFly()
{
    return g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x10)) != 0 && getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(5)) != 0;
}

uint32_t Player::hasInvisiblePrivilege()
{
    return g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x12)) != 0 && getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(7)) != 0;
}

uint32_t Player::isAllowedToAttackPlayers()
{
    return !hasInvisiblePrivilege()
        && g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x0b)) != 0
        && (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x0c)) != 0 || getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(3)) == 0);
}

uint32_t Player::isAllowedToIgnoreExhaustion()
{
    return g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x11)) != 0 && getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(6)) != 0;
}

void Player::causeFoodExhaustion(float exhaustion)
{
    if (isAllowedToIgnoreExhaustion()) return;
    if (isAllowedToFly() && mAbilities.mFlying) return;
    if (mAbilities.mInvulnerable || hasInvulnerablePrivilege()) return;
    if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x0c)) == 0 && getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(1)) != 0) return;
    if (mLevel != NULL && !mLevel->mIsClientSide) getFoodData()->addExhaustion(exhaustion);
}

uint32_t Player::isAllowedToMine()
{
    return g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x0c)) != 0 || getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(0)) == 0;
}

uint32_t Player::isAllowedToAttackAnimals()
{
    return g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x0c)) != 0 || getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(10)) == 0;
}

bool Player::canCreateParticles()
{
    return !hasInvisiblePrivilege();
}

int Player::GetSuffocateTime()
{
    return -20;
}

float Player::GetInhaleTime()
{
    return 3.75f;
}

bool Player::generatesBubbles()
{
    return false;
}

int Player::getDimensionChangingDelay()
{
    return 10;
}

SoundSource::ESoundSource Player::getSoundSource()
{
    return static_cast<SoundSource::ESoundSource>(7);
}

double Player::getRidingHeight()
{
    return -0.3499999940395355;
}

void Player::stopRiding()
{
    LivingEntity::stopRiding();
    mVehicle.reset();
}

void Player::SetSmallSize()
{
    mField0x828 = 1;
}

void Player::SetNoCollisionSize()
{
    mField0x828 = 2;
}

void Player::updatePlayerSize()
{
    float width;
    float height;
    if (mField0x828 == 1) {
        width = 0.3f;
        height = 0.3f;
    } else if (mField0x828 == 2) {
        width = 0.0f;
        height = 0.0f;
    } else if (isSpectator()) {
        width = 0.6f;
        height = 1.8f;
    } else if (isSleeping()) {
        width = 0.2f;
        height = 0.2f;
    } else if (isFallFlying() || isSwimming()) {
        width = 0.6f;
        height = 0.6f;
    } else {
        width = 0.6f;
        height = isSneaking() ? 1.65f : 1.8f;
    }
    setSize(width, height);
}

void Player::playSound(const SoundEvent* sound, float volume, float pitch)
{
    mLevel->playSound(boost::static_pointer_cast<Player>(shared_from_this()), mX, mY, mZ,
                      sound, getSoundSource(), volume, pitch, 16.0f);
}

uint64_t Player::GetType()
{
    return 0x000000000C800000ULL;
}

uint32_t Player::GetThirdPersonView(bool)
{
    return 0;
}

void Player::SetThirdPersonView(int)
{
}

uint32_t Player::CanChangeThirdPersonView()
{
    return 0;
}

bool Player::PositionLocked()
{
    return mField0x814;
}

void Player::SetPositionLocked(bool locked)
{
    mField0x814 = locked;
}

bool Player::TakeGlideCollisionDamage()
{
    return mField0x815;
}

void Player::SetGlideCollisionDamage(bool enabled)
{
    mField0x815 = enabled;
}

double Player::GetLiftForceModifier()
{
    return mField0x818;
}

void Player::SetLiftForceModifier(double modifier)
{
    mField0x818 = modifier;
}

GameMode* Player::GetGameMode()
{
    return 0;
}

bool Player::isPickable()
{
    return !LivingEntity::isPickable();
}

void Player::setPlayerDefaultSkin(uint32_t skin)
{
    mField0x7d8 = skin;
}

void Player::SetAdditionalModelParts(std::vector<ModelPart*>* parts)
{
    mAdditionalModelParts = parts;
}

void Player::Follow(bool follow)
{
    if (!follow) mField0x808 = -1;
}

double Player::getUnderwaterLightLevel()
{
    return static_cast<double>(mField0x854);
}

void Player::setUnderwaterLightLevel(float level)
{
    mField0x854 = level;
}

double Player::getUnderwaterVisionScale()
{
    return static_cast<double>(mField0x858);
}

void Player::setUnderwaterVisionScale(float scale)
{
    mField0x858 = scale;
}

void Player::onCrafted(not_null_ptr<ItemInstance>)
{
}

void Player::setPlayerGamePrivilege(uint32_t& privileges, EPlayerGamePrivileges privilege, uint32_t enabled)
{
    if (privilege == ePlayerGamePrivilege_All) {
        privileges = enabled;
    } else if (privilege < 0x20) {
        const uint32_t mask = 1u << (privilege & 0x3f);
        if (enabled) privileges |= mask;
        else privileges &= ~mask;
    }
}

void Player::setPlayerGamePrivilege(EPlayerGamePrivileges privilege, uint32_t enabled)
{
    setPlayerGamePrivilege(mField0x7fc, privilege, enabled);
}

bool Player::isModerator()
{
    return getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(4)) != 0;
}

void Player::enableAllPlayerPrivileges(uint32_t& privileges, bool enabled)
{
    const uint32_t inverse = enabled ? 0u : 1u;
    setPlayerGamePrivilege(privileges, static_cast<EPlayerGamePrivileges>(0), inverse);
    setPlayerGamePrivilege(privileges, static_cast<EPlayerGamePrivileges>(1), inverse);
    setPlayerGamePrivilege(privileges, static_cast<EPlayerGamePrivileges>(3), inverse);
    setPlayerGamePrivilege(privileges, static_cast<EPlayerGamePrivileges>(10), inverse);
    setPlayerGamePrivilege(privileges, static_cast<EPlayerGamePrivileges>(11), enabled);
    setPlayerGamePrivilege(privileges, static_cast<EPlayerGamePrivileges>(12), enabled);
}

void Player::enableAllPlayerPrivileges(bool enabled)
{
    enableAllPlayerPrivileges(mField0x7fc, enabled);
}

void Player::SetLastAteKelp(bool ate)
{
    mField0x85c = ate;
    if (!ate) ResetTimerAchievement(0);
}

void Player::startSpinAttack()
{
    setDamageNearbyMobs(true);
    updatePlayerSize();
}

void Player::setInvisible(bool invisible)
{
    Entity::setInvisible(invisible);
}

void Player::SpectatePlayerNext()
{
    Follow(true);
}

void Player::SpectatePlayerPrev()
{
    Follow(true);
}

void Player::handleCollectItem(not_null_ptr<ItemInstance>)
{
}

bool Player::ShouldRenderShadow()
{
    return !(isSpectator() && IsSpectatingOtherPlayer());
}

std::wstring Player::getDisplayName()
{
    return mName;
}

void Player::stopSpinAttack()
{
    setDamageNearbyMobs(false);
    updatePlayerSize();
}

void Player::makeStuckInWeb()
{
    if (!mAbilities.mFlying) Entity::makeStuckInWeb();
}

void Player::respawn()
{
    mField0x6cc = 0;
    InsomniaComponent* insomnia = getInsomniaComponent();
    if (insomnia != NULL) insomnia->restartTimer();
}

int Player::getPortalWaitTime()
{
    return mAbilities.mInvulnerable ? 0 : 80;
}

int Player::getXpNeededForNextLevel()
{
    if (mExperienceLevel > 29) return mExperienceLevel * 9 - 158;
    if (mExperienceLevel > 14) return mExperienceLevel * 5 - 38;
    return mExperienceLevel * 2 + 7;
}

uint32_t Player::getEnchantmentSeed()
{
    return static_cast<uint32_t>(mEnchantmentSeed);
}

uint32_t Player::isLocalPlayer()
{
    return 0;
}

bool Player::isInWall()
{
    return !isSleeping() && Entity::isInWall();
}

std::wstring Player::getGameProfile()
{
    return mName;
}

uint32_t Player::bedInRange(BlockPos* position, Direction* direction)
{
    if (position == NULL || direction == NULL) return 0;
    double dx = mX - position->x;
    double dy = mY - position->y;
    double dz = mZ - position->z;
    if (dx >= -3.0 && dx <= 3.0 && dy >= -2.0 && dy <= 2.0 && dz >= -3.0 && dz <= 3.0) return 1;

    BlockPos opposite = position->relative(direction->getOpposite());
    dx = mX - opposite.x;
    dy = mY - opposite.y;
    dz = mZ - opposite.z;
    return dx >= -3.0 && dx <= 3.0 && dy >= -2.0 && dy <= 2.0 && dz >= -3.0 && dz <= 3.0;
}

uint32_t Player::isSleepingLongEnough()
{
    return mField0x6c0 != 0 && mField0x6c8 > 99;
}

uint32_t Player::getSleepTimer()
{
    return mField0x6c8;
}

uint32_t Player::getDeathFadeTimer()
{
    return mField0x6cc;
}

void Player::displayClientMessage(int, bool)
{
}

BlockPos* Player::getRespawnPosition()
{
    return mField0x6e0;
}

uint8_t Player::isRespawnForced()
{
    return mField0x6e4 != 0;
}

uint32_t Player::isModelPartShown(const PlayerModelPart*)
{
    return 1;
}

uint8_t Player::isReducedDebugInfo()
{
    return mField0x760 != 0;
}

bool Player::isAlwaysExperienceDropper()
{
    return true;
}

bool Player::shouldShowName()
{
    return true;
}

void Player::doWaterSplashEffect()
{
    if (!isSpectator()) Entity::doWaterSplashEffect();
}

bool Player::isPushedByWater()
{
    return !mAbilities.mFlying;
}

double Player::getAttackStrengthScale(float)
{
    return 1.0;
}

uint8_t Player::mayBuild()
{
    return mAbilities.mMayBuild;
}

uint8_t Player::getTexture()
{
    if (mField0x7d8 > 1 && mField0x7d8 < 0x13) {
        static const uint8_t textures[20] = {0, 0, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0xaa, 0xa9, 0};
        return textures[mField0x7d8];
    }
    return 0x25;
}

void Player::UpdateSize()
{
    SetNormalSize();
}

void Player::hurtCurrentlyUsedShield(float)
{
}

void Player::disableShield(bool)
{
}

void Player::crit(boost::shared_ptr<Entity>)
{
}

void Player::magicCrit(boost::shared_ptr<Entity>)
{
}

void Player::openTextEdit(boost::shared_ptr<SignBlockEntity>)
{
}

void Player::openMinecartCommandBlock(uint32_t)
{
}

void Player::openCommandBlock(uint32_t)
{
}

uint32_t Player::openTrading(uint32_t, std::wstring*, int32_t)
{
    return 0;
}

uint32_t Player::openContainer(boost::shared_ptr<Container>)
{
    return 0;
}

int Player::openHorseInventory(boost::shared_ptr<AbstractHorse>, boost::shared_ptr<Container>)
{
    return 0;
}

uint32_t Player::startBlockInteraction(InteractionObject*)
{
    return 0;
}

void Player::openItemInstanceGui(not_null_ptr<ItemInstance>, uint32_t)
{
}

bool Player::isSleeping()
{
    return mField0x6c0;
}

uint32_t Player::hasAchievement(Achievement*)
{
    return 0;
}

void Player::resetStat(Stat*)
{
}

void Player::onUpdateAbilities()
{
}

void Player::setGameMode(GameType*)
{
}

void Player::OnEquipArmor(not_null_ptr<ItemInstance>)
{
}

void Player::OnTakeFromAnvil(not_null_ptr<ItemInstance>)
{
}

uint32_t Player::GetModelTypeFromAnimBitmask(uint32_t value)
{
    if ((value & 0x40000u) != 0) return 1;
    if ((value & 0x80000u) == 0) return 0;
    return 2;
}

uint32_t Player::GetModelTypeFromTextureId(int32_t value)
{
    if (value == 0x25) return 1;
    if (value < 0x70) return 0;
    if (value <= 0x77) return 2;
    if (value == 0xa9 || value == 0xaa) return 2;
    return 0;
}

void Player::AdvanceTimerAchievement(int32_t index, int32_t amount)
{
    if (amount < 1) return;
    int32_t* timers = &mField0x860;
    timers[index] += amount;
}

void Player::SetPowerupTicks(int32_t powerup, int32_t ticks)
{
    (&mField0x830)[powerup] = ticks;
}

void Player::isStarving()
{
    mFoodData.isStarving();
}

void Player::resetAttackStrengthTicker()
{
    mAttackStrengthTicker = 0;
}

bool Player::isSendMessageEnabled()
{
    return mLevel != NULL && mLevel->getGameRules()->getBoolean(9);
}

bool Player::makeStepSound()
{
    return !mAbilities.mFlying;
}

const SoundEvent* Player::getSwimSound()
{
    return SoundEvents::PLAYER_SWIM;
}

const SoundEvent* Player::getSwimSplashSound()
{
    return SoundEvents::PLAYER_SPLASH;
}

const SoundEvent* Player::getHeavySplashSound()
{
    return SoundEvents::PLAYER_SPLASH_HIGH_SPEED;
}

const SoundEvent* Player::getDeathSound()
{
    return SoundEvents::PLAYER_DEATH;
}

FoodData* Player::getFoodData()
{
    return &mFoodData;
}

boost::shared_ptr<PlayerEnderChestContainer> Player::getEnderChestInventory()
{
    return mEnderChestInventory;
}

ItemCooldowns* Player::getCooldowns()
{
    return mField0x738;
}

void Player::hurtArmor(float damage)
{
    if (mInventory) mInventory->hurtArmor(damage);
}

double Player::getArmorCoverPercentage()
{
    if (!mInventory) return 0.0;
    int worn = 0;
    for (int slot = 0; slot < 4; ++slot) {
        if (!mInventory->getArmor(slot)->isEmpty()) ++worn;
    }
    return static_cast<double>(worn) / 4.0;
}

uint8_t Player::canUseCommandBlocks()
{
    return mAbilities.mInstabuild;
}

not_null_ptr<ItemInstance> Player::getItemSlot(const EquipmentSlot* slot)
{
    if (!mInventory || slot == NULL) return ItemInstance::EMPTY;
    switch (slot->getEIndex()) {
    case EquipmentSlot::eIndex_MainHand:
        return mInventory->getSelected();
    case EquipmentSlot::eIndex_OffHand:
        return mInventory->getItem(40);
    case EquipmentSlot::eIndex_Feet:
    case EquipmentSlot::eIndex_Legs:
    case EquipmentSlot::eIndex_Chest:
    case EquipmentSlot::eIndex_Head:
        return mInventory->getArmor(slot->getIndex());
    }
    return ItemInstance::EMPTY;
}

void Player::setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item)
{
    if (!mInventory || slot == NULL) return;
    int index = slot->getEIndex() == EquipmentSlot::eIndex_OffHand ? 40 : slot->getIndex();
    if (slot->getType() == EquipmentSlot::ARMOR) index += 36;
    mInventory->setItem(index, item);
}

uint32_t Player::addItem(not_null_ptr<ItemInstance> item)
{
    return mInventory && mInventory->add(item);
}

double Player::getCurrentItemAttackStrengthDelay()
{
    AttributeInstance* attackSpeed = getAttribute(SharedMonsterAttributes::ATTACK_SPEED);
    return attackSpeed == NULL ? 0.0 : 20.0 / attackSpeed->getValue();
}

float Player::getSpeed()
{
    AttributeInstance* movementSpeed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
    return movementSpeed == NULL ? 0.0f : static_cast<float>(movementSpeed->getValue());
}

double Player::getLuck()
{
    AttributeInstance* luck = getAttribute(SharedMonsterAttributes::LUCK);
    return luck == NULL ? 0.0 : static_cast<float>(luck->getValue());
}

std::wstring Player::getName()
{
    return mName;
}

std::wstring Player::getNetworkName()
{
    return mName;
}

void Player::setName(const std::wstring& name)
{
    mName = name;
}

uint32_t Player::hash_fnct(boost::shared_ptr<Player> player)
{
    return player ? static_cast<uint32_t>(boost::hash<std::wstring>()(player->mName)) : 0u;
}

bool Player::eq_test(boost::shared_ptr<Player> first, boost::shared_ptr<Player> second)
{
    return first && second && first->mName == second->mName;
}

bool Player::isImmobile()
{
    return getHealth() <= 0.0f || isSleeping();
}

uint64_t Player::CheckTimerAchievements()
{
    if (mLevel == NULL || mLevel->mIsClientSide) return 0;
    if (mField0x85d == 0 && ++mField0x860 > 72000) {
        AwardTimerAchievement(0);
    }
    if (mField0x85e == 0 && ++mField0x864 > 24000) {
        AwardTimerAchievement(1);
    }
    return 0;
}

void Player::AwardTimerAchievement(int32_t achievement)
{
    if (achievement == 0) {
        if (mField0x85c == 0) {
            mField0x85d = 0;
        } else {
            awardStat(GenericStats::castaway(), GenericStats::param_castaway());
            mField0x85d = 1;
        }
    } else if (achievement == 1) {
        awardStat(GenericStats::sleepWithTheFishes(), GenericStats::param_sleepWithTheFishes());
        mField0x85e = 1;
    }
}

void Player::onChangedBlock(const BlockPos& position)
{
    if (!isSwimming()) LivingEntity::onChangedBlock(position);
}

void Player::serverAiStep()
{
    LivingEntity::serverAiStep();
    mSpeed = getSpeed();
}

bool Player::IsCreativeFlying()
{
    return mAbilities.mFlying;
}

uint32_t Player::IsSpectatingOtherPlayer()
{
    if (mCamera || mField0x808 != -1) {
        return 1;
    }
    return 0;
}

void Player::SetCameraController(CameraController* controller, bool owned)
{
    if (mField0x780) delete mField0x77c;
    mField0x780 = owned;
    mField0x77c = controller;
    if (controller != NULL) SetThirdPersonView(controller->getCameraMode());
}

void Player::SetCamera(boost::shared_ptr<Entity> entity)
{
    boost::shared_ptr<Entity> self = shared_from_this();
    if (!entity || entity == self) {
        mCamera.reset();
    } else if (boost::dynamic_pointer_cast<Player>(entity)) {
        mCamera = entity;
    } else {
        mCamera.reset();
    }
    mField0x808 = -1;
}

boost::shared_ptr<Entity> Player::GetCamera()
{
    return mCamera ? mCamera : shared_from_this();
}

uint32_t Player::SpectateAsOtherEntity()
{
    if (!isLocalPlayer()) return 0;
    int mode = Minecraft::GetInstance()->GetSpectateMode();
    return mode == 1 || mode == 2 || mode == 3;
}

uint32_t Player::SpectateAsFloatingHead()
{
    return isLocalPlayer() && Minecraft::GetInstance()->GetSpectateMode() == 4;
}

uint32_t Player::SpectateAsInvisible()
{
    return isLocalPlayer() && Minecraft::GetInstance()->GetSpectateMode() == 0;
}

uint64_t Player::GetSpectateEntity()
{
    return Minecraft::GetInstance()->GetSpectateEntity();
}

void Player::StopSpectatingPlayer()
{
    Follow(false);
    SetCamera(boost::shared_ptr<Entity>());
}

void Player::pushEntities()
{
    if (!isSpectator()) LivingEntity::pushEntities();
}

void Player::push(boost::shared_ptr<Entity> entity)
{
    if (!isSleeping()) Entity::push(entity);
}

bool Player::canDestroy(const BlockState* state)
{
    return mInventory->canDestroy(state);
}

float Player::getDestroySpeed(const BlockState* state)
{
    float speed = mInventory->getDestroySpeed(state);
    if (isInWater() && !EnchantmentHelper::hasAquaAffinity(
            boost::static_pointer_cast<LivingEntity>(shared_from_this()))) speed /= 5.0f;
    if (!mOnGround) speed /= 5.0f;
    return speed;
}

void Player::jumpFromGround()
{
    LivingEntity::jumpFromGround();
    causeFoodExhaustion(static_cast<float>(isSprinting()
        ? FoodConstants::ExhaustionSprintJump() : FoodConstants::ExhaustionJump()));
}

void Player::awardStat(Stat*, arrayWithLength<unsigned char> parameters)
{
    delete[] parameters.data;
}

void Player::calculateFlapping()
{
    mOFlap = mFlap;
    mOFlapSpeed = mFlapSpeed;
    mFlapSpeed = Mth::clamp(mFlapSpeed + (mOnGround ? -0.3f : 1.2f), 0.0f, 1.0f);
    if (!mOnGround && mField0x850 < 1.0f) mField0x850 = 1.0f;
    mField0x850 *= 0.9f;
    mFlap += mField0x850 * 2.0f;
}

uint32_t Player::CheckPowerup(PowerupItems::eGlide_Timed_Powerup_ID powerup)
{
    int32_t& ticks = (&mField0x830)[powerup];
    if (ticks <= 0) return 0;
    --ticks;
    return 1;
}

uint32_t Player::canEat(bool alwaysEdible)
{
    if (alwaysEdible) return 1;
    if (!mFoodData.needsFood()) return 0;
    if (mAbilities.mInvulnerable) return 0;
    return hasInvulnerablePrivilege() ? 0 : 1;
}

void Player::increaseXp(int32_t amount)
{
    increaseScore(amount);
    if (amount > INT_MAX - mTotalExperience) amount = INT_MAX - mTotalExperience;

    mExperienceProgress += static_cast<float>(amount) / getXpNeededForNextLevel();
    mTotalExperience += amount;
    while (mExperienceProgress >= 1.0f) {
        mExperienceProgress = (mExperienceProgress - 1.0f) * getXpNeededForNextLevel();
        giveExperienceLevels(1);
        mExperienceProgress /= getXpNeededForNextLevel();
    }
}

void Player::onEnchantmentPerformed(int32_t cost)
{
    if (!mAbilities.mInstabuild) {
        mExperienceLevel -= cost;
        if (mExperienceLevel < 0) {
            mExperienceLevel = 0;
            mTotalExperience = 0;
            mExperienceProgress = 0.0f;
        }
    }
    mEnchantmentSeed = mRandom->nextInt();
}

void Player::giveExperienceLevels(int32_t levels)
{
    mExperienceLevel += levels;
    if (mExperienceLevel < 0) {
        mExperienceLevel = 0;
        mTotalExperience = 0;
        mExperienceProgress = 0.0f;
    }

    if (levels > 0 && mExperienceLevel % 5 == 0 && mTickCount - mField0x75c > 100) {
        float volume = mExperienceLevel > 30 ? 1.0f : static_cast<float>(mExperienceLevel) / 30.0f;
        mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ,
                          SoundEvents::EXPERIENCE_ORB_PICKUP, getSoundSource(),
                          volume * 0.75f, 1.0f, 16.0f);
        mField0x75c = mTickCount;
    }
}

void Player::setXuid(PlayerUID xuid)
{
    m_0x784 = xuid;
    if ((xuid.flags & 0x20000000) != 0 || mIsGuest) setName(xuid.toString());
}

Scoreboard* Player::GetScoreboard()
{
    return mLevel->GetScoreboard();
}

Team* Player::getTeam()
{
    PlayerUID uid = m_0x784 == PlayerUID(0) ? m_0x798 : m_0x784;
    return GetScoreboard()->getPlayersTeam(uid);
}

double Player::getSleepRotation()
{
    if (mField0x6c4 == NULL) return 0.0;
    const BlockState* state = mLevel->getBlockState(*mField0x6c4);
    if (state->getBlock() != Blocks::bed) return 0.0;
    int direction = state->getValue(HorizontalDirectionalBlock::FACING)->get3DDataValue();
    if (direction == 2) return 270.0;
    if (direction == 3) return 90.0;
    if (direction == 5) return 180.0;
    return 0.0;
}

const SoundEvent* Player::getFallDamageSound(int32_t damage)
{
    if (Minecraft::InMiniGame(eMiniGameId_Glide, true)) {
        return damage > 1 ? SoundEvents::MG03_FALL_BIG : SoundEvents::MG03_FALL_SMALL;
    }
    return damage > 4 ? SoundEvents::PLAYER_BIG_FALL : SoundEvents::PLAYER_SMALL_FALL;
}

bool Player::canUnlock(LockCode* lock)
{
    if (lock->isEmpty()) return true;
    not_null_ptr<ItemInstance> item = getMainHandItem();
    return !item->isEmpty() && item->hasCustomHoverName() && item->getHoverName() == lock->getKey();
}

bool Player::isInvisibleTo(boost::shared_ptr<Player> player)
{
    if (!isInvisible()) return false;
    if (player->isLocalPlayer() && !isLocalPlayer()) return false;
    Team* team = getTeam();
    if (team != NULL && team == player->getTeam() && team->canSeeFriendlyInvisibles()) return false;
    return true;
}

Vec3* Player::getAttachPos(Entity::EEntityLocation location, float partialTicks)
{
    Vec3* position = Entity::getAttachPos(location, partialTicks);
    if (location == static_cast<Entity::EEntityLocation>(7) && isFallFlying()) position->y -= 1.100000023841858;
    return position;
}

uint32_t Player::CanRecordStats(Stat* stat, arrayWithLength<unsigned char> parameters)
{
    return stat != NULL && g_pApp->CanRecordStatsAndAchievements(stat, parameters) != 0;
}

void Player::setAbsorptionAmount(float amount)
{
    if (amount < 0.0f) amount = 0.0f;
    mEntityData->set(DATA_PLAYER_ABSORPTION_ID, amount, true);
}

float Player::getAbsorptionAmount()
{
    return mEntityData->get(DATA_PLAYER_ABSORPTION_ID);
}

const HumanoidArm* Player::getMainArm()
{
    return mEntityData->get(DATA_MAIN_HAND) == 0 ? HumanoidArm::LEFT : HumanoidArm::RIGHT;
}

void Player::setMainArm(const HumanoidArm* arm)
{
    mEntityData->set(DATA_MAIN_HAND, static_cast<unsigned char>(arm == HumanoidArm::LEFT ? 0 : 1), true);
}

void Player::ResetTimerAchievement(int32_t index)
{
    if (mLevel != NULL && mLevel->mIsClientSide) return;
    (&mField0x860)[index] = 0;
}

uint64_t Player::setRespawnPosition(BlockPos* position, bool forced)
{
    if (mField0x6e0 == position) return 0;
    delete mField0x6e0;
    mField0x6e0 = position;
    mField0x6e4 = position != NULL && forced;
    return 0;
}

void Player::closeContainer()
{
    mContainerMenu = mInventoryMenu;
}

CameraController* Player::GetCameraController()
{
    return mField0x77c;
}

void Player::setBedOffset(Direction* direction)
{
    if (direction == NULL) return;
    Vec3i normal = direction->getNormal();
    mField0x6d0 = -1.8f * static_cast<float>(normal.x);
    mField0x6d8 = -1.8f * static_cast<float>(normal.z);
}

void Player::sendMessage(uint32_t, int32_t*, uint32_t, std::wstring*, uint32_t)
{
}

uint32_t Player::isHurt()
{
    const float health = getHealth();
    return health > 0.0f && health < getMaxHealth();
}

int32_t Player::getExperienceReward(boost::shared_ptr<Player>)
{
    int32_t reward = 0;
    if (mLevel != NULL && !mLevel->getGameRules()->getBoolean(2)) {
        reward = mExperienceLevel * 7;
        if (reward > 100) reward = 100;
    }
    return reward;
}

_SkinAdjustments Player::getSkinAdjustmentsById(uint32_t id)
{
    _SkinAdjustments adjustments;
    if (static_cast<int32_t>(id) < 0) {
        adjustments = g_pApp->GetSkinAdjustments(id);
        id &= 0x7fffffff;
    }

    if (id == 3 || id == 200 || id == 201 || id == 404 || id == 405
        || id == 0x1f8 || id == 0x220 || id == 0x23a || id == 0x23d || id == 0x247) {
        adjustments.mXMLVersion = 2;
    } else if (id == 500) {
        adjustments.mXMLVersion = 5;
    } else if (id == 0x1fa) {
        adjustments.mXMLVersion = 6;
    }
    return adjustments;
}

std::wstring Player::getCapePathFromId(uint32_t id)
{
    std::wostringstream path;
    path << std::setfill(L'0');
    if (static_cast<int32_t>(id) < 0) {
        path << L"dlccape" << std::setw(8) << std::dec << (id & 0x7fffffff) << L".png";
    } else if ((id & 0x7fffffe0) == 0) {
        path << L"defcape" << std::setw(8) << std::uppercase << std::hex << (id & 0x1f) << L".png";
    } else {
        path << L"ugccape" << std::setw(8) << std::uppercase << std::hex << id << L".png";
    }
    return path.str();
}

uint32_t Player::getCapeIdFromPath(const std::wstring& path)
{
    if (path.length() < 14) return 0;
    bool downloadable = path.compare(0, 3, L"dlc") == 0;
    std::wstring value = path.substr(7);
    std::wstring::size_type extension = value.find(L'.');
    if (extension != std::wstring::npos) value.erase(extension);

    uint32_t id = 0;
    std::wistringstream parser(value);
    if (downloadable) parser >> std::dec >> id;
    else parser >> std::hex >> id;
    return (id & 0x7fffffff) | (downloadable ? 0x80000000 : 0);
}

std::vector<boost::shared_ptr<ItemInstance> >* Player::getHandSlots()
{
    std::vector<boost::shared_ptr<ItemInstance> >* items = new std::vector<boost::shared_ptr<ItemInstance> >();
    items->push_back(getMainHandItem());
    items->push_back(getOffhandItem());
    return items;
}

std::vector<boost::shared_ptr<ItemInstance> >* Player::getArmorSlots()
{
    std::vector<boost::shared_ptr<ItemInstance> >* items = new std::vector<boost::shared_ptr<ItemInstance> >();
    items->push_back(mInventory->getArmor(0));
    items->push_back(mInventory->getArmor(1));
    items->push_back(mInventory->getArmor(2));
    items->push_back(mInventory->getArmor(3));
    return items;
}

double Player::getEyeHeight()
{
    double height = 1.6200000047683716;
    if (isSleeping()) height = 0.20000000298023224;
    else if (isFallFlying() || isSwimming()) height = 0.4000000059604645;
    else if (SpectateAsFloatingHead()) height -= 1.5;
    else if (isSneaking()) height -= 0.07999999821186066;
    return height;
}

bool Player::setSlot(int slot, const not_null_ptr<ItemInstance>& item)
{
    if (slot >= 0 && slot < static_cast<int>(mInventory->getContainerSize())) {
        mInventory->setItem(slot, item);
        return 1;
    }
    if (slot == 98) {
        setItemSlot(EquipmentSlot::MAINHAND, item);
        return 1;
    }
    if (slot == 99) {
        setItemSlot(EquipmentSlot::OFFHAND, item);
        return 1;
    }
    for (int i = 2; i < 6; ++i) {
        if (slot == EquipmentSlot::SLOTS[i]->getIndex() + 100) {
            setItemSlot(EquipmentSlot::SLOTS[i], item);
            return 1;
        }
    }
    if (slot >= 200 && mEnderChestInventory
        && slot - 200 < static_cast<int>(mEnderChestInventory->getContainerSize())) {
        mEnderChestInventory->setItem(slot - 200, item);
        return 1;
    }
    return 0;
}

uint32_t Player::getModelName()
{
    _SkinAdjustments adjustments = getSkinAdjustmentsById(mField0x7dc);
    if (static_cast<int32_t>(mField0x7dc) < 0)
        return GetModelTypeFromAnimBitmask(adjustments.mAnimFlags);
    return GetModelTypeFromTextureId(getTexture());
}

void Player::setCustomSkin(uint32_t skin)
{
    setInvisible(false);
    setSkinAdjustments(getSkinAdjustmentsById(skin));
    uint32_t defaultSkin = ((skin & 0x7fffffe0) == 0 && (skin & 0x1f) != 0)
        ? skin & 0x1f : static_cast<uint32_t>(mPlayerIndex + 1);
    setPlayerDefaultSkin(defaultSkin);
    mField0x7dc = skin;
    mField0x804 = 0;
    mField0x805 = 0;
    SetAdditionalModelParts(NULL);
}

uint64_t Player::setCustomCape(uint32_t cape)
{
    mField0x7e0 = cape;
    return cape;
}

std::vector<ModelPart*>* Player::GetAdditionalModelParts()
{
    if (mAdditionalModelParts != NULL) return mAdditionalModelParts;
    if (mField0x804 != 0) return NULL;
    mAdditionalModelParts = g_pApp->GetAdditionalModelParts(static_cast<unsigned int>(mField0x7dc));
    mField0x804 = 1;
    if (mAdditionalModelParts != NULL) setSkinAdjustments(getSkinAdjustmentsById(mField0x7dc));
    return mAdditionalModelParts;
}

void Player::tickArmor()
{
    not_null_ptr<ItemInstance> helmet = getItemSlot(EquipmentSlot::HEAD);
    if (!helmet->isEmpty() && helmet->getItem() == Items::turtleHelmet && !isInWater()) {
        addEffect(new MobEffectInstance(MobEffects::WATER_BREATHING, 200, 0, false, false),
                  boost::shared_ptr<Entity>());
    }
}

void Player::updateFrameTick()
{
    if (mTakeXpDelay > 0) --mTakeXpDelay;
    if (isSleeping()) {
        if (mField0x6c8 < 100) ++mField0x6c8;
        if (!mLevel->mIsClientSide && (!checkBedExists() || mLevel->isDay()))
            stopSleepInBed(false, true, true);
    } else if (mField0x6c8 > 0) {
        if (++mField0x6c8 >= 110) mField0x6c8 = 0;
    }
    if (!isAlive()) {
        if (mField0x6cc < 21) ++mField0x6cc;
    } else {
        mField0x6cc = 0;
    }
}

void Player::tickCamera()
{
    if (mField0x808 != -1 && !mCamera) mField0x808 = -1;
    if (!isSpectator() && mCamera) StopSpectatingPlayer();
}

void Player::PlayerRingSoundEffect(int ring)
{
    if (mLevel == NULL || !mLevel->mIsClientSide) return;
    mField0x83c = !mField0x83c;
    if (ring > 6) mField0x83c = 0;
}

void Player::prepareCustomTextures()
{
    if (g_pApp->GetMojangDataForXuid(m_0x798) != NULL) mField0x804 = 0;
}

void Player::_init()
{
    mInventory = boost::shared_ptr<Inventory>(new Inventory(this));
    mEnderChestInventory.reset(new PlayerEnderChestContainer());
    mInventoryMenu = NULL;
    mContainerMenu = NULL;
    mPrevCameraYaw = 0.0f;
    mCameraYaw = 0.0f;
    mTakeXpDelay = 0;
    mPrevChasingPosX = 0.0;
    mPrevChasingPosY = 0.0;
    mPrevChasingPosZ = 0.0;
    mChasingPosX = 0.0;
    mChasingPosY = 0.0;
    mChasingPosZ = 0.0;
    mField0x6c0 = 0;
    mField0x6c4 = NULL;
    mField0x6c8 = 0;
    mField0x6cc = 0;
    mExperienceLevel = 0;
    mTotalExperience = 0;
    mExperienceProgress = 0.0f;
    mFishing.reset();
    mEnchantmentSeed = mRandom->nextInt();
    mField0x738 = NULL;
    mField0x77c = NULL;
    mField0x780 = false;
    mDebugOptions = 0;
    mField0x7fc = 0;
    mAdditionalModelParts = NULL;
    mField0x804 = 0;
    mField0x805 = 0;
    mField0x808 = -1;
    mField0x818 = 1.0;
    mField0x820 = mY;
}

void Player::tick()
{
    updateFrameTick();
    tickArmor();
    if (mField0x738 != NULL) mField0x738->tick();
    LivingEntity::tick();
    if (mInventory) mInventory->tick();
    tickCamera();
    calculateFlapping();
    CheckTimerAchievements();
}

void Player::rideTick()
{
    LivingEntity::rideTick();
    mPrevCameraYaw = mCameraYaw;
    mCameraYaw = 0.0f;
    checkRidingStatistiscs(mX - mXOld, mY - mYOld, mZ - mZOld);
}

void Player::aiStep()
{
    LivingEntity::aiStep();
    checkMovementStatistiscs(mX - mXOld, mY - mYOld, mZ - mZOld);
}

void Player::touch(boost::shared_ptr<Entity> entity)
{
    if (!entity) return;
    boost::shared_ptr<ItemEntity> item = boost::dynamic_pointer_cast<ItemEntity>(entity);
    if (item) item->playerTouch(boost::static_pointer_cast<Player>(shared_from_this()));
}

void Player::destroyVanishingCursedItems()
{
    if (!mInventory) return;
    for (unsigned int i = 0; i < mInventory->getContainerSize(); ++i) {
        not_null_ptr<ItemInstance> item = mInventory->getItem(i);
        if (!item->isEmpty() && EnchantmentHelper::hasVanishingCurse(item))
            mInventory->setItem(i, ItemInstance::EMPTY);
    }
}

void Player::addWalkers(DataFixerUpper* fixer)
{
    Entity::addDataWalkers(fixer);
    ItemInstance::addDataWalkers(fixer);
}

void Player::readAdditionalSaveData(CompoundTag* tag)
{
    LivingEntity::readAdditionalSaveData(tag);
    mExperienceLevel = tag->getInt(L"XpLevel");
    mTotalExperience = tag->getInt(L"XpTotal");
    mEnchantmentSeed = tag->getInt(L"XpSeed");
    mField0x6e4 = tag->getBoolean(L"SpawnForced");
    mAbilities.loadSaveData(tag);
}

void Player::addAdditonalSaveData(CompoundTag* tag)
{
    LivingEntity::addAdditonalSaveData(tag);
    tag->putInt(L"XpLevel", mExperienceLevel);
    tag->putInt(L"XpTotal", mTotalExperience);
    tag->putInt(L"XpSeed", mEnchantmentSeed);
    tag->putBoolean(L"SpawnForced", mField0x6e4 != 0);
    mAbilities.addSaveData(tag);
}

boost::shared_ptr<ItemEntity> Player::drop(not_null_ptr<ItemInstance> item, bool randomThrow, bool retainOwnership)
{
    if (item->isEmpty()) return boost::shared_ptr<ItemEntity>();
    boost::shared_ptr<ItemEntity> dropped = spawnAtLocation(item, 0.0f);
    if (!dropped) return dropped;
    dropped->setPickUpDelay(40);
    dropped->setThrower(getName());
    if (retainOwnership) dropped->setOwner(getName());
    if (randomThrow) dropped->setExtendedLifetime();
    return reallyDrop(dropped);
}

boost::shared_ptr<ItemEntity> Player::drop(not_null_ptr<ItemInstance> item, bool randomThrow)
{
    return drop(item, randomThrow, false);
}

boost::shared_ptr<ItemEntity> Player::drop(bool dropAll)
{
    not_null_ptr<ItemInstance> selected = mInventory->getSelected();
    if (selected->isEmpty()) return boost::shared_ptr<ItemEntity>();
    int count = dropAll ? selected->getCount() : 1;
    not_null_ptr<ItemInstance> dropped = selected->split(count);
    if (selected->getCount() == 0) mInventory->removeItem(selected);
    return drop(dropped, false, true);
}

boost::shared_ptr<ItemEntity> Player::reallyDrop(boost::shared_ptr<ItemEntity> item)
{
    if (item) item->setThrower(getName());
    return item;
}

void Player::die(const DamageSource* source)
{
    LivingEntity::die(source);
    setPos(mX, mY + 0.10000000149011612, mZ);
    if (!isSpectator() && !mLevel->getGameRules()->getBoolean(2)) {
        destroyVanishingCursedItems();
        mInventory->dropAll();
    }
}

const SoundEvent* Player::getHurtSound(const DamageSource* source)
{
    if (source != NULL && source->isFire()) return SoundEvents::PLAYER_HURT_ON_FIRE;
    return SoundEvents::PLAYER_HURT;
}

bool Player::hurt(const DamageSource* source, float damage)
{
    if (hasInvulnerablePrivilege() && source != NULL && !source->isBypassInvul()) return false;
    if (source != NULL) {
        boost::shared_ptr<Player> attacker = boost::dynamic_pointer_cast<Player>(source->getEntity());
        if (attacker && !canHarmPlayer(attacker)) return false;
    }
    return LivingEntity::hurt(source, damage);
}

void Player::blockUsingShield(const boost::shared_ptr<LivingEntity>& attacker)
{
    LivingEntity::blockUsingShield(attacker);
}

void Player::actuallyHurt(const DamageSource* source, float damage)
{
    causeFoodExhaustion(source == NULL ? 0.0f : source->getFoodExhaustion());
    LivingEntity::actuallyHurt(source, damage);
}

void Player::checkRidingStatistiscs(double x, double y, double z)
{
    int distance = static_cast<int>(std::sqrt(x * x + y * y + z * z) * 100.0);
    if (distance <= 0 || gStats == NULL) return;
    awardStat(gStats->get_minecartOneM(), gStats->getParam_minecartOneM(distance));
}

void Player::checkMovementStatistiscs(double x, double y, double z)
{
    int horizontal = static_cast<int>(std::sqrt(x * x + z * z) * 100.0);
    int total = static_cast<int>(std::sqrt(x * x + y * y + z * z) * 100.0);
    if (gStats == NULL) return;
    if (isSwimming() || isInWater()) {
        if (total > 0) awardStat(gStats->get_swimOneM(), gStats->getParam_swimOneM(total));
    } else if (isFallFlying()) {
        if (total > 0) awardStat(gStats->get_walkOneM(), gStats->getParam_walkOneM(total));
    } else if (horizontal > 0) {
        awardStat(gStats->get_walkOneM(), gStats->getParam_walkOneM(horizontal));
    }
}

void Player::causeFallDamage(float distance, float multiplier)
{
    if (mAbilities.mFlying) return;
    LivingEntity::causeFallDamage(distance, multiplier);
    int statistic = static_cast<int>(distance * 100.0f);
    if (statistic > 0 && gStats != NULL)
        awardStat(gStats->get_fallOneM(), gStats->getParam_fallOneM(statistic));
}

void Player::travel(float x, float y, float z)
{
    double oldX = mX;
    double oldY = mY;
    double oldZ = mZ;
    LivingEntity::travel(x, y, z);
    checkMovementStatistiscs(mX - oldX, mY - oldY, mZ - oldZ);
}

void Player::playShoulderEntityAmbientSound(CompoundTag* tag)
{
    if (tag == NULL || tag->isEmpty()) return;
    if (tag->contains(L"Silent") && tag->getBoolean(L"Silent")) return;
    boost::shared_ptr<Entity> entity = EntityIO::newEntity(tag, mLevel);
    boost::shared_ptr<Mob> mob = boost::dynamic_pointer_cast<Mob>(entity);
    if (mob) mob->playAmbientSound();
}

void Player::respawnEntityOnShoulder(CompoundTag* tag)
{
    if (mLevel->mIsClientSide || tag == NULL || tag->isEmpty()) return;
    boost::shared_ptr<Entity> entity = EntityIO::newEntity(tag, mLevel);
    if (!entity) return;
    entity->setPos(mX, mY + 0.699999988079071, mZ);
    mLevel->addEntity(entity);
}

void Player::removeEntitiesOnShoulder()
{
    CompoundTag* left = getShoulderEntityLeft();
    respawnEntityOnShoulder(left);
    if (left != NULL && !left->isEmpty()) setShoulderEntityLeft(new CompoundTag());
    CompoundTag* right = getShoulderEntityRight();
    respawnEntityOnShoulder(right);
    if (right != NULL && !right->isEmpty()) setShoulderEntityRight(new CompoundTag());
}

void Player::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    LivingEntity::onSyncedDataUpdated(accessor);
    if (accessor == DATA_SHOULDER_LEFT) playShoulderEntityAmbientSound(getShoulderEntityLeft());
    if (accessor == DATA_SHOULDER_RIGHT) playShoulderEntityAmbientSound(getShoulderEntityRight());
}

void Player::remove()
{
    Entity::remove();
    removeEntitiesOnShoulder();
}

uint32_t Player::startSleepInBed(const BlockPos& position, bool force)
{
    if (isSleeping() || !isAlive()) return 1;
    if (!force && !bedInRange(const_cast<BlockPos*>(&position), NULL)) return 2;
    if (!force && mLevel->isDay()) return 3;
    SetSmallSize();
    mField0x6c4 = new BlockPos(position);
    mField0x6c0 = 1;
    mField0x6c8 = 0;
    setPos(position.x + 0.5, position.y + 0.6875, position.z + 0.5);
    if (!mLevel->mIsClientSide) mLevel->updateSleepingPlayerList();
    return 0;
}

void Player::stopSleepInBed(bool resetTimer, bool updateLevel, bool setSpawn)
{
    SetNormalSize();
    mField0x6c0 = 0;
    mField0x6c8 = resetTimer ? 0 : 100;
    if (updateLevel && !mLevel->mIsClientSide) mLevel->updateSleepingPlayerList();
    if (setSpawn && mField0x6c4 != NULL)
        setRespawnPosition(new BlockPos(*mField0x6c4), false);
}

BlockPos* Player::checkBedValidRespawnPosition(Level* level, const BlockPos& position, bool forced)
{
    const BlockState* state = level->getBlockState(position);
    if (state->getBlock() == Blocks::bed || forced) return new BlockPos(position);
    return NULL;
}

void Player::fjCheckSuperSonic(double oldX, double oldY, double oldZ,
                               double newX, double newY, double newZ)
{
    double speed = std::sqrt(mXd * mXd + mYd * mYd + mZd * mZd);
    if (speed <= 2.0 || gStats == NULL) return;
    int distance = static_cast<int>(std::sqrt((newX - oldX) * (newX - oldX)
        + (newY - oldY) * (newY - oldY) + (newZ - oldZ) * (newZ - oldZ)) * 20.0);
    if (distance > 0) awardStat(GenericStats::superSonic(), GenericStats::param_superSonic(distance));
}

void Player::killed(boost::shared_ptr<LivingEntity> entity)
{
    if (!entity || gStats == NULL) return;
    boost::shared_ptr<Mob> mob = boost::dynamic_pointer_cast<Mob>(entity);
    if (mob) awardStat(GenericStats::killEnemy(),
                       GenericStats::param_mobKill(boost::static_pointer_cast<Player>(shared_from_this()),
                                                   mob, getLastDamageSource()));
}

int Player::getItemInHandIcon(not_null_ptr<ItemInstance> item, int frame)
{
    return LivingEntity::getItemInHandIcon(item, frame);
}

uint32_t Player::mayUseItemAt(const BlockPos& position, const Direction* direction,
                              not_null_ptr<ItemInstance> item)
{
    if (mayBuild()) return 1;
    if (!item->isEmpty() && item->mayBePlacedInAdventureMode()) return 1;
    if (direction == NULL) return 0;
    BlockPos adjacent = position.relative(direction->getOpposite());
    Block* block = mLevel->getBlockState(adjacent)->getBlock();
    return item->hasAdventureModePlaceTagForBlock(block)
        || item->mayBePlacedInAdventureMode();
}

uint32_t Player::isAllowedToUse(Block* block)
{
    if (block == NULL || g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x0c)) != 0) return 1;
    unsigned int id = Block::getId(block);
    if (getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(12)) != 0) {
        if (id == 0x17 || id == 0x36 || id == 0x3a || id == 0x3d || id == 0x3e
            || id == 0x74 || id == 0x75 || id == 0x82 || id == 0x8a || id == 0x91
            || id == 0x92 || id == 0x9a || id == 0x9e || (id > 0xda && id < 0xeb)) return 1;
    }
    return getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(1)) == 0;
}

uint32_t Player::isAllowedToUse(not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty() || g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x0c)) != 0) return 1;
    unsigned int id = Item::getId(item->getItem());
    if (id == 0x104 || id == 0x105 || (id >= 0x107 && id <= 0x10a)
        || id == 0x10d || id == 0x110 || id == 0x114 || id == 0x11a || id == 0x11b
        || (id >= 0x129 && id <= 0x13d) || id == 0x13e || id == 0x140 || id == 0x142
        || id == 0x15d || id == 0x15e || id == 0x165 || id == 0x168
        || (id >= 0x16b && id <= 0x16f) || id == 0x175 || id == 0x180
        || id == 0x1b6 || id == 0x1b9) return 1;
    return getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(1)) == 0;
}

bool Player::isAllowedToInteract(boost::shared_ptr<Entity> entity)
{
    if (!entity) return false;
    if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x0c)) != 0) return true;
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
    if (player) return getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(12)) != 0;
    return getPlayerGamePrivilege(static_cast<EPlayerGamePrivileges>(1)) == 0;
}

uint32_t Player::isAllowedToHurtEntity(boost::shared_ptr<Entity> entity)
{
    if (!entity) return 0;
    unsigned long long type = entity->GetType();
    if (!isAllowedToMine() || g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1f)) != 0) {
        if (type == 0xc000001 || type == 0xa080003 || type == 0xa080002
            || type == 0xa080001 || type == 0xa080000 || type == 0x8000008) return 0;
    }
    if ((type & 0xd680000) == 0xd680000) return isAllowedToAttackAnimals();
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
    if (player) return isAllowedToAttackPlayers() && !player->mAbilities.mInstabuild && !player->isSpectator();
    return 1;
}

uint32_t Player::SpectateAsSkull()
{
    return SpectateAsFloatingHead() && GetModelTypeFromTextureId(getTexture()) == 2;
}

int32_t Player::AutoEquip(not_null_ptr<ItemInstance>& item, not_null_ptr<ItemInstance>& replaced,
                          bool& replacedExisting)
{
    replacedExisting = false;
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) || item->isEmpty() || !item->getItem()->IsArmor()) return 0;
    ArmorItem* armor = dynamic_cast<ArmorItem*>(item->getItem());
    if (armor == NULL) return 0;
    const EquipmentSlot* slot = armor->getSlot();
    not_null_ptr<ItemInstance> equipped = getItemSlot(slot);
    if (!equipped->isEmpty()) {
        if (equipped->isEnchanted()) return 0;
        replacedExisting = true;
        replaced = equipped;
    }
    if (!setSlot(slot->getIndex() + 100, item->copy())) return 0;
    item->setCount(0);
    return 1;
}

uint32_t Player::interactOn(boost::shared_ptr<Entity> entity, InteractionHand::EInteractionHand hand)
{
    if (!entity || !isAllowedToInteract(entity)) return 0;
    boost::shared_ptr<Player> self = boost::static_pointer_cast<Player>(shared_from_this());
    if (entity->interact(self, hand)) return 1;
    not_null_ptr<ItemInstance> item = getItemInHand(hand);
    boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
    if (living && !item->isEmpty() && item->interactEnemy(self, living, hand)) return 1;
    return 0;
}

uint64_t Player::attack(boost::shared_ptr<Entity> entity)
{
    if (!entity || !entity->isAttackable() || entity->skipAttackInteraction(shared_from_this())
        || !isAllowedToHurtEntity(entity)) return 0;

    AttributeInstance* attackDamage = getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE);
    float strength = static_cast<float>(getAttackStrengthScale(0.5f));
    float damage = attackDamage == NULL ? 1.0f : static_cast<float>(attackDamage->getValue());
    damage *= 0.2f + strength * strength * 0.8f;
    boost::shared_ptr<EntityDamageSource> source =
        DamageSource::CreatePlayerAttack(boost::static_pointer_cast<Player>(shared_from_this()));
    bool damaged = entity->hurt(source.get(), damage);
    if (damaged) {
        boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
        not_null_ptr<ItemInstance> item = getMainHandItem();
        if (living && !item->isEmpty()) item->hurtEnemy(living,
            boost::static_pointer_cast<Player>(shared_from_this()));
        causeFoodExhaustion(0.10000000149011612f);
    }
    resetAttackStrengthTicker();
    return damaged ? 1 : 0;
}
