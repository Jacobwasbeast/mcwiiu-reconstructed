#include "game/item/DoorItem.hpp"

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GenericStats.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

DoorItem::DoorItem(Block* block)
    : Item(),
      mBlock(block)
{
}

DoorItem::~DoorItem()
{
}

void DoorItem::place(Level* level, const BlockPos& pos, const Direction* direction, Block* block, bool hingeRight)
{
    BlockPos clockwise = pos.relative(direction->getClockWise());
    BlockPos counterClockwise = pos.relative(direction->getCounterClockWise());
    const BlockState* clockwiseState = level->getBlockState(clockwise);
    const BlockState* counterClockwiseState = level->getBlockState(counterClockwise);
    const BlockState* clockwiseAboveState = level->getBlockState(clockwise.above());
    const BlockState* counterClockwiseAboveState = level->getBlockState(counterClockwise.above());
    int counterClockwiseSupport = counterClockwiseState->getBlock()->isSolidBlockingCubeAndNotSignalSource(counterClockwiseState)
        + counterClockwiseAboveState->getBlock()->isSolidBlockingCubeAndNotSignalSource(counterClockwiseAboveState);
    int clockwiseSupport = clockwiseState->getBlock()->isSolidBlockingCubeAndNotSignalSource(clockwiseState)
        + clockwiseAboveState->getBlock()->isSolidBlockingCubeAndNotSignalSource(clockwiseAboveState);
    bool counterClockwiseDoor = counterClockwiseState->getBlock() == block
        || counterClockwiseAboveState->getBlock() == block;
    bool clockwiseDoor = clockwiseState->getBlock() == block
        || clockwiseAboveState->getBlock() == block;
    if ((counterClockwiseDoor && !clockwiseDoor) || counterClockwiseSupport < clockwiseSupport)
    {
        hingeRight = true;
    }
    else if ((clockwiseDoor && !counterClockwiseDoor) || clockwiseSupport < counterClockwiseSupport)
    {
        hingeRight = false;
    }

    BlockPos above = pos.above();
    bool powered = level->hasNeighborSignal(pos) || level->hasNeighborSignal(above);
    const BlockState* state = block->defaultBlockState();
    {
        TypedBoxed<const Direction*> facing(direction);
        state = state->setBoxedValue(DoorBlock::FACING, &facing);
    }
    {
        TypedBoxed<DoorBlock::HingeSide*> hinge(hingeRight ? DoorBlock::RIGHT : DoorBlock::LEFT);
        state = state->setBoxedValue(DoorBlock::HINGE, &hinge);
    }
    {
        TypedBoxed<bool> open(powered);
        state = state->setBoxedValue(DoorBlock::OPEN, &open);
    }
    {
        TypedBoxed<bool> poweredValue(powered);
        state = state->setBoxedValue(DoorBlock::POWERED, &poweredValue);
    }
    const BlockState* lowerState;
    {
        TypedBoxed<DoorBlock::DoorHalf*> lower(DoorBlock::LOWER);
        lowerState = state->setBoxedValue(DoorBlock::HALF, &lower);
    }
    level->setBlock(pos, lowerState, Block::UPDATE_ALL, false);
    const BlockState* upperState;
    {
        TypedBoxed<DoorBlock::DoorHalf*> upper(DoorBlock::UPPER);
        upperState = state->setBoxedValue(DoorBlock::HALF, &upper);
    }
    level->setBlock(above, upperState, Block::UPDATE_ALL, false);
    level->updateNeighborsAt(pos, block, false);
    level->updateNeighborsAt(above, block, false);
}

InteractionResult::EInteractionResult DoorItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                       const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                       const Direction* direction, float hitX, float hitY, float hitZ,
                                                       bool simulate)
{
    if (direction != Direction::UP)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    BlockPos placePos = pos;
    if (!level->getBlockState(placePos)->getBlock()->mayReplaceWithPlace(level, placePos))
    {
        placePos = placePos.relative(direction);
    }

    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!player->mayUseItemAt(placePos, direction, itemInstance) ||
        !mBlock->mayPlace(level, placePos))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (!simulate)
    {
        player->awardStat(gStats->get_blocksPlaced(Block::getId(mBlock)),
                          GenericStats::param_blocksPlaced(mBlock->getBlockState(itemInstance->getAuxValue()), 1));

        const Direction* facing = Direction::fromYRot((double)player->mYRot);
        int stepX = facing->getStepX();
        int stepZ = facing->getStepZ();
        bool hingeRight = (stepX < 0 && hitZ < 0.5f) || (stepX > 0 && hitZ > 0.5f) ||
            (stepZ < 0 && hitX > 0.5f) || (stepZ > 0 && hitX < 0.5f);

        place(level, placePos, facing, mBlock, hingeRight);

        const SoundType* soundType = mBlock->getSoundType();
        level->playSound(player, placePos, soundType->getPlaceSound(), SoundSource::eSoundSource_Blocks,
                         (soundType->getVolume() + 1.0f) * 0.5f, soundType->getPitch() * 0.8f, 16.0f);
        itemInstance->shrink(1);
    }

    return InteractionResult::eInteractionResult_Success;
}
