#ifndef GAME_CORE_LAYERSIZE_HPP
#define GAME_CORE_LAYERSIZE_HPP

class Random;

class LayerSize
{
public:
    virtual ~LayerSize() {}

    virtual int GetLayerSize(Random& random, double blend) const = 0;
};

#endif
