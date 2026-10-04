#include "game/world/generation/MineShaftFeature.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/MineShaftStart.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/biome/MesaBiome.hpp"
#include "game/world/block/BlockPos.hpp"

MineShaftFeature::MineShaftFeature()
    : StructureFeature(), mChance(0.004)
{
}

MineShaftFeature::MineShaftFeature(SuperflatConfig* config)
    : StructureFeature(), mChance((float)config->maybeOption(0x23, 0, 1) / 1000.0f)
{
}

std::wstring MineShaftFeature::getFeatureName()
{
    return std::wstring(L"Mineshaft");
}

bool MineShaftFeature::isFeatureChunk(int chunkX, int chunkZ, bool flatWorld)
{
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    bool forced = false;
    if (options != NULL)
        forced = options->isFeatureChunk(chunkX, chunkZ, eFeatureType_Mineshaft, NULL);

    int distanceX = abs(chunkX);
    int distanceZ = abs(chunkZ);
    return forced || (random.nextDouble() < mChance &&
                      random.nextInt(80) < (distanceX < distanceZ ? distanceZ : distanceX));
}

BlockPos* MineShaftFeature::getNearestGeneratedFeature(Level* levelIn,
                                                        const BlockPos& pos,
                                                        bool skipExistingChunks)
{
    int chunkX = pos.x >> 4;
    int chunkZ = pos.z >> 4;
    int radius = 0;
    int remaining = 1001;

    while (true)
    {
        for (int offsetX = -radius; offsetX <= radius; ++offsetX)
        {
            bool edgeX = offsetX == -radius || offsetX == radius;
            for (int offsetZ = -radius; offsetZ <= radius; ++offsetZ)
            {
                bool edgeZ = offsetZ == -radius || offsetZ == radius;
                if (edgeX || edgeZ)
                {
                    int candidateX = chunkX + offsetX;
                    int candidateZ = chunkZ + offsetZ;
                    long long seed = levelIn->getSeed();
                    random.setSeed(seed ^ (long long)(candidateX ^ candidateZ));
                    random.nextInt();
                    if (isFeatureChunk(candidateX, candidateZ, false) &&
                        (!skipExistingChunks || !levelIn->isChunkGeneratedAt(candidateX, candidateZ)))
                        return new BlockPos(candidateX * 16 + 8, 64, candidateZ * 16 + 8);
                }
            }
        }

        --remaining;
        ++radius;
        if (remaining == 0)
            return NULL;
    }
}

StructureStart* MineShaftFeature::createStructureStart(int chunkX, int chunkZ)
{
    gApp.AddTerrainFeaturePosition((_eTerrainFeatureType)2, chunkX, chunkZ);
    bool mesa = dynamic_cast<MesaBiome*>(
        level->getBiome(BlockPos(chunkX * 16 + 8, 64, chunkZ * 16 + 8))) != NULL;
    return new MineShaftStart(level, random, chunkX, chunkZ,
                              mesa ? eMineShaftType_Mesa : eMineShaftType_Normal);
}
