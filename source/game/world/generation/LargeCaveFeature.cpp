#include "game/world/generation/LargeCaveFeature.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"

unsigned int LargeCaveFeature::canReplaceBlock(int blockId, int data)
{
    if (blockId == 1 || blockId == 3 || blockId == 2 || blockId == 172 || blockId == 159 || blockId == 24 || blockId == 179 || blockId == 110 || blockId == 78)
        return 1;
    if (blockId == 12 || blockId == 13)
    {
        Block* block = Block::byId(data);
        if (block != NULL && block->getCraftingMaterial() != Material::WATER)
            return 1;
    }
    return 0;
}

void LargeCaveFeature::setCaveBlocks(Level* level, int minX, int maxX, int minY, int maxY, int minZ, int maxZ, int chunkX, int chunkZ, double centerX, double centerY, double centerZ, bool hasWater, double radiusX, double radiusZ, ChunkPrimer* primer)
{
    bool found = false;
    for (int localX = minX; localX < maxX; ++localX)
    {
        const double xd = (localX + chunkX * 16 + 0.5 - centerX) / radiusX;
        if (xd * xd >= 1.0) continue;
        for (int localZ = minZ; localZ < maxZ; ++localZ)
        {
            const double zd = (localZ + chunkZ * 16 + 0.5 - centerZ) / radiusX;
            if (xd * xd + zd * zd >= 1.0) continue;
            bool snow = false;
            Biome* biome = level->getBiome(BlockPos(localX + chunkX * 16, 0, localZ + chunkZ * 16));
            if (biome->getDepth() < -0.9f && biome->isSnowCovered()) snow = true;
            if (hasWater && !snow) continue;
            int index = (localX << 11) | (localZ << 7) | (maxY - 1);
            bool surfaceFound = false;
            for (int localY = maxY - 1; localY >= minY; --localY, --index)
            {
                const double yd = (localY - 1 + 0.5 - centerY) / radiusZ;
                if (yd <= -0.7 || yd * yd + xd * xd + zd * zd >= 1.0) continue;
                const int blockId = primer->getBlockId(index);
                const int data = primer->getBlockId(index + 1);
                if (blockId == 2 || blockId == 110) surfaceFound = true;
                if ((!hasWater || blockId == 174) && canReplaceBlock(blockId, data) != 0)
                {
                    if (!found)
                    {
                        Biome* startBiome = level->getBiome(BlockPos(centerX, centerY, centerZ));
                        if (startBiome->getBiomeType() == Biome::eBiomeType_Ocean) return;
                    }
                    found = true;
                    if (localY - 1 < 10)
                        primer->setBlockAndData(index, 11, 0);
                    else if (snow && localY < level->getSeaLevel() && blockId == 174)
                        primer->setBlockAndData(index, 9, 0);
                    else
                        primer->setBlockAndData(index, 0, 0);
                    if (localY - 1 >= 10 && surfaceFound && primer->getBlockId(index - 1) == 3)
                    {
                        Biome* replacementBiome = level->getBiome(BlockPos(localX + chunkX * 16, 0, localZ + chunkZ * 16));
                        const int replacementId = replacementBiome->mTopBlock.blockId;
                        Block* replacement = Block::byId(replacementId);
                        if (replacement != NULL)
                            primer->setBlockAndData(index - 1, replacementId, replacement->convertBlockStateToLegacyData(replacement->defaultBlockState()));
                    }
                }
            }
        }
    }
}

void LargeCaveFeature::addTunnel(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y, double z, float width, float yaw, float pitch, int start, int length, double scale)
{
    Random tunnelRandom(seed);
    const double originX = chunkX * 16 + 8;
    const double originZ = chunkZ * 16 + 8;
    const BlockPos startPos(x, y, z);
    if (length < 1)
    {
        const int maxLength = range * 16 - 16;
        length = maxLength - tunnelRandom.nextInt(maxLength / 4);
    }
    const bool room = start == -1;
    if (room) start = length / 2;
    const int branchPoint = tunnelRandom.nextInt(length / 2) + length / 4;
    const bool gentlePitch = tunnelRandom.nextInt(6) == 0;
    float yawVariance = 0.0f;
    float pitchVariance = 0.0f;
    for (int step = start; step < length; ++step)
    {
        const double radius = 1.5 + Mth::sin(step * 3.1415927f / length) * width;
        const double verticalRadius = radius * scale;
        const float horizontal = Mth::cos(pitch);
        x += Mth::cos(yaw) * horizontal;
        y += Mth::sin(pitch);
        z += Mth::sin(yaw) * horizontal;
        pitch *= gentlePitch ? 0.92f : 0.7f;
        pitch += pitchVariance * 0.1f;
        yaw += yawVariance * 0.1f;
        float first = tunnelRandom.nextFloat();
        float second = tunnelRandom.nextFloat();
        float strength = tunnelRandom.nextFloat();
        pitchVariance = pitchVariance * 0.9f + (first - second) * strength * 2.0f;
        first = tunnelRandom.nextFloat();
        second = tunnelRandom.nextFloat();
        strength = tunnelRandom.nextFloat();
        yawVariance = yawVariance * 0.75f + (first - second) * strength * 4.0f;
        if (!room && step == branchPoint && width > 1.0f)
        {
            long long branchSeed = tunnelRandom.nextLong();
            float branchWidth = tunnelRandom.nextFloat() * 0.5f + 0.5f;
            addTunnel(level, branchSeed, chunkX, chunkZ, primer, x, y, z, branchWidth, yaw - 1.5707964f, pitch / 3.0f, step, length, 1.0);
            branchSeed = tunnelRandom.nextLong();
            branchWidth = tunnelRandom.nextFloat() * 0.5f + 0.5f;
            addTunnel(level, branchSeed, chunkX, chunkZ, primer, x, y, z, branchWidth, yaw + 1.5707964f, pitch / 3.0f, step, length, 1.0);
            return;
        }
        if (!room && tunnelRandom.nextInt(4) == 0) continue;
        const double dx = x - originX;
        const double dz = z - originZ;
        const double remaining = length - step;
        const double reach = width + 18.0f;
        if (dx * dx + dz * dz - remaining * remaining > reach * reach) return;
        if (x < originX - 16.0 - radius * 2.0 || z < originZ - 16.0 - radius * 2.0 ||
            x > originX + 16.0 + radius * 2.0 || z > originZ + 16.0 + radius * 2.0) continue;
        int minX = static_cast<int>(std::floor(x - radius)) - chunkX * 16 - 1;
        int maxX = static_cast<int>(std::floor(x + radius)) - chunkX * 16 + 1;
        int minY = static_cast<int>(std::floor(y - verticalRadius)) - 1;
        int maxY = static_cast<int>(std::floor(y + verticalRadius)) + 1;
        int minZ = static_cast<int>(std::floor(z - radius)) - chunkZ * 16 - 1;
        int maxZ = static_cast<int>(std::floor(z + radius)) - chunkZ * 16 + 1;
        if (minX < 0) minX = 0;
        if (maxX > 16) maxX = 16;
        if (minY < 1) minY = 1;
        if (maxY > 120) maxY = 120;
        if (minZ < 0) minZ = 0;
        if (maxZ > 16) maxZ = 16;
        bool hasWater = false;
        for (int localX = minX; !hasWater && localX < maxX; ++localX)
        {
            for (int localZ = minZ; !hasWater && localZ < maxZ; ++localZ)
            {
                for (int localY = maxY + 1; !hasWater && localY >= minY - 1; --localY)
                {
                    if (localY >= 0 && localY < 128)
                    {
                        const int blockId = primer->getBlockId((localX << 11) | (localZ << 7) | localY);
                        if (blockId == 8 || blockId == 9) hasWater = true;
                        if (localY != minY - 1 && localX != minX && localX != maxX - 1 &&
                            localZ != minZ && localZ != maxZ - 1) localY = minY;
                    }
                }
            }
        }
        Biome* biome = level->getBiome(BlockPos(originX, 0.0, originZ));
        if (!hasWater || (biome->getDepth() < -0.9f && biome->isSnowCovered()))
        {
            if (level->getBiome(startPos)->getBiomeType() == Biome::eBiomeType_Ocean) return;
            setCaveBlocks(level, minX, maxX, minY, maxY, minZ, maxZ, chunkX, chunkZ, x, y, z, hasWater, radius, verticalRadius, primer);
            if (room) return;
        }
    }
}

void LargeCaveFeature::addRoom(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y, double z)
{
    addTunnel(level, seed, chunkX, chunkZ, primer, x, y, z, random.nextFloat() * 6.0f + 1.0f, 0.0f, 0.0f, -1, -1, 0.5);
}

void LargeCaveFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ, ChunkPrimer* primer)
{
    int count = random.nextInt(40);
    count = random.nextInt(count + 1);
    count = random.nextInt(count + 1);
    if (random.nextInt(15) != 0) count = 0;
    for (int i = 0; i < count; ++i)
    {
        const double x = chunkX * 16 + random.nextInt(16);
        const double y = random.nextInt(random.nextInt(120) + 8);
        const double z = chunkZ * 16 + random.nextInt(16);
        int branches = 1;
        if (random.nextInt(4) == 0)
        {
            addRoom(level, random.nextLong(), originChunkX, originChunkZ, primer, x, y, z);
            branches = random.nextInt(4) + 1;
        }
        for (int branch = 0; branch < branches; ++branch)
        {
            const float yaw = random.nextFloat() * 3.1415927f * 2.0f;
            const float pitch = (random.nextFloat() - 0.5f) * 2.0f * 0.125f;
            const float firstWidth = random.nextFloat();
            float tunnelWidth = firstWidth * 2.0f + random.nextFloat();
            if (random.nextInt(10) == 0)
            {
                const float firstScale = random.nextFloat();
                tunnelWidth *= firstScale * random.nextFloat() * 3.0f + 1.0f;
            }
            addTunnel(level, random.nextLong(), originChunkX, originChunkZ, primer, x, y, z, tunnelWidth, yaw, pitch, 0, 0, 1.0);
        }
    }
}

LargeCaveFeature::~LargeCaveFeature()
{
}
