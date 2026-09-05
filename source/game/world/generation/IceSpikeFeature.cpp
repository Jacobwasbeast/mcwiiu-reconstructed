#include "game/world/generation/IceSpikeFeature.hpp"

#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

IceSpikeFeature::~IceSpikeFeature()
{
}

bool IceSpikeFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    BlockPos position = origin;
    while (level->isEmptyBlock(position) && position.y > 2)
    {
        position = position.below();
    }

    if (level->getBlockState(position)->getBlock() != Blocks::snow)
    {
        return false;
    }

    position = position.above(random.nextInt(4));
    int height = random.nextInt(4) + 7;
    int width = (height >> 2) + random.nextInt(2);

    if (width > 1 && random.nextInt(60) == 0)
    {
        position = position.above(random.nextInt(30) + 10);
    }

    const float edgeInset = 0.25f;
    const float edgeChance = 0.75f;

    for (int y = 0; y < height; ++y)
    {
        const float radius = (1.0f - static_cast<float>(y) / static_cast<float>(height)) * static_cast<float>(width);
        int extent = static_cast<int>(radius);
        if (static_cast<float>(extent) < radius)
        {
            ++extent;
        }

        for (int x = -extent; x <= extent; ++x)
        {
            for (int z = -extent; z <= extent; ++z)
            {
                const float distanceX = static_cast<float>(Mth::abs(x)) - edgeInset;
                const float distanceZ = static_cast<float>(Mth::abs(z)) - edgeInset;
                if (!((x == 0 && z == 0) || distanceX * distanceX + distanceZ * distanceZ <= radius * radius))
                {
                    continue;
                }

                if (x != -extent && x != extent && z != -extent && z != extent)
                {
                    if (random.nextFloat() > edgeChance)
                    {
                        continue;
                    }
                }

                BlockPos target = position.offset(x, y, z);
                const BlockState* state = level->getBlockState(target);
                Block* block = state->getBlock();
                if (state->getMaterial() == Material::AIR || block == Blocks::AIR
                    || block == Blocks::dirt || block == Blocks::snow || block == Blocks::ice)
                {
                    level->setBlock(target, Blocks::packed_ice->defaultBlockState(), 0, false);
                }

                if (y != 0 && width > 1)
                {
                    target = position.offset(x, y - 1, z);
                    state = level->getBlockState(target);
                    block = state->getBlock();
                    if (state->getMaterial() == Material::AIR || block == Blocks::AIR
                        || block == Blocks::dirt || block == Blocks::snow || block == Blocks::ice)
                    {
                        level->setBlock(target, Blocks::packed_ice->defaultBlockState(), 0, false);
                    }
                }
            }
        }
    }

    int baseRadius = width - 1;
    if (baseRadius < 0)
    {
        baseRadius = 0;
    }
    if (baseRadius > 1)
    {
        baseRadius = 1;
    }

    for (int x = -baseRadius; x <= baseRadius; ++x)
    {
        for (int z = -baseRadius; z <= baseRadius; ++z)
        {
            const int distance = Mth::abs(x) == 1 && Mth::abs(z) == 1 ? 1 : 0;
            BlockPos target = position.offset(x, -1, z);
            int remaining = 50;
            if (distance == 1)
            {
                remaining = random.nextInt(5);
            }

            while (remaining > 0)
            {
                const BlockState* state = level->getBlockState(target);
                Block* block = state->getBlock();
                if (state->getMaterial() == Material::AIR || block == Blocks::AIR
                    || block == Blocks::dirt || block == Blocks::snow || block == Blocks::ice
                    || block == Blocks::packed_ice)
                {
                    level->setBlock(target, Blocks::packed_ice->defaultBlockState(), 0, false);
                    target = target.below();
                }
                else
                {
                    break;
                }

                --remaining;
                if (remaining < 1)
                {
                    remaining = random.nextInt(5);
                    target = target.below(random.nextInt(5) + 1);
                }
            }
        }
    }

    return true;
}
