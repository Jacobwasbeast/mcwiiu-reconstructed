#include "game/world/generation/UnderwaterLargeCaveFeature.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"

unsigned int UnderwaterLargeCaveFeature::isDiggable(int blockId)
{
    if (blockId < 0x6e)
    {
        if (blockId < 4)
            return 1;
        if (blockId > 7)
            return blockId < 0xe || blockId == 0x18 || blockId == 0x31;
    }
    else
        return blockId == 0x6e || blockId == 0x9f || blockId == 0xac || blockId == 0xb3;
    return 0;
}

void UnderwaterLargeCaveFeature::apply(Level* level, int chunkX, int chunkZ, ChunkPrimer* primer)
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

void UnderwaterLargeCaveFeature::addTunnel(Level* level, long long seed, int chunkX, int chunkZ,
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
    int branchPoint;
    float yawVariance = 1.65F;
    float pitchVariance = 1.65F;
    if (room)
    {
        start = length / 2;
        branchPoint = tunnelRandom.nextInt() + length / 4;
    }
    else
        branchPoint = tunnelRandom.nextInt(length / 2) + length / 4;

    const float pi = 3.1415927F;
    const float halfPi = 1.5707964F;
    const double span = length - start;
    const int steeringMode = tunnelRandom.nextInt(6);
    for (int step = start; step < length; ++step)
    {
        const float progress = static_cast<float>(step - start) / static_cast<float>(span);
        const float radius = std::sin(progress * pi) * width + 1.5F;
        const double radiusY = static_cast<double>(radius * scale);
        x += std::cos(yaw) * std::cos(pitch);
        y += std::sin(pitch);
        z += std::sin(yaw) * std::cos(pitch);
        if (steeringMode == 0)
        {
            pitch += pitchVariance * 0.1F;
            yaw = yaw * 0.92F + yawVariance * 0.1F;
        }
        else
        {
            pitch += pitchVariance * 0.1F;
            yaw = yaw * 0.7F + yawVariance * 0.1F;
        }
        yawVariance = yawVariance * 0.75F + tunnelRandom.nextGaussianFloat() * tunnelRandom.nextFloat() * 4.0F;
        pitchVariance = pitchVariance * 0.9F + tunnelRandom.nextGaussianFloat() * tunnelRandom.nextFloat() * 2.0F;
        if (!room && steeringMode == 0 && step == branchPoint && width > 1.875F)
        {
            addTunnel(level, tunnelRandom.nextLong(), chunkX, chunkZ, primer, x, y, z, tunnelRandom.nextFloat() * 6.0F + 6.0F, yaw - halfPi, pitch / 3.0F, step, length, 1.0);
            addTunnel(level, tunnelRandom.nextLong(), chunkX, chunkZ, primer, x, y, z, tunnelRandom.nextFloat() * 6.0F + 6.0F, yaw + halfPi, pitch / 3.0F, step, length, 1.0);
            return;
        }
        if (!room && tunnelRandom.nextInt(4) != 0)
            continue;
        const float distance = static_cast<float>(step - start);
        if (distance * distance > static_cast<float>(scale * scale) *
            (static_cast<float>(length - step) * static_cast<float>(length - step) + 0.25F * length * length))
            break;
        if (x < originX - 32.0F || z < originZ - 32.0F || x > originX + 32.0F || z > originZ + 32.0F)
            continue;
        int minX = static_cast<int>(std::floor(x - radius)) - chunkX * 16 - 1;
        int maxX = static_cast<int>(std::floor(x + radius)) - chunkX * 16 + 1;
        int minZ = static_cast<int>(std::floor(z - radius)) - chunkZ * 16 - 1;
        int maxZ = static_cast<int>(std::floor(z + radius)) - chunkZ * 16 + 1;
        const int maxY = Mth::clamp(static_cast<int>(std::floor(y + radiusY)) + 1, 1, 120);
        const int minY = Mth::clamp(static_cast<int>(std::floor(y - radiusY)) - 1, 1, maxY);
        if (minX < 0) minX = 0;
        if (maxX > 16) maxX = 16;
        if (minZ < 0) minZ = 0;
        if (maxZ > 16) maxZ = 16;
        bool found = false;
        for (int localX = minX; localX < maxX; ++localX)
        {
            const float xd = (localX + chunkX * 16 + 0.5F - static_cast<float>(x)) / radius;
            for (int localZ = minZ; localZ < maxZ; ++localZ)
            {
                const float zd = (localZ + chunkZ * 16 + 0.5F - static_cast<float>(z)) / radius;
                if (xd * xd + zd * zd >= 1.0F)
                    continue;
                for (int localY = maxY - 1; localY >= minY; --localY)
                {
                    const float yd = (localY + 0.5F - static_cast<float>(y)) / static_cast<float>(radiusY);
                    const int index = (localX << 11) | (localZ << 7) | localY;
                    if (yd <= -1.8F || xd * xd + zd * zd + yd * yd >= 1.0F || localY >= 0x3e || !isDiggable(primer->getBlockId(index)))
                        continue;
                    if (!found && level != NULL && level->getBiome(BlockPos(x, y, z))->getBiomeType() != Biome::eBiomeIDs_Ocean)
                        return;
                    found = true;
                    if (localY < 9)
                        primer->setBlockAndData(index, 0xb, 0);
                    else if (localY < 10)
                        primer->setBlockAndData(index, tunnelRandom.nextFloat() < 0.5F ? 0x31 : 0xd5, 0);
                    else
                    {
                        bool water = false;
                        if (localX < 15 && primer->getBlockId(index + 0x800) == 0) { primer->setBlockAndData(index + 0x800, 8, 0); water = true; }
                        if (localX > 0 && primer->getBlockId(index - 0x800) == 0) { primer->setBlockAndData(index - 0x800, 8, 0); water = true; }
                        if (localZ < 15 && primer->getBlockId(index + 0x80) == 0) { primer->setBlockAndData(index + 0x80, 8, 0); water = true; }
                        if (localZ > 0 && primer->getBlockId(index - 0x80) == 0) { primer->setBlockAndData(index - 0x80, 8, 0); water = true; }
                        if (!water) primer->setBlockAndData(index, 9, 0);
                    }
                }
            }
        }
    }
}

void UnderwaterLargeCaveFeature::addRoom(Level* level, long long seed, int chunkX, int chunkZ,
                                          ChunkPrimer* primer, double x, double y, double z)
{
    addTunnel(level, seed, chunkX, chunkZ, primer, x, y, z, random.nextFloat() * 6.0F + 1.0F, 0.0F, 0.0F, -1, -1, 1.0);
}

void UnderwaterLargeCaveFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX,
                                             int originChunkZ, ChunkPrimer* primer)
{
    int count = random.nextInt(40);
    count = random.nextInt(count + 1);
    count = random.nextInt(count + 1);
    if (random.nextInt(15) != 0)
        count = 0;
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
            float tunnelWidth = random.nextFloat() * 2.0F + random.nextFloat();
            if (random.nextInt(10) == 0)
                tunnelWidth *= random.nextFloat() * random.nextFloat() * 3.0F + 1.0F;
            addTunnel(level, random.nextLong(), chunkX, chunkZ, primer, x, y, z, tunnelWidth,
                      random.nextFloat() * 6.2831855F, (random.nextFloat() - 0.5F) * 0.25F,
                      0, 0, 1.0);
        }
    }
}

UnderwaterLargeCaveFeature::~UnderwaterLargeCaveFeature()
{
}
