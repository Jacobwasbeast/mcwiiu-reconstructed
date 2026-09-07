#include "game/item/MilkBucketItem.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GenericStats.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

MilkBucketItem::~MilkBucketItem()
{
}

not_null_ptr<ItemInstance> MilkBucketItem::finishUsingItem(not_null_ptr<ItemInstance> itemInstance,
                                                           Level* level,
                                                           boost::shared_ptr<LivingEntity> entity)
{
    bool isPlayer = (entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER;
    boost::shared_ptr<Player> player;

    if (isPlayer)
    {
        player = boost::static_pointer_cast<Player>(entity);
        if (!player->mAbilities.mInstabuild)
        {
            itemInstance->shrink(1);
        }
    }

    if (!level->mIsClientSide)
    {
        entity->removeAllEffects();
    }

    if (isPlayer)
    {
        player->awardStat(gStats->get_itemsUsed(Item::VALIDgetId(this)),
                          GenericStats::param_itemsUsed(player, itemInstance));
    }

    if (itemInstance->isEmpty())
    {
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::bucket));
    }

    return itemInstance;
}

int MilkBucketItem::getUseDuration(not_null_ptr<ItemInstance> itemInstance)
{
    return 32;
}

int MilkBucketItem::getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const
{
    return 2;
}

InteractionResultHolder MilkBucketItem::use(Level* level,
                                            boost::shared_ptr<Player> player,
                                            InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    player->startUsingItem(hand);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

unsigned int MilkBucketItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return 0x32374037;
}
