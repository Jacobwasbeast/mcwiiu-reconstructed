#include "game/world/biome/BiomeOverrideLayer.hpp"

#include "game/core/IntCache.hpp"

BiomeOverrideLayer::~BiomeOverrideLayer()
{
}

arrayWithLength<int> BiomeOverrideLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> area = IntCache::allocate(width * height);

    x += overrideWidth >> 1;
    z += overrideHeight >> 1;

    if (x < 0)
    {
        x = 0;
    }

    if (x >= overrideWidth)
    {
        x = overrideWidth - 1;
    }

    if (z < 0)
    {
        z = 0;
    }

    if (z >= overrideHeight)
    {
        z = overrideHeight - 1;
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int sampleX = x + j;
            int sampleZ = z + i;

            if (sampleX >= overrideWidth)
            {
                sampleX = overrideWidth - 1;
            }

            if (sampleZ >= overrideHeight)
            {
                sampleZ = overrideHeight - 1;
            }

            area.data[i * width + j] = overrideBiomes[sampleX + sampleZ * overrideWidth];
        }
    }

    return area;
}
