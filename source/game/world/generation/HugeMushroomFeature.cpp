#include "game/world/generation/HugeMushroomFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/HugeMushroomBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

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
        block = random.nextBoolean() ? Blocks::brown_mushroom_block : Blocks::red_mushroom_block;

    int height = random.nextInt(3) + 4;
    if (random.nextInt(12) == 0)
        height *= 2;

    if (origin.y < 1 || origin.y + height + 1 >= 256)
        return false;

    bool free = true;
    for (int y = origin.y; y <= origin.y + height + 1; ++y)
    {
        int radius = y <= origin.y + 3 ? 0 : 3;
        BlockPos checkPos;
        for (int x = origin.x - radius; x <= origin.x + radius && free; ++x)
        {
            for (int z = origin.z - radius; z <= origin.z + radius && free; ++z)
            {
                if (y >= 0 && y < 256)
                {
                    checkPos = BlockPos(x, y, z);
                    Material* material = level->getBlockState(checkPos)->getMaterial();
                    if (material != Material::AIR && material != Material::LEAVES)
                        free = false;
                }
                else
                {
                    free = false;
                }
            }
        }
    }
    if (!free)
        return false;

    Block* ground = level->getBlockState(origin.below())->getBlock();
    if (ground != Blocks::dirt && ground != Blocks::grass && ground != Blocks::mycelium)
        return false;

    int top = origin.y + height;
    int capBottom = top;
    if (block == Blocks::red_mushroom_block)
        capBottom = top - 3;

    for (int y = capBottom; y <= top; ++y)
    {
        int radius = 1;
        if (y < top)
            radius = 2;
        if (block == Blocks::brown_mushroom_block)
            radius = 3;

        int minX = origin.x - radius;
        int maxX = origin.x + radius;
        int minZ = origin.z - radius;
        int maxZ = origin.z + radius;
        for (int x = minX; x <= maxX; ++x)
        {
            for (int z = minZ; z <= maxZ; ++z)
            {
                int data = 5;
                if (x == minX)
                    --data;
                else if (x == maxX)
                    ++data;
                if (z == minZ)
                    data -= 3;
                else if (z == maxZ)
                    data += 3;
                HugeMushroomBlock::Variant* variant = HugeMushroomBlock::Variant::byData(data);

                if (block == Blocks::brown_mushroom_block || y < top)
                {
                    if ((x == minX || x == maxX) && (z == minZ || z == maxZ))
                        continue;

                    if (x == origin.x - (radius - 1) && z == minZ)
                        variant = HugeMushroomBlock::Variant::NORTH_WEST;
                    if (x == minX && z == origin.z - (radius - 1))
                        variant = HugeMushroomBlock::Variant::NORTH_WEST;
                    if (x == origin.x + (radius - 1) && z == minZ)
                        variant = HugeMushroomBlock::Variant::NORTH_EAST;
                    if (x == maxX && z == origin.z - (radius - 1))
                        variant = HugeMushroomBlock::Variant::NORTH_EAST;
                    if (x == origin.x - (radius - 1) && z == maxZ)
                        variant = HugeMushroomBlock::Variant::SOUTH_WEST;
                    if (x == minX && z == origin.z + (radius - 1))
                        variant = HugeMushroomBlock::Variant::SOUTH_WEST;
                    if (x == origin.x + (radius - 1) && z == maxZ)
                        variant = HugeMushroomBlock::Variant::SOUTH_EAST;
                    if (x == maxX && z == origin.z + (radius - 1))
                        variant = HugeMushroomBlock::Variant::SOUTH_EAST;
                }

                if (variant == HugeMushroomBlock::Variant::CENTER && y < top)
                    variant = HugeMushroomBlock::Variant::ALL_INSIDE;

                if (origin.y >= top - 1 || variant != HugeMushroomBlock::Variant::ALL_INSIDE)
                {
                    BlockPos capPos(x, y, z);
                    if (!level->getBlockState(capPos)->isSolid())
                    {
                        TypedBoxed<HugeMushroomBlock::Variant*> value(variant);
                        setBlock(level, capPos, block->defaultBlockState()->setBoxedValue(HugeMushroomBlock::VARIANT, &value), Block::UPDATE_NONE);
                    }
                }
            }
        }
    }

    for (int y = 0; y < height; ++y)
    {
        if (!level->getBlockState(origin.above(y))->isSolid())
        {
            TypedBoxed<HugeMushroomBlock::Variant*> value(HugeMushroomBlock::Variant::STEM);
            setBlock(level, origin.above(y), block->defaultBlockState()->setBoxedValue(HugeMushroomBlock::VARIANT, &value), Block::UPDATE_NONE);
        }
    }
    return true;
}
