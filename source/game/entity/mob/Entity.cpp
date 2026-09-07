#include "game/entity/mob/Entity.hpp"

#include <cmath>
#include <cstdarg>

#include <boost/unordered_set.hpp>

#include "game/client/audio/SoundType.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Direction.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/Explosion.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/LightningBolt.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Mth.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/core/Team.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/FloatTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/EntityPos.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/EntityDataWalker.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/LeashFenceKnotEntity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/TrackedEntity.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/item/DataItem.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PortalForcer.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DoubleTag.hpp"
#include "game/data/StringTag.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/ClientboundSetPassengersPacket.hpp"
#include "game/network/ClientboundSetEntityLinkPacket.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/item/ProtectionEnchantment.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/Vec3i.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/FenceBlock.hpp"
#include "game/world/block/FenceGateBlock.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/PortalBlock.hpp"
#include "game/world/block/WallBlock.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerTeam.hpp"

class ServerPlayer;

unsigned int TlsAlloc();
void* TlsGetValue(unsigned int index);
void TlsSetValue(unsigned int index, void* value);
void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();
void MemSect(int section);

static unsigned int gSmallIdTlsIndex = TlsAlloc();

const EntityDataAccessor<int>* Entity::DATA_SHARED_FLAGS_ID;
const EntityDataAccessor<int>* Entity::DATA_AIR_SUPPLY_ID;
const EntityDataAccessor<std::wstring>* Entity::DATA_CUSTOM_NAME;
const EntityDataAccessor<bool>* Entity::DATA_CUSTOM_NAME_VISIBLE;
const EntityDataAccessor<bool>* Entity::DATA_SILENT;
const EntityDataAccessor<bool>* Entity::DATA_NO_GRAVITY;
const EntityDataAccessor<bool>* Entity::DATA_MOVING;
const EntityDataAccessor<bool>* Entity::DATA_NAMED_BY_RESTRICTED_PLAYER;
const EntityDataAccessor<bool>* Entity::DATA_CREATED_ON_HOST;
const EntityDataAccessor<std::wstring>* Entity::DATA_OWNER_UUID;

double Entity::sViewScale = 1.0;
int Entity::sEntityCount = 2048;
unsigned int Entity::sSmallIdsInUse[64];
unsigned int Entity::sSmallIdsWandering[64];
unsigned int Entity::sSmallIdsToRemove[64];
int Entity::sWanderingCount;
int Entity::sWanderingTick;
int Entity::sWanderingIds[3];

bool eTYPE_DERIVED_FROM__FULT1(unsigned int typeHigh, unsigned int typeLow,
                               unsigned int entityTypeHigh,
                               unsigned int entityTypeLow)
{
    if ((typeLow & 0x3F) != 0)
    {
        return typeHigh == entityTypeHigh && typeLow == entityTypeLow;
    }

    return (typeHigh & entityTypeHigh) == typeHigh
        && (typeLow & entityTypeLow) == typeLow;
}

int indexOf(const std::vector<boost::weak_ptr<Entity> >& entities,
            const boost::shared_ptr<Entity>& entity)
{
    for (unsigned int i = 0; i < entities.size(); ++i)
    {
        if (entities.at(i).lock() == entity)
        {
            return (int)i;
        }
    }

    return -1;
}

bool isPickable_apply(const boost::shared_ptr<Entity>& entity)
{
    if (entity->isPickable())
    {
        return true;
    }

    return entity->GetType() == 0x000000000800000AULL;
}

void Entity::staticCtor()
{
    DATA_SHARED_FLAGS_ID = SynchedEntityData::defineId(ENTITY_TYPE_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::INT);
    DATA_AIR_SUPPLY_ID = SynchedEntityData::defineId(ENTITY_TYPE_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::INT);
    DATA_CUSTOM_NAME = SynchedEntityData::defineId(ENTITY_TYPE_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::STRING);
    DATA_CUSTOM_NAME_VISIBLE = SynchedEntityData::defineId(ENTITY_TYPE_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::BOOLEAN);
    DATA_SILENT = SynchedEntityData::defineId(ENTITY_TYPE_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::BOOLEAN);
    DATA_NO_GRAVITY = SynchedEntityData::defineId(ENTITY_TYPE_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::BOOLEAN);
    DATA_MOVING = SynchedEntityData::defineId(ENTITY_TYPE_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::BOOLEAN);
    DATA_OWNER_UUID = SynchedEntityData::defineId(ENTITY_TYPE_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::STRING);
    DATA_CREATED_ON_HOST = SynchedEntityData::defineId(ENTITY_TYPE_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::BOOLEAN);
    DATA_NAMED_BY_RESTRICTED_PLAYER = SynchedEntityData::defineId(ENTITY_TYPE_ENTITY, ENTITY_TYPE_ENTITY, EntityDataSerializers::BOOLEAN);
}

bool Entity::instanceof(const boost::shared_ptr<Entity>& entity, unsigned long long type)
{
    if (!entity)
    {
        return false;
    }

    unsigned long long entityType = entity->GetType();

    if ((type & 0x3FULL) == 0)
    {
        if ((entityType & type) != type)
        {
            return false;
        }
    }
    else if (entityType != type)
    {
        return false;
    }

    return true;
}

void Entity::useSmallIds()
{
    TlsSetValue(gSmallIdTlsIndex, (void*)1);
}

int Entity::getSmallId()
{
    bool skipRemoved = false;

    if (TlsGetValue(gSmallIdTlsIndex) != NULL && gMinecraftServer != NULL)
    {
        skipRemoved = gMinecraftServer->flagEntitiesToBeRemoved(sSmallIdsToRemove);
    }

    for (int word = 0, id = 0; word < 64; word++, id += 32)
    {
        if (sSmallIdsInUse[word] == 0xFFFFFFFF)
        {
            continue;
        }

        for (int bit = 0; bit < 32; bit++)
        {
            unsigned int mask = 0x80000000U >> bit;

            if (skipRemoved && (sSmallIdsToRemove[word] & mask) != 0)
            {
                continue;
            }

            if ((sSmallIdsInUse[word] & mask) == 0)
            {
                sSmallIdsInUse[word] |= mask;
                return id + bit;
            }
        }
    }

    return -1;
}

void Entity::freeSmallId(int smallId)
{
    if (TlsGetValue(gSmallIdTlsIndex) == NULL || smallId >= 2048 || gMinecraftServer == NULL)
    {
        return;
    }

    if (!gMinecraftServer->shuttingDown && mLevel != NULL && mLevel->mIsClientSide)
    {
        return;
    }

    int word = smallId >> 5;
    unsigned int mask = 0x80000000U >> (smallId & 31);

    sSmallIdsInUse[word] &= ~mask;
    sSmallIdsWandering[word] &= ~mask;
}

void Entity::fjDebugCheckSmallIdLeaks()
{
}

void Entity::tickExtraWandering()
{
    sWanderingTick++;

    if (sWanderingTick % 600 != 0)
    {
        return;
    }

    sWanderingCount = 0;

    for (int smallId = 0; smallId < 2048; smallId++)
    {
        if ((sSmallIdsWandering[smallId >> 5] & (0x80000000U >> (smallId & 31))) != 0)
        {
            sWanderingIds[sWanderingCount] = smallId;
            sWanderingCount++;

            if (sWanderingCount > 2)
            {
                return;
            }
        }
    }
}

void Entity::considerForExtraWandering(bool consider)
{
    if (TlsGetValue(gSmallIdTlsIndex) == NULL)
    {
        return;
    }

    int smallId = getId();

    if (smallId >= 2048)
    {
        return;
    }

    unsigned int mask = 0x80000000U >> (smallId & 31);

    if (consider)
    {
        sSmallIdsWandering[smallId >> 5] |= mask;
    }
    else
    {
        sSmallIdsWandering[smallId >> 5] &= ~mask;
    }
}

bool Entity::isExtraWanderingEnabled()
{
    if (TlsGetValue(gSmallIdTlsIndex) == NULL)
    {
        return false;
    }

    int smallId = getId();

    if (smallId >= 2048)
    {
        return false;
    }

    for (int i = 0; i < sWanderingCount; i++)
    {
        if (sWanderingIds[i] == smallId)
        {
            return true;
        }
    }

    return false;
}

int Entity::getWanderingQuadrant()
{
    return (getId() + sWanderingTick / 600) & 3;
}

Entity::Entity(Level* level, bool createEntityData)
    : mLevel(level)
{
    MemSect(16);
    _init(createEntityData, level);
    MemSect(0);

    setPos(0.0, 0.0, 0.0);

    if (level != NULL)
    {
        mDimension = level->mDimension->getType()->getId();
    }

    if (createEntityData)
    {
        mEntityData = boost::shared_ptr<SynchedEntityData>(new SynchedEntityData(this));

        mEntityData->createDateItem(DATA_SHARED_FLAGS_ID, 0);
        mEntityData->createDateItem<int>(DATA_AIR_SUPPLY_ID, getMaxAirSupply());
        mEntityData->createDateItem(DATA_CUSTOM_NAME_VISIBLE, false);
        mEntityData->createDateItem(DATA_CUSTOM_NAME, std::wstring(L""));
        mEntityData->createDateItem(DATA_SILENT, false);
        mEntityData->createDateItem(DATA_NO_GRAVITY, false);
        mEntityData->createDateItem(DATA_MOVING, false);
        mEntityData->createDateItem(DATA_NAMED_BY_RESTRICTED_PLAYER, false);
        mEntityData->createDateItem(DATA_CREATED_ON_HOST, false);
        mEntityData->createDateItem(DATA_OWNER_UUID, std::wstring(L""));
    }
    else
    {
        mEntityData.reset();
    }

    mCategories = 0;
    mWaterCheckBox = AABB::newPermanent(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    mCollisionCacheValid = false;
    mCollisionCacheMinX = 0;
    mCollisionCacheMinY = 0;
    mCollisionCacheMinZ = 0;
    mCollisionCacheMaxX = 0;
    mCollisionCacheMaxY = 0;
    mCollisionCacheMaxZ = 0;
    mLiquidCacheValid = false;
    mLiquidCacheMinX = 0;
    mLiquidCacheMinY = 0;
    mLiquidCacheMinZ = 0;
    mLiquidCacheMaxX = 0;
    mLiquidCacheMaxY = 0;
    mLiquidCacheMaxZ = 0;
    mInWallCacheResult = false;
    mInWallCacheX = 0.0;
    mInWallCacheY = 0.0;
    mInWallCacheZ = 0.0;
    mInWallCacheYRot = 0.0f;
    mInWallCacheXRot = 0.0f;
    mAutoSpinAttackTicks = 0;
}

void Entity::_init(bool createEntityData, Level* level)
{
    if (level != NULL && TlsGetValue(gSmallIdTlsIndex) != NULL)
    {
        mId = getSmallId();
    }
    else
    {
        mId = sEntityCount;
        sEntityCount++;

        if (sEntityCount == 0x7FFFFFF)
        {
            sEntityCount = 2048;
        }
    }

    sViewScale = 1.0;
    mBlocksBuilding = false;

    mVehicle.reset();
    mForcedLoading = false;

    mXo = 0.0;
    mYo = 0.0;
    mZo = 0.0;
    mX = 0.0;
    mY = 0.0;
    mZ = 0.0;
    mXd = 0.0;
    mYd = 0.0;
    mZd = 0.0;
    mYRot = 0.0f;
    mXRot = 0.0f;
    mYRotO = 0.0f;
    mXRotO = 0.0f;
    mBoardingCooldown = 0;
    mVehicleXRotDelta = 0.0;
    mVehicleYRotDelta = 0.0;
    mBoundingBox = AABB::newPermanent(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    mOnGround = false;
    mHorizontalCollision = false;
    mVerticalCollision = false;
    mCollision = false;
    mHurtMarked = false;
    mIsInWeb = false;
    mOutsideWorldBorder = false;
    mBlocksSpawning = true;
    mRemoved = false;
    mBbWidth = 0.6f;
    mBbHeight = 1.8f;
    mWalkDistO = 0.0f;
    mWalkDist = 0.0f;
    mMoveDist = 0.0f;
    mFallDistance = 0.0f;
    mFallTicks = 0;
    mFallCount = 0;
    mNextStep = 1;
    mNextFlap = 1.0f;
    mXOld = 0.0;
    mYOld = 0.0;
    mZOld = 0.0;
    mMaxUpStep = 0.0f;
    mNoPhysics = false;
    mPushthrough = 0.0f;
    mRandom = new Random();
    mTickCount = 0;
    mIsInWater = false;
    mHeadInWater = false;
    mInvulnerableTime = 0;
    mFirstTick = true;
    mFireImmune = false;
    mPreventEntitySpawning = false;
    mChunkX = 0;
    mChunkY = 0;
    mChunkZ = 0;
    mLastChunkX = 0;
    mLastChunkY = 0;
    mLastChunkZ = 0;
    mLastSentX = 0;
    mLastSentY = 0;
    mLastSentZ = 0;
    mLastSentYRot = 0;
    mLastSentXRot = 0;
    mAddedToChunk = false;
    mHasImpulse = false;
    mIsInsidePortal = false;
    mPortalTime = 0;
    mDimension = 0;
    mPortalEntranceOffset = NULL;
    mPortalEntranceForwards = NULL;
    mGlowing = false;
    mIgnoreVerticalCollision = false;
    mIgnorePortals = false;

    if (level != NULL)
    {
        mUUID = std::wstring(L"ent") + Mth::createInsecureUUID(mRandom);
    }

    mCommandStats = new CommandStats();
    mRemainingFireTicks = -1;

    mPistonDeltas = new double[3];
    mPistonDeltas[0] = 0.0;
    mPistonDeltas[1] = 0.0;
    mPistonDeltas[2] = 0.0;
    mPistonDeltaCount = 3;
    mPistonDeltasGameTime = 0;

    mAllSlots = NULL;
    mAllSlotCount = 0;
    mAllSlotsOwner.reset();

    mLeashed = false;
    mLeashInfoTag = NULL;
    mType = 0;
    mLeashRestoreTicks = 0;
    mTeleported = false;

    mWasOnFire = false;
    mFireTransitionStartTick = -1;

    mEyeHeight = getEyeHeight();
    mEyeHeightO = mEyeHeight;
    mSwimAmount = 0.0f;
    mSwimAmountO = 0.0f;
}

Entity::~Entity()
{
    freeSmallId(mId);

    delete mRandom;
    delete mBoundingBox;
    delete mCommandStats;
    delete[] mAllSlots;
    delete[] mPistonDeltas;
    delete mWaterCheckBox;

    delete mLeashInfoTag;
    mLeashInfoTag = NULL;
}

void Entity::fjDerivedCtorCalls()
{
    mType = GetType();

    defineSynchedData();
}

void Entity::fjCheckDerivedConstruction()
{
}

int Entity::getId()
{
    return mId;
}

void Entity::setId(int id)
{
    mId = id;
}

boost::shared_ptr<SynchedEntityData> Entity::getEntityData()
{
    return mEntityData;
}

bool Entity::hasCategory(EntityCategory::E_EntityCategory category) const
{
    return (mCategories & category) == category;
}

void Entity::sendMessage(ClientboundChatPacket::EChatPacketMessage message, int* intParams, unsigned int numIntParams, std::wstring* stringParams, unsigned int numStringParams)
{
}

bool Entity::hasPermission(EGameCommand command)
{
    return true;
}

std::wstring Entity::getDisplayName()
{
    return getName();
}

BlockPos Entity::getCommandSenderBlockPosition()
{
    return BlockPos(mX, mY + 0.5, mZ);
}

Vec3* Entity::getCommandSenderWorldPosition()
{
    return Vec3::newTemp(mX, mY, mZ);
}

Level* Entity::getCommandSenderWorld()
{
    return mLevel;
}

boost::shared_ptr<Entity> Entity::getCommandSenderEntity()
{
    return shared_from_this();
}

bool Entity::isSendMessageEnabled()
{
    return false;
}

void Entity::setCommandStats(CommandStats::CommandStatType statType, int value)
{
    if (mLevel != NULL && !mLevel->mIsClientSide)
    {
        mCommandStats->setCommandStats(mLevel->getServer(), boost::dynamic_pointer_cast<CommandSender>(shared_from_this()), statType, value);
    }
}

MinecraftServer* Entity::getServer()
{
    return mLevel->getServer();
}

void Entity::kill()
{
    remove();
}

void Entity::remove()
{
    mRemoved = true;

    stopCurrentLerp();
}

void Entity::setDropContainerContent(bool dropContainerContent)
{
}

void Entity::resetPos()
{
    if (mLevel == NULL)
    {
        return;
    }

    while (mY > 0.0 && mY < 256.0)
    {
        setPos(mX, mY, mZ);

        if (mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox, false, false, false)->empty())
        {
            break;
        }

        mY += 1.0;
    }

    mXd = 0.0;
    mYd = 0.0;
    mZd = 0.0;
    mXRot = 0.0f;
}

void Entity::setSize(float width, float height)
{
    if (width == mBbWidth && height == mBbHeight)
    {
        return;
    }

    float oldWidth = mBbWidth;

    mBbWidth = width;
    mBbHeight = height;

    if (mBbWidth < oldWidth)
    {
        double halfWidth = (double)width * 0.5;

        setBoundingBox(AABB::newTemp(mX - halfWidth, mY, mZ - halfWidth,
                                     mX + halfWidth, mY + (double)height, mZ + halfWidth));
        return;
    }

    AABB* box = mBoundingBox;

    setBoundingBox(AABB::newTemp(box->mMinX, box->mMinY, box->mMinZ,
                                 box->mMinX + (double)mBbWidth, box->mMinY + (double)mBbHeight,
                                 box->mMinZ + (double)mBbWidth));

    if (oldWidth < mBbWidth && !mFirstTick && !mLevel->mIsClientSide)
    {
        move(eMoverType_Self, (double)(oldWidth - mBbWidth), 0.0, (double)(oldWidth - mBbWidth), false);
    }
}

void Entity::setPos(double x, double y, double z)
{
    mX = x;
    mY = y;
    mZ = z;

    double halfWidth = (double)(mBbWidth * 0.5f);

    setBoundingBox(AABB::newTemp(x - halfWidth, y, z - halfWidth,
                                 x + halfWidth, y + (double)mBbHeight, z + halfWidth));
}

void Entity::setRot(float yRot, float xRot)
{
    mYRot = Mth::wrapDegrees(yRot);
    mXRot = Mth::wrapDegrees(xRot);
}

void Entity::setPos(EntityPos* pos)
{
    if (pos->mHasPos)
    {
        setPos(pos->mX, pos->mY, pos->mZ);
    }
    else
    {
        setPos(mX, mY, mZ);
    }

    if (pos->mHasRot)
    {
        setRot(pos->mYRot, pos->mXRot);
    }
    else
    {
        setRot(mYRot, mXRot);
    }
}

void Entity::turn(float yRotDelta, float xRotDelta)
{
    float oldYRot = mYRot;
    float oldXRot = mXRot;

    mXRot = -(xRotDelta * 0.15f - mXRot);
    mYRot = yRotDelta * 0.15f + mYRot;
    mXRot = Mth::clamp(mXRot, -90.0f, 90.0f);

    mXRotO += mXRot - oldXRot;
    mYRotO += mYRot - oldYRot;

    if (mVehicle)
    {
        mVehicle->onPassengerTurned(shared_from_this());
    }
}

void Entity::interpolateTurn(float yRotDelta, float xRotDelta)
{
    mXRot = -(xRotDelta * 0.15f - mXRot);
    mYRot = yRotDelta * 0.15f + mYRot;
    mXRot = Mth::clamp(mXRot, -90.0f, 90.0f);
}

void Entity::DamageNearbyMobs()
{
    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntities(shared_from_this(), mBoundingBox->grow(1.0, 1.0, 1.0), NULL);

    if (!entities->empty())
    {
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
             it != entities->end(); ++it)
        {
            DoAutoAttackOnTouch(*it);

            mXd = -(mXd * 0.20000000298023224);
            mYd = -(mYd * 0.20000000298023224);
            mZd = -(mZd * 0.20000000298023224);
            mFallDistance = 0.0f;

            setDamageNearbyMobs(false);
            stopSpinAttack();
        }
    }
    else if (mOnGround)
    {
        setDamageNearbyMobs(false);
        stopSpinAttack();
    }
}

void Entity::tick()
{
    if (!mLevel->mIsClientSide)
    {
        setSharedFlag(8, isGlowing());
    }

    if (getDamageNearbyMobs())
    {
        DamageNearbyMobs();

        mAutoSpinAttackTicks++;

        if (mAutoSpinAttackTicks > 19 || (mAutoSpinAttackTicks > 5 && mOnGround))
        {
            setDamageNearbyMobs(false);
            stopSpinAttack();
        }
    }
    else
    {
        mAutoSpinAttackTicks = 0;
    }

    PIXBeginNamedEvent(0.0f, "Entity tick");
    baseTick();
    PIXEndNamedEvent();
}

bool Entity::isInWaterOrRain()
{
    if (mIsInWater)
    {
        return true;
    }

    if (!mLevel->isRaining())
    {
        return false;
    }

    if (mLevel->isRainingAt(BlockPos(mX, mY, mZ)))
    {
        return true;
    }

    return mLevel->isRainingAt(BlockPos(mX, mY + (double)mBbHeight, mZ));
}

bool Entity::isInWaterRainOrBubble()
{
    if (mLevel->getBlock(BlockPos(mX, mY, mZ)) == Blocks::bubble_column_segment)
    {
        return true;
    }

    return isInWaterOrRain();
}

bool Entity::isInWaterOrBubble()
{
    if (mLevel->getBlock(BlockPos(mX, mY, mZ)) == Blocks::bubble_column_segment)
    {
        return true;
    }

    return isInWater();
}

bool Entity::isInLava()
{
    return mLevel->isInLava(this, mBoundingBox->grow(-0.10000000149011612, -0.4000000059604645,
                                                     -0.10000000149011612));
}

void Entity::lavaHurt()
{
    if (mFireImmune)
    {
        return;
    }

    hurt(DamageSource::LAVA, 4.0f);
    setSecondsOnFire(15);
}

void Entity::baseTick()
{
    if (isPassenger() && getVehicle()->mRemoved)
    {
        stopRiding();
    }

    if (mBoardingCooldown > 0)
    {
        mBoardingCooldown--;
    }

    mWalkDistO = mWalkDist;
    mXo = mX;
    mYo = mY;
    mZo = mZ;
    mYRotO = mYRot;
    mXRotO = mXRot;

    if (!mLevel->mIsClientSide)
    {
        bool portalsEnabled = true;

        if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
        {
            portalsEnabled = !Minecraft::GetInstance()->GetMiniGame()->ArePortalsDisabled();
        }

        if (!mIgnorePortals && portalsEnabled)
        {
            if (mIsInsidePortal)
            {
                if (dynamic_cast<ServerLevel*>(mLevel)->getServer()->isNetherEnabled())
                {
                    if (!isPassenger())
                    {
                        int waitTime = getPortalWaitTime();

                        if (mPortalTime++ >= waitTime)
                        {
                            mPortalTime = waitTime;
                            mPortalCooldown = getDimensionChangingDelay();

                            int dimension = -1;

                            if (mLevel->mDimension->getType() == DimensionType::NETHER)
                            {
                                dimension = 0;
                            }

                            setSprinting(false);
                            changeDimension(dimension);
                        }
                    }

                    mIsInsidePortal = false;
                    clearFire();
                }
            }
            else
            {
                if (mPortalTime > 0)
                {
                    mPortalTime -= 4;

                    if (mPortalTime < 0)
                    {
                        mPortalTime = 0;
                    }
                }
                else if (mPortalTime < 0)
                {
                    mPortalTime = 0;
                }

                processDimensionDelay();
            }
        }
    }

    updateSprintingState();
    updateInWaterState();

    mSwimAmountO = mSwimAmount;

    if (isSwimming())
    {
        mSwimAmount += 0.10000000149011612f;

        if (mSwimAmount > 1.0f)
        {
            mSwimAmount = 1.0f;
        }
    }
    else
    {
        mSwimAmount -= 0.10000000149011612f;

        if (mSwimAmount < 0.0f)
        {
            mSwimAmount = 0.0f;
        }
    }

    if (mFirstTick)
    {
        mWasOnFire = isOnFire();

        if (mWasOnFire)
        {
            mFireTransitionStartTick = 0;
        }
    }
    else if (isOnFire() != mWasOnFire)
    {
        mWasOnFire = isOnFire();
        mFireTransitionStartTick = mTickCount;
    }

    if (mLevel->mIsClientSide)
    {
        clearFire();
    }
    else if (mRemainingFireTicks > 0)
    {
        if (mFireImmune)
        {
            mRemainingFireTicks -= 4;

            if (mRemainingFireTicks < 0)
            {
                clearFire();
            }
        }
        else
        {
            if (mRemainingFireTicks % 20 == 0)
            {
                hurt(DamageSource::ON_FIRE, 1.0f);
            }

            mRemainingFireTicks--;
        }
    }

    PIXBeginNamedEvent(0.0f, "Check for fire");

    bool inWater = isInWaterRainOrBubble();

    if (mLevel->containsFireBlock(this, mBoundingBox->shrink(0.001, 0.001, 0.001)))
    {
        burn(1);

        if (!inWater && ++mRemainingFireTicks == 0)
        {
            setSecondsOnFire(8);
        }
    }
    else if (mRemainingFireTicks < 1)
    {
        mRemainingFireTicks = -1;
    }

    PIXEndNamedEvent();

    if (inWater && isOnFire())
    {
        PIXBeginNamedEvent(0.0f, "Play fizz sound");
        playSound(SoundEvents::GENERIC_EXTINGUISH_FIRE, 0.699999988079071f,
                  (mRandom->nextFloat() - mRandom->nextFloat()) * 0.4000000059604645f + 1.0f);
        mRemainingFireTicks = -1;
        PIXEndNamedEvent();
    }

    if (isInLava())
    {
        lavaHurt();
        mFallDistance *= 0.5f;
    }

    if (mY < -64.0)
    {
        outOfWorld();
    }

    if (!mLevel->mIsClientSide)
    {
        setSharedFlag(0, mRemainingFireTicks > 0);
    }

    mFirstTick = false;
}

void Entity::processDimensionDelay()
{
    if (mPortalCooldown > 0)
    {
        mPortalCooldown--;
    }
}

int Entity::getPortalWaitTime()
{
    return 1;
}

void Entity::setSecondsOnFire(int seconds)
{
    int ticks = seconds * 20;

    if ((GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
    {
        ticks = ProtectionEnchantment::getFireAfterDampener(
            boost::static_pointer_cast<LivingEntity>(shared_from_this()), ticks);
    }

    if (mRemainingFireTicks < ticks)
    {
        mRemainingFireTicks = ticks;
    }
}

void Entity::clearFire()
{
    mRemainingFireTicks = 0;
}

void Entity::outOfWorld()
{
    remove();
}

bool Entity::isFree(AABB* box)
{
    return mLevel->getCollisionAABBs(shared_from_this(), box, false, false, false)->empty()
        && !mLevel->containsAnyLiquid(box);
}

bool Entity::isFree(double x, double y, double z)
{
    return isFree(mBoundingBox->move(x, y, z));
}

std::vector<boost::weak_ptr<Entity> >* Entity::getPassengers()
{
    return &mPassengers;
}

void Entity::move(MoverType type, double x, double y, double z, bool pushedByPiston)
{
    std::vector<boost::weak_ptr<Entity> > passengers = *getPassengers();

    for (std::vector<boost::weak_ptr<Entity> >::iterator it = passengers.begin();
         it != passengers.end(); ++it)
    {
        boost::shared_ptr<Entity> passenger = it->lock();

        if (passenger && passenger->GetType() == ENTITY_TYPE_LOCAL_PLAYER)
        {
            if (!boost::static_pointer_cast<LocalPlayer>(passenger)->canMoveOrBeMovedFromTo(
                    mX, mY, mZ, mX + x, mY + y, mZ + z, NULL))
            {
                return;
            }
        }
    }

    if (mNoPhysics)
    {
        setBoundingBox(mBoundingBox->move(x, y, z));
        setLocationFromBoundingbox();
        return;
    }

    if (type == eMoverType_Piston)
    {
        long long gameTime = mLevel->getGameTime();

        if (gameTime != mPistonDeltasGameTime)
        {
            delete[] mPistonDeltas;
            mPistonDeltas = new double[3];
            mPistonDeltas[0] = 0.0;
            mPistonDeltas[1] = 0.0;
            mPistonDeltas[2] = 0.0;
            mPistonDeltaCount = 3;
            mPistonDeltasGameTime = gameTime;
        }

        if (x != 0.0)
        {
            int axis = Direction::Axis::X->getId();
            double moved = Mth::clamp(x + mPistonDeltas[axis], -0.51, 0.51);

            x = moved - mPistonDeltas[axis];
            mPistonDeltas[axis] = moved;

            if (fabs(x) <= 2.220446049250313e-16)
            {
                return;
            }
        }
        else if (y != 0.0)
        {
            int axis = Direction::Axis::Y->getId();
            double moved = Mth::clamp(y + mPistonDeltas[axis], -0.51, 0.51);

            y = moved - mPistonDeltas[axis];
            mPistonDeltas[axis] = moved;

            if (fabs(y) <= 2.220446049250313e-16)
            {
                return;
            }
        }
        else if (z != 0.0)
        {
            int axis = Direction::Axis::Z->getId();
            double moved = Mth::clamp(z + mPistonDeltas[axis], -0.51, 0.51);

            z = moved - mPistonDeltas[axis];
            mPistonDeltas[axis] = moved;

            if (fabs(z) <= 2.220446049250313e-16)
            {
                return;
            }
        }
        else
        {
            return;
        }
    }

    double startX = mX;
    double startY = mY;
    double startZ = mZ;

    if (mIsInWeb)
    {
        mIsInWeb = false;

        x *= 0.25;
        y *= 0.05000000074505806;
        z *= 0.25;

        mXd = 0.0;
        mYd = 0.0;
        mZd = 0.0;
    }

    double wantedX = x;
    double wantedY = y;
    double wantedZ = z;

    if ((type == eMoverType_Self || type == eMoverType_Player) && mOnGround && isSneaking()
        && (GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        while (x != 0.0
               && mLevel->getCollisionAABBs(shared_from_this(),
                                            mBoundingBox->move(x, -(double)mMaxUpStep, 0.0),
                                            false, false, false)->empty())
        {
            if (x < 0.05 && x >= -0.05)
            {
                x = 0.0;
            }
            else if (x > 0.0)
            {
                x -= 0.05;
            }
            else
            {
                x += 0.05;
            }
        }

        while (z != 0.0
               && mLevel->getCollisionAABBs(shared_from_this(),
                                            mBoundingBox->move(0.0, -(double)mMaxUpStep, z),
                                            false, false, false)->empty())
        {
            if (z < 0.05 && z >= -0.05)
            {
                z = 0.0;
            }
            else if (z > 0.0)
            {
                z -= 0.05;
            }
            else
            {
                z += 0.05;
            }
        }

        while (x != 0.0 && z != 0.0
               && mLevel->getCollisionAABBs(shared_from_this(),
                                            mBoundingBox->move(x, -(double)mMaxUpStep, z),
                                            false, false, false)->empty())
        {
            if (x < 0.05 && x >= -0.05)
            {
                x = 0.0;
            }
            else if (x > 0.0)
            {
                x -= 0.05;
            }
            else
            {
                x += 0.05;
            }

            if (z < 0.05 && z >= -0.05)
            {
                z = 0.0;
            }
            else if (z > 0.0)
            {
                z -= 0.05;
            }
            else
            {
                z += 0.05;
            }
        }
    }

    PIXBeginNamedEvent(0.0f, "Get cubes");

    std::vector<AABB*>* cubes = mLevel->getCollisionAABBs(
        shared_from_this(),
        mBoundingBox->expand(x, y, z)->expand(0.0, -y, 0.0)->expand(0.0, (double)mMaxUpStep, 0.0),
        pushedByPiston, true, false);

    PIXEndNamedEvent();

    AABB* startBox = AABB::newTemp(mBoundingBox);

    int chunkX = Mth::absFloor(mX * 0.0625);
    int chunkZ = Mth::absFloor(mZ * 0.0625);

    bool spectator = false;

    if ((GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        spectator = boost::static_pointer_cast<Player>(shared_from_this())->isSpectator();
    }

    if (!spectator)
    {
        if (y != 0.0 && (!mLevel->mIsClientSide || mLevel->reallyHasChunk(chunkX, chunkZ)))
        {
            PIXBeginNamedEvent(0.0f, "Clipy");

            int count = (int)cubes->size();

            for (int i = 0; y != 0.0; i++)
            {
                if (i >= count)
                {
                    setBoundingBox(mBoundingBox->move(0.0, y, 0.0));
                    break;
                }

                y = mBoundingBox->clipYCollide(cubes->at(i), y);
            }

            PIXEndNamedEvent();
        }

        PIXBeginNamedEvent(0.0f, "ClipX");

        if (x != 0.0)
        {
            int count = (int)cubes->size();

            for (int i = 0; x != 0.0; i++)
            {
                if (i >= count)
                {
                    setBoundingBox(mBoundingBox->move(x, 0.0, 0.0));
                    break;
                }

                x = mBoundingBox->clipXCollide(cubes->at(i), x);
            }
        }

        PIXEndNamedEvent();
        PIXBeginNamedEvent(0.0f, "ClipZ");

        if (z != 0.0)
        {
            int count = (int)cubes->size();

            for (int i = 0; z != 0.0; i++)
            {
                if (i >= count)
                {
                    setBoundingBox(mBoundingBox->move(0.0, 0.0, z));
                    break;
                }

                z = mBoundingBox->clipZCollide(cubes->at(i), z);
            }
        }

        PIXEndNamedEvent();
    }

    bool steppedOnGround = mOnGround || (wantedY != y && wantedY <= 0.0);

    PIXBeginNamedEvent(0.0f, "Step up");

    double clippedX = x;
    double clippedY = y;
    double clippedZ = z;

    if (mMaxUpStep > 0.0f && steppedOnGround && (wantedX != x || wantedZ != z))
    {
        AABB* clippedBox = AABB::newTemp(mBoundingBox);

        setBoundingBox(startBox);

        x = wantedX;
        z = wantedZ;
        y = (double)mMaxUpStep;

        AABB* stepBox = mBoundingBox->expand(wantedX, 0.0, wantedZ);

        double stepY = y;
        AABB* stepped = mBoundingBox;

        if (!mLevel->mIsClientSide || mLevel->reallyHasChunk(chunkX, chunkZ))
        {
            for (std::vector<AABB*>::iterator it = cubes->begin(); stepY != 0.0; ++it)
            {
                if (it == cubes->end())
                {
                    stepped = stepped->move(0.0, stepY, 0.0);
                    break;
                }

                stepY = stepBox->clipYCollide(*it, stepY);
            }
        }

        double steppedX = x;

        for (std::vector<AABB*>::iterator it = cubes->begin(); steppedX != 0.0; ++it)
        {
            if (it == cubes->end())
            {
                stepped = stepped->move(steppedX, 0.0, 0.0);
                break;
            }

            steppedX = stepped->clipXCollide(*it, steppedX);
        }

        double steppedZ = z;

        for (std::vector<AABB*>::iterator it = cubes->begin(); steppedZ != 0.0; ++it)
        {
            if (it == cubes->end())
            {
                stepped = stepped->move(0.0, 0.0, steppedZ);
                break;
            }

            steppedZ = stepped->clipZCollide(*it, steppedZ);
        }

        AABB* raised = AABB::newTemp(mBoundingBox);

        double raisedY = (double)mMaxUpStep;

        if (!mLevel->mIsClientSide || mLevel->reallyHasChunk(chunkX, chunkZ))
        {
            for (std::vector<AABB*>::iterator it = cubes->begin(); raisedY != 0.0; ++it)
            {
                if (it == cubes->end())
                {
                    raised = raised->move(0.0, raisedY, 0.0);
                    break;
                }

                raisedY = raised->clipYCollide(*it, raisedY);
            }
        }

        double raisedX = x;

        for (std::vector<AABB*>::iterator it = cubes->begin(); raisedX != 0.0; ++it)
        {
            if (it == cubes->end())
            {
                raised = raised->move(raisedX, 0.0, 0.0);
                break;
            }

            raisedX = raised->clipXCollide(*it, raisedX);
        }

        double raisedZ = z;

        for (std::vector<AABB*>::iterator it = cubes->begin(); raisedZ != 0.0; ++it)
        {
            if (it == cubes->end())
            {
                raised = raised->move(0.0, 0.0, raisedZ);
                break;
            }

            raisedZ = raised->clipZCollide(*it, raisedZ);
        }

        if (steppedX * steppedX + steppedZ * steppedZ <= raisedX * raisedX + raisedZ * raisedZ)
        {
            x = raisedX;
            z = raisedZ;
            y = -raisedY;
            setBoundingBox(raised);
        }
        else
        {
            x = steppedX;
            z = steppedZ;
            y = -stepY;
            setBoundingBox(stepped);
        }

        y = -(double)mMaxUpStep;

        for (std::vector<AABB*>::iterator it = cubes->begin(); y != 0.0; ++it)
        {
            if (it == cubes->end())
            {
                setBoundingBox(mBoundingBox->move(0.0, y, 0.0));
                break;
            }

            y = mBoundingBox->clipYCollide(*it, y);
        }

        if (x * x + z * z <= clippedX * clippedX + clippedZ * clippedZ)
        {
            x = clippedX;
            y = clippedY;
            z = clippedZ;
            setBoundingBox(clippedBox);
        }
    }

    PIXEndNamedEvent();

    setLocationFromBoundingbox();

    mHorizontalCollision = wantedX != x || wantedZ != z;
    mVerticalCollision = !mIgnoreVerticalCollision && wantedY != y;
    mOnGround = mVerticalCollision && wantedY < 0.0;
    mCollision = mHorizontalCollision || mVerticalCollision;

    if ((GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY && mCollision
        && !g_pApp->IsAppPaused())
    {
        boost::shared_ptr<LivingEntity> living =
            boost::static_pointer_cast<LivingEntity>(shared_from_this());

        if (living->isFallFlying())
        {
            living->takeFallFlyingCollisionDamage();
        }
    }

    BlockPos landingPos(Mth::absFloor(mX), Mth::absFloor(mY - 0.20000000298023224), Mth::absFloor(mZ));
    Block* landingBlock = mLevel->getBlock(landingPos);

    PIXBeginNamedEvent(0.0f, "Falling check");

    if (landingBlock->mMaterial == Material::AIR)
    {
        Block* below = mLevel->getBlock(landingPos.below());

        if (below != NULL
            && (dynamic_cast<FenceBlock*>(below) != NULL
                || dynamic_cast<WallBlock*>(below) != NULL
                || dynamic_cast<FenceGateBlock*>(below) != NULL))
        {
            landingPos = landingPos.below();
            landingBlock = below;
        }
    }

    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Check fall damage");
    checkFallDamage(y, mOnGround, landingBlock, landingPos);
    PIXEndNamedEvent();

    updateDeltaAfterMove(landingBlock, wantedX, wantedY, wantedZ, x, y, z);

    if (makeStepSound()
        && (!mOnGround || !isSneaking() || (GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER)
        && !isPassenger())
    {
        PIXBeginNamedEvent(0.0f, "Make step sound");

        double walkedX = mX - startX;
        double walkedY = 0.0;
        double walkedZ = mZ - startZ;

        if (landingBlock == Blocks::ladder)
        {
            walkedY = mY - startY;
        }

        if (mOnGround)
        {
            landingBlock->stepOn(mLevel, landingPos, shared_from_this());
        }

        mWalkDist += (float)(Mth::sqrt(walkedX * walkedX + walkedZ * walkedZ) * 0.6000000238418579);
        mMoveDist += (float)(Mth::sqrt(walkedX * walkedX + walkedY * walkedY + walkedZ * walkedZ)
                             * 0.6000000238418579);

        if (mMoveDist > (float)mNextStep && landingBlock->mMaterial != Material::AIR)
        {
            mNextStep = (int)mMoveDist + 1;

            if (isInWater())
            {
                playUnderwaterSound(landingPos, landingBlock);
            }
            else
            {
                playStepSound(landingPos, landingBlock);
            }

            PIXEndNamedEvent();
        }
        else
        {
            if (mMoveDist > mNextFlap && makeFlySound() && landingBlock->mMaterial == Material::AIR)
            {
                mNextFlap = playFlySound(mMoveDist);
            }

            PIXEndNamedEvent();
        }
    }

    PIXBeginNamedEvent(0.0f, "checkInsideBlocks");
    checkInsideBlocks();
    PIXEndNamedEvent();
}

void Entity::updateDeltaAfterMove(Block* block, double x, double y, double z,
                                  double& movedX, double& movedY, double& movedZ)
{
    if (x != movedX)
    {
        mXd = 0.0;
    }

    if (z != movedZ)
    {
        mZd = 0.0;
    }

    if (y != movedY)
    {
        PIXBeginNamedEvent(0.0f, "Update entity after falling");
        block->updateEntityAfterFallOn(mLevel, shared_from_this());
        PIXEndNamedEvent();
    }
}

void Entity::setLocationFromBoundingbox()
{
    AABB* box = mBoundingBox;

    mX = (box->mMinX + box->mMaxX) * 0.5;
    mY = box->mMinY;
    mZ = (box->mMinZ + box->mMaxZ) * 0.5;
}

const SoundEvent* Entity::getSwimSound()
{
    return SoundEvents::GENERIC_SWIM;
}

const SoundEvent* Entity::getSwimSplashSound()
{
    return SoundEvents::GENERIC_SPLASH;
}

const SoundEvent* Entity::getHeavySplashSound()
{
    return getSwimSplashSound();
}

void Entity::checkInsideBlocks()
{
    AABB* box = mBoundingBox;

    BlockPos from(box->mMinX + 0.001, box->mMinY + 0.001, box->mMinZ + 0.001);
    BlockPos to(box->mMaxX - 0.001, box->mMaxY - 0.001, box->mMaxZ - 0.001);

    bool inMiniGame = Minecraft::InMiniGame(eMiniGameId_Glide, true);

    if (!mLevel->hasChunksAt(from, to))
    {
        return;
    }

    for (int x = from.x; x <= to.x; x++)
    {
        for (int y = from.y; y <= to.y; y++)
        {
            for (int z = from.z; z <= to.z; z++)
            {
                BlockPos pos(x, y, z);
                const BlockState* state = mLevel->getBlockState(mTickCache, pos);

                if (inMiniGame && !g_pApp->ShouldCollideWith(state->getBlock()))
                {
                    continue;
                }

                state->getBlock()->entityInside(mLevel, pos, state, shared_from_this());
                onInsideBlock(state);
            }
        }
    }
}

void Entity::onInsideBlock(const BlockState* state)
{
}

_SkinAdjustments Entity::getSkinAdjustments()
{
    if (GameSettings::GetGameSettings(eGameSetting_SkinAdjustments) == 0
        && (mCategories & 0x1F1F810) == 0)
    {
        return _SkinAdjustments();
    }

    return mSkinAdjustments;
}

void Entity::setSkinAdjustments(_SkinAdjustments skinAdjustments)
{
    mSkinAdjustments = skinAdjustments;
}

void Entity::playStepSound(const BlockPos& pos, Block* block)
{
    const SoundType* soundType = block->getSoundType();

    MemSect(31);

    if ((GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER
        || (getSkinAdjustments().mAnimFlags & 0x4) == 0)
    {
        Block* above = mLevel->getBlockState(mTickCache, pos.above())->getBlock();

        if (above == Blocks::snow_layer)
        {
            const SoundType* snowType = above->getSoundType();

            playSound(snowType->getStepSound(), snowType->getVolume() * 0.15000000596046448f,
                      snowType->getPitch());
        }
        else if (!block->defaultBlockState()->getMaterial()->isLiquid())
        {
            playSound(soundType->getStepSound(), soundType->getVolume() * 0.15000000596046448f,
                      soundType->getPitch());
        }
    }

    MemSect(0);
}

void Entity::playUnderwaterSound(const BlockPos& pos, Block* block)
{
    boost::shared_ptr<Entity> source = shared_from_this();

    if (isVehicle() && getVehicle())
    {
        source = getVehicle();
    }

    float volume = source == shared_from_this() ? 0.6000000238418579f : 0.550000011920929f;

    float speed = (float)sqrt(source->mXd * source->mXd * 0.20000000298023224
                              + source->mYd * source->mYd
                              + source->mZd * source->mZd * 0.20000000298023224);

    float amount = speed * volume;

    if (amount > 1.0f)
    {
        amount = 1.0f;
    }

    playSound(getSwimSound(), amount,
              (mRandom->nextFloat() - mRandom->nextFloat()) * 0.4000000059604645f + 1.0f);
}

float Entity::playFlySound(float moveDist)
{
    return 0.0f;
}

bool Entity::makeFlySound()
{
    return false;
}

void Entity::playSoundToAllPlayers(const SoundEvent* sound, float volume, float pitch)
{
    if (isSilent())
    {
        return;
    }

    mLevel->playSoundToAllPlayers(BlockPos(mX, mY, mZ), sound, getSoundSource(), volume, pitch, 16.0f);
}

void Entity::playSound(const SoundEvent* sound, float volume, float pitch)
{
    if (isSilent())
    {
        return;
    }

    mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, sound, getSoundSource(), volume,
                      pitch, 16.0f);
}

bool Entity::isSilent()
{
    return mEntityData->get(DATA_SILENT);
}

void Entity::setSilent(bool silent)
{
    mEntityData->set(DATA_SILENT, silent, true);
}

bool Entity::isNoGravity()
{
    return mEntityData->get(DATA_NO_GRAVITY);
}

void Entity::setNoGravity(bool noGravity)
{
    mEntityData->set(DATA_NO_GRAVITY, noGravity, true);
}

bool Entity::isMoving()
{
    return mEntityData->get(DATA_MOVING);
}

void Entity::setMoving(bool moving)
{
    mEntityData->set(DATA_MOVING, moving, true);
}

bool Entity::makeStepSound()
{
    return true;
}

void Entity::checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos)
{
    if (!onGround)
    {
        if (y < 0.0)
        {
            mFallTicks++;
            mFallDistance -= (float)y;
        }

        return;
    }

    if (mFallDistance > 0.0f)
    {
        block->fallOn(mLevel, pos, shared_from_this(), mFallDistance);
        mFallCount++;
    }

    mFallDistance = 0.0f;
    mFallTicks = 0;
}

AABB* Entity::getCollideBox()
{
    return NULL;
}

void Entity::burn(int damage)
{
    if (mFireImmune)
    {
        return;
    }

    hurt(DamageSource::IN_FIRE, (float)damage);
}

bool Entity::isFireImmune()
{
    return mFireImmune;
}

void Entity::causeFallDamage(float fallDistance, float damageMultiplier)
{
    if (!isVehicle())
    {
        return;
    }

    std::vector<boost::weak_ptr<Entity> > passengers = *getPassengers();

    for (std::vector<boost::weak_ptr<Entity> >::iterator it = passengers.begin();
         it != passengers.end(); ++it)
    {
        boost::shared_ptr<Entity> passenger = it->lock();

        if (passenger)
        {
            passenger->causeFallDamage(fallDistance, damageMultiplier);
        }
    }
}

bool Entity::isInWater()
{
    return mIsInWater;
}

bool Entity::headInWater()
{
    return mHeadInWater;
}

bool Entity::isOverWater()
{
    return mLevel->checkAndHandleWater(
        mBoundingBox->grow(0.0, -20.0, 0.0)->shrink(0.001), Material::WATER, shared_from_this());
}

Vec3* Entity::getEyePosition(float partialTicks)
{
    if (partialTicks >= 1.0f)
    {
        return Vec3::newTemp(mX, mY + (double)getEyeHeight(), mZ);
    }

    return Vec3::newTemp((mX - mXo) * (double)partialTicks + mXo,
                         (mY - mYo) * (double)partialTicks + mYo + (double)getEyeHeight(),
                         (mZ - mZo) * (double)partialTicks + mZo);
}

bool Entity::updateInWaterState()
{
    mHeadInWater = mLevel->getBlock(BlockPos(getEyePosition(1.0f)))->defaultBlockState()
                       ->getMaterial()->isLiquid();

    AABB* box = mBoundingBox->grow(0.0, -0.4000000059604645, 0.0)->shrink(0.001, 0.001, 0.001);

    if (!mIsInWater && !mTickCache.mCacheChanged)
    {
        if (Mth::absFloor(box->mMinX) == Mth::absFloor(mWaterCheckBox->mMinX)
            && (int)ceil(box->mMaxX) == (int)ceil(mWaterCheckBox->mMaxX)
            && Mth::absFloor(box->mMinY) == Mth::absFloor(mWaterCheckBox->mMinY)
            && (int)ceil(box->mMaxY) == (int)ceil(mWaterCheckBox->mMaxY)
            && Mth::absFloor(box->mMinZ) == Mth::absFloor(mWaterCheckBox->mMinZ)
            && (int)ceil(box->mMaxZ) == (int)ceil(mWaterCheckBox->mMaxZ))
        {
            mWaterCheckBox->set(box);
            return false;
        }
    }

    mWaterCheckBox->set(box);

    bool inBoat = false;

    if (getVehicle() && getVehicle()->GetType() == ENTITY_TYPE_BOAT)
    {
        inBoat = true;
    }

    if (!inBoat)
    {
        bool inWater = true;

        if (!mLevel->checkAndHandleWater(
                mBoundingBox->grow(0.0, -0.4000000059604645, 0.0)->shrink(0.001, 0.001, 0.001),
                Material::WATER, shared_from_this()))
        {
            if (!isInLiquid(Material::WATER, getAttachPos(ENTITY_LOCATION_EYES, 0.0f)))
            {
                inWater = false;
            }
        }

        if (inWater)
        {
            if (!mIsInWater && !mFirstTick && canCreateParticles())
            {
                doWaterSplashEffect();
                mFallCount++;
            }

            mFallDistance = 0.0f;
            mFallTicks = 0;
            mIsInWater = true;
            clearFire();

            return mIsInWater;
        }
    }

    mIsInWater = false;

    return false;
}

void Entity::doWaterSplashEffect()
{
    boost::shared_ptr<Entity> source = shared_from_this();

    if (isVehicle() && getVehicle())
    {
        source = getVehicle();
    }

    float volume = source == shared_from_this() ? 0.20000000298023224f : 0.8999999761581421f;

    float speed = (float)sqrt(source->mXd * source->mXd * 0.20000000298023224
                              + source->mYd * source->mYd
                              + source->mZd * source->mZd * 0.20000000298023224) * volume;

    MemSect(31);

    if (speed > 1.0f)
    {
        speed = 1.0f;
    }

    if (speed < 0.25f)
    {
        playSound(getSwimSplashSound(), speed + 0.25f,
                  (mRandom->nextFloat() - mRandom->nextFloat()) * 0.4000000059604645f + 1.0f);
    }
    else
    {
        playSound(getHeavySplashSound(), speed + 0.25f,
                  (mRandom->nextFloat() - mRandom->nextFloat()) * 0.4000000059604645f + 1.0f);
    }

    MemSect(0);

    float surfaceY = (float)Mth::absFloor(mBoundingBox->mMinY);

    for (int i = 0; (float)i < mBbWidth * 20.0f + 1.0f; i++)
    {
        mLevel->addParticle(PARTICLE_BUBBLE,
                            mX + (double)((mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth),
                            (double)(surfaceY + 1.0f),
                            mZ + (double)((mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth),
                            mXd, mYd - (double)(mRandom->nextFloat() * volume), mZd,
                            arrayWithLength<int>());
    }

    for (int i = 0; (float)i < mBbWidth * 20.0f + 1.0f; i++)
    {
        mLevel->addParticle(PARTICLE_SPLASH,
                            mX + (double)((mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth),
                            (double)(surfaceY + 1.0f),
                            mZ + (double)((mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth),
                            mXd, mYd, mZd, arrayWithLength<int>());
    }
}

void Entity::updateSprintingState()
{
    if (isSprinting() && !isInWater())
    {
        doSprintParticleEffect();
    }
}

void Entity::doSprintParticleEffect()
{
    BlockPos pos(Mth::absFloor(mX), Mth::absFloor(mY - 0.20000000298023224), Mth::absFloor(mZ));
    const BlockState* state = mLevel->getBlockState(mTickCache, pos);

    if (state->getRenderShape() == -1)
    {
        return;
    }

    int stateId = Block::getIdWithData(state);

    arrayWithLength<int> particleData(1, false);
    particleData.data[0] = stateId;
    mLevel->addParticle(PARTICLE_BLOCK_DUST,
                        mX + (double)((mRandom->nextFloat() - 0.5f) * mBbWidth),
                        mBoundingBox->mMinY + 0.1,
                        mZ + (double)((mRandom->nextFloat() - 0.5f) * mBbWidth),
                        -(mXd * 4.0), 1.5, -(mZd * 4.0), particleData);
}

bool Entity::isUnderLiquid(Material* material)
{
    if (getVehicle() && getVehicle()->GetType() == ENTITY_TYPE_BOAT)
    {
        return false;
    }

    double eyeY = mY + (double)getEyeHeight();
    BlockPos pos(mX, eyeY, mZ);

    const BlockState* state = mLevel->getBlockState(mTickCache, pos);

    if (state->getMaterial() != material)
    {
        unsigned short element = mLevel->getElement(pos.x, pos.y, pos.z);

        if (element == 0)
        {
            return false;
        }

        state = Block::getStateByElementId(element);
    }

    if (state->getMaterial() != material)
    {
        return false;
    }

    double surface = (double)((float)(pos.y + 1)
                              - (float)(LiquidBlock::getHeightFromData(
                                            state->getBlock()->convertBlockStateToLegacyData(state))
                                        - 0.1111111119389534));

    if (eyeY < surface)
    {
        return true;
    }

    return (GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER ? false : false;
}

bool Entity::isInLiquid(Material* material, Vec3* pos)
{
    if (getVehicle() && getVehicle()->GetType() == ENTITY_TYPE_BOAT)
    {
        return false;
    }

    BlockPos blockPos(pos->x, pos->y, pos->z);
    const BlockState* state = mLevel->getBlockState(mTickCache, blockPos);

    if (state->getMaterial() != material)
    {
        unsigned short element = mLevel->getElement(blockPos.x, blockPos.y, blockPos.z);

        if (element == 0)
        {
            return false;
        }

        state = Block::getStateByElementId(element);
    }

    if (state->getMaterial() != material)
    {
        return false;
    }

    double surface = (double)((float)(blockPos.y + 1)
                              - (float)(LiquidBlock::getHeightFromData(
                                            state->getBlock()->convertBlockStateToLegacyData(state))
                                        - 0.1111111119389534));

    if (pos->y < surface)
    {
        return true;
    }

    return false;
}

void Entity::moveRelative(float x, float y, float z, float speed)
{
    float lengthSqr = x * x + y * y + z * z;

    if (lengthSqr < 9.999999747378752e-05f)
    {
        return;
    }

    float length = sqrtf(lengthSqr);

    if (length < 1.0f)
    {
        length = 1.0f;
    }

    float scale = speed / length;

    float sin = Mth::sin(mYRot * 0.01745329238474369f);
    float cos = Mth::cos(mYRot * 0.01745329238474369f);

    mXd += (double)(x * scale * cos - z * scale * sin);
    mYd += (double)(y * scale);
    mZd += (double)(z * scale * cos + x * scale * sin);
}

int Entity::GetLightColor(Entity* entity)
{
    MutableBlockPos pos(Mth::absFloor(entity->mX), 0, Mth::absFloor(entity->mZ));

    if (!entity->mLevel->hasChunkAt(pos))
    {
        return 0;
    }

    pos.setY(Mth::absFloor(entity->mY + (double)entity->getEyeHeight()));

    return entity->mLevel->getLightColor(pos, 0, -1);
}

int Entity::getLightColor()
{
    return GetLightColor(this);
}

float Entity::getBrightness()
{
    MutableBlockPos pos(Mth::absFloor(mX), 0, Mth::absFloor(mZ));

    if (!mLevel->hasChunkAt(pos))
    {
        return 0.0f;
    }

    pos.setY(Mth::absFloor(mY + (double)getEyeHeight()));

    return mLevel->getBrightness(pos);
}

void Entity::setLevel(Level* level)
{
    mLevel = level;
}

void Entity::absMoveTo(double x, double y, double z, float yRot, float xRot)
{
    mLevel->clampWithinLevelBounds(x, z, 0.0);

    mXo = x;
    mX = x;
    mYo = y;
    mZo = z;
    mY = y;
    mZ = z;

    float wrapped = Mth::wrapDegrees(xRot);

    mYRot = yRot;
    mXRot = wrapped;
    mXRotO = wrapped;
    mYRotO = yRot;

    double delta = (double)(mYRot - yRot);

    if (delta < -180.0)
    {
        mYRotO += 360.0f;
    }

    if (delta >= 180.0)
    {
        mYRotO -= 360.0f;
    }

    setPos(mX, mY, mZ);
    setRot(yRot, wrapped);
}

void Entity::absMoveTo(double x, double y, double z)
{
    mLevel->clampWithinLevelBounds(x, z, 0.0);

    mX = x;
    mXo = x;
    mZo = z;
    mYo = y;
    mZ = z;
    mY = y;

    setPos(x, y, z);
}

void Entity::moveTo(const BlockPos& pos, float yRot, float xRot)
{
    moveTo((double)pos.x, (double)pos.y, (double)pos.z + 0.5, yRot, xRot);
}

void Entity::moveTo(double x, double y, double z, float yRot, float xRot)
{
    mXOld = x;
    mYOld = y;
    mZOld = z;
    mXo = x;
    mYo = y;
    mZo = z;
    mX = x;
    mY = y;
    mZ = z;
    mYRot = yRot;
    mXRot = xRot;

    setPos(mX, mY, mZ);
}

double Entity::distanceTo(boost::shared_ptr<Entity> entity) const
{
    float dx = (float)(mX - entity->mX);
    float dy = (float)(mY - entity->mY);
    float dz = (float)(mZ - entity->mZ);

    return (double)sqrtf(dx * dx + dy * dy + dz * dz);
}

double Entity::distanceToSqr(const BlockPos& pos) const
{
    return pos.distSqr(mX, mY, mZ);
}

double Entity::distanceAsIfHitscanned(const BlockPos& pos, const AABB& box) const
{
    Vec3* center = Vec3::newTemp((double)pos.x + 0.5, (double)pos.y + 0.5, (double)pos.z + 0.5);

    double distanceSqr = center->distanceToSqr(box.getCenter());
    double radius = (box.mMaxX - box.mMinX) * 0.5 + (box.mMaxY - box.mMinY) * 0.5 + 1.8;

    return distanceSqr - radius * radius;
}

double Entity::distanceSqrToBlockPosCenter(const BlockPos& pos) const
{
    return pos.distSqrToCenter(mX, mY, mZ);
}

double Entity::distanceTo(double x, double y, double z) const
{
    double dx = mX - x;
    double dy = mY - y;
    double dz = mZ - z;

    return sqrt(dx * dx + dy * dy + dz * dz);
}

double Entity::distanceToSqr(boost::shared_ptr<Entity> entity) const
{
    double dx = mX - entity->mX;
    double dy = mY - entity->mY;
    double dz = mZ - entity->mZ;

    return dx * dx + dy * dy + dz * dz;
}

double Entity::distanceToSqr(double x, double y, double z) const
{
    double dx = mX - x;
    double dy = mY - y;
    double dz = mZ - z;

    return dx * dx + dy * dy + dz * dz;
}

void Entity::playerTouch(boost::shared_ptr<Player> player)
{
}

void Entity::push(boost::shared_ptr<Entity> entity)
{
    if (isPassengerOfSameVehicle(entity))
    {
        return;
    }

    if (entity->mNoPhysics || mNoPhysics)
    {
        return;
    }

    double dx = entity->mX - mX;
    double dz = entity->mZ - mZ;
    double max = Mth::asbMax(dx, dz);

    if (max < 0.009999999776482582)
    {
        return;
    }

    double length = sqrt(max);
    double scale = 1.0 / length;

    if (scale > 1.0)
    {
        scale = 1.0;
    }

    double strength = (double)(1.0f - mPushthrough);

    dx = (dx / length) * scale * 0.05000000074505806 * strength;
    dz = (dz / length) * scale * 0.05000000074505806 * strength;

    if (!isVehicle())
    {
        push(-dx, 0.0, -dz);
    }

    if (!entity->isVehicle())
    {
        entity->push(dx, 0.0, dz);
    }
}

void Entity::push(double x, double y, double z)
{
    mXd += x;
    mYd += y;
    mZd += z;
    mHasImpulse = true;
}

int Entity::getFireTransitionStartTick() const
{
    return mFireTransitionStartTick;
}

void Entity::markHurt()
{
    mHurtMarked = true;
}

bool Entity::hurt(const DamageSource* source, float damage)
{
    if (!isInvulnerableTo(source) || source->isBypassInvul())
    {
        markHurt();
    }

    return false;
}

Vec3* Entity::calculateViewVector(float xRot, float yRot)
{
    float yaw = -(yRot * 0.01745329238474369f) - 3.1415927410125732f;
    float pitch = -(xRot * 0.01745329238474369f);

    float cosYaw = Mth::cos(yaw);
    float sinYaw = Mth::sin(yaw);
    float cosPitch = Mth::cos(pitch);
    float sinPitch = Mth::sin(pitch);

    return Vec3::newTemp((double)(sinYaw * -cosPitch), (double)sinPitch,
                         (double)(cosYaw * -cosPitch));
}

Vec3* Entity::getViewVector(float partialTicks)
{
    float yRot = mYRot;
    float xRot = mXRot;

    if (partialTicks != 1.0f)
    {
        xRot = (xRot - mXRotO) * partialTicks + mXRotO;
        yRot = (yRot - mYRotO) * partialTicks + mYRotO;
    }

    return calculateViewVector(xRot, yRot);
}

bool Entity::isPickable()
{
    return false;
}

bool Entity::isPushable(boost::shared_ptr<Entity> entity)
{
    return false;
}

void Entity::awardKillScore(boost::shared_ptr<Entity> killed, int score, const DamageSource* source)
{
}

bool Entity::shouldRender(double x, double y, double z)
{
    double dx = mX - x;
    double dy = mY - y;
    double dz = mZ - z;

    return shouldRender(dx * dx + dy * dy + dz * dz);
}

bool Entity::shouldRender(double distanceSqr)
{
    return shouldRenderAtSqrDistance(distanceSqr);
}

bool Entity::shouldRenderAtSqrDistance(double distanceSqr)
{
    double range = mBoundingBox->getSize() * 48.0 * sViewScale;

    return distanceSqr < range * range;
}

HitResult* Entity::pick(double distance, float partialTicks)
{
    Vec3* from = getEyePosition(partialTicks);
    Vec3* view = getViewVector(partialTicks);
    Vec3* to = Vec3::newTemp(from->x, from->y, from->z)
                   ->add(view->x * distance, view->y * distance, view->z * distance);

    boost::unordered_set<Block*>* ignored = NULL;

    if (Minecraft::InMiniGame(eMiniGameId_Tumble, true))
    {
        ignored = new boost::unordered_set<Block*>(11, boost::hash<Block*>(),
                                                   std::equal_to<Block*>(),
                                                   std::allocator<Block*>());
        ignored->insert(Blocks::snow_layer);
        ignored->insert(Blocks::wool);
    }

    HitResult* hit = mLevel->clip(from, to, false, false, true, false, ignored, false);

    delete ignored;

    return hit;
}

std::wstring Entity::getEncodeId()
{
    ResourceLocation key = EntityIO::getKey(mType);

    if (key.getPath().empty())
    {
        return std::wstring(L"");
    }

    return key.toString();
}

ListTag* Entity::newDoubleList(unsigned int count, double first, ...)
{
    ListTag* list = new ListTag(6);

    list->add(new DoubleTag(first));

    va_list args;
    va_start(args, first);

    for (unsigned int i = 1; i < count; i++)
    {
        list->add(new DoubleTag(va_arg(args, double)));
    }

    va_end(args);

    return list;
}

ListTag* Entity::newFloatList(unsigned int count, float first, float second)
{
    ListTag* list = new ListTag(5);

    list->add(new FloatTag(first));
    list->add(new FloatTag(second));

    return list;
}

int Entity::getAirSupply()
{
    return mEntityData->get(DATA_AIR_SUPPLY_ID);
}

unsigned int Entity::getMaxAirSupply()
{
    return 300;
}

void Entity::setAirSupply(int airSupply)
{
    mEntityData->set(DATA_AIR_SUPPLY_ID, airSupply, true);
}

bool Entity::getSharedFlag(int flag)
{
    if (!mEntityData)
    {
        return false;
    }

    return ((unsigned int)mEntityData->get(DATA_SHARED_FLAGS_ID) >> flag & 1) != 0;
}

void Entity::setSharedFlag(int flag, bool value)
{
    if (!mEntityData)
    {
        return;
    }

    int flags = mEntityData->get(DATA_SHARED_FLAGS_ID);

    if (value)
    {
        flags |= 1 << flag;
    }
    else
    {
        flags &= ~(1 << flag);
    }

    mEntityData->set(DATA_SHARED_FLAGS_ID, flags, true);
}

CompoundTag* Entity::saveWithoutId(CompoundTag* tag)
{
    tag->put(std::wstring(L"Pos"), newDoubleList(3, mX, mY, mZ));
    tag->put(std::wstring(L"Motion"), newDoubleList(3, mXd, mYd, mZd));
    tag->put(std::wstring(L"Rotation"), newFloatList(2, mYRot, mXRot));
    tag->putFloat(std::wstring(L"FallDistance"), mFallDistance);
    tag->putShort(std::wstring(L"Fire"), (short)mRemainingFireTicks);
    tag->putShort(std::wstring(L"Air"), (short)getAirSupply());
    tag->putBoolean(std::wstring(L"OnGround"), mOnGround);
    tag->putInt(std::wstring(L"Dimension"), mDimension);
    tag->putBoolean(std::wstring(L"Invulnerable"), getSharedFlag(2));
    tag->putInt(std::wstring(L"PortalCooldown"), mPortalCooldown);
    tag->putString(std::wstring(L"UUID"), getUUID());

    if (!getCustomName().empty())
    {
        tag->putString(std::wstring(L"CustomName"), getCustomName());
        tag->putBoolean(std::wstring(L"namedByRestrictedPlayer"),
                        mEntityData->get(DATA_NAMED_BY_RESTRICTED_PLAYER));
        tag->putString(std::wstring(L"ownerUUID"), mEntityData->get(DATA_OWNER_UUID));
        tag->putBoolean(std::wstring(L"createdOnHost"), mEntityData->get(DATA_CREATED_ON_HOST));
    }

    if (isCustomNameVisible())
    {
        tag->putBoolean(std::wstring(L"CustomNameVisible"), isCustomNameVisible());
    }

    if (isSilent())
    {
        tag->putBoolean(std::wstring(L"Silent"), isSilent());
    }

    if (isNoGravity())
    {
        tag->putBoolean(std::wstring(L"NoGravity"), isNoGravity());
    }

    if (mGlowing)
    {
        tag->putBoolean(std::wstring(L"Glowing"), mGlowing);
    }

    if (!mTags.empty())
    {
        ListTag* tags = new ListTag(8);

        for (std::set<std::wstring>::const_iterator it = mTags.begin(); it != mTags.end(); ++it)
        {
            tags->add(new StringTag(*it));
        }

        tag->put(std::wstring(L"Tags"), tags);
    }

    addAdditonalSaveData(tag);

    if (isVehicle())
    {
        ListTag* passengers = new ListTag(10);

        std::vector<boost::weak_ptr<Entity> > riders = *getPassengers();

        for (std::vector<boost::weak_ptr<Entity> >::iterator it = riders.begin();
             it != riders.end(); ++it)
        {
            boost::shared_ptr<Entity> passenger = it->lock();

            CompoundTag* passengerTag = new CompoundTag();

            if (passenger && passenger->saveAsPassenger(passengerTag))
            {
                passengers->add(passengerTag);
            }
            else
            {
                delete passengerTag;
            }
        }

        if (passengers->isEmpty())
        {
            delete passengers;
        }
        else
        {
            tag->put(std::wstring(L"Passengers"), passengers);
        }
    }

    return tag;
}

bool Entity::saveAsPassenger(CompoundTag* tag)
{
    std::wstring id = getEncodeId();

    if (mRemoved || id.empty())
    {
        return false;
    }

    tag->putString(std::wstring(L"id"), id);
    saveWithoutId(tag);

    return true;
}

bool Entity::save(CompoundTag* tag)
{
    std::wstring id = getEncodeId();

    if (mRemoved || id.empty() || isPassenger())
    {
        return false;
    }

    tag->putString(std::wstring(L"id"), id);
    saveWithoutId(tag);

    return true;
}

void Entity::addDataWalkers(DataFixerUpper* dataFixerUpper)
{
    dataFixerUpper->addWalker(DataFixTypes::eDataFixTypes_Entity, new EntityDataWalker());
}

void Entity::load(CompoundTag* tag)
{
    ListTag* pos = tag->getList(std::wstring(L"Pos"), 6);
    ListTag* motion = tag->getList(std::wstring(L"Motion"), 6);
    ListTag* rotation = tag->getList(std::wstring(L"Rotation"), 5);

    mXd = motion->getDouble(0);
    mYd = motion->getDouble(1);
    mZd = motion->getDouble(2);

    if (fabs(mXd) > 10.0)
    {
        mXd = 0.0;
    }

    if (fabs(mYd) > 10.0)
    {
        mYd = 0.0;
    }

    if (fabs(mZd) > 10.0)
    {
        mZd = 0.0;
    }

    mX = pos->getDouble(0);
    mY = pos->getDouble(1);
    mZ = pos->getDouble(2);
    mXOld = mX;
    mYOld = mY;
    mZOld = mZ;
    mXo = mX;
    mYo = mY;
    mZo = mZ;

    mYRot = rotation->getFloat(0);
    mXRot = rotation->getFloat(1);
    mYRotO = mYRot;
    mXRotO = mXRot;

    setYHeadRot(mYRot);
    setYBodyRot(mYRot);

    mFallDistance = tag->getFloat(std::wstring(L"FallDistance"));
    mRemainingFireTicks = tag->getShort(std::wstring(L"Fire"));

    setAirSupply(tag->getShort(std::wstring(L"Air")));

    mOnGround = tag->getBoolean(std::wstring(L"OnGround"));

    if (tag->contains(std::wstring(L"Dimension")))
    {
        mDimension = tag->getInt(std::wstring(L"Dimension"));
    }

    setInvulnerable(tag->getBoolean(std::wstring(L"Invulnerable")));

    mPortalCooldown = tag->getInt(std::wstring(L"PortalCooldown"));

    if (tag->contains(std::wstring(L"UUID")))
    {
        mUUID = tag->getString(std::wstring(L"UUID"));
    }

    setPos(mX, mY, mZ);
    setRot(mYRot, mXRot);

    if (tag->contains(std::wstring(L"CustomName"), 8)
        && !tag->getString(std::wstring(L"CustomName")).empty())
    {
        setCustomName(tag->getString(std::wstring(L"CustomName")),
                      tag->getString(std::wstring(L"ownerUUID")),
                      tag->getBoolean(std::wstring(L"namedByRestrictedPlayer")));

        mEntityData->set(DATA_CREATED_ON_HOST, tag->getBoolean(std::wstring(L"createdOnHost")), true);
    }

    setCustomNameVisible(tag->getBoolean(std::wstring(L"CustomNameVisible")));
    setSilent(tag->getBoolean(std::wstring(L"Silent")));
    setNoGravity(tag->getBoolean(std::wstring(L"NoGravity")));
    setGlowing(tag->getBoolean(std::wstring(L"Glowing")));

    if (tag->contains(std::wstring(L"Tags"), 9))
    {
        mTags.clear();

        ListTag* tags = tag->getList(std::wstring(L"Tags"), 8);

        int count = tags->size();

        if (count > 1024)
        {
            count = 1024;
        }

        for (int i = 0; i < count; i++)
        {
            mTags.insert(tags->getString(i));
        }
    }

    readAdditionalSaveData(tag);

    if (repositionEntityAfterLoad())
    {
        setPos(mX, mY, mZ);
    }
}

bool Entity::shouldTick()
{
    if (mLevel->getNearestPlayerPredicate(mX, mY, mZ, 128.0, NULL))
    {
        return true;
    }

    mLastSentXRot++;

    return mLastSentXRot % 3 == 0;
}

bool Entity::repositionEntityAfterLoad()
{
    return true;
}

void Entity::spawnAtLocation(Item* item, int count)
{
    spawnAtLocation(item, count, 0.0f);
}

boost::shared_ptr<ItemEntity> Entity::spawnAtLocation(Item* item, int count, float yOffset)
{
    return spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(item, count, 0)), yOffset);
}

boost::shared_ptr<ItemEntity> Entity::spawnAtLocation(not_null_ptr<ItemInstance> itemInstance,
                                                      float yOffset)
{
    if (itemInstance->isEmpty())
    {
        return boost::shared_ptr<ItemEntity>();
    }

    boost::shared_ptr<ItemEntity> itemEntity(
        new ItemEntity(mLevel, mX, mY + (double)yOffset, mZ, itemInstance));

    itemEntity->setDefaultPickUpDelay();

    mLevel->addEntity(itemEntity);

    return itemEntity;
}

bool Entity::isAlive()
{
    return !mRemoved;
}

bool Entity::isInWall()
{
    if (mNoPhysics)
    {
        return false;
    }

    if (!mTickCache.mCacheChanged && mInWallCacheX == mX && mInWallCacheY == mY
        && mInWallCacheZ == mZ && mInWallCacheYRot == mYRot && mInWallCacheXRot == mXRot)
    {
        return mInWallCacheResult;
    }

    mInWallCacheX = mX;
    mInWallCacheY = mY;
    mInWallCacheZ = mZ;
    mInWallCacheYRot = mYRot;
    mInWallCacheXRot = mXRot;

    float width = mBbWidth * 0.8f;
    double eyeHeight = getEyeHeight();

    BlockPos lastPos((int)0x80000000, (int)0x80000000, (int)0x80000000);

    bool inMiniGame = Minecraft::InMiniGame(eMiniGameId_Glide, true);

    for (int i = 0; i < 8; i++)
    {
        int x = Mth::absFloor(mX + (double)(((float)(i % 2) - 0.5f) * width));
        int y = Mth::absFloor(mY + (double)(((float)((i / 4) % 2) - 0.5f) * 0.10000000149011612f)
                           + (double)eyeHeight);
        int z = Mth::absFloor(mZ + (double)(((float)((i / 2) % 2) - 0.5f) * width));

        BlockPos pos(x, y, z);

        if (lastPos.x == x && lastPos.y == y && lastPos.z == z)
        {
            continue;
        }

        lastPos = pos;

        Block* block = mLevel->getBlockState(mTickCache, pos)->getBlock();

        if (block->isViewBlocking(mLevel->getBlockState(mTickCache, pos))
            && (!inMiniGame || g_pApp->ShouldCollideWith(block)))
        {
            mInWallCacheResult = true;
            return true;
        }
    }

    mInWallCacheResult = false;

    return false;
}

bool Entity::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    return false;
}

AABB* Entity::getCollideAgainstBox(boost::shared_ptr<Entity> entity)
{
    return NULL;
}

void Entity::rideTick()
{
    boost::shared_ptr<Entity> vehicle = getVehicle();

    if (isPassenger() && vehicle->mRemoved)
    {
        stopRiding();
        return;
    }

    mXd = 0.0;
    mYd = 0.0;
    mZd = 0.0;

    tick();

    if (!isPassenger())
    {
        return;
    }

    vehicle->positionRider(shared_from_this());

    if (vehicle->GetType() != ENTITY_TYPE_MINECART_RIDEABLE)
    {
        return;
    }

    mVehicleYRotDelta += (double)(vehicle->mYRot - vehicle->mYRotO);
    mVehicleXRotDelta += (double)(vehicle->mXRot - vehicle->mXRotO);

    while (mVehicleYRotDelta >= 180.0)
    {
        mVehicleYRotDelta -= 360.0;
    }

    while (mVehicleYRotDelta < -180.0)
    {
        mVehicleYRotDelta += 360.0;
    }

    while (mVehicleXRotDelta >= 180.0)
    {
        mVehicleXRotDelta -= 360.0;
    }

    while (mVehicleXRotDelta < -180.0)
    {
        mVehicleXRotDelta += 360.0;
    }

    double yRotStep = mVehicleYRotDelta * 0.5;
    double xRotStep = mVehicleXRotDelta * 0.5;

    if (yRotStep > 10.0)
    {
        yRotStep = 10.0;
    }

    if (yRotStep < -10.0)
    {
        yRotStep = -10.0;
    }

    if (xRotStep > 10.0)
    {
        xRotStep = 10.0;
    }

    if (xRotStep < -10.0)
    {
        xRotStep = -10.0;
    }

    mVehicleYRotDelta -= yRotStep;
    mVehicleXRotDelta -= xRotStep;

    mYRot += (float)yRotStep;
    mXRot += (float)xRotStep;
}

void Entity::positionRider(const boost::shared_ptr<Entity>& passenger)
{
    if (!hasPassenger(passenger))
    {
        return;
    }

    if (RiderPositionOverrides::OverridePosition(shared_from_this(), passenger))
    {
        return;
    }

    passenger->setPos(mX, mY + getRidingHeight() + passenger->getRideHeight(), mZ);
}

void Entity::onPassengerTurned(boost::shared_ptr<Entity> passenger)
{
}

double Entity::getRidingHeight()
{
    return 0.0;
}

double Entity::getRideHeight()
{
    return (double)mBbHeight * 0.75;
}

bool Entity::showVehicleHealth()
{
    return (GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY;
}

bool Entity::startRiding(boost::shared_ptr<Entity> vehicle)
{
    return startRiding(vehicle, false);
}

bool Entity::startRiding(boost::shared_ptr<Entity> vehicle, bool force)
{
    for (boost::shared_ptr<Entity> root = vehicle; root->mVehicle; root = root->mVehicle)
    {
        if (root->mVehicle == shared_from_this())
        {
            return false;
        }
    }

    if (!force && (!canRide(vehicle) || !vehicle->canAddPassenger(shared_from_this())))
    {
        return false;
    }

    mVehicleXRotDelta = 0.0;
    mVehicleYRotDelta = 0.0;

    if (mVehicle)
    {
        stopRiding();
    }

    mVehicle = vehicle;
    mVehicle->addPassenger(shared_from_this());

    return true;
}

unsigned int Entity::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_pItemInstance
        && Item::getId(tooltipData.m_pItemInstance->getItem()) == 421
        && !tooltipData.m_pItemInstance->hasCustomHoverName()
        && tooltipData.m_pItemInstance->getHoverName() != getCustomName())
    {
        return 0x5592C692;
    }

    return 0xFFFFFFFF;
}

unsigned int Entity::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bAllowedToHurt)
    {
        return 0xFFFFFFFF;
    }

    return 0xFDEF7118;
}

bool Entity::HideUseTooltips()
{
    return false;
}

bool Entity::canRide(boost::shared_ptr<Entity> vehicle)
{
    return mBoardingCooldown <= 0;
}

void Entity::ejectPassengers()
{
    for (int i = (int)mPassengers.size() - 1; i >= 0; i--)
    {
        boost::shared_ptr<Entity> passenger = mPassengers.at(i).lock();

        if (passenger)
        {
            passenger->stopRiding();
        }
    }
}

void Entity::stopRiding()
{
    if (!mVehicle)
    {
        return;
    }

    boost::shared_ptr<Entity> vehicle = mVehicle;

    mVehicle.reset();

    vehicle->removePassenger(shared_from_this());
}

void Entity::fjImmediateBroadcastPassengerPacket(const boost::shared_ptr<Entity>& vehicle)
{
    if (!vehicle)
    {
        return;
    }

    boost::shared_ptr<ClientboundSetPassengersPacket> packet(
        new ClientboundSetPassengersPacket(vehicle));

    boost::shared_ptr<TrackedEntity> tracked =
        dynamic_cast<ServerLevel*>(mLevel)->getTracker()->getTracker(vehicle);

    if (tracked)
    {
        tracked->broadcast(packet);
    }
}

void Entity::stopRidingBeforeTeleport()
{
    boost::shared_ptr<Entity> vehicle = getVehicle();

    stopRiding();

    fjImmediateBroadcastPassengerPacket(vehicle);
}

void Entity::addPassenger(boost::shared_ptr<Entity> passenger)
{
    if (passenger->mVehicle.get() != this)
    {
        return;
    }

    bool front = false;

    if (!mLevel->mIsClientSide && (passenger->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        boost::shared_ptr<Entity> controller = getControllingPassenger();

        if (!controller || (controller->GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER)
        {
            front = true;
        }
    }

    if (front)
    {
        mPassengers.insert(mPassengers.begin(), boost::weak_ptr<Entity>(passenger));
    }
    else
    {
        mPassengers.push_back(boost::weak_ptr<Entity>(passenger));
    }
}

void Entity::removePassenger(boost::shared_ptr<Entity> passenger)
{
    if (passenger->getVehicle() == shared_from_this())
    {
        return;
    }

    for (std::vector<boost::weak_ptr<Entity> >::iterator it = mPassengers.begin();
         it != mPassengers.end(); ++it)
    {
        if (it->lock() == passenger)
        {
            mPassengers.erase(it);
            break;
        }
    }

    passenger->mBoardingCooldown = 60;
}

bool Entity::canAddPassenger(boost::shared_ptr<Entity> passenger)
{
    return mPassengers.empty();
}

void Entity::lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport)
{
    setPos(x, y, z);
    setRot(yRot, xRot);
}

double Entity::getPickRadius()
{
    return 0.0;
}

Vec3* Entity::getLookAngle()
{
    return calculateViewVector(mXRot, mYRot);
}

phys::Vec2* Entity::getRotationVector()
{
    return phys::Vec2::newTemp(mXRot, mYRot);
}

Vec3* Entity::getForward()
{
    return Vec3::directionFromRotation(getRotationVector());
}

void Entity::handleInsidePortal(const BlockPos& pos)
{
    if (mPortalCooldown > 0)
    {
        mPortalCooldown = getDimensionChangingDelay();
    }
    else
    {
        if (!mLevel->mIsClientSide && !(pos == mPortalEntrancePos))
        {
            mPortalEntrancePos = pos;

            BlockPattern::BlockPatternMatch* portal =
                PortalBlock::getPortalShape(mLevel, mPortalEntrancePos);

            double axis = portal->getForwards()->getAxis() == Direction::Axis::X
                              ? (double)portal->getFrontTopLeft().z
                              : (double)portal->getFrontTopLeft().x;

            double across = portal->getForwards()->getAxisDirection() == Direction::AxisDirection::NEGATIVE
                                ? Mth::pct(mZ, axis, axis - (double)portal->getWidth())
                                : Mth::pct(mX, axis, axis - (double)portal->getWidth());

            double up = Mth::pct(mY, (double)portal->getFrontTopLeft().y,
                                 (double)(portal->getFrontTopLeft().y - portal->getHeight()));

            mPortalEntranceOffset = Vec3::newPermanent(across, up, 0.0);
            mPortalEntranceForwards = portal->getForwards();

            delete portal;
        }

        mIsInsidePortal = true;
    }
}

int Entity::getDimensionChangingDelay()
{
    return 300;
}

void Entity::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
}

void Entity::handleEntityEvent(unsigned char event, int data)
{
    if (event == 39)
    {
        spawnEatParticles(ItemInstance(Item::byId(data), 1, 0), 5);
    }
}

void Entity::animateHurt()
{
}

std::vector<boost::shared_ptr<ItemInstance> >* Entity::getHandSlots()
{
    return new std::vector<boost::shared_ptr<ItemInstance> >();
}

std::vector<boost::shared_ptr<ItemInstance> >* Entity::getArmorSlots()
{
    return new std::vector<boost::shared_ptr<ItemInstance> >();
}

std::vector<boost::shared_ptr<ItemInstance> >* Entity::getAllSlots()
{
    std::vector<boost::shared_ptr<ItemInstance> >* hands = getHandSlots();
    std::vector<boost::shared_ptr<ItemInstance> >* armor = getArmorSlots();

    if (mAllSlotCount == 0)
    {
        delete[] mAllSlots;

        mAllSlotCount = (int)(hands->size() + armor->size());
        mAllSlots = new not_null_ptr<ItemInstance>[mAllSlotCount];
    }

    std::vector<boost::shared_ptr<ItemInstance> >* all =
        new std::vector<boost::shared_ptr<ItemInstance> >();

    for (std::vector<boost::shared_ptr<ItemInstance> >::iterator it = hands->begin();
         it != hands->end(); ++it)
    {
        all->push_back(*it);
    }

    for (std::vector<boost::shared_ptr<ItemInstance> >::iterator it = armor->begin();
         it != armor->end(); ++it)
    {
        all->push_back(*it);
    }

    return all;
}

void Entity::setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> itemInstance)
{
}

bool Entity::isOnFire()
{
    bool clientSide = mLevel != NULL && mLevel->mIsClientSide;

    if (mFireImmune)
    {
        return false;
    }

    return mRemainingFireTicks > 0 || (clientSide && getSharedFlag(0));
}

bool Entity::isPassenger()
{
    return mVehicle.get() != NULL;
}

bool Entity::isVehicle()
{
    return !mPassengers.empty();
}

bool Entity::isSneaking()
{
    return getSharedFlag(1);
}

void Entity::setSneaking(bool sneaking)
{
    setSharedFlag(1, sneaking);
}

bool Entity::isIdle()
{
    return getSharedFlag(6);
}

void Entity::setIsIdle(bool idle)
{
    setSharedFlag(6, idle);
}

bool Entity::isSprinting()
{
    return getSharedFlag(3);
}

void Entity::setSprinting(bool sprinting)
{
    setSharedFlag(3, sprinting);
}

bool Entity::isSwimming()
{
    return getSharedFlag(11);
}

bool Entity::isSwimmer()
{
    return getSharedFlag(10);
}

void Entity::startSwimming()
{
    setSharedFlag(11, true);
}

void Entity::stopSwimming()
{
    setSharedFlag(11, false);
}

void Entity::setSwimmer(bool swimmer)
{
    setSharedFlag(10, swimmer);
}

float Entity::getSwimAmount(float partialTicks) const
{
    return Mth::Lerp(mSwimAmountO, mSwimAmount, partialTicks);
}

void Entity::setDamageNearbyMobs(bool damageNearbyMobs)
{
    mAutoSpinAttackTicks = 0;

    setSharedFlag(12, damageNearbyMobs);
}

bool Entity::getDamageNearbyMobs()
{
    return getSharedFlag(12);
}

bool Entity::isGlowing()
{
    return mGlowing || (mLevel->mIsClientSide && getSharedFlag(8));
}

void Entity::setGlowing(bool glowing)
{
    mGlowing = glowing;

    if (mLevel->mIsClientSide)
    {
        return;
    }

    setSharedFlag(8, glowing);
}

bool Entity::isInvisible()
{
    return getSharedFlag(5);
}

bool Entity::isInvisibleTo(boost::shared_ptr<Player> player)
{
    if (player->isSpectator())
    {
        return false;
    }

    Team* team = getTeam();

    if (team != NULL && player->getTeam() == team && team->canSeeFriendlyInvisibles())
    {
        return false;
    }

    return isInvisible();
}

Team* Entity::getTeam()
{
    if ((GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER)
    {
        return NULL;
    }

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(shared_from_this());

    return mLevel->GetScoreboard()->getPlayersTeam(player->m_0x784);
}

bool Entity::isAlliedTo(boost::shared_ptr<Entity> entity)
{
    return isAlliedTo(entity->getTeam());
}

bool Entity::isAlliedTo(Team* team)
{
    Team* ownTeam = getTeam();

    if (ownTeam == NULL)
    {
        return false;
    }

    return ownTeam->isAlliedTo(team);
}

void Entity::setInvisible(bool invisible)
{
    setSharedFlag(5, invisible);
}

bool Entity::ShouldRenderShadow()
{
    return true;
}

bool Entity::canFly()
{
    return false;
}

Vec3* Entity::getPos(float partialTicks)
{
    if (partialTicks >= 1.0f)
    {
        return Vec3::newTemp(mX, mY, mZ);
    }

    return Vec3::newTemp((mX - mXo) * (double)partialTicks + mXo,
                         (mY - mYo) * (double)partialTicks + mYo,
                         (mZ - mZo) * (double)partialTicks + mZo);
}

Vec3* Entity::getAttachPos(EEntityLocation location, float partialTicks)
{
    switch (location)
    {
        case ENTITY_LOCATION_MIDDLE:
        case ENTITY_LOCATION_CENTER:
            return mBoundingBox->getCenter();

        case ENTITY_LOCATION_MOUNT:
        case ENTITY_LOCATION_LEASH:
        case ENTITY_LOCATION_SADDLE:
        case ENTITY_LOCATION_CHEST_LEFT:
        case ENTITY_LOCATION_CHEST_RIGHT:
            return getPos(partialTicks)->add(
                Vec3::newTemp(0.0, (double)(mBbHeight * 0.5f), 0.0)
                    ->yRot(-(mYRot * 0.01745329238474369f)));

        default:
            return getPos(partialTicks);
    }
}

bool Entity::isWeakened()
{
    return getSharedFlag(7);
}

void Entity::setWeakened(bool weakened)
{
    setSharedFlag(7, weakened);
}

void Entity::thunderHit(const LightningBolt* lightningBolt)
{
    hurt(DamageSource::LIGHTNING_BOLT, 5.0f);

    mRemainingFireTicks++;

    if (mRemainingFireTicks == 0)
    {
        setSecondsOnFire(8);
    }
}

void Entity::onAboveBubbleCol(bool down)
{
    if (down)
    {
        mYd = mYd - 0.03 < -0.9 ? -0.9 : mYd - 0.03;
    }
    else
    {
        mYd = mYd + 0.1 > 1.8 ? 1.8 : mYd + 0.1;
    }
}

void Entity::onInsideBubbleColumn(bool down)
{
    if (down)
    {
        mYd = mYd - 0.03 < -0.3 ? -0.3 : mYd - 0.03;
    }
    else
    {
        mYd = mYd + 0.1 > 0.4 ? 0.4 : mYd + 0.1;
    }

    mFallDistance = 0.0f;
}

void Entity::killed(boost::shared_ptr<LivingEntity> killed)
{
}

bool Entity::checkInBlock(double x, double y, double z)
{
    BlockPos pos(x, y, z);

    double fx = x - (double)pos.x;
    double fy = y - (double)pos.y;
    double fz = z - (double)pos.z;

    if (!mLevel->hasBlockCubes(mBoundingBox))
    {
        return false;
    }

    const Direction* pushDirection = NULL;
    double best = 1.7976931348623157e+308;

    if (!mLevel->isFullAABBBlock(pos.west()) && fx < best)
    {
        best = fx;
        pushDirection = Direction::WEST;
    }

    if (!mLevel->isFullAABBBlock(pos.east()) && 1.0 - fx < best)
    {
        best = 1.0 - fx;
        pushDirection = Direction::EAST;
    }

    if (!mLevel->isFullAABBBlock(pos.north()) && fz < best)
    {
        best = fz;
        pushDirection = Direction::NORTH;
    }

    if (!mLevel->isFullAABBBlock(pos.south()) && 1.0 - fz < best)
    {
        best = 1.0 - fz;
        pushDirection = Direction::SOUTH;
    }

    if (!mLevel->isFullAABBBlock(pos.above()) && 1.0 - fy < best)
    {
        best = 1.0 - fy;
        pushDirection = Direction::UP;
    }

    if (pushDirection == NULL)
    {
        return false;
    }

    float speed = mRandom->nextFloat() * 0.20000000298023224f + 0.10000000149011612f;
    float step = (float)pushDirection->getAxisDirection()->getStep();

    if (pushDirection->getAxis() == Direction::Axis::X)
    {
        mXd = (double)(step * speed);
        mYd *= 0.75;
        mZd *= 0.75;
    }
    else if (pushDirection->getAxis() == Direction::Axis::Y)
    {
        mXd *= 0.75;
        mYd = (double)(step * speed);
        mZd *= 0.75;
    }
    else
    {
        mXd *= 0.75;
        mYd *= 0.75;
        mZd = (double)(step * speed);
    }

    return true;
}

void Entity::spawnEatParticles(const ItemInstance& itemInstance, int count)
{
}

void Entity::makeStuckInWeb()
{
    mFallDistance = 0.0f;
    mFallTicks = 0;
    mIsInWeb = true;
    mFallCount++;
}

std::wstring Entity::getName()
{
    if (hasCustomName())
    {
        return getCustomName();
    }

    return std::wstring(L"");
}

bool Entity::isInvulnerableTo(const DamageSource* source)
{
    return getSharedFlag(2) && source != DamageSource::OUT_OF_WORLD
        && source != DamageSource::CREATIVE_PLAYER && !source->isCreativePlayer();
}

bool Entity::isInvulnerable()
{
    return getSharedFlag(2);
}

void Entity::setInvulnerable(bool invulnerable)
{
    setSharedFlag(2, invulnerable);
}

std::vector<boost::shared_ptr<Entity> >* Entity::getSubEntities()
{
    return NULL;
}

bool Entity::is(boost::shared_ptr<Entity> entity)
{
    return entity == shared_from_this();
}

float Entity::getYHeadRot()
{
    return 0.0f;
}

void Entity::setYHeadRot(float yHeadRot)
{
}

void Entity::setYBodyRot(float yBodyRot)
{
}

bool Entity::isAttackable()
{
    return true;
}

bool Entity::skipAttackInteraction(boost::shared_ptr<Entity> entity)
{
    return false;
}

void Entity::copyPosition(boost::shared_ptr<Entity> entity)
{
    moveTo(entity->mX, entity->mY, entity->mZ, entity->mYRot, entity->mXRot);
}

void Entity::restoreFrom(boost::shared_ptr<Entity> entity)
{
    CompoundTag* tag = new CompoundTag();

    entity->saveWithoutId(tag);
    tag->remove(std::wstring(L"Dimension"));

    load(tag);

    delete tag;

    mPortalCooldown = entity->mPortalCooldown;
    mPortalEntrancePos = entity->mPortalEntrancePos;
    mPortalEntranceOffset = entity->mPortalEntranceOffset;
    mPortalEntranceForwards = entity->mPortalEntranceForwards;
}

boost::shared_ptr<Entity> Entity::changeDimension(int dimension)
{
    if (mLevel->mIsClientSide || mRemoved)
    {
        return boost::shared_ptr<Entity>();
    }

    bool dropsLeash = false;

    if ((GetType() & ENTITY_TYPE_MOB) == ENTITY_TYPE_MOB && isLeashed())
    {
        dropsLeash = true;
        dropLeash(true, true);
    }

    if (GetType() == ENTITY_TYPE_LEASH_FENCE_KNOT)
    {
        mRemoved = true;
        return boost::shared_ptr<Entity>();
    }

    ServerLevel* fromLevel = dynamic_cast<ServerLevel*>(mLevel);

    if (fromLevel->atEntityLimit(shared_from_this()))
    {
        return boost::shared_ptr<Entity>();
    }

    if ((GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER
        && !mLevel->canCreateMore(mType, 2))
    {
        return boost::shared_ptr<Entity>();
    }

    ServerLevel* toLevel = dynamic_cast<ServerLevel*>(mLevel->getServer()->getLevel(dimension));

    mDimension = toLevel->mDimension->getType()->getId();

    stopRidingBeforeTeleport();

    mRemoved = false;

    BlockPos spawnPos;

    if (dimension == 1)
    {
        if (dropsLeash)
        {
            BlockPos* dimensionSpawn = toLevel->getDimensionSpecificSpawn();

            if (dimensionSpawn != NULL)
            {
                spawnPos = *dimensionSpawn;
            }
            else
            {
                spawnPos = toLevel->getSharedSpawnPos();
            }
        }
        else
        {
            spawnPos = toLevel->getSharedSpawnPos();
        }
    }
    else
    {
        double scale = (double)toLevel->getLevelData()->getHellScale();

        double x = mX;
        double z = mZ;

        if (dimension == -1)
        {
            x /= scale;
            z /= scale;

            x = Mth::clamp(x, toLevel->getWorldBorder()->getMinX() + 16.0,
                           toLevel->getWorldBorder()->getMaxX() - 16.0);
            z = Mth::clamp(z, toLevel->getWorldBorder()->getMinZ() + 16.0,
                           toLevel->getWorldBorder()->getMaxZ() - 16.0);
        }
        else if (dimension == 0)
        {
            x *= scale;
            z *= scale;

            x = Mth::clamp(x, toLevel->getWorldBorder()->getMinX() + 16.0,
                           toLevel->getWorldBorder()->getMaxX() - 16.0);
            z = Mth::clamp(z, toLevel->getWorldBorder()->getMinZ() + 16.0,
                           toLevel->getWorldBorder()->getMaxZ() - 16.0);
        }

        mLevel->clampWithinLevelBounds(x, z, 0.0);

        moveTo(x, mY, z, 90.0f, 0.0f);

        toLevel->getPortalForcer()->findPortal(shared_from_this(), 0.0f, dimension, dimension);

        spawnPos = BlockPos(this);
    }

    fromLevel->removeEntity(shared_from_this());

    boost::shared_ptr<Entity> copy = EntityIO::newEntity(mType, toLevel);

    if (copy)
    {
        copy->restoreFrom(shared_from_this());

        if (dropsLeash && dimension == 1)
        {
            copy->moveTo(toLevel->getTopSolidBlockPos(toLevel->getSharedSpawnPos()), copy->mYRot,
                         copy->mXRot);
        }
        else
        {
            copy->moveTo(spawnPos, copy->mYRot, copy->mXRot);
        }

        bool forced = copy->mForcedLoading;

        copy->mForcedLoading = true;
        toLevel->addEntity(copy);
        copy->mForcedLoading = forced;

        toLevel->tick(copy, false);
    }

    mRemoved = true;

    fromLevel->resetEmptyTime();
    toLevel->resetEmptyTime();

    return copy;
}

bool Entity::canChangeDimensions()
{
    return true;
}

float Entity::getBlockExplosionResistance(Explosion* explosion, Level* level, const BlockPos& pos)
{
    return level->getBlock(pos)->getExplosionResistance(shared_from_this());
}

bool Entity::shouldBlockExplode(Explosion* explosion, Level* level, const BlockPos& pos, float power)
{
    return true;
}

int Entity::getMaxFallDistance()
{
    return 3;
}

BlockPos Entity::getPortalEntranceBlock()
{
    return mPortalEntrancePos;
}

Vec3* Entity::getPortalEntranceOffset()
{
    return mPortalEntranceOffset;
}

const Direction* Entity::getPortalEntranceForwards()
{
    return mPortalEntranceForwards;
}

bool Entity::isIgnoringBlockTriggers()
{
    return false;
}

bool Entity::displayFireAnimation()
{
    return isOnFire();
}

void Entity::setUUID(const std::wstring& uuid)
{
    if (uuid.compare(0, uuid.size(), L"") == 0)
    {
        return;
    }

    mUUID = uuid;
    mStringUUID = uuid;
}

std::wstring Entity::getUUID()
{
    return mUUID;
}

std::wstring Entity::getStringUUID()
{
    return mStringUUID;
}

bool Entity::isPushedByWater()
{
    return true;
}

double Entity::getViewScale()
{
    return sViewScale;
}

void Entity::setViewScale(double viewScale)
{
    sViewScale = viewScale;
}

std::wstring Entity::getNetworkName()
{
    return getName();
}

void Entity::setCustomName(const std::wstring& customName, const std::wstring& ownerUUID,
                           bool namedByRestrictedPlayer)
{
    bool createdOnHost = CGameNetworkManager::sInstance.IsHost()
        && g_pApp->IsUUIDLocal(ownerUUID) != 0;

    mEntityData->set(DATA_CREATED_ON_HOST, createdOnHost, true);
    mEntityData->set(DATA_NAMED_BY_RESTRICTED_PLAYER, namedByRestrictedPlayer, true);
    mEntityData->set(DATA_CUSTOM_NAME, customName, true);
    mEntityData->set(DATA_OWNER_UUID, ownerUUID, true);
}

std::wstring Entity::getCustomName()
{
    return mEntityData->get(DATA_CUSTOM_NAME);
}

std::wstring Entity::getOwnerUUID()
{
    return mEntityData->get(DATA_OWNER_UUID);
}

bool Entity::wasNamedByRestrictedPlayer()
{
    return mEntityData->get(DATA_NAMED_BY_RESTRICTED_PLAYER);
}

bool Entity::wasNamedOnHost()
{
    return mEntityData->get(DATA_CREATED_ON_HOST);
}

bool Entity::hasCustomName()
{
    if (getCustomName().empty())
    {
        return false;
    }

    bool ownerIsLocal = g_pApp->IsUUIDLocal(getOwnerUUID());
    bool censored = g_pApp->IsConsoleChatCensored();

    if (!ownerIsLocal && mEntityData->get(DATA_NAMED_BY_RESTRICTED_PLAYER) && censored)
    {
        return false;
    }

    if (!mEntityData->get(DATA_CREATED_ON_HOST) && !CGameNetworkManager::sInstance.IsHost()
        && censored)
    {
        return false;
    }

    return true;
}

void Entity::setCustomNameVisible(bool customNameVisible)
{
    mEntityData->set(DATA_CUSTOM_NAME_VISIBLE, customNameVisible, true);
}

bool Entity::isCustomNameVisible()
{
    return mEntityData->get(DATA_CUSTOM_NAME_VISIBLE);
}

void Entity::teleportTo(double x, double y, double z)
{
    mTeleported = true;

    setPos(x, y, z);

    mLevel->ensureAdded(shared_from_this());
}

bool Entity::shouldShowName()
{
    return isCustomNameVisible();
}

void Entity::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
}

const Direction* Entity::getDirection()
{
    return Direction::from2DDataValue(Mth::absFloor((double)(mYRot / 360.0f * 4.0f) + 0.5) & 3);
}

const Direction* Entity::getMotionDirection()
{
    return getDirection();
}

bool Entity::broadcastToPlayer(boost::shared_ptr<ServerPlayer> player)
{
    return true;
}

AABB* Entity::getBoundingBoxForCulling()
{
    return mBoundingBox;
}

void Entity::setBoundingBox(AABB* boundingBox)
{
    mBoundingBox->set(boundingBox);
}

double Entity::getEyeHeight()
{
    return (double)(mBbHeight * 0.8500000238418579f);
}

void Entity::setOutsideWorldBorder(bool outsideWorldBorder)
{
    mOutsideWorldBorder = outsideWorldBorder;
}

bool Entity::setSlot(int slot, const not_null_ptr<ItemInstance>& itemInstance)
{
    return false;
}

bool Entity::interactAt(const boost::shared_ptr<Player>& player, Vec3* pos,
                        InteractionHand::EInteractionHand hand)
{
    return true;
}

bool Entity::ignoreExplosion()
{
    return false;
}

void Entity::doEnchantDamageEffects(boost::shared_ptr<LivingEntity> attacker,
                                    boost::shared_ptr<Entity> target)
{
    if ((target->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
    {
        EnchantmentHelper::doPostHurtEffects(
            boost::dynamic_pointer_cast<LivingEntity>(target),
            boost::static_pointer_cast<Entity>(attacker));
    }

    EnchantmentHelper::doPostDamageEffects(attacker, target);
}

void Entity::startSeenByPlayer(boost::shared_ptr<ServerPlayer> player)
{
}

void Entity::stopSeenByPlayer(boost::shared_ptr<ServerPlayer> player)
{
}

float Entity::rotate(Rotation* rotation)
{
    float yRot = Mth::wrapDegrees(mYRot);

    switch (rotation->getType())
    {
        case Rotation::eTYPE_Clockwise90:
            return yRot + 90.0f;

        case Rotation::eTYPE_Clockwise180:
            return yRot + 180.0f;

        case Rotation::eTYPE_CounterClockwise90:
            return yRot + 270.0f;

        default:
            return yRot;
    }
}

float Entity::mirror(Mirror* mirror)
{
    float yRot = Mth::wrapDegrees(mYRot);

    if (mirror->getType() == Mirror::eTYPE_LeftRight)
    {
        return -yRot;
    }

    if (mirror->getType() == Mirror::eTYPE_FrontBack)
    {
        return 180.0f - yRot;
    }

    return yRot;
}

bool Entity::onlyOpCanSetNbt()
{
    return false;
}

bool Entity::checkAndResetTeleportedFlag()
{
    bool teleported = mTeleported;

    mTeleported = false;

    return teleported;
}

boost::shared_ptr<Entity> Entity::getControllingPassenger()
{
    if (mPassengers.empty())
    {
        return boost::shared_ptr<Entity>();
    }

    return mPassengers.at(0).lock();
}

bool Entity::hasPassenger(boost::shared_ptr<Entity> passenger)
{
    for (std::vector<boost::weak_ptr<Entity> >::iterator it = mPassengers.begin();
         it != mPassengers.end(); ++it)
    {
        boost::shared_ptr<Entity> rider = it->lock();

        if (rider && rider == passenger)
        {
            return true;
        }
    }

    return false;
}

bool Entity::hasPassenger(unsigned long long type)
{
    for (std::vector<boost::weak_ptr<Entity> >::iterator it = mPassengers.begin();
         it != mPassengers.end(); ++it)
    {
        boost::shared_ptr<Entity> rider = it->lock();

        if (!rider)
        {
            continue;
        }

        unsigned long long riderType = rider->GetType();

        if ((type & 0x3FULL) == 0)
        {
            if ((riderType & type) == type)
            {
                return true;
            }
        }
        else if (riderType == type)
        {
            return true;
        }
    }

    return false;
}

void Entity::fillIndirectPassengers(unsigned long long type,
                                    std::set<boost::shared_ptr<Entity> >* passengers)
{
    for (std::vector<boost::weak_ptr<Entity> >::iterator it = mPassengers.begin();
         it != mPassengers.end(); ++it)
    {
        boost::shared_ptr<Entity> rider = it->lock();

        if (!rider)
        {
            continue;
        }

        unsigned long long riderType = rider->GetType();

        bool matches;

        if ((type & 0x3FULL) == 0)
        {
            matches = (riderType & type) == type;
        }
        else
        {
            matches = riderType == type;
        }

        if (matches)
        {
            passengers->insert(rider);
        }

        rider->fillIndirectPassengers(type, passengers);
    }
}

std::set<boost::shared_ptr<Entity> >* Entity::getIndirectPassengers(unsigned long long type)
{
    std::set<boost::shared_ptr<Entity> >* passengers = new std::set<boost::shared_ptr<Entity> >();

    fillIndirectPassengers(type, passengers);

    return passengers;
}

std::set<boost::shared_ptr<Entity> >* Entity::getIndirectPassengers()
{
    std::set<boost::shared_ptr<Entity> >* passengers = new std::set<boost::shared_ptr<Entity> >();

    fillIndirectPassengers(ENTITY_TYPE_ENTITY, passengers);

    return passengers;
}

boost::shared_ptr<Entity> Entity::getRootVehicle()
{
    boost::shared_ptr<Entity> root = shared_from_this();

    while (root->getVehicle())
    {
        root = root->getVehicle();
    }

    return root;
}

bool Entity::isPassengerOfSameVehicle(boost::shared_ptr<Entity> entity)
{
    return getRootVehicle() == entity->getRootVehicle();
}

bool Entity::hasIndirectPassenger(boost::shared_ptr<Entity> passenger)
{
    for (std::vector<boost::weak_ptr<Entity> >::iterator it = mPassengers.begin();
         it != mPassengers.end(); ++it)
    {
        boost::shared_ptr<Entity> rider = it->lock();

        if (!rider)
        {
            continue;
        }

        if (rider == passenger || rider->hasIndirectPassenger(passenger))
        {
            return true;
        }
    }

    return false;
}

bool Entity::hasIndirectPassenger(unsigned long long type)
{
    for (std::vector<boost::weak_ptr<Entity> >::iterator it = mPassengers.begin();
         it != mPassengers.end(); ++it)
    {
        boost::shared_ptr<Entity> rider = it->lock();

        if (!rider)
        {
            continue;
        }

        unsigned long long riderType = rider->GetType();

        if ((type & 0x3FULL) == 0)
        {
            if ((riderType & type) == type)
            {
                return true;
            }
        }
        else if (riderType == type)
        {
            return true;
        }
    }

    return false;
}

bool Entity::isControlledByLocalInstance()
{
    boost::shared_ptr<Entity> controller = getControllingPassenger();

    if (instanceof(controller, ENTITY_TYPE_PLAYER))
    {
        return boost::static_pointer_cast<Player>(controller)->isLocalPlayer();
    }

    return !mLevel->mIsClientSide;
}

boost::shared_ptr<Entity> Entity::getVehicle()
{
    return mVehicle;
}

int Entity::getPistonPushReaction()
{
    return 0;
}

SoundSource::ESoundSource Entity::getSoundSource()
{
    return SoundSource::eSoundSource_Neutral;
}

bool Entity::isCreative()
{
    return false;
}

bool Entity::isDespawnProtected()
{
    return false;
}

void Entity::setDespawnProtected()
{
}

bool Entity::couldWander()
{
    return false;
}

bool Entity::canCreateParticles()
{
    return true;
}

void Entity::stopCurrentLerp()
{
    mXo = mX;
    mYo = mY;
    mZo = mZ;
    mXOld = mX;
    mYOld = mY;
    mZOld = mZ;
    mYRotO = mYRot;
    mXRotO = mXRot;
}

bool Entity::controllingPassengerIsA(unsigned long long type)
{
    boost::shared_ptr<Entity> controller = getControllingPassenger();

    if (!controller)
    {
        return false;
    }

    unsigned long long controllerType = controller->GetType();

    if ((type & 0x3FULL) == 0)
    {
        return (controllerType & type) == type;
    }

    return controllerType == type;
}

void Entity::DoAutoAttackOnTouch(boost::shared_ptr<Entity> entity)
{
}

bool Entity::isImmersedInWater()
{
    Vec3* pos = getPos(0.0f);

    double half = mBoundingBox->getSize() * 0.5;

    Block* top = mLevel->getBlock(BlockPos(Vec3::newTemp(pos->x, pos->y + half, pos->z)));
    Block* bottom = mLevel->getBlock(BlockPos(Vec3::newTemp(pos->x, pos->y - half, pos->z)));

    return top->mMaterial != Material::AIR && bottom->mMaterial != Material::AIR;
}

void Entity::tickLeash()
{
    if (mLeashInfoTag != NULL)
    {
        restoreLeashFromSave();
    }

    if (!mLeashed)
    {
        return;
    }

    if (!isAlive())
    {
        dropLeash(true, true);
        return;
    }

    boost::shared_ptr<Entity> holder = getLeashHolder();

    if (!holder || holder->mRemoved)
    {
        dropLeash(true, true);
    }
}

void Entity::dropLeash(bool broadcast, bool dropItem)
{
    if (!mLeashed)
    {
        return;
    }

    mLeashed = false;
    mLeashHolder.reset();

    if (!mLevel->mIsClientSide && dropItem)
    {
        spawnAtLocation(Items::lead, 1);
    }

    if (!mLevel->mIsClientSide && broadcast
        && dynamic_cast<ServerLevel*>(mLevel) != NULL)
    {
        dynamic_cast<ServerLevel*>(mLevel)->getTracker()->broadcast(
            shared_from_this(),
            boost::shared_ptr<ClientboundSetEntityLinkPacket>(
                new ClientboundSetEntityLinkPacket(0, shared_from_this(),
                                                   boost::shared_ptr<Entity>())));
    }
}

bool Entity::canBeLeashed(const boost::shared_ptr<Player>& player)
{
    return false;
}

bool Entity::isLeashed()
{
    return mLeashed;
}

boost::shared_ptr<Entity> Entity::getLeashHolder()
{
    return mLeashHolder.lock();
}

bool Entity::isLeashedToPlayer(const boost::shared_ptr<Player>& player)
{
    return mLeashed && getLeashHolder() == player;
}

void Entity::setLeashedTo(boost::shared_ptr<Entity> leashHolder, bool broadcast)
{
    mLeashed = true;
    mLeashHolder = boost::weak_ptr<Entity>(leashHolder);

    ServerLevel* serverLevel = dynamic_cast<ServerLevel*>(mLevel);

    if (serverLevel != NULL && !mLevel->mIsClientSide && broadcast)
    {
        serverLevel->getTracker()->broadcast(
            shared_from_this(),
            boost::shared_ptr<ClientboundSetEntityLinkPacket>(
                new ClientboundSetEntityLinkPacket(0, shared_from_this(), leashHolder)));
    }

    if (isPassenger())
    {
        stopRiding();
    }
}

void Entity::restoreLeashFromSave()
{
    if (!mLeashed || mLeashInfoTag == NULL)
    {
        return;
    }

    if (mLeashInfoTag->contains(std::wstring(L"UUID")))
    {
        std::wstring uuid = mLeashInfoTag->getString(std::wstring(L"UUID"));

        std::vector<boost::shared_ptr<Entity> >* nearby = mLevel->getEntitiesOfClass(
            typeid(LivingEntity), mBoundingBox->grow(10.0, 10.0, 10.0));

        for (std::vector<boost::shared_ptr<Entity> >::iterator it = nearby->begin();
             it != nearby->end(); ++it)
        {
            boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(*it);

            if (living && living->getUUID() == uuid)
            {
                setLeashedTo(living, true);
                break;
            }
        }
    }
    else if (mLeashInfoTag->contains(std::wstring(L"X"), 99)
             && mLeashInfoTag->contains(std::wstring(L"Y"), 99)
             && mLeashInfoTag->contains(std::wstring(L"Z"), 99))
    {
        BlockPos pos(mLeashInfoTag->getInt(std::wstring(L"X")),
                     mLeashInfoTag->getInt(std::wstring(L"Y")),
                     mLeashInfoTag->getInt(std::wstring(L"Z")));

        boost::shared_ptr<LeashFenceKnotEntity> knot =
            LeashFenceKnotEntity::findKnotAt(mLevel, pos);

        if (!knot)
        {
            knot = LeashFenceKnotEntity::createAndAddKnot(mLevel, pos);
        }

        setLeashedTo(knot, true);
    }
    else
    {
        dropLeash(false, true);
    }

    if (getLeashHolder())
    {
        setLeashedTo(getLeashHolder(), true);
    }

    delete mLeashInfoTag;
    mLeashInfoTag = NULL;
}
