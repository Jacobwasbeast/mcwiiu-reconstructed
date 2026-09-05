#include "game/world/generation/CanyonFeature.hpp"

#include <cmath>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/LargeCaveFeature.hpp"

class Biome
{
public:
    struct BlockPair
    {
        int blockId;
        int blockData;
    };

    BlockPair mTopBlock;

    enum BiomeType
    {
        eBiomeType_Ocean = 10
    };

    BiomeType getBiomeType() const;
};

void CanyonFeature::addTunnel(Level* level, long long seed, int chunkX, int chunkZ,
                              ChunkPrimer* primer, double x, double y, double z,
                              float width, float yaw, float pitch, int start, int length,
                              double scale)
{
    Random tunnelRandom(seed);
    int end = length * 16 + 8;
    if (length < 1)
        end = 128 - tunnelRandom.nextInt(32);

    const bool room = start == -1;
    if (room)
    {
        start = end / 2;
        length = tunnelRandom.nextInt(end / 4) + end / 4;
        pitch = static_cast<float>(tunnelRandom.nextInt(6));
    }
    else
    {
        length = tunnelRandom.nextInt(end / 2) + end / 4;
        pitch = static_cast<float>(tunnelRandom.nextInt(6));
    }

    const double originX = chunkX * 16 + 8;
    const double originZ = chunkZ * 16 + 8;
    const float pi = 3.1415927F;
    const double span = end - start;
    int branchPoint = start;
    float widthScale[128];
    int widthIndex = 0;

    while (widthIndex < 128)
    {
        widthScale[widthIndex] = static_cast<float>(tunnelRandom.nextFloat() * tunnelRandom.nextFloat() + 1.0F);
        ++widthIndex;
        if (widthIndex >= 128)
            break;
        if (tunnelRandom.nextInt(3) == 0)
            break;
    }

    for (int step = start; step < end; ++step)
    {
        const double progress = (step - start) / span;
        const double radius = std::sin(progress * pi) * width + 1.5;
        const double verticalRadius = radius * scale;

        x += std::cos(yaw) * std::cos(pitch);
        y += std::sin(pitch);
        z += std::sin(yaw) * std::cos(pitch);

        yaw += (tunnelRandom.nextFloat() - tunnelRandom.nextFloat()) * tunnelRandom.nextFloat() * 0.5F;
        pitch = pitch * 0.7F + (tunnelRandom.nextFloat() - tunnelRandom.nextFloat()) * tunnelRandom.nextFloat() * 0.25F;

        if (!room && step == branchPoint && width > 1.0F)
        {
            addTunnel(level, tunnelRandom.nextLong(), chunkX, chunkZ, primer, x, y, z,
                      tunnelRandom.nextFloat() * 6.0F + 6.0F, yaw - 1.5707964F, pitch / 3.0F,
                      step, end, 1.0);
            addTunnel(level, tunnelRandom.nextLong(), chunkX, chunkZ, primer, x, y, z,
                      tunnelRandom.nextFloat() * 6.0F + 6.0F, yaw + 1.5707964F, pitch / 3.0F,
                      step, end, 1.0);
            return;
        }

        if (!room && tunnelRandom.nextInt(4) != 0)
            continue;

        const double distance = step - start;
        if (radius * radius < distance * distance -
            ((x - (chunkX * 16 + 8)) * (x - (chunkX * 16 + 8)) +
             (z - (chunkZ * 16 + 8)) * (z - (chunkZ * 16 + 8))))
            break;

        if (x < originX - 16.0 || z < originZ - 16.0 || x > originX + 16.0 || z > originZ + 16.0)
            continue;

        int minX = Mth::floor(x - radius) - chunkX * 16 - 1;
        int maxX = Mth::floor(x + radius) - chunkX * 16 + 1;
        int minY = Mth::floor(y - verticalRadius) - 1;
        int maxY = Mth::floor(y + verticalRadius) + 1;
        int minZ = Mth::floor(z - radius) - chunkZ * 16 - 1;
        int maxZ = Mth::floor(z + radius) - chunkZ * 16 + 1;

        if (minX < 0) minX = 0;
        if (maxX > 16) maxX = 16;
        if (minZ < 0) minZ = 0;
        if (maxZ > 16) maxZ = 16;
        if (minY < 1) minY = 1;
        if (maxY > 120) maxY = 120;

        bool found = false;
        for (int localX = minX; localX < maxX; ++localX)
        {
            const double xd = (localX + chunkX * 16 + 0.5 - x) / radius;
            if (xd * xd >= 1.0)
                continue;
            for (int localZ = minZ; localZ < maxZ; ++localZ)
            {
                const double zd = (localZ + chunkZ * 16 + 0.5 - z) / radius;
                if (xd * xd + zd * zd >= 1.0)
                    continue;
                bool foundLiquid = false;
                for (int localY = maxY - 1; localY >= minY; --localY)
                {
                    const double yd = (localY + 0.5 - y) / verticalRadius;
                    if (xd * xd + zd * zd + yd * yd >= 1.0 || localY >= 120)
                        continue;

                    const int index = (localX << 11) | (localZ << 7) | localY;
                    const int blockId = primer->getBlockId(index);
                    const int data = primer->getBlockId(index + 1);
                    if (blockId == 8 || blockId == 9)
                        foundLiquid = true;
                    if (LargeCaveFeature::canReplaceBlock(blockId, data) == 0)
                        continue;

                    if (!found && level != NULL &&
                        level->getBiome(BlockPos(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)))->getBiomeType() == Biome::eBiomeType_Ocean)
                        return;

                    found = true;
                    if (localY < 10)
                        primer->setBlockAndData(index, 11, 0);
                    else
                        primer->setBlockAndData(index, 0, 0);

                    if (foundLiquid && primer->getBlockId(index - 1) == 3 && level != NULL)
                    {
                        Biome* biome = level->getBiome(BlockPos(chunkX * 16 + localX, localY, chunkZ * 16 + localZ));
                        primer->setBlockAndData(index - 1, biome->mTopBlock.blockId, biome->mTopBlock.blockData);
                    }
                }
            }
        }
    }
}

void CanyonFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX,
                               int originChunkZ, ChunkPrimer* primer)
{
    if (random.nextInt(50) != 0)
        return;

    const double x = chunkX * 16 + random.nextInt(16);
    const double y = random.nextInt(random.nextInt(40) + 8) + 20;
    const double z = chunkZ * 16 + random.nextInt(16);
    const float width = random.nextFloat() * 2.0F + random.nextFloat();
    const float yaw = random.nextFloat() * 6.2831855F;
    const float pitch = (random.nextFloat() - 0.5F) * 0.25F;
    const long long seed = random.nextLong();
    addTunnel(level, seed, chunkX, chunkZ, primer, x, y, z, width, yaw, pitch,
              originChunkX, originChunkZ, 1.0);
    g_pApp->AddTerrainFeaturePosition(4, static_cast<int>(x * 0.0625),
                                      static_cast<int>(y * 0.0625));
}

CanyonFeature::~CanyonFeature()
{
}
