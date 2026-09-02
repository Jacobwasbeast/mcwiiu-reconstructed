#include "game/world/block/BlockPlanterItem.hpp"

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Stats.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/SnowLayerBlock.hpp"
#include "game/world/generation/Player.hpp"

BlockPlanterItem::BlockPlanterItem(Block* block)
    : mBlock(block)
{
}

BlockPlanterItem::~BlockPlanterItem()
{
}

InteractionResult::EInteractionResult BlockPlanterItem::useOn(boost::shared_ptr<Player> player,
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
    const BlockState* state = level->getBlockState(placePos);
    Block* block = state->getBlock();

    if (block == Blocks::snow_layer && state->getValue(SnowLayerBlock::LAYERS) < 1)
    {
        direction = Direction::UP;
    }
    else if (!block->mayReplaceWithPlace(level, placePos))
    {
        placePos = placePos.relative(direction);
    }

    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (itemInstance->isEmpty() || !player->mayUseItemAt(placePos, direction, itemInstance)
        || !level->mayPlace(mBlock, placePos, false, direction, boost::shared_ptr<Entity>()))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (!simulate)
    {
        const BlockState* placedState =
            mBlock->getStateForPlacement(level, placePos, direction, hitX, hitY, hitZ, 0, player);

        if (!level->setBlock(placePos, placedState, 11, false))
        {
            return InteractionResult::eInteractionResult_Fail;
        }

        player->awardStat(Stats::blockPlacedStats.data[Block::getId(mBlock)],
                          GenericStats::param_blocksPlaced(placedState, 1));

        const BlockState* placedBlockState = level->getBlockState(placePos);

        if (placedBlockState->getBlock() == mBlock)
        {
            BlockItem::updateCustomBlockEntityTag(level, player, placePos, itemInstance);
            placedBlockState->getBlock()->setPlacedBy(level, placePos, placedBlockState, player, itemInstance);
        }

        const SoundType* soundType = mBlock->getSoundType();
        level->playSound(player,
                         placePos,
                         soundType->getPlaceSound(),
                         SoundSource::eSoundSource_Blocks,
                         (soundType->getVolume() + 1.0f) * 0.5f,
                         soundType->getPitch() * 0.8f,
                         16.0f);

        itemInstance->shrink(1);
    }

    return InteractionResult::eInteractionResult_Success;
}
