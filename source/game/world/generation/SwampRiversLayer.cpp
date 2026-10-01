#include "game/world/generation/SwampRiversLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/world/biome/Biome.hpp"

SwampRiversLayer::SwampRiversLayer(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

arrayWithLength<int> SwampRiversLayer::getArea(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    int parentHeight = height + 2;

    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, parentWidth, parentHeight);

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            initRandom((long long)(x + j), (long long)(z + i));

            int centre = parentArea.data[j + 1 + (i + 1) * parentWidth];

            if (centre == Biome::SWAMPLAND->id && nextRandom(6) == 0)
            {
                area.data[j + i * width] = Biome::RIVER->id;
            }
            else if ((centre == Biome::JUNGLE->id || centre == Biome::JUNGLE_HILLS->id)
                     && nextRandom(8) == 0)
            {
                area.data[j + i * width] = Biome::RIVER->id;
            }
            else
            {
                area.data[j + i * width] = centre;
            }
        }
    }

    return area;
}
