#include "game/world/generation/BonusChestFeature.hpp"
#include "game/core/EntityType.hpp"

#include <boost/pointer_cast.hpp>

#include "game/core/BuiltInLootTables.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/Material.hpp"

BonusChestFeature::~BonusChestFeature()
{
}

bool BonusChestFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    return place(level, random, origin, false);
}

bool BonusChestFeature::place(Level* level, Random& random, const BlockPos& origin, bool useOrigin)
{
    BlockPos position = origin;

    if (!useOrigin)
    {
        const BlockState* state = level->getBlockState(position);
        while ((state->getMaterial() == Material::AIR || state->getMaterial() == Material::LEAVES) && position.y > 1)
        {
            position = position.below();
            state = level->getBlockState(position);
        }

        if (position.y < 1)
        {
            return false;
        }

        position = position.above();
    }

    for (int attempt = 0; attempt < 4; ++attempt)
    {
        BlockPos candidate(0, 0, 0);
        if (useOrigin)
        {
            candidate = position.offset(0, -1, 0);
        }
        else
        {
            const int xOffsetStart = random.nextInt(4);
            const int xOffsetEnd = random.nextInt(4);
            const int yOffsetStart = random.nextInt(3);
            const int yOffsetEnd = random.nextInt(3);
            const int zOffsetStart = random.nextInt(4);
            const int zOffsetEnd = random.nextInt(4);
            candidate = position.offset(xOffsetStart - xOffsetEnd,
                                        yOffsetStart - yOffsetEnd,
                                        zOffsetStart - zOffsetEnd);
        }

        if (level->isEmptyBlock(candidate))
        {
            const BlockPos support = candidate.below();
            if (level->getBlockState(support)->isTopSolidBlocking())
            {
                level->setBlock(candidate, Blocks::chest->defaultBlockState(), Block::UPDATE_CLIENTS, false);

                boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(candidate);
                const bool isChest = blockEntity->GetType() == BlockEntityType::CHEST;
                if (isChest)
                {
                    boost::shared_ptr<ChestBlockEntity> chest =
                        boost::dynamic_pointer_cast<ChestBlockEntity>(blockEntity);
                    if (chest)
                    {
                        chest->setLootTable(&BuiltInLootTables::SPAWN_BONUS_CHEST, random.nextLong());
                        chest->mIsBonusChest = true;
                    }
                }

                const BlockPos east = candidate.east();
                const BlockPos west = candidate.west();
                const BlockPos north = candidate.north();
                const BlockPos south = candidate.south();

                if (level->isEmptyBlock(west) && level->getBlockState(west.below())->isTopSolidBlocking())
                {
                    level->setBlock(west, Blocks::torch->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                }

                if (level->isEmptyBlock(east) && level->getBlockState(east.below())->isTopSolidBlocking())
                {
                    level->setBlock(east, Blocks::torch->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                }

                if (level->isEmptyBlock(north) && level->getBlockState(north.below())->isTopSolidBlocking())
                {
                    level->setBlock(north, Blocks::torch->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                }

                if (level->isEmptyBlock(south) && level->getBlockState(south.below())->isTopSolidBlocking())
                {
                    level->setBlock(south, Blocks::torch->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                }

                return true;
            }
        }
    }

    return false;
}
