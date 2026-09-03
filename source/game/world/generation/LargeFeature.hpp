#ifndef GAME_WORLD_GENERATION_LARGEFEATURE_HPP
#define GAME_WORLD_GENERATION_LARGEFEATURE_HPP

#include "game/util/Random.hpp"

class ChunkPrimer;
class Level;

class LargeFeature
{
public:
    LargeFeature();
    virtual ~LargeFeature();

    virtual void apply(Level* level, int chunkX, int chunkZ, ChunkPrimer* primer);
    virtual void addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ, ChunkPrimer* primer);

protected:
    static long long setupChunkSeed(long long baseSeed, Random& random, int chunkX, int chunkZ);

    int range;
    Random random;
    Level* level;
};

#endif
