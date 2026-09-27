#include "game/world/generation/MineShaftFeature.hpp"

#include "game/core/CMinecraftApp.hpp"
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

MineShaftFeature::~MineShaftFeature()
{
}

std::wstring MineShaftFeature::getFeatureName()
{
    return std::wstring(L"Mineshaft");
}

bool MineShaftFeature::isFeatureChunk(int chunkX, int chunkZ, bool flatWorld)
{
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (options != NULL && options->isFeatureChunk(chunkX, chunkZ, eFeatureType_Mineshaft, NULL))
        return true;

    if (random.nextDouble() < mChance)
    {
        int distanceX = chunkX < 0 ? -chunkX : chunkX;
        int distanceZ = chunkZ < 0 ? -chunkZ : chunkZ;
        int distance = distanceX > distanceZ ? distanceX : distanceZ;
        if (random.nextInt(80) < distance)
            return true;
    }
    return false;
}

BlockPos* MineShaftFeature::getNearestGeneratedFeature(Level* levelIn,
                                                        const BlockPos& pos,
                                                        bool skipExistingChunks)
{
    int chunkX = pos.x >> 4;
    int chunkZ = pos.z >> 4;
    int remaining = 1001;

    for (int radius = 0; ; ++radius)
    {
        for (int offsetX = -radius; offsetX <= radius; ++offsetX)
        {
            for (int offsetZ = -radius; offsetZ <= radius; ++offsetZ)
            {
                if (offsetX != -radius && offsetX != radius &&
                    offsetZ != -radius && offsetZ != radius)
                    continue;

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

        --remaining;
        if (remaining == 0)
            return NULL;
    }
}

StructureStart* MineShaftFeature::createStructureStart(int chunkX, int chunkZ)
{
    g_pApp->AddTerrainFeaturePosition((_eTerrainFeatureType)2, chunkX, chunkZ);
    BlockPos pos(chunkX * 16 + 8, 64, chunkZ * 16 + 8);
    bool mesa = dynamic_cast<MesaBiome*>(level->getBiome(pos)) != NULL;
    return new MineShaftStart(level, random, chunkX, chunkZ,
                              mesa ? eMineShaftType_Mesa : eMineShaftType_Normal);
}
