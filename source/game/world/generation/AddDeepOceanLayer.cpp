#include "game/world/generation/AddDeepOceanLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/Biome.hpp"

AddDeepOceanLayer::AddDeepOceanLayer(long long seed, boost::shared_ptr<Layer> parentLayer)
    : Layer(seed)
{
    parent = parentLayer;
}

AddDeepOceanLayer::~AddDeepOceanLayer()
{
}

arrayWithLength<int> AddDeepOceanLayer::getArea(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    int parentHeight = height + 2;
    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "AddDeepOceanLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int north = parentArea.data[j + 1 + i * parentWidth];
            int east = parentArea.data[j + 2 + (i + 1) * parentWidth];
            int west = parentArea.data[j + (i + 1) * parentWidth];
            int south = parentArea.data[j + 1 + (i + 2) * parentWidth];
            int centre = parentArea.data[j + 1 + (i + 1) * parentWidth];

            unsigned char shallowNeighbours = isShallowOcean(north) ? 1 : 0;
            if (isShallowOcean(east))
            {
                shallowNeighbours++;
            }
            if (isShallowOcean(west))
            {
                shallowNeighbours++;
            }
            if (isShallowOcean(south))
            {
                shallowNeighbours++;
            }

            if (isShallowOcean(centre) && shallowNeighbours > 3)
            {
                if (centre == Biome::eBiomeIDs_Ocean)
                {
                    centre = Biome::eBiomeIDs_DeepOcean;
                }
                else if (centre == Biome::eBiomeIDs_WarmOcean)
                {
                    centre = Biome::eBiomeIDs_DeepWarmOcean;
                }
                else if (centre == Biome::eBiomeIDs_LukewarmOcean)
                {
                    centre = Biome::eBiomeIDs_DeepLukewarmOcean;
                }
                else if (centre == Biome::eBiomeIDs_ColdOcean)
                {
                    centre = Biome::eBiomeIDs_DeepColdOcean;
                }
                else if (centre == Biome::eBiomeIDs_FrozenOcean)
                {
                    centre = Biome::eBiomeIDs_DeepFrozenOcean;
                }
            }

            area.data[j + i * width] = centre;
        }
    }

    PIXEndNamedEvent();

    return area;
}
