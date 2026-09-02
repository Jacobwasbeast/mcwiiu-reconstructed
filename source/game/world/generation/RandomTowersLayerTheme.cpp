#include <cstddef>

#include "game/world/generation/RandomTowersLayerTheme.hpp"

const BlockState* RandomTowersLayerTheme::BlockAt(Random& random, float fNoise, int iHeightAboveSurface, bool bIsAir) const
{
    if (fNoise < m_dTowerThreshold && iHeightAboveSurface < 2)
    {
        unsigned int uIndex = 1 - iHeightAboveSurface;
        if (uIndex < m_apTowerBlocks.size())
        {
            return m_apTowerBlocks[uIndex];
        }
    }

    return NULL;
}
