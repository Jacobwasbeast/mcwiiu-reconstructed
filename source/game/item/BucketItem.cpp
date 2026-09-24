#include "game/item/BucketItem.hpp"

#include <string>

#include "game/core/GenericStats.hpp"
#include "game/core/L10N.hpp"
#include "game/core/HitResult.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

int BucketItem::getSchoolName(int a, int b, int c, int d)
{
    static const unsigned int values[11][10] = {
        { 1, 7, 1, 1, 0x32c4c9ab, 7, 7, 1, 0, 0xd50701c3 },
        { 7, 3, 0, 1, 0xcd035b9e, 0, 7, 0, 4, 0x47b1b097 },
        { 11, 7, 0, 1, 0x6ccc17a8, 1, 0, 0, 0, 0x9adcd29d },
        { 6, 3, 0, 5, 0xd2b1c3e2, 10, 4, 1, 3, 0x246fab99 },
        { 0, 14, 1, 5, 0xa50ad51a, 0, 4, 0, 5, 0x6ad3fd21 },
        { 0, 7, 1, 2, 0xa7dc5b3c, 0, 1, 1, 5, 0xf1e7d6f7 },
        { 9, 6, 0, 3, 0x206ac466, 5, 3, 0, 4, 0xf9431ba7 },
        { 14, 0, 1, 4, 0x9b750144, 7, 14, 0, 2, 0xb31348e5 },
        { 14, 0, 1, 3, 0x38e1599f, 0, 4, 1, 0, 0xf9785bdf },
        { 14, 0, 0, 1, 0xc02e273a, 7, 0, 0, 1, 0x968e5c0c },
        { 4, 4, 1, 1, 0x395fe7d7, 0, 3, 9, 4, 0x0a5c6461 }
    };
    int i;
    for (i = 0; i < 11; ++i)
    {
        if (values[i][0] == a && values[i][1] == b && values[i][2] == c && values[i][3] == d)
        {
            return static_cast<int>(values[i][4]);
        }
        if (values[i][5] == a && values[i][6] == b && values[i][7] == c && values[i][8] == d)
        {
            return static_cast<int>(values[i][9]);
        }
    }
    return -1;
}

BucketItem::BucketItem(BucketFillType fillType)
    : Item(), mFillType(fillType)
{
    mMaxStackSize = 1;

    if (fillType == EMPTY || fillType == NONE)
    {
        mBlock = Blocks::AIR;
    }
    else if (fillType == LAVA)
    {
        mBlock = Blocks::flowing_lava;
    }
    else if (fillType == MILK || fillType == FISH || fillType == SALMON || fillType == PUFFER
             || fillType == TROPICAL || fillType == WATER)
    {
        mBlock = Blocks::FLOWING_WATER;
    }
}

BucketItem::~BucketItem()
{
}

bool BucketItem::isFishBucket()
{
    return mFillType >= FISH && mFillType <= TROPICAL;
}

bool BucketItem::supportsEntityType(const unsigned long long& type, int& variant) const
{
    unsigned int id = (unsigned int)(type >> 32);
    if (id == 0x0d502002U)
    {
        variant = 2;
        return true;
    }
    if (id == 0x0d502003U)
    {
        variant = 5;
        return true;
    }
    if (id == 0x0d502004U)
    {
        variant = 3;
        return true;
    }
    if (id == 0x0d502005U)
    {
        variant = 4;
        return true;
    }
    return false;
}

void BucketItem::addBucketEntitySaveData(boost::shared_ptr<Entity> entity, not_null_ptr<ItemInstance> itemInstance)
{
    if (!entity)
    {
        return;
    }
    std::wstring key(L"FishBucket");
    if (itemInstance->hasTag())
    {
        itemInstance->getTag()->remove(key);
    }
    CompoundTag* tag = new CompoundTag();
    entity->save(tag);
    std::wstring customName = entity->getCustomName();
    if (!customName.empty())
    {
        std::wstring customName2 = entity->getCustomName();
        tag->putString(L"CustomName", customName2);
        std::wstring ownerUUID = entity->getOwnerUUID();
        tag->putString(L"OwnerUUID", ownerUUID);
        bool restricted = entity->wasNamedByRestrictedPlayer();
        tag->putBoolean(L"CreatedByRestrictedPlayer", restricted);
        bool host = entity->wasNamedOnHost();
        tag->putBoolean(L"CreatedOnHost", host);
    }
    itemInstance->addTagElement(key, tag);
}

not_null_ptr<ItemInstance> BucketItem::createResultItem(not_null_ptr<ItemInstance> itemInstance,
                                                        boost::shared_ptr<Player> player,
                                                        Item* item,
                                                        boost::shared_ptr<Entity> entity)
{
    not_null_ptr<ItemInstance> result(new ItemInstance(item));
    if (item != Items::waterBucket && item != Items::lavaBucket)
    {
        addBucketEntitySaveData(entity, result);
    }
    if (!player->mAbilities.mInstabuild)
    {
        itemInstance->shrink(1);
    }
    if (!itemInstance->isEmpty())
    {
        if (!player->mInventory->add(result))
        {
            not_null_ptr<ItemInstance> dropped(new ItemInstance(item, 1, 0));
            player->drop(dropped, false);
        }
    }
    return result;
}

not_null_ptr<ItemInstance> BucketItem::collectFish(boost::shared_ptr<Player> player,
                                                   boost::shared_ptr<Entity> entity,
                                                   boost::shared_ptr<ItemInstance> itemInstance)
{
    int variant = FISH;
    unsigned long long type = entity ? entity->GetType() : 0;
    supportsEntityType(type, variant);
    Item* resultItem = this;
    if (variant == SALMON) resultItem = Item::byId(460);
    if (variant == PUFFER) resultItem = Item::byId(461);
    if (variant == TROPICAL) resultItem = Item::byId(462);
    return createResultItem(itemInstance, player, resultItem, entity);
}

InteractionResult::EInteractionResult BucketItem::useOn(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos,
                                                        InteractionHand::EInteractionHand hand, const Direction* direction,
                                                        float hitX, float hitY, float hitZ, bool simulate)
{
    if (simulate)
    {
        return InteractionResult::eInteractionResult_Success;
    }
    return TestUse(level, player, hand) ? InteractionResult::eInteractionResult_Success : InteractionResult::eInteractionResult_Fail;
}

namespace
{
bool isPickableBucketEntity_apply(boost::shared_ptr<Entity> entity)
{
    return isPickable_apply(entity);
}

Predicate<boost::shared_ptr<Entity> >* getBucketPickEntityPredicate()
{
    static Predicate<boost::shared_ptr<Entity> >* sPickPredicate = NULL;
    if (sPickPredicate == NULL)
    {
        static Predicates<boost::shared_ptr<Entity> >::FunctionPredicate sIsPickablePredicate(
            isPickableBucketEntity_apply);
        static const Predicate<boost::shared_ptr<Entity> >* sPredicates[2] = {
            Predicates<boost::shared_ptr<Entity> >::alwaysTrue, &sIsPickablePredicate};
        sPickPredicate = new Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate(
            Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate::eOperator_And, sPredicates, 2);
    }
    return sPickPredicate;
}

void deleteBucketHitResult(HitResult* hit)
{
    if (hit != NULL)
    {
        delete hit;
    }
}
}

HitResult* BucketItem::ScanForHitResultEntities(boost::shared_ptr<Player> player, Level* level, const HitResult* result)
{
    static const double kPickDistanceCompare = 3.0;
    static const double kCreativePickRange = 6.0;
    static const double kNoHitDistance = 0.0;
    static const float kPartialTick = 1.0f;

    if (result == NULL)
    {
        return NULL;
    }

    MultiPlayerGameMode* gameMode = Minecraft::GetInstance()->mGameMode;
    const double pickRange = gameMode->getPickRange();
    Player* playerRaw = player.get();
    const double eyeHeight = playerRaw->getEyeHeight();
    Vec3* eyePosition = Vec3::newTemp(playerRaw->mX, playerRaw->mY + eyeHeight, playerRaw->mZ);

    double entityReach = kCreativePickRange;
    double blockHitDistance = kPickDistanceCompare;
    if (!gameMode->hasFarPickRange())
    {
        blockHitDistance = eyePosition->distanceTo(result->mLocation);
        entityReach = pickRange;
    }
    else
    {
        blockHitDistance = eyePosition->distanceTo(result->mLocation);
    }

    Vec3* viewVector = playerRaw->getViewVector(kPartialTick);
    AABB* searchBox = playerRaw->mBoundingBox->expand(viewVector->x * entityReach,
                                                      viewVector->y * entityReach,
                                                      viewVector->z * entityReach)
                          ->grow(1.0);

    std::vector<boost::shared_ptr<Entity> >* entities =
        level->getEntities(player, searchBox, getBucketPickEntityPredicate(), NULL);

    Vec3* rayEnd = Vec3::newTemp(eyePosition->x + viewVector->x * entityReach,
                                 eyePosition->y + viewVector->y * entityReach,
                                 eyePosition->z + viewVector->z * entityReach);

    Entity* closestEntity = NULL;
    boost::shared_ptr<Entity> closestEntityShared;
    HitResult* closestClipHit = NULL;
    double closestDistance = kNoHitDistance;

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
         it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> candidate = *it;
        Entity* candidateRaw = candidate.get();
        if (candidateRaw == NULL)
        {
            continue;
        }

        AABB* grownBox = candidateRaw->mBoundingBox->grow(candidateRaw->getPickRadius());
        HitResult* clipHit = grownBox->clip(eyePosition, rayEnd);
        if (!grownBox->contains(eyePosition))
        {
            if (clipHit == NULL)
            {
                continue;
            }

            const double hitDistance = eyePosition->distanceTo(clipHit->mLocation);
            if (hitDistance < closestDistance || closestDistance == kNoHitDistance)
            {
                if (candidateRaw->getId() == playerRaw->getId() && hitDistance != kNoHitDistance)
                {
                    delete clipHit;
                    continue;
                }

                if (closestClipHit != NULL)
                {
                    delete closestClipHit;
                }
                closestEntity = candidateRaw;
                closestEntityShared = candidate;
                closestClipHit = clipHit;
                closestDistance = hitDistance;
            }
            else
            {
                delete clipHit;
            }
        }
        else if (kNoHitDistance <= closestDistance)
        {
            if (candidateRaw->getId() == playerRaw->getId() && closestDistance != kNoHitDistance)
            {
                if (clipHit != NULL)
                {
                    delete clipHit;
                }
                continue;
            }

            if (clipHit != NULL)
            {
                delete clipHit;
            }
            if (closestClipHit != NULL)
            {
                delete closestClipHit;
            }
            closestEntity = candidateRaw;
            closestEntityShared = candidate;
            closestClipHit = new HitResult(Vec3::newTemp(eyePosition->x, eyePosition->y, eyePosition->z), NULL);
            closestDistance = kNoHitDistance;
        }
        else if (clipHit != NULL)
        {
            delete clipHit;
        }
    }

    HitResult* output = const_cast<HitResult*>(result);
    if (closestEntity != NULL && closestClipHit != NULL)
    {
        Vec3* closestHitLocation = closestClipHit->mLocation;
        if (gameMode->hasFarPickRange() || pickRange <= kPickDistanceCompare
            || eyePosition->distanceTo(closestHitLocation) <= kPickDistanceCompare)
        {
            if (closestDistance < blockHitDistance)
            {
                deleteBucketHitResult(output);
                closestClipHit->mLocation = NULL;
                delete closestClipHit;
                output = new HitResult(closestEntityShared, closestHitLocation);
                closestClipHit = NULL;
            }
            else
            {
                deleteBucketHitResult(output);
                closestClipHit->mLocation = NULL;
                delete closestClipHit;
                output = new HitResult(HitResult::MISS, closestHitLocation, NULL, BlockPos(closestHitLocation));
                closestClipHit = NULL;
            }
        }
        else
        {
            deleteBucketHitResult(output);
            closestClipHit->mLocation = NULL;
            delete closestClipHit;
            output = new HitResult(HitResult::MISS, closestHitLocation, NULL, BlockPos(closestHitLocation));
            closestClipHit = NULL;
        }
    }

    if (closestClipHit != NULL)
    {
        delete closestClipHit;
    }

    return output;
}

HitResult* BucketItem::GetPlayerHitResult(boost::shared_ptr<Player> player, Level* level, bool useLiquids)
{
    static const float kPartialTick = 1.0f;

    MultiPlayerGameMode* gameMode = Minecraft::GetInstance()->mGameMode;
    double pickRange = gameMode->getPickRange();
    HitResult* result = player->pick(pickRange, kPartialTick);
    if (useLiquids || result == NULL || result->mType == HitResult::MISS)
    {
        result = Item::getPlayerPOVHitResult(level, player, useLiquids);
    }
    return ScanForHitResultEntities(player, level, result);
}

bool BucketItem::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    bool useLiquids = mBlock == Blocks::AIR || isFishBucket();
    HitResult* result = GetPlayerHitResult(player, level, useLiquids);
    if (result == NULL)
    {
        return false;
    }

    BlockPos constraintPos(result->mLocation);
    if (!level->mDimension->inPlayerConstraints(constraintPos))
    {
        return false;
    }

    BlockPos pos = result->getBlockPos();
    if (pos.y >= level->getMaxBuildHeight() - 1)
    {
        if (result->mDirection == Direction::UP)
        {
            delete result;
            return false;
        }
        if (pos.y >= level->getMaxBuildHeight())
        {
            delete result;
            return false;
        }
    }

    if (result->mType == HitResult::ENTITY)
    {
        boost::shared_ptr<Entity> entity = result->mEntity;
        int variant = 0;
        unsigned long long type = entity->GetType();
        if (supportsEntityType(type, variant) && entity->isInWater()
            && (mFillType == EMPTY || mFillType == WATER))
        {
            return true;
        }

    }
    if (result->mType != HitResult::BLOCK)
    {
        if (mBlock == NULL && result->mEntity->GetType() == 0x2d6a0000ULL)
        {
            delete result;
            return true;
        }
        delete result;
        return false;
    }

    Block* block = level->getBlock(pos);
    Block* liquidBlock = level->getLiquidBlock(pos);
    const BlockState* blockState = block->defaultBlockState();
    const BlockState* liquidState = liquidBlock->defaultBlockState();
    const AABB* clipBox = block->getClipAABB(blockState, level, pos);
    bool hasLiquidMaterial = blockState->getMaterial()->isLiquid()
                          || liquidState->getMaterial()->isLiquid();
    if (clipBox == NULL && !hasLiquidMaterial && !mBlock->canContainLiquid(blockState))
    {
        delete result;
        return false;
    }

    if (!level->mayInteract(player, pos, mBlock))
    {
        delete result;
        return false;
    }

    Block* cauldronPositionBlock = level->getBlock(pos);
    if (cauldronPositionBlock == Blocks::cauldron && mFillType != EMPTY && mFillType != WATER)
    {
        delete result;
        return false;
    }

    BlockPos placePos = pos.relative(result->mDirection);
    if (player->mayUseItemAt(placePos, result->mDirection, itemInstance) != 1)
    {
        delete result;
        return false;
    }

    GameMode* gameMode = player->GetGameMode();
    if (!gameMode->MayPlace(placePos, mBlock))
    {
        delete result;
        return false;
    }

    const BlockState* placeState = level->getBlockState(placePos);
    Block* placeBlock = placeState->getBlock();
    bool canUseAtPlace = level->isEmptyBlock(placePos) || !placeBlock->isWaterBlocking()
                      || placeBlock == Blocks::FLOWING_WATER || placeBlock == Blocks::WATER
                      || placeBlock == Blocks::flowing_lava || placeBlock == Blocks::lava;
    if (!canUseAtPlace)
    {
        const BlockState* hitState = level->getBlockState(pos);
        const BlockState* extraHitState = level->getExtraBlockState(pos);
        Material* hitMaterial = hitState->getMaterial();
        Material* extraHitMaterial = extraHitState->getMaterial();
        if (hitMaterial == Material::WATER || hitMaterial == Material::LAVA)
        {
            canUseAtPlace = hitState->getValue(LiquidBlock::LEVEL) == 0;
        }
        if (!canUseAtPlace
            && (extraHitMaterial == Material::WATER || extraHitMaterial == Material::LAVA))
        {
            canUseAtPlace = extraHitState->getValue(LiquidBlock::LEVEL) == 0;
        }
    }

    if (canUseAtPlace)
    {
        delete result;
        return true;
    }

    delete result;
    return false;
}

bool BucketItem::isFishBucketNameCensored(const std::wstring& name, const std::wstring& owner, bool restricted, bool host)
{
    bool censored = !g_pApp->IsUUIDLocal(owner) &&
                    (g_pApp->IsConsoleChatCensored() || restricted);
    bool hidden = !host && censored;
    return (!CGameNetworkManager::sInstance.IsHost() || !hidden) &&
           (CGameNetworkManager::sInstance.IsHost() || !censored);
}

bool BucketItem::readBucketEntitySaveData(boost::shared_ptr<Entity> entity, not_null_ptr<ItemInstance> itemInstance)
{
    std::wstring key(L"FishBucket");
    CompoundTag* tag = static_cast<CompoundTag*>(itemInstance->getTagElement(key));
    if (tag == NULL)
    {
        return false;
    }
    entity->load(tag);
    if (tag->contains(L"CustomName"))
    {
        std::wstring customName = tag->getString(L"CustomName");
        std::wstring ownerUUID = tag->getString(L"OwnerUUID");
        bool restricted = tag->getBoolean(L"CreatedByRestrictedPlayer");
        bool host = tag->getBoolean(L"CreatedOnHost");
        if (!isFishBucketNameCensored(customName, ownerUUID, restricted, host))
        {
            entity->setCustomName(customName, ownerUUID, restricted);
        }
    }
    return true;
}

bool BucketItem::emptyBucket(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos, not_null_ptr<ItemInstance> itemInstance)
{
    if (mFillType == EMPTY)
    {
        return false;
    }
    if (level->setBlock(pos, mBlock->defaultBlockState(), 11, false))
    {
        itemInstance->shrink(1);
        return true;
    }
    return false;
}

InteractionResultHolder BucketItem::use(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (TestUse(level, player, hand))
    {
        player->awardStat(gStats->get_itemsUsed(Item::VALIDgetId(this)), GenericStats::param_itemsUsed(player, itemInstance));
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
    }
    return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
}

not_null_ptr<ItemInstance> BucketItem::finishUsingItem(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                       boost::shared_ptr<LivingEntity> entity)
{
    if (mFillType == MILK)
    {
        bool notInstabuild = false;
        unsigned long long type = entity->GetType();
        if ((static_cast<unsigned int>(type) & 0xc800000U) == 0xc800000U)
        {
            boost::shared_ptr<Player> asPlayer = boost::static_pointer_cast<Player>(entity);
            if (!asPlayer->mAbilities.mInstabuild)
            {
                notInstabuild = true;
            }
        }
        if (notInstabuild)
        {
            itemInstance->shrink(1);
        }
        if (!level->mIsClientSide)
        {
            entity->removeAllEffects();
        }
        type = entity->GetType();
        if ((static_cast<unsigned int>(type) & 0xc800000U) == 0xc800000U)
        {
            boost::shared_ptr<Player> asPlayer = boost::static_pointer_cast<Player>(entity);
            asPlayer->awardStat(gStats->get_itemsUsed(Item::VALIDgetId(this)), GenericStats::param_itemsUsed(asPlayer, itemInstance));
        }
    }
    if (!itemInstance->isEmpty())
    {
        return itemInstance;
    }
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::bucket));
}

int BucketItem::getUseDuration(not_null_ptr<ItemInstance> itemInstance)
{
    return mFillType == MILK ? 32 : 0;
}

int BucketItem::getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const
{
    return mFillType == MILK ? 2 : 0;
}

unsigned int BucketItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (mFillType == MILK)
    {
        return 0x32374037U;
    }
    if (!TestUse(tooltipData.m_pLevel, tooltipData.m_pPlayer, InteractionHand::eInteractionHand_MainHand))
    {
        return 0xFFFFFFFFU;
    }
    if (isFishBucket())
    {
        return 0xe80ccf48U;
    }
    return 0x4c4246b3U;
}

std::wstring BucketItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    if (isFishBucket() && itemInstance->hasCustomHoverName())
    {
        return itemInstance->getHoverName();
    }
    if (mFillType == FISH) return L"item.fishBucket.name";
    if (mFillType == SALMON) return L"item.salmonBucket.name";
    if (mFillType == PUFFER) return L"item.pufferBucket.name";
    if (mFillType == TROPICAL) return L"item.tropicalBucket.name";
    return Item::getName(itemInstance);
}
