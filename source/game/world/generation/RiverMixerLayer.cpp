#include "game/world/generation/RiverMixerLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/Biome.hpp"

RiverMixerLayer::RiverMixerLayer(long long seed,
                                 boost::shared_ptr<Layer> biomeLayer,
                                 boost::shared_ptr<Layer> riverLayer)
    : Layer(seed)
{
    this->biomeLayer = biomeLayer;
    this->riverLayer = riverLayer;
}

void RiverMixerLayer::init(long long seed)
{
    biomeLayer->init(seed);
    riverLayer->init(seed);
    Layer::init(seed);
}

arrayWithLength<int> RiverMixerLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> biomes = biomeLayer->getArea(x, z, width, height);
    arrayWithLength<int> rivers = riverLayer->getArea(x, z, width, height);

    PIXBeginNamedEvent(0.0f, "RiverMixerLayer::getArea");

    arrayWithLength<int> result = IntCache::allocate(width * height);

    for (int index = 0; index < width * height; ++index)
    {
        int biome = biomes.data[index];
        int river = rivers.data[index];

        if (!Layer::isOcean(biome) && river == Biome::RIVER->id)
        {
            if (biome == Biome::ICE_PLAINS->id)
            {
                result.data[index] = Biome::FROZEN_RIVER->id;
            }
            else if (biome != Biome::MUSHROOM_ISLAND->id && biome != Biome::MUSHROOM_ISLAND_SHORE->id)
            {
                result.data[index] = river & 255;
            }
            else
            {
                result.data[index] = biome;
            }
        }
        else
        {
            result.data[index] = biome;
        }
    }

    PIXEndNamedEvent();
    return result;
}
