#include "game/world/generation/MonsterRoomFeature.hpp"

#include <string>
#include <vector>

#include "game/core/Direction.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"

MonsterRoomFeature::~MonsterRoomFeature()
{
}

std::wstring MonsterRoomFeature::randomEntityId(Random& random)
{
    static bool initialized = false;
    static std::wstring entityIds[4];

    if (!initialized)
    {
        initialized = true;
        entityIds[0] = EntityIO::getKey(0x000000005D540441ULL).toString();
        entityIds[1] = EntityIO::getKey(0x000000005D550040ULL).toString();
        entityIds[2] = EntityIO::getKey(0x000000005D550040ULL).toString();
        entityIds[3] = EntityIO::getKey(0x000000005D560040ULL).toString();
    }

    return entityIds[random.nextInt(4)];
}

bool MonsterRoomFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    const int xRadius = random.nextInt(2) + 3;
    const int zRadius = random.nextInt(2) + 3;
    int openings = 0;

    for (int x = -xRadius; x <= xRadius; ++x)
    {
        const bool xEdge = x == -xRadius || x == xRadius;
        for (int y = -1; y <= 4; ++y)
        {
            for (int z = -zRadius; z <= zRadius; ++z)
            {
                const BlockPos position = origin.offset(x, y, z);
                const bool solid = level->getBlockState(position)->isSolidRender();
                if ((y == -1 || y == 4) && !solid)
                {
                    return false;
                }

                if ((xEdge || z == -zRadius || z == zRadius) && y == 0 && solid
                    && !level->getBlockState(position.above())->isSolidRender())
                {
                    ++openings;
                }
            }
        }
    }

    if (openings < 1 || openings > 5)
    {
        return false;
    }

    for (int x = -xRadius; x <= xRadius; ++x)
    {
        for (int y = 3; y >= -1; --y)
        {
            for (int z = -zRadius; z <= zRadius; ++z)
            {
                const BlockPos position = origin.offset(x, y, z);
                const bool edge = x == -xRadius || x == xRadius || z == -zRadius || z == zRadius;
                if (edge || y == -1)
                {
                    if (!level->getBlockState(position.below())->isSolidRender())
                    {
                        level->setBlock(position, Blocks::mossy_cobblestone->defaultBlockState(), 2, false);
                    }

                    if (level->getBlockState(position)->getBlock() != Blocks::AIR)
                    {
                        Block* block = y == -1 && random.nextInt(4) != 0
                            ? Blocks::cobblestone : Blocks::mossy_cobblestone;
                        level->setBlock(position, block->defaultBlockState(), 2, false);
                    }
                }
                else if (level->getBlockState(position)->getBlock() != Blocks::AIR)
                {
                    level->setBlock(position, Blocks::AIR->defaultBlockState(), 2, false);
                }
            }
        }
    }

    for (int chest = 0; chest < 2; ++chest)
    {
        for (int attempt = 0; attempt < 3; ++attempt)
        {
            const BlockPos position = origin.offset(random.nextInt(xRadius * 2 + 1) - xRadius,
                                                     0,
                                                     random.nextInt(zRadius * 2 + 1) - zRadius);
            if (!level->isEmptyBlock(position))
            {
                continue;
            }

            int solidNeighbours = 0;
            std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
            while (it != Direction::VALUES.end())
            {
                if (level->getBlockState(position.relative(*it))->isSolidRender())
                {
                    ++solidNeighbours;
                }
                ++it;
            }

            if (solidNeighbours == 1)
            {
                level->setBlock(position, Blocks::chest->defaultBlockState(), 2, false);
                break;
            }
        }
    }

    level->setBlock(origin, Blocks::mob_spawner->defaultBlockState(), 2, false);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(origin);
    if (blockEntity->GetType() == 0x0000000001200201ULL)
    {
        boost::shared_ptr<MobSpawnerBlockEntity> spawnerBlockEntity =
            boost::static_pointer_cast<MobSpawnerBlockEntity>(blockEntity);
        ResourceLocation entityId(randomEntityId(random));
        spawnerBlockEntity->getSpawner()->setEntityId(entityId);
    }
    return true;
}
