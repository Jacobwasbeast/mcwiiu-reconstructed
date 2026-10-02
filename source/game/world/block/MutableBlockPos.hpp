#ifndef GAME_WORLD_BLOCK_MUTABLEBLOCKPOS_HPP
#define GAME_WORLD_BLOCK_MUTABLEBLOCKPOS_HPP

#include "game/world/block/BlockPos.hpp"

class MutableBlockPos : public BlockPos
{
public:
    MutableBlockPos();
    MutableBlockPos(int x, int y, int z);
    MutableBlockPos(const Vec3i& other);

    void setX(int x);
    void setY(int y);
    void setZ(int z);

    void set(int x, int y, int z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    MutableBlockPos* set(const BlockPos& pos);

    MutableBlockPos* move(const Direction* direction, int steps);
    MutableBlockPos* move(const Direction* direction);

    void add(int dx, int dy, int dz);
    void addX(int dx);
    void addY(int dy);
    void addZ(int dz);

};

#endif
