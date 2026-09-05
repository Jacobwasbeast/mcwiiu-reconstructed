#include "game/world/block/BaseEntityBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/Nameable.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

BaseEntityBlock::BaseEntityBlock(Material* material)
    : Block(material)
{
    mBlockEntity = true;
}

BaseEntityBlock::BaseEntityBlock(Material* material, const MaterialColor* materialColor)
    : Block(material, materialColor)
{
    mBlockEntity = true;
}

BaseEntityBlock::~BaseEntityBlock()
{
}

bool BaseEntityBlock::isCactus(Level* level, const BlockPos& pos, const Direction* direction)
{
    return level->getBlockState(pos.relative(direction))->getBlock() == Blocks::cactus;
}

bool BaseEntityBlock::touchesCactus(Level* level, const BlockPos& pos)
{
    return isCactus(level, pos, Direction::NORTH)
        || isCactus(level, pos, Direction::EAST)
        || isCactus(level, pos, Direction::SOUTH)
        || isCactus(level, pos, Direction::WEST);
}

EBlockRenderShape BaseEntityBlock::getRenderShape(const BlockState*)
{
    return (EBlockRenderShape)-1;
}

void BaseEntityBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onRemove(level, pos, state);
    level->removeBlockEntity(pos);
}

void BaseEntityBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player,
                                    const BlockPos& pos, const BlockState* state,
                                    boost::shared_ptr<BlockEntity> blockEntity,
                                    not_null_ptr<ItemInstance> tool)
{
    boost::shared_ptr<Nameable> nameable = boost::dynamic_pointer_cast<Nameable>(blockEntity);
    if (nameable.get() == NULL || !nameable->hasCustomName())
    {
        Block::playerDestroy(level, player, pos, state, blockEntity, tool);
        return;
    }

    player->causeFoodExhaustion(FoodConstants::ExhaustionMine());
    Item* resource = getResource(state, NULL, 0);
    if (resource != NULL)
    {
        not_null_ptr<ItemInstance> item(new ItemInstance(resource, 1));
        item->setHoverName(nameable->getCustomName());
        popResource(level, pos, item);
    }
}

bool BaseEntityBlock::triggerEvent(const BlockState*, Level* level, const BlockPos& pos,
                                   int eventId, int eventParam)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity.get() == NULL)
    {
        return false;
    }

    return blockEntity->triggerEvent(eventId, eventParam);
}

EPistonPushReaction BaseEntityBlock::getPistonPushReaction(const BlockState*)
{
    return (EPistonPushReaction)2;
}
