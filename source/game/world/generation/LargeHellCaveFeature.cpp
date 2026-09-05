#include "game/world/generation/LargeHellCaveFeature.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"

LargeHellCaveFeature::~LargeHellCaveFeature()
{
}

void LargeHellCaveFeature::addRoom(long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y, double z)
{
    const float scale = 6.0f * random.nextFloat() + 1.0f;
    addTunnel(seed, chunkX, chunkZ, primer, x, y, z, scale, 0.0f, 0.0f, -1, -1, 1.75);
}

void LargeHellCaveFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ, ChunkPrimer* primer)
{
    (void)level;

    int count = random.nextInt(10);
    count = random.nextInt(count + 1);
    count = random.nextInt(count + 1);

    if (random.nextInt(5) != 0)
    {
        count = 0;
    }

    for (int i = 0; i < count; ++i)
    {
        const double x = (double)(chunkX * 16 + random.nextInt(16));
        const double y = (double)random.nextInt(128);
        const double z = (double)(chunkZ * 16 + random.nextInt(16));

        int branches = 1;
        if (random.nextInt(4) == 0)
        {
            addRoom(random.nextLong(), chunkX, chunkZ, primer, x, y, z);
            branches = random.nextInt(4) + 1;
        }

        for (int branch = 0; branch < branches; ++branch)
        {
            const float width = (random.nextFloat() * 2.0f + random.nextFloat()) * 2.0f;
            const float yaw = random.nextFloat() * 6.2831855f;
            const float pitch = (random.nextFloat() - 0.5f) * 0.25f;
            const long long branchSeed = random.nextLong();
            addTunnel(branchSeed, chunkX, chunkZ, primer, x, y, z, width, yaw, pitch,
                      -1, originChunkX, 0.5);
        }
    }
}

void LargeHellCaveFeature::addTunnel(long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y, double z,
                                     float yaw, float pitch, float width, int start, int length, double verticalScale)
{
    Random tunnelRandom(seed);
    const double centerX = (double)(chunkX * 16 + 8);
    const double centerZ = (double)(chunkZ * 16 + 8);
    int maxLength = length;

    if (maxLength <= 0)
    {
        maxLength = 112 - tunnelRandom.nextInt(4);
    }

    int branchPoint = start;
    const bool room = start == -1;
    if (branchPoint == -1)
    {
        branchPoint = maxLength / 2 + tunnelRandom.nextInt(maxLength / 2);
    }

    const int branchLength = maxLength - branchPoint;

    for (int step = start; step < maxLength; ++step)
    {
        const float progress = (float)step / (float)maxLength;
        const double radius = (double)(Mth::sin(progress * 3.1415927f) * width) + 1.0;
        const double heightRadius = radius * verticalScale;

        x += (double)Mth::cos(pitch) * (double)Mth::cos(yaw);
        y += (double)Mth::sin(yaw);
        z += (double)Mth::cos(pitch) * (double)Mth::sin(yaw);

        pitch = pitch * 0.9f + (tunnelRandom.nextFloat() - tunnelRandom.nextFloat()) * tunnelRandom.nextFloat() * 2.0f;
        yaw = yaw * 0.75f + (tunnelRandom.nextFloat() - tunnelRandom.nextFloat()) * tunnelRandom.nextFloat() * 4.0f;

        if (step == branchPoint && width > 1.0f && branchLength > 0)
        {
            addTunnel(tunnelRandom.nextLong(), chunkX, chunkZ, primer, x, y, z,
                      tunnelRandom.nextFloat() * 0.5f + 1.5f, pitch - 1.0f, tunnelRandom.nextFloat() * 0.5f + 0.5f,
                      step, branchLength, verticalScale);
            addTunnel(tunnelRandom.nextLong(), chunkX, chunkZ, primer, x, y, z,
                      tunnelRandom.nextFloat() * 0.5f + 1.5f, pitch + 1.0f, tunnelRandom.nextFloat() * 0.5f + 0.5f,
                      step, branchLength, verticalScale);
            return;
        }

        if (room || tunnelRandom.nextInt(4) != 0)
        {
            const int minX = (int)(x - radius) - chunkX * 16 - 1;
        const int maxX = (int)(x + radius) - chunkX * 16 + 1;
        const int minY = (int)(y - heightRadius) - 1;
        const int maxY = (int)(y + heightRadius) + 1;
        const int minZ = (int)(z - radius) - chunkZ * 16 - 1;
        const int maxZ = (int)(z + radius) - chunkZ * 16 + 1;

        if (maxX < 0 || minX > 16 || maxZ < 0 || minZ > 16 || maxY < 1 || minY >= 128)
        {
            continue;
        }

        const int clampedMinX = minX < 0 ? 0 : minX;
        const int clampedMaxX = maxX > 16 ? 16 : maxX;
        const int clampedMinY = minY < 1 ? 1 : minY;
        const int clampedMaxY = maxY > 120 ? 120 : maxY;
        const int clampedMinZ = minZ < 0 ? 0 : minZ;
        const int clampedMaxZ = maxZ > 16 ? 16 : maxZ;

        bool liquid = false;
        for (int ix = clampedMinX; ix < clampedMaxX && !liquid; ++ix)
        {
            for (int iz = clampedMinZ; iz < clampedMaxZ && !liquid; ++iz)
            {
                for (int iy = clampedMaxY; iy >= clampedMinY; --iy)
                {
                    const BlockState* state = primer->getState(ix, iy, iz);
                    if (state->getBlock() == Blocks::flowing_lava || state->getBlock() == Blocks::lava)
                    {
                        liquid = true;
                        break;
                    }
                }
            }
        }

        if (liquid)
        {
            return;
        }

        for (int ix = clampedMinX; ix < clampedMaxX; ++ix)
        {
            const double dx = ((double)(ix + chunkX * 16) + 0.5 - x) / radius;
            if (dx * dx >= 1.0)
            {
                continue;
            }

            for (int iz = clampedMinZ; iz < clampedMaxZ; ++iz)
            {
                const double dz = ((double)(iz + chunkZ * 16) + 0.5 - z) / radius;
                if (dx * dx + dz * dz >= 1.0)
                {
                    continue;
                }

                for (int iy = clampedMaxY; iy > clampedMinY; --iy)
                {
                    const double dy = ((double)iy + 0.5 - y) / heightRadius;
                    if (dx * dx + dz * dz + dy * dy >= 1.0)
                    {
                        continue;
                    }

                    const BlockState* state = primer->getState(ix, iy, iz);
                    if (state == NULL)
                    {
                        continue;
                    }

                    Block* block = state->getBlock();
                    if (block == Blocks::netherrack || block == Blocks::soul_sand || block == Blocks::gravel)
                    {
                        primer->setState(ix, iy, iz, Blocks::AIR->defaultBlockState());
                    }
                }
            }
        }
    }
    }
}
