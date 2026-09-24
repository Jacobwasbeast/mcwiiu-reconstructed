#include "game/item/BottleItem.hpp"

#include <typeinfo>
#include <boost/shared_ptr.hpp>

#include "game/core/GenericStats.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

BottleItem::BottleItem()
    : Item()
{
}

BottleItem::~BottleItem()
{
}

not_null_ptr<ItemInstance> BottleItem::turnBottleIntoItem(not_null_ptr<ItemInstance> itemInstance,
                                    boost::shared_ptr<Player> player,
                                    not_null_ptr<ItemInstance> result)
{
    itemInstance->shrink(1);
    player->awardStat(gStats->get_itemsUsed(VALIDgetId(this)),
                      GenericStats::param_itemsUsed(player, itemInstance));

    if (!itemInstance->isEmpty())
    {
        if (!player->mInventory->add(result))
        {
            player->drop(result, false);
        }
        return itemInstance;
    }

    return result;
}

InteractionResultHolder BottleItem::use(Level* level,
                                        boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    AABB* area = player->mBoundingBox->grow(2.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        level->getEntitiesOfClass(typeid(AreaEffectCloud), area, NULL, false);

    if (entities != NULL && !entities->empty())
    {
        boost::shared_ptr<AreaEffectCloud> cloud =
            boost::static_pointer_cast<AreaEffectCloud>(entities->at(0));
        cloud->setRadius(cloud->getRadius() - 0.5f);
        player->awardStat(GenericStats::youNeedAMint(), GenericStats::param_youNeedAMint());
        not_null_ptr<ItemInstance> result(new ItemInstance(Items::dragonBreath));
        PotionUtils::setPotion(result, Potions::WATER);
        not_null_ptr<ItemInstance> usedItem = turnBottleIntoItem(itemInstance, player, result);
        delete entities;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, usedItem);
    }

    delete entities;

    HitResult* hit = getPlayerPOVHitResult(level, player, true);
    if (hit != NULL)
    {
        BlockPos constraintPos(hit->mLocation);
        if (level->mDimension->inPlayerConstraints(constraintPos) && hit->mType == HitResult::BLOCK)
        {
            BlockPos pos = hit->getBlockPos();
            if (level->mayInteract(player, pos, NULL)
                && player->mayUseItemAt(pos, hit->mDirection, itemInstance) == 1
                && level->getBlockState(pos)->getMaterial() == Material::WATER)
            {
                level->playSound(player, player->mX, player->mY, player->mZ, SoundEvents::BOTTLE_FILL,
                                 SoundSource::eSoundSource_Neutral, 1.0f, 1.0f, 16.0f);
                delete hit;
                not_null_ptr<ItemInstance> result(new ItemInstance(Items::potion));
                PotionUtils::setPotion(result, Potions::WATER);
                not_null_ptr<ItemInstance> usedItem = turnBottleIntoItem(itemInstance, player, result);
                return InteractionResultHolder(InteractionResult::eInteractionResult_Success, usedItem);
            }
        }
        delete hit;
    }

    return Item::use(level, player, hand);
}

bool BottleItem::TestUse(Level* level,
                         boost::shared_ptr<Player> player,
                         InteractionHand::EInteractionHand hand)
{
    AABB* area = player->mBoundingBox->grow(2.0);
    if (level->hasEntitiesOfClass(typeid(AreaEffectCloud), area, NULL, false))
    {
        return true;
    }

    not_null_ptr<ItemInstance> itemInHand = player->getItemInHand(hand);

    HitResult* hit = getPlayerPOVHitResult(level, player, true);
    if (hit == NULL)
    {
        return false;
    }

    BlockPos constraintPos(hit->mLocation);
    if (!level->mDimension->inPlayerConstraints(constraintPos))
    {
        return false;
    }
    if (hit->mType != HitResult::BLOCK)
    {
        delete hit;
        return false;
    }
    BlockPos pos = hit->getBlockPos();
    if (!level->mayInteract(player, pos, NULL))
    {
        delete hit;
        return false;
    }
    if (player->mayUseItemAt(pos, hit->mDirection, itemInHand) != 1)
    {
        delete hit;
        return false;
    }

    bool isWater = level->getBlockState(pos)->getMaterial() == Material::WATER;
    delete hit;
    return isWater;
}

unsigned int BottleItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (TestUse(static_cast<Level*>(tooltipData.m_pLevel),
                boost::static_pointer_cast<Player>(tooltipData.m_pPlayer),
                InteractionHand::eInteractionHand_MainHand))
    {
        return 0x4c4246b3u;
    }
    return 0xffffffffu;
}

TextureAtlasSprite* BottleItem::getIcon(int)
{
    return Items::potion->getIcon(0);
}

void BottleItem::registerIcons(IconRegister*)
{
}
