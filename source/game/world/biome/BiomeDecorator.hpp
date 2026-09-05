#ifndef GAME_WORLD_BIOME_BIOMEDECORATOR_HPP
#define GAME_WORLD_BIOME_BIOMEDECORATOR_HPP

#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Feature.hpp"

class Biome;
class FlowerFeature;
class Level;
class Random;
class CustomizableSourceSettings;

class BiomeDecorator
{
public:
    BiomeDecorator();
    virtual ~BiomeDecorator();
    void decorate(Level* level, Random* random, Biome* biome, const BlockPos& origin);
    void decorate(Biome* biome, Level* level, Random* random);
    void decorateDepthSpan(Level* level, Random* random, int count, Feature* feature, int minY, int maxY);
    void decorateDepthAverage(Level* level, Random* random, int count, Feature* feature, int centreY, int radius);
    void decorateOres(Level* level, Random* random);

    bool generateLakes;
    BlockPos position;
    CustomizableSourceSettings* settings;
    Feature* dirtGen;
    Feature* gravelGen;
    Feature* graniteGen;
    Feature* dioriteGen;
    Feature* andesiteGen;
    Feature* coalGen;
    Feature* ironGen;
    Feature* goldGen;
    Feature* redstoneGen;
    Feature* diamondGen;
    Feature* lapisGen;
    FlowerFeature* flowerGen;
    int sandPerChunk;
    int clayPerChunk;
    int gravelPerChunk;
    int decorationParameter1;
    int decorationParameter2;
    int decorationParameter3;
    int decorationParameter4;
    int decorationParameter5;
    int decorationParameter6;
    int decorationParameter7;
    int decorationParameter8;
    int decorationParameter9;
    int decorationParameter10;
    int decorationParameter11;
    int decorationParameter12;
    int waterlilyPerChunk;
    int treesPerChunk;
    float extraTreeChance;
    int flowersPerChunk;
    int grassPerChunk;
    int deadBushPerChunk;
    int mushroomsPerChunk;
    int reedsPerChunk;
    int cactiPerChunk;
    int decorationParameter13;
    int sandPerChunk2;
    int clayPerChunk2;
    int bigMushroomsPerChunk;
    int icebergsPerChunk;
    int decorationParameter14;
    int decorationParameter15;
    bool generateLava;
};

#endif
