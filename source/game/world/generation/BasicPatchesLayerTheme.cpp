#include "game/world/generation/BasicPatchesLayerTheme.hpp"

#include "game/util/Random.hpp"

BasicPatchesLayerTheme::BasicPatchesLayerTheme(unsigned int seed, double noiseScale, int depth, const std::vector<const BlockState*>& blocks, float noiseThreshold)
    : RandomPatchesLayerTheme(seed, noiseScale)
    , mNoiseThreshold(noiseThreshold)
    , mDepth(depth)
    , mBlocks(blocks)
{
}

const BlockState* BasicPatchesLayerTheme::BlockAt(Random&, float noise, int heightAboveSurface, bool isAir) const
{
    if (noise <= mNoiseThreshold || heightAboveSurface > mDepth)
    {
        return 0;
    }
    int index = static_cast<int>(mBlocks.size()) - 1;
    const int depth = mDepth - heightAboveSurface;
    if (depth < index)
    {
        index = depth;
    }
    return mBlocks.at(static_cast<std::vector<const BlockState*>::size_type>(index));
}
