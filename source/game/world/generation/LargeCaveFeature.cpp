#include "game/world/generation/LargeCaveFeature.hpp"

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
        if (block == NULL || block->getCraftingMaterial() == Material::AIR)
            return 1;
    }
    return 0;
}

void LargeCaveFeature::setCaveBlocks(Level* level, int x, int y, int z, int minX, int maxX, int minY, int maxY, int minZ, double centerX, double centerY, double centerZ, bool hasWater, double radiusX, double radiusZ, ChunkPrimer* primer)
{
    bool found = false;
    for (int localX = minX; localX < maxX; ++localX)
    {
        const double xd = (localX + x * 16 + 0.5 - centerX) / radiusX;
        if (xd * xd >= 1.0) continue;
        for (int localZ = minZ; localZ < maxZ; ++localZ)
        {
            const double zd = (localZ + z * 16 + 0.5 - centerZ) / radiusX;
            if (xd * xd + zd * zd >= 1.0) continue;
            bool snow = false;
            Biome* biome = level->getBiome(BlockPos(localX + x * 16, 0, localZ + z * 16));
            if (biome->getDepth() < -0.7 && biome->isSnowCovered()) snow = true;
            if (hasWater && !snow) continue;
            int index = (localX << 11) | (localZ << 7) | (maxY - 1);
            bool liquid = false;
            for (int localY = maxY - 1; localY >= minY; --localY, --index)
            {
                const double yd = (localY + 0.5 - centerY) / radiusZ;
                if (yd * yd + xd * xd + zd * zd >= 1.0) continue;
                const int blockId = primer->getBlockId(index);
                const int data = primer->getBlockId(index + 1);
                if (blockId == 8 || blockId == 9) liquid = true;
                if ((!hasWater || blockId == 174) && canReplaceBlock(blockId, data) != 0)
                {
                    if (!found)
                    {
                        Biome* startBiome = level->getBiome(BlockPos((int)centerX, (int)centerY, (int)centerZ));
                        if (startBiome->getBiomeType() == Biome::eBiomeType_Ocean) return;
                    }
                    found = true;
                    if (localY < 10)
                            primer->setBlockAndData(index, 11, 0);
                    else if (snow && localY < level->getSeaLevel() && blockId == 174)
                        primer->setBlockAndData(index, 9, 0);
                    else
                        primer->setBlockAndData(index, 0, 0);
                    if (liquid && primer->getBlockId(index - 1) == 3)
                    {
                        Biome* replacementBiome = level->getBiome(BlockPos(localX + x * 16, 0, localZ + z * 16));
                        const int replacementId = replacementBiome->mTopBlock.blockId;
                        Block* replacement = Block::byId(replacementId);
                        if (replacement != NULL)
                            primer->setBlockAndData(index - 1, replacementId, replacementBiome->mTopBlock.blockData);
                    }
                }
            }
        }
    }
}

void LargeCaveFeature::addTunnel(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y, double z, float width, float yaw, float pitch, int start, int length, double scale)
{
    Random tunnelRandom(seed);
    int end = length * 16 + 8;
    if (length < 1) end = 240 - tunnelRandom.nextInt(224);
    const bool room = start == -1;
    if (room)
    {
        start = end / 2;
        length = tunnelRandom.nextInt(end / 4) + end / 4;
        pitch = (float)tunnelRandom.nextInt(6);
    }
    else
    {
        length = tunnelRandom.nextInt(end / 2) + end / 4;
        pitch = (float)tunnelRandom.nextInt(6);
    }
    const double originX = chunkX * 16 + 8;
    const double originZ = chunkZ * 16 + 8;
    const float pi = 3.1415927f;
    const float halfPi = 1.5707964f;
    const double span = end - start;
    int branchPoint = start;
    for (int step = start; step < end; ++step)
    {
        const double progress = (step - start) / span;
        const double radius = width * Mth::sin((float)(progress * pi)) + 1.0;
        const double verticalRadius = radius * scale;
        x += Mth::cos((float)yaw) * Mth::cos((float)pitch);
        y += Mth::sin((float)pitch);
        z += Mth::sin((float)yaw) * Mth::cos((float)pitch);
        if (pitch != 0.0f)
        {
            yaw += (tunnelRandom.nextFloat() - tunnelRandom.nextFloat()) * tunnelRandom.nextFloat() * 2.0f;
            pitch = pitch * 0.9f + (tunnelRandom.nextFloat() - tunnelRandom.nextFloat()) * tunnelRandom.nextFloat() * 0.2f;
        }
        else
        {
            yaw = yaw * 0.75f + (tunnelRandom.nextFloat() - tunnelRandom.nextFloat()) * tunnelRandom.nextFloat() * 0.1f;
            pitch = pitch * 0.92f + (tunnelRandom.nextFloat() - tunnelRandom.nextFloat()) * tunnelRandom.nextFloat() * 0.2f;
        }
        if (!room && step == branchPoint && width > 1.0f)
        {
            addTunnel(level, tunnelRandom.nextLong(), chunkX, chunkZ, primer, x, y, z, tunnelRandom.nextFloat() * 6.0f + 6.0f, yaw - halfPi, pitch / 3.0f, step, end, 1.0);
            addTunnel(level, tunnelRandom.nextLong(), chunkX, chunkZ, primer, x, y, z, tunnelRandom.nextFloat() * 6.0f + 6.0f, yaw + halfPi, pitch / 3.0f, step, end, 1.0);
            return;
        }
        if (room || tunnelRandom.nextInt(4) == 0)
        {
            const int minX = (int)(x - radius) - chunkX * 16 - 1;
            const int maxX = (int)(x + radius) - chunkX * 16 + 1;
            const int minY = (int)(y - verticalRadius) - 1;
            const int maxY = (int)(y + verticalRadius) + 1;
            const int minZ = (int)(z - radius) - chunkZ * 16 - 1;
            const int maxZ = (int)(z + radius) - chunkZ * 16 + 1;
            if (level->getBiome(BlockPos((int)originX, (int)y, (int)originZ))->getBiomeType() != Biome::eBiomeType_Ocean)
                setCaveBlocks(level, chunkX, (int)y, chunkZ, minX, maxX, minY, maxY, minZ, x, y, z, false, radius, verticalRadius, primer);
        }
    }
}

void LargeCaveFeature::addRoom(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y, double z)
{
    addTunnel(level, seed, chunkX, chunkZ, primer, x, y, z, random.nextFloat() * 6.0f + 1.0f, 0.0f, 0.0f, -1, -1, 1.0);
}

void LargeCaveFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ, ChunkPrimer* primer)
{
    int count = random.nextInt(40);
    count = random.nextInt(count + 1);
    count = random.nextInt(count + 1);
    if (random.nextInt(15) == 0) count = 0;
    for (int i = 0; i < count; ++i)
    {
        const double x = chunkX * 16 + random.nextInt(16);
        const double y = random.nextInt(random.nextInt(120) + 8);
        const double z = chunkZ * 16 + random.nextInt(16);
        int branches = 1;
        if (random.nextInt(4) == 0)
        {
            addRoom(level, random.nextLong(), chunkX, chunkZ, primer, x, y, z);
            branches = random.nextInt(4) + 1;
        }
        for (int branch = 0; branch < branches; ++branch)
        {
            float tunnelWidth = random.nextFloat() * 2.0f + random.nextFloat();
            if (random.nextInt(10) == 0) tunnelWidth *= random.nextFloat() * random.nextFloat() * 3.0f + 1.0f;
            addTunnel(level, random.nextLong(), chunkX, chunkZ, primer, x, y, z, tunnelWidth, random.nextFloat() * 6.2831855f, (random.nextFloat() - 0.5f) * 0.25f, 0, 0, 1.0);
        }
    }
}

LargeCaveFeature::~LargeCaveFeature()
{
}
