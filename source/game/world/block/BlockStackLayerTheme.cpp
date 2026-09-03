#include "game/world/block/BlockStackLayerTheme.hpp"

#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/block/BlockTransaction.hpp"

BlockStackLayerTheme::BlockStackLayerTheme(const std::vector<const BlockState*>& states)
    : mStates(states)
{
}

void BlockStackLayerTheme::Apply(Random&, BlockTransaction* transaction, int&, int&) const
{
    for (int x = 0; x < transaction->mSize.x; ++x)
    {
        for (int z = 0; z < transaction->mSize.z; ++z)
        {
            int stateIndex = 0;
            for (int y = transaction->mSize.y - 1; y >= 0; --y)
            {
                Vec3i position(x, y, z);
                if (transaction->getBlock(position) != 0)
                {
                    int lastState = (int)mStates.size() - 1;
                    int index = stateIndex;
                    if (lastState < index)
                        index = lastState;
                    transaction->setBlock(position, mStates[index]);
                    ++stateIndex;
                }
            }
        }
    }
}
