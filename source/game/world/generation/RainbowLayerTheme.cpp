#include "game/world/generation/RainbowLayerTheme.hpp"
#include <cmath>

RainbowLayerTheme::RainbowLayerTheme(unsigned int seed, double noiseScale, const std::vector<const BlockState*>& blocks)
    : RandomPatchesLayerTheme(seed, noiseScale), mBlocks(blocks)
{
}

const BlockState* RainbowLayerTheme::BlockAt(Random&, float noise, int heightAboveSurface, bool isAir) const
{
    if (heightAboveSurface < 1 && !isAir && !mBlocks.empty())
    {
        const double scaledIndex = (static_cast<double>(mBlocks.size() - 1) * 0.5) * (static_cast<double>(noise) + 1.0);
        return mBlocks.at(static_cast<std::size_t>(std::floor(scaledIndex)));
    }
    return NULL;
}
