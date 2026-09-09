#include "game/item/BucketItem.hpp"

#include <string>

#include "game/core/GenericStats.hpp"
#include "game/core/L10N.hpp"
#include "game/core/HitResult.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameMode.hpp"
#include "game/network/CGameNetworkManager.hpp"
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
    CompoundTag* tag = new CompoundTag();
    std::wstring key(L"FishBucket");
    if (itemInstance->hasTag())
    {
        itemInstance->getTag()->remove(key);
    }
    entity->saveWithoutId(tag);
    itemInstance->addTagElement(key, tag);
}

not_null_ptr<ItemInstance> BucketItem::createResultItem(not_null_ptr<ItemInstance> itemInstance,
                                                        boost::shared_ptr<Player> player,
                                                        Item* item,
                                                        boost::shared_ptr<Entity> entity)
{
    not_null_ptr<ItemInstance> result(new ItemInstance(item));
    if (entity)
    {
        addBucketEntitySaveData(entity, result);
    }
    if (player && !player->mAbilities.mInstabuild)
    {
        itemInstance->shrink(1);
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

HitResult* BucketItem::ScanForHitResultEntities(boost::shared_ptr<Player> player, Level* level, const HitResult* result)
{
    return const_cast<HitResult*>(result);
}

HitResult* BucketItem::GetPlayerHitResult(boost::shared_ptr<Player> player, Level* level, bool useLiquids)
{
    return Item::getPlayerPOVHitResult(level, player, useLiquids);
}

bool BucketItem::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    HitResult* result = GetPlayerHitResult(player, level, mFillType != EMPTY);
    if (result == NULL)
    {
        return false;
    }
    return itemInstance->TestUse(level, player, hand);
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
    if (tag == NULL || !entity)
    {
        return false;
    }
    entity->load(tag);
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
    if (entity && !level->mIsClientSide)
    {
        entity->removeAllEffects();
    }
    if (itemInstance->isEmpty())
    {
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::bucket));
    }
    return itemInstance;
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
