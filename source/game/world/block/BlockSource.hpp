#ifndef GAME_WORLD_BLOCK_BLOCKSOURCE_HPP
#define GAME_WORLD_BLOCK_BLOCKSOURCE_HPP

#include <boost/shared_ptr.hpp>

class BlockEntity;
class BlockPos;
class BlockState;
class LocatableSource;
class Material;

class BlockSource : public LocatableSource
{
public:
    virtual ~BlockSource();

    virtual double getX() = 0;
    virtual double getY() = 0;
    virtual double getZ() = 0;
    virtual BlockPos getPos() = 0;
    virtual const BlockState* getBlockState() = 0;
    virtual Material* getMaterial() = 0;
    virtual boost::shared_ptr<BlockEntity> getEntity() = 0;
};

#endif
