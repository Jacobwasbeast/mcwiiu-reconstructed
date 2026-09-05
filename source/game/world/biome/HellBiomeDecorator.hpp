#ifndef GAME_WORLD_BIOME_HELLBIOMEDECORATOR_HPP
#define GAME_WORLD_BIOME_HELLBIOMEDECORATOR_HPP

class Biome;
class BlockPos;
class Level;
class Random;

class HellBiomeDecorator
{
public:
    virtual ~HellBiomeDecorator();

    virtual void decorate(Level* level, Random* random, Biome* biome, const BlockPos& origin);
};

#endif
