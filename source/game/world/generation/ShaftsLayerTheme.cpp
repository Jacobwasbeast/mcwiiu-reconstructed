#include "game/world/generation/ShaftsLayerTheme.hpp"

#include "game/util/Random.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/world/generation/RandomPatchesLayerTheme.hpp"

ShaftsLayerTheme::ShaftsLayerTheme(int heightOffset, const std::vector<const BlockState*>& states)
    : mHeightOffset(heightOffset), mStates(states)
{
}

bool ShaftsLayerTheme::hasShaft(int x, int z) const
{
    return x % 5 == 0 && z % 5 == 0;
}

void ShaftsLayerTheme::Apply(Random&, BlockTransaction* transaction, int& width, int& depth) const
{
    if (mStates.empty())
    {
        return;
    }

    const int top = transaction->mSize.y;
    for (int x = 0; x < transaction->mSize.x; ++x)
    {
        for (int z = 0; z < transaction->mSize.z; ++z)
        {
            if (!hasShaft(x + transaction->mSize.x / 2, z + transaction->mSize.z / 2))
            {
                continue;
            }

            const int surface = RandomPatchesLayerTheme::HighestSolidBlock(transaction, x, z);
            if (surface < 0)
            {
                continue;
            }

            int height = mHeightOffset + surface;
            if (height > top - 1)
            {
                height = top - 1;
            }
            for (int y = height; y > 0; --y)
            {
                int stateIndex = surface + mHeightOffset - y;
                if (stateIndex >= static_cast<int>(mStates.size()))
                {
                    stateIndex = static_cast<int>(mStates.size()) - 1;
                }
                const BlockState* state = mStates.at(stateIndex);
                if (state != NULL)
                {
                    transaction->setBlock(BlockPos(x, y, z), state);
                }
            }
        }
    }
}
