#include "game/world/generation/EndGatewayFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EndGatewayBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TypedBoxed.hpp"

EndGatewayFeature::EndGatewayFeature(int portalNumber)
    : Feature(false),
      mPortalNumber(portalNumber)
{
}

bool EndGatewayFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    MutableBlockPos position(0, 0, 0);
    for (int x = origin.x - 1; x <= origin.x + 1; ++x)
    {
        for (int z = origin.z - 1; z <= origin.z + 1; ++z)
        {
            for (int y = origin.y - 2; y <= origin.y + 2; ++y)
            {
                const bool centerZ = z == origin.z;
                const bool innerY = y - origin.y != 2 && origin.y - y != 2;
                position.set(x, y, z);

                if (y == origin.y)
                {
                    if (x != origin.x || !centerZ)
                    {
                        setBlock(level, position, Blocks::air->defaultBlockState(), Block::UPDATE_NONE);
                    }
                    else
                    {
                        TypedBoxed<int> portalNumber(mPortalNumber);
                        const BlockState* state = Blocks::end_gateway->defaultBlockState()->setBoxedValue(
                            EndGatewayBlock::PORTAL_NUMBER, &portalNumber);
                        setBlock(level, position, state, Block::UPDATE_NONE);
                    }
                }
                else if (x != origin.x)
                {
                    if (centerZ && innerY)
                    {
                        setBlock(level, position, Blocks::bedrock->defaultBlockState(), Block::UPDATE_NONE);
                    }
                    else
                    {
                        setBlock(level, position, Blocks::air->defaultBlockState(), Block::UPDATE_NONE);
                    }
                }
                else if (innerY || centerZ)
                {
                    setBlock(level, position, Blocks::bedrock->defaultBlockState(), Block::UPDATE_NONE);
                }
                else
                {
                    setBlock(level, position, Blocks::air->defaultBlockState(), Block::UPDATE_NONE);
                }
            }
        }
    }

    return true;
}
