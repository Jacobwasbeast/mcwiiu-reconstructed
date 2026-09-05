#ifndef GAME_WORLD_GENERATION_CANYONFEATURE_HPP
#define GAME_WORLD_GENERATION_CANYONFEATURE_HPP

#include "game/world/generation/LargeFeature.hpp"

class ChunkPrimer;
class Level;

class CanyonFeature : public LargeFeature
{
public:
    virtual ~CanyonFeature();

    void addTunnel(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer,
                   double x, double y, double z, float width, float yaw, float pitch,
                   int start, int length, double scale);

    virtual void addFeature(Level* level, int chunkX, int chunkZ, int originChunkX,
                            int originChunkZ, ChunkPrimer* primer);
};

#endif
