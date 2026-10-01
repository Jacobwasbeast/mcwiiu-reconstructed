#include "game/world/generation/RiverMixerLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

RiverMixerLayer::RiverMixerLayer(long long seed,
                                 boost::shared_ptr<Layer> biomeLayer,
                                 boost::shared_ptr<Layer> riverLayer)
    : Layer(seed), biomeLayer(biomeLayer), riverLayer(riverLayer)
{
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

        if (!Layer::isOcean(biome) && river == 7)
        {
            if (biome == 12)
            {
                result.data[index] = 11;
            }
            else if (biome != 14 && biome != 15)
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
