#include "game/world/biome/BiomeCacheLayer.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/IntCache.hpp"

BiomeCacheLayer::BiomeCacheLayer(int seed,
                                 boost::shared_ptr<Layer> parent,
                                 unsigned int xzSize,
                                 unsigned char biomeScale,
                                 int centreXChunk,
                                 int centreZChunk)
    : Layer(seed)
{
    this->parent = parent;

    biomeCache = arrayWithLength<unsigned char>();

    cacheSize = (xzSize & 0x0fffffff) << 2;

    int scale = (int)pow(2.0, (double)biomeScale);

    cacheX = (centreXChunk * scale * 16 >> 2) - (cacheSize / 2);
    cacheZ = (centreZChunk * scale * 16 >> 2) - (cacheSize / 2);
}

BiomeCacheLayer::~BiomeCacheLayer()
{
    delete[] biomeCache.data;
}

arrayWithLength<int> BiomeCacheLayer::getArea(int x, int z, int width, int height)
{
    if (biomeCache.data == NULL)
    {
        biomeCache = arrayWithLength<unsigned char>(cacheSize * cacheSize, true);

        arrayWithLength<int> parentArea = parent->getArea(cacheX, cacheZ, cacheSize, cacheSize);

        for (unsigned int i = 0; i < biomeCache.length; i++)
        {
            biomeCache[i] = (unsigned char)parentArea[i];
        }
    }

    arrayWithLength<int> area = IntCache::allocate(width * height);

    x -= cacheX;
    z -= cacheZ;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int cacheColumn = std::max(0, std::min(cacheSize - 1, x + j));
            int cacheRow = std::max(0, std::min(cacheSize - 1, z + i));

            area[j + i * width] = biomeCache[cacheColumn + cacheRow * cacheSize];
        }
    }

    return area;
}
