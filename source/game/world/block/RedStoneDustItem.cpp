#include "game/world/block/RedStoneDustItem.hpp"

#include "game/core/GenericStats.hpp"
#include "game/core/Stats.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"

RedStoneDustItem::RedStoneDustItem()
    : Item()
{
}

RedStoneDustItem::~RedStoneDustItem()
{
}

InteractionResult::EInteractionResult RedStoneDustItem::useOn(boost::shared_ptr<Player> player,
                                                               Level* level,
                                                               const BlockPos& pos,
                                                               InteractionHand::EInteractionHand hand,
                                                               const Direction* direction,
                                                               float hitX,
                                                               float hitY,
                                                               float hitZ,
                                                               bool simulate)
{
    BlockPos placePos = pos;
    if (!level->getBlockState(placePos)->getBlock()->mayReplaceWithPlace(level, placePos))
    {
        placePos = placePos.relative(direction);
    }

    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (itemInstance->isEmpty() || !player->mayUseItemAt(placePos, direction, itemInstance)
        || !level->mayPlace(Blocks::redstone_wire, placePos, false, direction, boost::shared_ptr<Entity>()))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (!simulate)
    {
        player->awardStat(Stats::blockPlacedStats.data[Block::getId(Blocks::redstone_wire)],
                          GenericStats::param_blocksPlaced(Blocks::redstone_wire->defaultBlockState(), 1));
        itemInstance->shrink(1);
        level->setBlock(placePos, Blocks::redstone_wire->defaultBlockState(), 11, false);
    }

    return InteractionResult::eInteractionResult_Success;
}
