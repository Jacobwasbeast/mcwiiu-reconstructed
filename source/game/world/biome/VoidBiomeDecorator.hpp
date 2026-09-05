#ifndef GAME_WORLD_BIOME_VOIDBIOMEDECORATOR_HPP
#define GAME_WORLD_BIOME_VOIDBIOMEDECORATOR_HPP

class Biome;
class BlockPos;
class Level;
class Random;

class VoidBiomeDecorator
{
public:
    virtual ~VoidBiomeDecorator();

    virtual void decorate(Level* level, Random* random, Biome* biome, const BlockPos& origin);
};

#endif
