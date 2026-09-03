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
    : Layer(seed),
      biomeCache(NULL),
      cacheLength(0)
{
    this->parent = parent;

    biomeCache = NULL;
    cacheLength = 0;

    cacheSize = xzSize << 2;

    int scale = (int)pow(2.0, (double)biomeScale);

    cacheX = (centreXChunk * scale * 16 >> 2) - (cacheSize >> 1);
    cacheZ = (centreZChunk * scale * 16 >> 2) - (cacheSize >> 1);
}

BiomeCacheLayer::~BiomeCacheLayer()
{
    delete[] biomeCache;
}

arrayWithLength<int> BiomeCacheLayer::getArea(int x, int z, int width, int height)
{
    if (biomeCache == NULL)
    {
        unsigned int count = cacheSize * cacheSize;

        unsigned char* biomes = new unsigned char[count];

        for (unsigned int i = 0; i < count; i++)
        {
            biomes[i] = 0;
        }

        biomeCache = biomes;
        cacheLength = count;

        arrayWithLength<int> parentArea = parent->getArea(cacheX, cacheZ, cacheSize, cacheSize);

        for (unsigned int i = 0; i < cacheLength; i++)
        {
            biomeCache[i] = (unsigned char)parentArea.data[i];
        }
    }

    arrayWithLength<int> area = IntCache::allocate(width * height);

    x -= cacheX;
    z -= cacheZ;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int cacheColumn = std::max(std::min(cacheSize - 1, x + j), 0);
            int cacheRow = std::max(std::min(cacheSize - 1, z + i), 0);

            area.data[j + i * width] = biomeCache[cacheColumn + cacheRow * cacheSize];
        }
    }

    return area;
}
