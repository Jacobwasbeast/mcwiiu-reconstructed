#ifndef GAME_WORLD_GENERATION_BASICPATCHESLAYERTHEME_HPP
#define GAME_WORLD_GENERATION_BASICPATCHESLAYERTHEME_HPP

#include <vector>

#include "game/world/generation/RandomPatchesLayerTheme.hpp"

class BasicPatchesLayerTheme : public RandomPatchesLayerTheme
{
public:
    BasicPatchesLayerTheme(unsigned int uSeed,
                           double dNoiseScale,
                           int iPatchDepth,
                           const std::vector<const BlockState*>& patchBlocks,
                           float fPatchThreshold);

    virtual const BlockState* BlockAt(Random& random, float fNoise, int iHeightAboveSurface, bool bIsAir) const;

    float m_fPatchThreshold;
    int m_iPatchDepth;
    std::vector<const BlockState*> m_apPatchBlocks;
};

#endif
