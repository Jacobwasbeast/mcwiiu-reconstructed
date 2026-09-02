#include <algorithm>
#include <cstddef>

#include "game/world/generation/BasicPatchesLayerTheme.hpp"

BasicPatchesLayerTheme::BasicPatchesLayerTheme(unsigned int uSeed,
                                               double dNoiseScale,
                                               int iPatchDepth,
                                               const std::vector<const BlockState*>& patchBlocks,
                                               float fPatchThreshold)
    : RandomPatchesLayerTheme(uSeed, dNoiseScale),
      m_fPatchThreshold(fPatchThreshold),
      m_iPatchDepth(iPatchDepth),
      m_apPatchBlocks(patchBlocks)
{
}

const BlockState* BasicPatchesLayerTheme::BlockAt(Random& random, float fNoise, int iHeightAboveSurface, bool bIsAir) const
{
    if (fNoise <= m_fPatchThreshold && iHeightAboveSurface <= m_iPatchDepth)
    {
        int iIndex = m_iPatchDepth - iHeightAboveSurface;
        return m_apPatchBlocks.at(std::min(static_cast<int>(m_apPatchBlocks.size()) - 1, iIndex));
    }

    return NULL;
}
