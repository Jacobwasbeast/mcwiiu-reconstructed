#include "game/world/generation/AddDeepOceanLayer.hpp"

#include "game/core/IntCache.hpp"
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

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int northWest = parentArea.data[j + i * parentWidth];
            int northEast = parentArea.data[j + 2 + i * parentWidth];
            int southWest = parentArea.data[j + (i + 2) * parentWidth];
            int southEast = parentArea.data[j + 2 + (i + 2) * parentWidth];
            int centre = parentArea.data[j + 1 + (i + 1) * parentWidth];

            if (isShallowOcean(centre)
                && isShallowOcean(northWest) && isShallowOcean(northEast)
                && isShallowOcean(southWest) && isShallowOcean(southEast))
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

    return area;
}
