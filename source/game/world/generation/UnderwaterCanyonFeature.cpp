#include "game/world/generation/UnderwaterCanyonFeature.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/UnderwaterLargeCaveFeature.hpp"

void UnderwaterCanyonFeature::apply(Level* level, int chunkX, int chunkZ, ChunkPrimer* primer)
{
    random.setSeed(level->getSeed());
    const int xStep = (random.nextInt() / 2) * 2 + 1;
    const int zStep = (random.nextInt() / 2) * 2 + 1;
    for (int x = chunkX - 8; x <= chunkX + 8; ++x)
    {
        const long long xSeed = static_cast<long long>(x) * xStep;
        for (int z = chunkZ - 8; z <= chunkZ + 8; ++z)
        {
            random.setSeed(level->getSeed() ^ (xSeed + static_cast<long long>(z) * zStep));
            addFeature(level, x, z, chunkX, chunkZ, primer);
        }
    }
}

void UnderwaterCanyonFeature::addTunnel(Level* level, long long seed, int chunkX, int chunkZ,
                                        ChunkPrimer* primer, double x, double y, double z,
                                        float width, float yaw, float pitch, int originChunkX,
                                        int originChunkZ, double scale)
{
    Random tunnelRandom(seed);
    int length = originChunkZ;
    if (length < 1)
        length = 0x70 - tunnelRandom.nextInt(0x1c);
    int start = originChunkX;
    const float originX = static_cast<float>(chunkX * 16 + 8);
    const float originZ = static_cast<float>(chunkZ * 16 + 8);
    const bool room = originChunkX == -1;
    if (room)
    {
        start = length / 2;
    }

    const float pi = 3.1415927F;
    const double span = length - start;
    float widthScale[128];
    int widthIndex = 0;
    while (widthIndex < 128)
    {
        const float factor = tunnelRandom.nextFloat() * tunnelRandom.nextFloat() + 1.0F;
        do
        {
            widthScale[widthIndex++] = factor * factor;
        }
        while (widthIndex < 128 && tunnelRandom.nextInt(3) != 0);
    }

    for (int step = start; step < length; ++step)
    {
        const double progress = static_cast<double>(step - start) / span;
        const double radius = std::sin(progress * pi) * width + 1.5;
        const double verticalRadius = radius * scale;
        x += std::cos(yaw) * std::cos(pitch);
        y += std::sin(pitch);
        z += std::sin(yaw) * std::cos(pitch);
        yaw += (tunnelRandom.nextFloat() - tunnelRandom.nextFloat()) * tunnelRandom.nextFloat() * 0.5F;
        pitch = pitch * 0.7F + (tunnelRandom.nextFloat() - tunnelRandom.nextFloat()) * tunnelRandom.nextFloat() * 0.25F;

        if (!room && tunnelRandom.nextInt(4) != 0)
            continue;

        const double distance = step - start;
        if (radius * radius < distance * distance -
            ((x - originX) * (x - originX) + (z - originZ) * (z - originZ)))
            break;
        if (x < originX - 16.0F || z < originZ - 16.0F || x > originX + 16.0F || z > originZ + 16.0F)
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
        minY = Mth::clamp(minY, 1, 120);
        maxY = Mth::clamp(maxY, 1, 120);

        bool found = false;
        for (int localX = minX; localX < maxX; ++localX)
        {
            const double xd = (localX + chunkX * 16 + 0.5 - x) / radius;
            for (int localZ = minZ; localZ < maxZ; ++localZ)
            {
                const double zd = (localZ + chunkZ * 16 + 0.5 - z) / radius;
                if (xd * xd + zd * zd >= 1.0)
                    continue;
                for (int localY = maxY - 1; localY >= minY; --localY)
                {
                    const double yd = (localY + 0.5 - y) / verticalRadius;
                    if (localY >= 0x3e || xd * xd + zd * zd >= 1.0 ||
                        (xd * xd + zd * zd) * widthScale[localY] + yd * yd / 8192.0 >= 1.0)
                        continue;
                    const int index = (localX << 11) | (localZ << 7) | localY;
                    if (UnderwaterLargeCaveFeature::isDiggable(primer->getBlockId(index)) == 0)
                        continue;
                    if (!found && level != NULL &&
                        level->getBiome(BlockPos(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)))->getBiomeType() != Biome::eBiomeIDs_Ocean)
                        return;

                    found = true;
                    if (localY < 10)
                    {
                        if (tunnelRandom.nextFloat() <= tunnelRandom.nextFloat())
                            primer->setBlockAndData(index, 0x31, 0);
                        else
                            primer->setBlockAndData(index, 0xd5, 0);
                    }
                    else
                    {
                        bool water = false;
                        if (localX < 15 && primer->getBlockId(index + 0x800) == 0)
                        {
                            primer->setBlockAndData(index + 0x800, 8, 0);
                            water = true;
                        }
                        if (localX > 0 && primer->getBlockId(index - 0x800) == 0)
                        {
                            primer->setBlockAndData(index - 0x800, 8, 0);
                            water = true;
                        }
                        if (localZ < 15 && primer->getBlockId(index + 0x80) == 0)
                        {
                            primer->setBlockAndData(index + 0x80, 8, 0);
                            water = true;
                        }
                        if (localZ > 0 && primer->getBlockId(index - 0x80) == 0)
                        {
                            primer->setBlockAndData(index - 0x80, 8, 0);
                            water = true;
                        }
                        if (!water)
                            primer->setBlockAndData(index, 9, 0);
                    }
                }
            }
        }
    }
}

void UnderwaterCanyonFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX,
                                         int originChunkZ, ChunkPrimer* primer)
{
    if (random.nextInt(0x14) != 0)
        return;
    const double x = chunkX * 16 + random.nextInt(16);
    const double y = random.nextInt(random.nextInt(0x28) + 8) + 0x14;
    const double z = chunkZ * 16 + random.nextInt(16);
    const float width = random.nextFloat() * 2.0F + random.nextFloat();
    const float yaw = random.nextFloat() * 6.2831855F;
    const float pitch = (random.nextFloat() - 0.5F) * 0.25F;
    addTunnel(level, random.nextLong(), chunkX, chunkZ, primer, x, y, z, width, yaw, pitch,
              originChunkX, originChunkZ, 3.0);
}

UnderwaterCanyonFeature::~UnderwaterCanyonFeature()
{
}
