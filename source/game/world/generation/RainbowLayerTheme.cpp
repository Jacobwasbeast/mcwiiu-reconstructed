#include "game/world/generation/RainbowLayerTheme.hpp"
#include <cmath>

RainbowLayerTheme::RainbowLayerTheme(unsigned int seed, double noiseScale, const std::vector<const BlockState*>& blocks)
    : RandomPatchesLayerTheme(seed, noiseScale), mBlocks(blocks)
{
}

const BlockState* RainbowLayerTheme::BlockAt(Random&, float noise, int heightAboveSurface, bool isAir) const
{
    if (heightAboveSurface < 1 && !isAir)
    {
        const float shifted = noise + 1.0f;
        const double scaledIndex = (static_cast<double>(static_cast<int>(mBlocks.size())) * 0.5) * shifted;
        return mBlocks.at(static_cast<int>(std::floor(scaledIndex)));
    }
    return NULL;
}
