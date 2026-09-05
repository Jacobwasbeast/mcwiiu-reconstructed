#include "game/world/generation/ReedsFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

class LevelGenerationOptions
{
public:
    bool checkIntersects(const BlockPos& min, const BlockPos& max);
};

class CMinecraftApp
{
public:
    LevelGenerationOptions* getLevelGenerationOptions();
};

extern CMinecraftApp* g_pApp;

ReedsFeature::~ReedsFeature()
{
}

bool ReedsFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    for (int attempt = 0; attempt < 20; ++attempt)
    {
        const BlockPos position = origin.offset(random.nextInt(4) - random.nextInt(4),
                                                 0,
                                                 random.nextInt(4) - random.nextInt(4));
        LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
        if (options != NULL && options->checkIntersects(position, position))
        {
            continue;
        }

        if (level->isEmptyBlock(position))
        {
            const BlockPos below = position.below();
            if (level->getBlockState(below.west())->getMaterial() != Material::WATER
                && level->getBlockState(below.east())->getMaterial() != Material::WATER
                && level->getBlockState(below.north())->getMaterial() != Material::WATER
                && level->getBlockState(below.south())->getMaterial() != Material::WATER)
            {
                continue;
            }

            const int height = random.nextInt(random.nextInt(3) + 1) + 2;
            for (int y = 0; y < height; ++y)
            {
                const BlockPos blockPosition = position.above(y);
                if (Blocks::reeds->mayPlace(level, blockPosition))
                {
                    level->setBlock(blockPosition, Blocks::reeds->defaultBlockState(), 2, false);
                }
            }
        }
    }

    return true;
}
