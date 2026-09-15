#include "game/entity/mob/HangingEntityItem.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GenericStats.hpp"
#include "game/core/Painting.hpp"
#include "game/core/Stats.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/HangingEntity.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DiodeBlock.hpp"

HangingEntityItem::HangingEntityItem(unsigned long long entityType)
    : Item(), mEntityType(entityType)
{
}

HangingEntityItem::~HangingEntityItem()
{
}

boost::shared_ptr<HangingEntity> HangingEntityItem::createEntity(Level* level, const BlockPos& pos,
                                                                  const Direction* direction, int auxValue)
{
    if (mEntityType == 0x000000000A080001ULL)
    {
        boost::shared_ptr<Painting> painting(new Painting(level, pos, direction));
        painting->PaintingPostConstructor(direction, -1);
        level->addEntity(boost::static_pointer_cast<Entity>(painting));
        return boost::static_pointer_cast<HangingEntity>(painting);
    }
    else if (mEntityType == 0x000000000A080002ULL)
    {
        boost::shared_ptr<ItemFrame> itemFrame(new ItemFrame(level, pos, direction));
        level->updateObservingBlocksAt(pos, NULL);
        level->addEntity(boost::static_pointer_cast<Entity>(itemFrame));
        return boost::static_pointer_cast<HangingEntity>(itemFrame);
    }
    return boost::shared_ptr<HangingEntity>();
}

InteractionResult::EInteractionResult HangingEntityItem::useOn(
    boost::shared_ptr<Player> player, Level* level, const BlockPos& pos,
    InteractionHand::EInteractionHand hand, const Direction* direction,
    float hitX, float hitY, float hitZ, bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    BlockPos placePos = pos.relative(direction);

    if (direction == Direction::UP || direction == Direction::DOWN)
        return InteractionResult::eInteractionResult_Fail;

    if (!player->mayUseItemAt(placePos, direction, itemInstance))
        return InteractionResult::eInteractionResult_Fail;

    if (simulate)
    {
        const BlockState* state = level->getBlockState(pos);
        if (state == NULL || (!state->isSolid() && !DiodeBlock::isDiode(state)))
            return InteractionResult::eInteractionResult_Fail;
        return InteractionResult::eInteractionResult_Success;
    }

    createEntity(level, placePos, direction, itemInstance->getAuxValue());

    if (!level->mIsClientSide)
    {
        Block* block = NULL;
        if (mEntityType == 0x000000000A080001ULL)
            block = Block::byItem(Item::byId(321));
        else if (mEntityType == 0x000000000A080002ULL)
            block = Block::byItem(Item::byId(389));

        if (block != NULL)
        {
            player->awardStat(Stats::blockPlacedStats.data[Block::getId(block)],
                              GenericStats::param_blocksPlaced(
                                  block->getBlockState(itemInstance->getAuxValue()), 1));
        }
    }

    itemInstance->shrink(1);
    return InteractionResult::eInteractionResult_Success;
}

void HangingEntityItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance,
                                        boost::shared_ptr<Player> player,
                                        std::vector<HtmlString>* lines, bool advanced)
{
}
