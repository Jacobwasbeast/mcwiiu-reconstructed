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
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

BottleItem::BottleItem()
    : Item()
{
}

BottleItem::~BottleItem()
{
}

void BottleItem::turnBottleIntoItem(not_null_ptr<ItemInstance> itemInstance,
                                    boost::shared_ptr<Player> player,
                                    not_null_ptr<ItemInstance> result)
{
    itemInstance->shrink(1);
    player->awardStat(gStats->get_itemsUsed(VALIDgetId(this)),
                      GenericStats::param_itemsUsed(player, itemInstance));

    if (!itemInstance->isEmpty())
    {
        if (!player->mField0x5f0->add(result))
        {
            player->drop(result, false);
        }
    }
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
        not_null_ptr<ItemInstance> result(new ItemInstance(Items::lingeringPotion));
        PotionUtils::setPotion(result, Potions::WATER);
        turnBottleIntoItem(itemInstance, player, result);
        delete entities;
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
    }

    delete entities;

    HitResult* hit = getPlayerPOVHitResult(level, player, true);
    if (hit != NULL && hit->mType == HitResult::BLOCK)
    {
        BlockPos pos = hit->getBlockPos();
        if (level->mayInteract(player, pos, NULL)
            && level->getBlockState(pos)->getMaterial() == Material::WATER)
        {
            not_null_ptr<ItemInstance> result(new ItemInstance(Items::potion));
            PotionUtils::setPotion(result, Potions::WATER);
            turnBottleIntoItem(itemInstance, player, result);
            return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
        }
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

    HitResult* hit = getPlayerPOVHitResult(level, player, true);
    if (hit == NULL || hit->mType != HitResult::BLOCK)
    {
        return false;
    }

    return level->mayInteract(player, hit->getBlockPos(), NULL);
}

unsigned int BottleItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (TestUse((Level*)tooltipData.m_pLevel,
                (boost::shared_ptr<Player>&)tooltipData.m_pPlayer,
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
