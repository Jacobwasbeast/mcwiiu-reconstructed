#include "game/item/BucketItem.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/core/Direction.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/DimensionType.hpp"
#include "game/util/AABB.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringUtils.hpp"
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
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

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

int BucketItem::getSchoolName(int a, int b, int c, int d)
{
    const unsigned int (*row)[10] = values;
    int remaining = 11;
    while (true)
    {
        if ((*row)[0] == a && (*row)[1] == b && (*row)[2] == c && (*row)[3] == d)
        {
            return (*row)[4];
        }
        if ((*row)[5] == a && (*row)[6] == b && (*row)[7] == c && (*row)[8] == d)
        {
            return (*row)[9];
        }
        --remaining;
        ++row;
        if (remaining == 0)
        {
            return -1;
        }
    }
}

BucketItem::BucketItem(BucketFillType fillType)
    : Item(), mFillType(fillType)
{
    mMaxStackSize = 1;

    switch (fillType)
    {
    case EMPTY:
    case NONE:
        mBlock = Blocks::air;
        break;
    case LAVA:
        mBlock = Blocks::flowing_lava;
        break;
    case MILK:
    case FISH:
    case SALMON:
    case TROPICAL:
    case PUFFER:
    case WATER:
        mBlock = Blocks::flowing_water;
        break;
    default:
        break;
    }
}

BucketItem::~BucketItem()
{
}

bool BucketItem::isFishBucket()
{
    return mFillType >= FISH && mFillType <= PUFFER;
}

bool BucketItem::supportsEntityType(const unsigned long long& type, int& variant) const
{
    switch (static_cast<unsigned int>(type))
    {
    case static_cast<unsigned int>(EntityType::FISH):
        variant = 2;
        return true;
    case static_cast<unsigned int>(EntityType::PUFFERFISH):
        variant = 5;
        return true;
    case static_cast<unsigned int>(EntityType::SALMON):
        variant = 3;
        return true;
    case static_cast<unsigned int>(EntityType::TROPICAL_FISH):
        variant = 4;
        return true;
    default:
        return false;
    }
}

void BucketItem::addBucketEntitySaveData(boost::shared_ptr<Entity> entity, not_null_ptr<ItemInstance> itemInstance)
{
    if (!entity)
    {
        return;
    }
    if (itemInstance->hasTag())
    {
        itemInstance->getTag()->remove(L"FishBucket");
    }
    CompoundTag* tag = new CompoundTag();
    entity->addAdditonalSaveData(tag);
    if (!entity->getCustomName().empty())
    {
        std::wstring customName = entity->getCustomName();
        tag->putString(L"CustomName", customName);
        std::wstring ownerUUID = entity->getOwnerUUID();
        tag->putString(L"OwnerUUID", ownerUUID);
        bool restricted = entity->wasNamedByRestrictedPlayer();
        tag->putBoolean(L"CreatedByRestrictedPlayer", restricted);
        bool host = entity->wasNamedOnHost();
        tag->putBoolean(L"CreatedOnHost", host);
    }
    itemInstance->addTagElement(L"FishBucket", tag);
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
    if (itemInstance->isEmpty())
        return result;

    if (!player->mInventory->add(result))
    {
        not_null_ptr<ItemInstance> dropped(new ItemInstance(item, 1, 0));
        if (item != Items::waterBucket && item != Items::lavaBucket)
            addBucketEntitySaveData(entity, dropped);
        player->drop(dropped, false);
    }
    return itemInstance;
}

not_null_ptr<ItemInstance> BucketItem::collectFish(boost::shared_ptr<Player> player,
                                                   boost::shared_ptr<Entity> entity,
                                                   boost::shared_ptr<ItemInstance> itemInstance)
{
    int variant = 0;
    if (!entity)
        return itemInstance;
    unsigned long long type = entity->GetType();
    if (!supportsEntityType(type, variant))
        return itemInstance;
    if (!entity->isInWater() || (mFillType != EMPTY && mFillType != WATER))
        return itemInstance;

    Item* resultItem = NULL;
    switch (variant)
    {
    case 2:
        resultItem = Items::fishBucket;
        break;
    case 3:
        resultItem = Items::salmonBucket;
        break;
    case 4:
        resultItem = Items::tropicalBucket;
        break;
    case 5:
        resultItem = Items::pufferBucket;
        break;
    default:
        break;
    }

    if (!player->mLevel->mIsClientSide)
    {
        std::vector<boost::shared_ptr<Entity> > removed;
        removed.push_back(entity);
        player->mLevel->removeEntities(&removed);
    }
    player->playSound(SoundEvents::BUCKET_FILL_FISH, 1.0f, 1.0f);
    return createResultItem(itemInstance, player, resultItem, entity);
}

InteractionResult::EInteractionResult BucketItem::useOn(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos,
                                                        InteractionHand::EInteractionHand hand, const Direction* direction,
                                                        float hitX, float hitY, float hitZ, bool simulate)
{
    InteractionResult::EInteractionResult result;
    if (!simulate)
    {
        result = Item::useOn(player, level, pos, hand, direction, hitX, hitY, hitZ, simulate);
    }
    else
    {
        result = InteractionResult::eInteractionResult_Fail;
        if (TestUse(level, player, hand))
            return InteractionResult::eInteractionResult_Success;
    }
    return result;
}

static bool isPickable_apply(boost::shared_ptr<Entity> entity)
{
    bool pickable = true;
    if (!entity->isPickable())
    {
        bool isItem = entity->GetType() == EntityType::ITEM_ENTITY;
        if (!isItem)
            pickable = false;
    }
    return pickable;
}

HitResult* BucketItem::ScanForHitResultEntities(boost::shared_ptr<Player> player, Level* level, const HitResult* result)
{
    static const double kPickDistanceCompare = 3.0;
    static const double kCreativePickRange = 6.0;
    static const double kNoHitDistance = 0.0;

    HitResult* output = const_cast<HitResult*>(result);
    if (result == NULL)
    {
        return output;
    }

    const double pickRange = Minecraft::GetInstance()->mGameMode->getPickRange();
    Vec3* eyePosition = Vec3::newTemp(player->mX, player->mY + player->getEyeHeight(), player->mZ);

    const bool farPickRange = Minecraft::GetInstance()->mGameMode->hasFarPickRange();
    double entityReach = kCreativePickRange;
    double blockHitDistance;
    bool limitedReach = false;
    if (!farPickRange)
    {
        blockHitDistance = result->mLocation->distanceTo(eyePosition);
        entityReach = pickRange;
        if (pickRange > kPickDistanceCompare)
        {
            limitedReach = true;
        }
    }
    else
    {
        blockHitDistance = result->mLocation->distanceTo(eyePosition);
    }

    float unit = 1.0f;
    Vec3* viewVector = player->getViewVector(unit);
    Vec3* rayEnd = eyePosition->add(viewVector->x * entityReach,
                                    viewVector->y * entityReach,
                                    viewVector->z * entityReach);

    boost::shared_ptr<Entity> pointedEntity;
    Vec3* entityHitLocation = NULL;
    double closestDistance = blockHitDistance;

    static Predicate<boost::shared_ptr<Entity> >* sIsPickablePredicate =
        new Predicates<boost::shared_ptr<Entity> >::FunctionPredicate(isPickable_apply);
    const Predicate<boost::shared_ptr<Entity> >* predicates[2] = {
        Predicates<boost::shared_ptr<Entity> >::alwaysTrue, sIsPickablePredicate};
    static Predicate<boost::shared_ptr<Entity> >* sPickPredicate =
        new Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate(
            Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate::eOperator_And, predicates, 2);

    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntities(
        player,
        player->mBoundingBox->expand(viewVector->x * entityReach, viewVector->y * entityReach, viewVector->z * entityReach)
            ->grow(unit, unit, unit),
        sPickPredicate, NULL);

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
         it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> candidate = *it;

        AABB* grownBox = candidate->mBoundingBox->grow(candidate->getPickRadius());
        HitResult* clipHit = grownBox->clip(eyePosition, rayEnd);
        if (grownBox->contains(eyePosition))
        {
            if (closestDistance >= kNoHitDistance)
            {
                pointedEntity = candidate;
                entityHitLocation = clipHit == NULL ? eyePosition : clipHit->mLocation;
                closestDistance = kNoHitDistance;
            }
        }
        else if (clipHit != NULL)
        {
            const double hitDistance = eyePosition->distanceTo(clipHit->mLocation);
            if (hitDistance < closestDistance || closestDistance == kNoHitDistance)
            {
                boost::shared_ptr<Entity> candidateRoot = candidate->getRootVehicle();
                boost::shared_ptr<Entity> playerRoot = player->getRootVehicle();
                if (candidateRoot.get() == playerRoot.get())
                {
                    if (closestDistance == kNoHitDistance)
                    {
                        pointedEntity = candidate;
                        entityHitLocation = clipHit->mLocation;
                    }
                }
                else
                {
                    pointedEntity = candidate;
                    entityHitLocation = clipHit->mLocation;
                    closestDistance = hitDistance;
                }
            }
        }
        if (clipHit != NULL)
        {
            delete clipHit;
        }
    }

    if (pointedEntity.get() != NULL)
    {
        if (!limitedReach || eyePosition->distanceTo(entityHitLocation) <= kPickDistanceCompare)
        {
            if (closestDistance < blockHitDistance || output == NULL)
            {
                delete output;
                output = new HitResult(pointedEntity, entityHitLocation);
            }
        }
        else
        {
            pointedEntity.reset();
            delete output;
            output = new HitResult(HitResult::MISS, entityHitLocation, NULL, BlockPos(entityHitLocation));
        }
    }

    return output;
}

HitResult* BucketItem::GetPlayerHitResult(boost::shared_ptr<Player> player, Level* level, bool useLiquids)
{
    static const float kPartialTick = 1.0f;

    HitResult* result = player->pick(Minecraft::GetInstance()->mGameMode->getPickRange(), kPartialTick);
    if (useLiquids || result == NULL || result->mType == HitResult::MISS)
    {
        result = Item::getPlayerPOVHitResult(level, player, useLiquids);
    }
    return ScanForHitResultEntities(player, level, result);
}

bool BucketItem::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    bool pickUp = mBlock == Blocks::air || isFishBucket();
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    HitResult* hitResult = GetPlayerHitResult(player, level, pickUp);
    if (hitResult == NULL)
        return false;
    if (!level->mDimension->inPlayerConstraints(BlockPos(hitResult->mLocation)))
        return false;

    const Direction* direction = hitResult->mDirection;
    if (hitResult->getBlockPos().y >= level->getMaxBuildHeight() - 1
        && (direction == Direction::UP || hitResult->getBlockPos().y >= level->getMaxBuildHeight()))
    {
        delete hitResult;
        return false;
    }

    int variant = 0;
    if (hitResult->mType == HitResult::ENTITY)
    {
        unsigned long long type = hitResult->mEntity->GetType();
        if (supportsEntityType(type, variant) && hitResult->mEntity->isInWater()
            && (mFillType == EMPTY || mFillType == WATER))
        {
            return true;
        }
    }

    if (hitResult->mType != HitResult::BLOCK)
    {
        if (mBlock == NULL && hitResult->mEntity->GetType() == EntityType::COW)
        {
            delete hitResult;
            return true;
        }
        delete hitResult;
        return false;
    }

    BlockPos pos = hitResult->getBlockPos();
    Block* block = level->getBlock(pos);
    Block* liquidBlock = level->getLiquidBlock(pos);
    const AABB* shape = block->getClipAABB(block->defaultBlockState(), level, pos);
    bool liquid = true;
    if (!block->defaultBlockState()->getMaterial()->isLiquid()
        && !liquidBlock->defaultBlockState()->getMaterial()->isLiquid())
    {
        liquid = false;
    }
    if (shape == NULL && !liquid && !block->canContainLiquid(block->defaultBlockState()))
    {
        delete hitResult;
        return false;
    }

    if (!level->mayInteract(player, pos, mBlock))
    {
        delete hitResult;
        return false;
    }

    if (!pickUp)
    {
        if (!mBlock->isWaterBlocking())
        {
            delete hitResult;
            return true;
        }
        if (level->getBlock(pos) == Blocks::cauldron && mFillType != EMPTY && mFillType != WATER)
        {
            delete hitResult;
            return false;
        }
        BlockPos placePos = pos.relative(direction);
        if (!player->mayUseItemAt(pos, direction, itemInstance))
        {
            delete hitResult;
            return false;
        }
        if (!player->GetGameMode()->MayPlace(placePos, mBlock))
        {
            delete hitResult;
            return false;
        }
        Block* placeBlock = level->getBlockState(placePos)->getBlock();
        if (level->isEmptyBlock(placePos) || !placeBlock->isWaterBlocking() || placeBlock == Blocks::flowing_water
            || placeBlock == Blocks::water || placeBlock == Blocks::flowing_lava || placeBlock == Blocks::lava)
        {
            delete hitResult;
            return true;
        }
        delete hitResult;
        return false;
    }

    if (!player->mayUseItemAt(pos.relative(direction), direction, itemInstance))
    {
        delete hitResult;
        return false;
    }

    const BlockState* state = level->getBlockState(pos);
    Material* material = state->getBlock()->mMaterial;
    const BlockState* extraState = level->getExtraBlockState(pos);
    Material* extraMaterial = extraState->getBlock()->mMaterial;
    if ((material == Material::WATER && state->getValue(LiquidBlock::LEVEL) == 0)
        || (extraMaterial == Material::WATER && extraState->getValue(LiquidBlock::LEVEL) == 0))
    {
        delete hitResult;
        return true;
    }
    if ((material == Material::LAVA && state->getValue(LiquidBlock::LEVEL) == 0)
        || (extraMaterial == Material::LAVA && extraState->getValue(LiquidBlock::LEVEL) == 0))
    {
        delete hitResult;
        return true;
    }
    delete hitResult;
    return false;
}

bool BucketItem::isFishBucketNameCensored(const std::wstring& name, const std::wstring& owner, bool restricted, bool host)
{
    bool censored = !gApp.IsUUIDLocal(owner) && (gApp.IsConsoleChatCensored() || restricted);
    bool hidden = !host && censored;
    if ((!CGameNetworkManager::sInstance.IsHost() || !hidden) && (CGameNetworkManager::sInstance.IsHost() || !censored))
    {
        return false;
    }
    return true;
}

bool BucketItem::readBucketEntitySaveData(boost::shared_ptr<Entity> entity, not_null_ptr<ItemInstance> itemInstance)
{
    CompoundTag* tag = static_cast<CompoundTag*>(itemInstance->getTagElement(L"FishBucket"));
    if (tag == NULL)
    {
        return false;
    }
    entity->readAdditionalSaveData(tag);
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
    if (mBlock == Blocks::air)
        return false;

    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    Material* material = state->getMaterial();
    bool notSolid = !material->isSolid();
    bool waterBlocking = block->isWaterBlocking();
    bool replaceable = block->mayReplaceWithPlace(level, pos);
    bool containsLiquid = false;
    if (mBlock != Blocks::flowing_lava && level->getBlock(pos)->canContainLiquid(state))
        containsLiquid = true;

    if (!level->isEmptyBlock(pos) && !notSolid && !replaceable && !containsLiquid)
        return false;

    if (level->mDimension->isUltraWarm() && mBlock == Blocks::flowing_water)
    {
        int x = pos.x;
        int y = pos.y;
        int z = pos.z;
        float first = level->mRandom->nextFloat();
        float second = level->mRandom->nextFloat();
        level->playSound(player, pos, SoundEvents::FIRE_EXTINGUISH, SoundSource::eSoundSource_Blocks, 0.5f,
                         2.6f + (first - second) * 0.8f, 16.0f);
        for (int i = 0; i < 8; ++i)
        {
            level->addParticle(PARTICLE_LARGE_SMOKE, (double)x + Math::random(), (double)y + Math::random(),
                               (double)z + Math::random(), 0.0, 0.0, 0.0, arrayWithLength<int>());
        }
    }
    else
    {
        bool waterlog = level->getBlock(pos)->canContainLiquid(state) && material != Material::AIR
            && mBlock != Blocks::flowing_lava;
        if (!level->mIsClientSide && (notSolid || replaceable) && !material->isLiquid() && waterBlocking
            && !containsLiquid)
        {
            level->destroyBlock(pos, true);
            waterlog = false;
        }

        const SoundEvent* sound = SoundEvents::BUCKET_EMPTY;
        if (mBlock == Blocks::flowing_lava)
            sound = SoundEvents::BUCKET_EMPTY_LAVA;
        else if (mBlock == Blocks::flowing_water && isFishBucket())
            sound = SoundEvents::BUCKET_EMPTY_FISH;
        level->playSound(player, pos, sound, SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, 16.0f);

        if (mBlock != Blocks::flowing_lava || !level->mIsClientSide)
        {
            if (waterlog)
            {
                level->setExtraBlock(pos, mBlock->defaultBlockState(), Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
            }
            else
            {
                if (block->waterSpreadCausesSpawn())
                    block->spawnResources(level, pos, state, 0);
                if (material == Material::AIR || notSolid)
                    level->setBlock(pos, mBlock->defaultBlockState(), Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
            }
        }
    }

    if (itemInstance == ItemInstance::EMPTY)
        return true;

    if (!level->mIsClientSide && mBlock == Blocks::flowing_water && isFishBucket())
    {
        unsigned long long fishType = EntityType::FISH;
        switch (mFillType)
        {
        case FISH:
            fishType = EntityType::FISH;
            break;
        case SALMON:
            fishType = EntityType::SALMON;
            break;
        case TROPICAL:
            fishType = EntityType::TROPICAL_FISH;
            break;
        case PUFFER:
            fishType = EntityType::PUFFERFISH;
            break;
        default:
            break;
        }
        int error = 0;
        boost::shared_ptr<Entity> fish = SpawnEggItem::spawnMobAt(level, EntityIO::getKey(fishType), pos.x + 0.5,
                                                                  pos.y, pos.z + 0.5, &error, L"");
        if (!fish)
            SpawnEggItem::DisplaySpawnError(player, error);
        else if (error == 0)
            readBucketEntitySaveData(fish, itemInstance);
    }
    return true;
}

InteractionResultHolder BucketItem::use(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    if (mFillType == MILK)
    {
        player->startUsingItem(hand);
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, player->getItemInHand(hand));
    }

    bool pickUp = mBlock == Blocks::air || isFishBucket();
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    HitResult* hitResult = GetPlayerHitResult(player, level, pickUp);
    if (hitResult == NULL || !level->mDimension->inPlayerConstraints(BlockPos(hitResult->mLocation)))
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);

    const Direction* direction = hitResult->mDirection;
    if (hitResult->getBlockPos().y >= level->getMaxBuildHeight() - 1
        && (direction == Direction::UP || hitResult->getBlockPos().y >= level->getMaxBuildHeight()))
    {
        delete hitResult;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
    }

    if (hitResult->mType != HitResult::BLOCK)
    {
        delete hitResult;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
    }

    BlockPos pos = hitResult->getBlockPos();
    Block* block = level->getBlock(pos);
    Block* liquidBlock = level->getLiquidBlock(pos);
    const AABB* shape = block->getClipAABB(block->defaultBlockState(), level, pos);
    bool liquid = true;
    if (!block->defaultBlockState()->getMaterial()->isLiquid()
        && !liquidBlock->defaultBlockState()->getMaterial()->isLiquid())
    {
        liquid = false;
    }
    if (shape == NULL && !liquid && !block->canContainLiquid(block->defaultBlockState()))
    {
        delete hitResult;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
    }

    bool mayNotInteract = true;
    if (level->mayInteract(player, pos, mBlock) && level->mayInteract(player, pos.relative(direction), mBlock))
        mayNotInteract = false;

    if (mayNotInteract)
    {
        boost::shared_ptr<ServerPlayer> serverPlayer = boost::dynamic_pointer_cast<ServerPlayer>(player);
        if (serverPlayer != NULL)
        {
            serverPlayer->mConnection->send(boost::shared_ptr<Packet>(new ClientboundChatPacket(
                L"", ClientboundChatPacket::eChatPacketMessage_InteractionNotAllowed, -1)));
        }
        delete hitResult;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
    }

    if (!pickUp || mBlock != Blocks::air)
    {
        if (level->getBlock(pos) == Blocks::cauldron && mFillType != EMPTY && mFillType != WATER)
        {
            delete hitResult;
            return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
        }

        bool fishIntoWater = false;
        Block* target = level->getBlock(pos);
        if ((target == Blocks::flowing_water || target == Blocks::water) && isFishBucket())
            fishIntoWater = true;

        BlockPos placePos = pos;
        if (!fishIntoWater
            && (!level->getBlock(pos)->canContainLiquid(level->getBlockState(pos)) || mBlock != Blocks::flowing_water))
        {
            placePos = pos.relative(direction);
        }
        if (level->getBlock(placePos.below()) == Blocks::snow_layer)
        {
            level->destroyBlock(placePos.below(), false);
            placePos = placePos.below();
        }

        if (!player->mayUseItemAt(placePos, direction, itemInstance))
        {
            delete hitResult;
            return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
        }
        if (!player->GetGameMode()->MayPlace(placePos, mBlock))
        {
            delete hitResult;
            return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
        }
        if (!emptyBucket(player, level, placePos, itemInstance))
        {
            delete hitResult;
            return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
        }
        delete hitResult;
        if (player->mAbilities.mInstabuild)
            return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
        boost::shared_ptr<ItemInstance> bucketItem(new ItemInstance(Items::bucket));
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, bucketItem);
    }

    if (!player->mayUseItemAt(pos.relative(direction), direction, itemInstance))
    {
        delete hitResult;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
    }

    const BlockState* state = level->getBlockState(pos);
    const BlockState* liquidState = level->getLiquidBlockState(pos);
    Material* material = state->getMaterial();
    Material* liquidMaterial = liquidState->getMaterial();
    const BlockState* liquidAbove = level->getLiquidBlock(pos.above())->defaultBlockState();
    Block* blockAbove = level->getBlock(pos.above());
    bool waterSourceAbove = false;
    if (liquidAbove->getMaterial() == Material::WATER && liquidAbove->getValue(LiquidBlock::LEVEL) == 0)
        waterSourceAbove = true;

    bool removeLiquid = true;
    if ((blockAbove->defaultBlockState()->isSolid() || waterSourceAbove) && liquidMaterial == Material::WATER)
    {
        removeLiquid = false;
        for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end();
             ++it)
        {
            if (*it == Direction::UP)
                continue;
            Block* neighbour = level->getLiquidBlock(pos.relative(*it));
            bool water = neighbour->defaultBlockState()->getMaterial() == Material::WATER;
            if (!neighbour->defaultBlockState()->isSolid())
            {
                if (!water || neighbour->defaultBlockState()->getValue(LiquidBlock::LEVEL) != 0)
                {
                    removeLiquid = true;
                    break;
                }
            }
            else if (water && neighbour->defaultBlockState()->getValue(LiquidBlock::LEVEL) != 0)
            {
                removeLiquid = true;
                break;
            }
        }
    }

    if ((material == Material::WATER && state->getValue(LiquidBlock::LEVEL) == 0)
        || (liquidMaterial == Material::WATER && liquidState->getValue(LiquidBlock::LEVEL) == 0))
    {
        if (liquidMaterial == Material::WATER && removeLiquid)
            level->setExtraBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
        if ((material == Material::WATER || material == Material::BUBBLE_COLUMN) && removeLiquid)
            level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
        player->playSound(SoundEvents::BUCKET_FILL, 1.0f, 1.0f);
        delete hitResult;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success,
                                       createResultItem(itemInstance, player, Items::waterBucket,
                                                        boost::shared_ptr<Entity>()));
    }

    if ((material == Material::LAVA && state->getValue(LiquidBlock::LEVEL) == 0)
        || (liquidMaterial == Material::LAVA && liquidState->getValue(LiquidBlock::LEVEL) == 0))
    {
        if (liquidMaterial == Material::LAVA)
            level->setExtraBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
        if (material == Material::LAVA)
            level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
        player->playSound(SoundEvents::BUCKET_FILL_LAVA, 1.0f, 1.0f);
        if (level->mDimension->getType() == DimensionType::NETHER)
            player->awardStat(gStats->get_netherLavaCollected(), GenericStats::param_noArgs());
        delete hitResult;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success,
                                       createResultItem(itemInstance, player, Items::lavaBucket,
                                                        boost::shared_ptr<Entity>()));
    }

    return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
}

not_null_ptr<ItemInstance> BucketItem::finishUsingItem(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                       boost::shared_ptr<LivingEntity> entity)
{
    if (mFillType != MILK)
        return itemInstance;

    bool notInstabuild = false;
    if ((entity->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        if (!player->mAbilities.mInstabuild)
            notInstabuild = true;
    }
    if (notInstabuild)
        itemInstance->shrink(1);
    if (!level->mIsClientSide)
        entity->removeAllEffects();
    if ((entity->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        player->awardStat(gStats->get_itemsUsed(Item::VALIDgetId(this)),
                          GenericStats::param_itemsUsed(boost::static_pointer_cast<Player>(entity), itemInstance));
    }
    if (!itemInstance->isEmpty())
        return itemInstance;
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::bucket));
}

int BucketItem::getUseDuration(not_null_ptr<ItemInstance> itemInstance)
{
    return mFillType == MILK ? 32 : 0;
}

int BucketItem::getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const
{
    return mFillType == MILK ? UseAnim_drink : UseAnim_none;
}

unsigned int BucketItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (mFillType == MILK)
    {
        return IDS_TOOLTIPS_DRINK;
    }
    bool canUse = TestUse(tooltipData.m_pLevel, tooltipData.m_pPlayer, InteractionHand::eInteractionHand_MainHand);
    if (!canUse)
    {
        return IDS_NONE;
    }
    if (!isFishBucket())
    {
        return IDS_TOOLTIPS_COLLECT;
    }
    return IDS_TOOLTIPS_EMPTY;
}

static const int TROPICAL_FISH_COLOR_NAMES[15] = {
    static_cast<int>(IDS_DESC_TROPICAL_COLOR_WHITE), IDS_DESC_TROPICAL_COLOR_ORANGE, IDS_DESC_TROPICAL_COLOR_MAGENTA, IDS_DESC_TROPICAL_COLOR_SKY, static_cast<int>(IDS_DESC_TROPICAL_COLOR_YELLOW),
    0x4E44ECC4, static_cast<int>(0xB03701A2), static_cast<int>(0xD02C583C), static_cast<int>(0xF41C4A76),
    static_cast<int>(0xF6303EFD), static_cast<int>(0xFF2145A5), 0x59210C0F, static_cast<int>(0xA6E7677F),
    0x3538B71A, 0x4A597242,
};

static const int TROPICAL_FISH_PATTERN_NAMES[24] = {
    0x1AC7A9D7, 0x0D35BF83, 0x14328023, 0x0CC5A1D0, static_cast<int>(0x86347D6E), static_cast<int>(0xBDBA78AC),
    static_cast<int>(0x8C4ED0B3), 0x1918A669, static_cast<int>(0x9983DE84), static_cast<int>(0x947BF784),
    static_cast<int>(0xC1AD361F), static_cast<int>(0xF13321A2), 0x2189FC84, static_cast<int>(0x9717CFF0),
    static_cast<int>(0xA9D36B90), 0x1D2AD27B, 0x54FEB165, static_cast<int>(0xAEEAEA77),
    static_cast<int>(0xACD88200), static_cast<int>(0xF10D126A), 0x2ADC42EF, static_cast<int>(0x8AD673EF),
    0x75F6484C, 0x340BB259,
};

std::wstring BucketItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    CompoundTag* tag = static_cast<CompoundTag*>(itemInstance->getTagElement(L"FishBucket"));
    bool fishBucket = isFishBucket();
    std::wstring customName = L"";
    bool censored = false;
    if (tag != NULL && tag->contains(L"CustomName") && fishBucket)
    {
        customName = tag->getString(L"CustomName");
        std::wstring ownerUUID = tag->getString(L"OwnerUUID");
        bool restricted = tag->getBoolean(L"CreatedByRestrictedPlayer");
        bool host = tag->getBoolean(L"CreatedOnHost");
        if (fishBucket)
            censored = isFishBucketNameCensored(customName, ownerUUID, restricted, host);
    }

    if (fishBucket && !customName.empty() && !censored)
    {
        std::wstring result = L10N::GetString(static_cast<int>(IDS_DESC_BUCKET_CUSTOM_FISH));
        result = replaceAll(result, L"{*CUSTOM_FISH*}", customName);
        return result;
    }

    if (mFillType != TROPICAL || tag == NULL)
        return Item::getName(itemInstance);

    int variant = tag->getInt(L"Variant");
    int pattern = ((variant >> 8) & 0xff) % 6;
    int color = tag->getInt(L"Color");
    int baseColor = (color & 0xff) % 15;
    int patternColor = ((color >> 8) & 0xff) % 15;
    int patternOffset = 0;
    if ((variant & 1) != 0)
        patternOffset = 6;
    if (baseColor != patternColor)
        patternOffset += 12;

    int schoolName = getSchoolName(baseColor, patternColor, variant & 1, pattern);
    if (schoolName != -1)
    {
        std::wstring result = L10N::GetString(static_cast<int>(IDS_DESC_BUCKET_CUSTOM_FISH));
        result = replaceAll(result, L"{*CUSTOM_FISH*}", L10N::GetString(schoolName));
        return result;
    }

    std::wstring baseName = L10N::GetString(TROPICAL_FISH_COLOR_NAMES[baseColor]);
    if (baseColor == patternColor)
    {
        std::wstring fishName = L10N::GetString(TROPICAL_FISH_PATTERN_NAMES[pattern + patternOffset]);
        fishName = replaceAll(fishName, L"{*COLOR_0*}", baseName);
        std::wstring result = L10N::GetString(static_cast<int>(IDS_DESC_BUCKET_CUSTOM_FISH));
        result = replaceAll(result, L"{*CUSTOM_FISH*}", fishName);
        return result;
    }

    std::wstring patternName = L10N::GetString(TROPICAL_FISH_COLOR_NAMES[patternColor]);
    std::wstring fishName = L10N::GetString(TROPICAL_FISH_PATTERN_NAMES[pattern + patternOffset]);
    fishName = replaceAll(fishName, L"{*COLOR_0*}", baseName);
    fishName = replaceAll(fishName, L"{*COLOR_1*}", patternName);
    std::wstring result = L10N::GetString(static_cast<int>(IDS_DESC_BUCKET_CUSTOM_FISH));
    result = replaceAll(result, L"{*CUSTOM_FISH*}", fishName);
    return result;
}
