#include "game/world/generation/HugeMushroomFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HugeMushroomBlock.hpp"
#include "game/world/block/Material.hpp"

HugeMushroomFeature::HugeMushroomFeature(Block* block)
    : Feature(true), mBlock(block)
{
}

HugeMushroomFeature::HugeMushroomFeature()
    : Feature(false), mBlock(NULL)
{
}

HugeMushroomFeature::~HugeMushroomFeature()
{
}

bool HugeMushroomFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    Block* block = mBlock;
    if (block == NULL)
    {
        block = random.nextBoolean() ? Blocks::brown_mushroom_block : Blocks::red_mushroom_block;
    }

    int height = random.nextInt(3) + 4;
    int extraHeight = random.nextInt(12);
    if (extraHeight == 0)
    {
        height *= 2;
    }

    bool clear = true;
    if (origin.y > 0 && origin.y + height + 1 < 256)
    {
        for (int y = origin.y; y <= origin.y + height + 1; ++y)
        {
            int radius = y <= origin.y + 3 ? 0 : 3;
            for (int x = origin.x - radius; x <= origin.x + radius && clear; ++x)
            {
                for (int z = origin.z - radius; z <= origin.z + radius; ++z)
                {
                    const BlockState* state = level->getBlockState(BlockPos(x, y, z));
                    if (state->getMaterial() != Material::AIR
                        && state->getMaterial() != Material::LEAVES)
                    {
                        clear = false;
                        break;
                    }
                }
            }
        }
        if (!clear)
        {
            return false;
        }

        const BlockState* below = level->getBlockState(origin.below());
        Block* belowBlock = below->getBlock();
        if (belowBlock == Blocks::dirt || belowBlock == Blocks::grass || belowBlock == Blocks::mycelium)
        {
            int top = origin.y + height;
            int layer = top;
            if (block == Blocks::brown_mushroom_block)
            {
                layer = top - 3;
            }

            for (int y = layer; y <= top; ++y)
            {
                int radius = 1;
                if (y < top)
                {
                    radius = 2;
                }
                if (block == Blocks::brown_mushroom_block && y == layer)
                {
                    radius = 3;
                }

                for (int x = origin.x - radius; x <= origin.x + radius; ++x)
                {
                    bool left = x == origin.x - radius;
                    bool right = x == origin.x + radius;
                    for (int z = origin.z - radius; z <= origin.z + radius; ++z)
                    {
                        bool north = z == origin.z - radius;
                        bool south = z == origin.z + radius;
                        int data = 5;
                        if (left)
                        {
                            data = 4;
                        }
                        if (right)
                        {
                            data = 6;
                        }
                        if (north)
                        {
                            data = left ? 1 : right ? 3 : 2;
                        }
                        if (south)
                        {
                            data = left ? 7 : right ? 9 : 8;
                        }

                        if (block == Blocks::brown_mushroom_block || y < top)
                        {
                            if ((!left && !right) || (!north && !south))
                            {
                                if (left && north)
                                {
                                    data = 1;
                                }
                                else if (right && north)
                                {
                                    data = 3;
                                }
                                else if (left && south)
                                {
                                    data = 7;
                                }
                                else if (right && south)
                                {
                                    data = 9;
                                }
                            }
                        }

                        const BlockState* state = level->getBlockState(BlockPos(x, y, z));
                        if ((state->getMaterial() == Material::AIR || state->getMaterial() == Material::LEAVES)
                            && ((data != 5 || y >= top) || y >= top - 1))
                        {
                            setBlock(level, BlockPos(x, y, z), block->getBlockState(data), 0);
                        }
                    }
                }
            }

            for (int y = 0; y < height; ++y)
            {
                BlockPos stem = origin.above(y);
                const BlockState* state = level->getBlockState(stem);
                if (state->getMaterial() == Material::AIR || state->getMaterial() == Material::LEAVES)
                {
                    setBlock(level, stem, block->getBlockState(10), 0);
                }
            }
            return true;
        }
    }
    return false;
}
