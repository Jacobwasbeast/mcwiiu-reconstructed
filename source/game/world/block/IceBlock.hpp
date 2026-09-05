#ifndef GAME_WORLD_BLOCK_ICEBLOCK_HPP
#define GAME_WORLD_BLOCK_ICEBLOCK_HPP

#include "game/world/block/HalfTransparentBlock.hpp"

#include <boost/shared_ptr.hpp>

class BlockEntity;
class BlockPos;
class BlockState;
class Level;
class Random;
class Player;
class ItemInstance;

class IceBlock : public HalfTransparentBlock
{
public:
    IceBlock(bool isClientSide);
    virtual ~IceBlock();

    virtual int getRenderLayer();
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                               const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                               not_null_ptr<ItemInstance> tool);
    virtual int getResourceCount(Random* random);
    static void melt(Level* level, const BlockPos& pos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual bool shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state);
};

#endif
