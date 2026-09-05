#include "game/world/biome/SwampBiome.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PerlinSimplexNoise.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/FossilFeature.hpp"
#include "game/world/generation/SwampTreeFeature.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

static PerlinSimplexNoise* sTemperatureNoise = new PerlinSimplexNoise(new Random(1234LL), 1);

SwampBiome::SwampBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Swamp, properties)
{
    mDecorator->waterlilyPerChunk = 4;
    mDecorator->treesPerChunk = 2;
    mDecorator->flowersPerChunk = 5;
    mDecorator->deadBushPerChunk = 1;
    mDecorator->mushroomsPerChunk = 8;
    mDecorator->reedsPerChunk = 10;
    mDecorator->cactiPerChunk = 0;
    mDecorator->sandPerChunk2 = 0;
    mDecorator->bigMushroomsPerChunk = 1;
    mDecorator->clayPerChunk = 10;
    mSlimes.push_back(new MobSpawnerData(1, 0x5D108000ULL, 1, 1));
    mMonsters.push_back(new MobSpawnerData(5, 0x5D550044ULL, 2, 4));
}

SwampBiome::~SwampBiome()
{
}

AbstractTreeFeature* SwampBiome::getTreeFeature(Random& random)
{
    (void)random;
    return new SwampTreeFeature();
}

unsigned int SwampBiome::getGrassColor(const BlockPos& pos)
{
    double noise = sTemperatureNoise->getValue((double)pos.x * 0.0225, (double)pos.z * 0.0225);
    if (noise < -0.1)
        return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Grass_Swamp1);
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Grass_Swamp2);
}

unsigned int SwampBiome::getFoliageColor(const BlockPos& pos)
{
    (void)pos;
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Foliage_Swampland);
}

FlowerBlock::Flower* SwampBiome::getRandomFlower(Random& random, const BlockPos& pos)
{
    (void)random;
    (void)pos;
    return NULL;
}

void SwampBiome::buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise)
{
    PIXBeginNamedEvent(0.0F, "SwampBiome::buildSurfaceAtDefault");
    double value = sTemperatureNoise->getValue((double)x * 0.25, (double)z * 0.25);
    if (value > 0.0)
    {
        int y = 127;
        int attempts = 32;
        const int localX = x & 15;
        const int localZ = z & 15;
        const int column = (localX << 11) | (localZ << 7);
        const bool water = value < 0.12;
        while (attempts != 0)
        {
            int index = column | y;
            const BlockState* state = primer->getState(localX, y, localZ);
            Block* block = state == NULL ? NULL : state->getBlock();
            if (block == NULL || Block::getId(block) != 1)
            {
                if (y == 62 && block != NULL && Block::getId(block) != 0)
                {
                    primer->setBlockAndData(index, 8, 0);
                    if (water)
                    {
                        primer->setBlockAndData(index + 1, 9, 0);
                        Biome::buildSurfaceAtDefault(level, random, primer, x, z, noise);
                        PIXEndNamedEvent();
                        return;
                    }
                }
                break;
            }
            --y;
            state = primer->getState(localX, y, localZ);
            block = state == NULL ? NULL : state->getBlock();
            if (block == NULL || Block::getId(block) != 1) continue;
            --y;
            state = primer->getState(localX, y, localZ);
            block = state == NULL ? NULL : state->getBlock();
            if (block == NULL || Block::getId(block) != 1) continue;
            --y;
            state = primer->getState(localX, y, localZ);
            block = state == NULL ? NULL : state->getBlock();
            if (block == NULL || Block::getId(block) != 1) continue;
            --attempts;
            y -= 4;
        }
    }
    Biome::buildSurfaceAtDefault(level, random, primer, x, z, noise);
    PIXEndNamedEvent();
}

void SwampBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    Biome::decorate(level, random, pos);
    if (random.nextInt(64) == 0)
    {
        FossilFeature fossil;
        fossil.place(level, random, pos);
    }
}

Biome::EBiomeBaseClass SwampBiome::getBaseClass()
{
    return eBiomeBaseClass_Swamp;
}
