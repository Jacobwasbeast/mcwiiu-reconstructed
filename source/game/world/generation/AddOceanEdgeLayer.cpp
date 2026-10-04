#include "game/world/generation/AddOceanEdgeLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/Biome.hpp"

AddOceanEdgeLayer::AddOceanEdgeLayer(long long seed, boost::shared_ptr<Layer> parent, bool useOceanTemperatures)
    : Layer(seed)
{
    this->parent = parent;
    (void)useOceanTemperatures;
}

arrayWithLength<int> AddOceanEdgeLayer::getArea(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    int parentHeight = height + 2;

    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "AddOceanEdgeLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int row = 0; row < height; row++)
    {
        for (int column = 0; column < width; column++)
        {
            initRandom((long long)(x + column), (long long)(z + row));

            int north = parentArea.data[column + 1 + row * parentWidth];
            int east = parentArea.data[column + 2 + (row + 1) * parentWidth];
            int west = parentArea.data[column + (row + 1) * parentWidth];
            int south = parentArea.data[column + 1 + (row + 2) * parentWidth];
            int centre = parentArea.data[column + 1 + (row + 1) * parentWidth];

            if (centre == Biome::eBiomeIDs_WarmOcean)
            {
                if (north == Biome::eBiomeIDs_FrozenOcean || east == Biome::eBiomeIDs_FrozenOcean || west == Biome::eBiomeIDs_FrozenOcean || south == Biome::eBiomeIDs_FrozenOcean)
                {
                    centre = Biome::eBiomeIDs_Ocean;
                }
            }
            else if (centre == Biome::eBiomeIDs_FrozenOcean)
            {
                if (north == Biome::eBiomeIDs_WarmOcean || east == Biome::eBiomeIDs_WarmOcean || west == Biome::eBiomeIDs_WarmOcean || south == Biome::eBiomeIDs_WarmOcean)
                {
                    centre = Biome::eBiomeIDs_Ocean;
                }
            }

            area.data[column + row * width] = centre;
        }
    }

    PIXEndNamedEvent();

    return area;
}
