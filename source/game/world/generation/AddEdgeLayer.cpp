#include "game/world/generation/AddEdgeLayer.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

AddEdgeLayer::AddEdgeLayer(long long seed, boost::shared_ptr<Layer> parentLayer, Mode mode)
    : Layer(seed), mMode(mode)
{
    parent = parentLayer;
}

AddEdgeLayer::~AddEdgeLayer()
{
}

arrayWithLength<int> AddEdgeLayer::getArea(int x, int z, int width, int height)
{
    if (mMode == HEAT_ICE)
    {
        return heatIce(x, z, width, height);
    }
    if (mMode != SPECIAL)
    {
        return coolWarm(x, z, width, height);
    }
    return introduceSpecial(x, z, width, height);
}

arrayWithLength<int> AddEdgeLayer::coolWarm(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    arrayWithLength<int> source = parent->getArea(x - 1, z - 1, parentWidth, height + 2);
    PIXBeginNamedEvent(0.0f, "AddEdgeLayer::getArea");
    arrayWithLength<int> result = IntCache::allocate(width * height);
    for (int row = 0; row < height; ++row)
    {
        for (int column = 0; column < width; ++column)
        {
            initRandom((long long)(x + column), (long long)(z + row));
            int north = source[column + 1 + row * parentWidth];
            int west = source[column + (row + 1) * parentWidth];
            int center = source[column + 1 + (row + 1) * parentWidth];
            int east = source[column + 2 + (row + 1) * parentWidth];
            int south = source[column + 1 + (row + 2) * parentWidth];
            if (center == 1)
            {
                bool nearCold = north == 3 || east == 3 || west == 3 || south == 3;
                bool nearIce = north == 4 || east == 4 || west == 4 || south == 4;
                if (nearCold || nearIce)
                {
                    center = 2;
                }
            }
            result[column + row * width] = center;
        }
    }
    PIXEndNamedEvent();
    return result;
}

arrayWithLength<int> AddEdgeLayer::heatIce(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    arrayWithLength<int> source = parent->getArea(x - 1, z - 1, parentWidth, height + 2);
    arrayWithLength<int> result = IntCache::allocate(width * height);
    for (int row = 0; row < height; ++row)
    {
        for (int column = 0; column < width; ++column)
        {
            int north = source[column + 1 + row * parentWidth];
            int west = source[column + (row + 1) * parentWidth];
            int center = source[column + 1 + (row + 1) * parentWidth];
            int east = source[column + 2 + (row + 1) * parentWidth];
            int south = source[column + 1 + (row + 2) * parentWidth];
            if (center == 4)
            {
                bool nearTemperate = north == 2 || east == 2 || west == 2 || south == 2;
                if (north == 1 || east == 1 || west == 1 || south == 1 || nearTemperate)
                {
                    center = 3;
                }
            }
            result[column + row * width] = center;
        }
    }
    return result;
}

arrayWithLength<int> AddEdgeLayer::introduceSpecial(int x, int z, int width, int height)
{
    arrayWithLength<int> source = parent->getArea(x, z, width, height);
    arrayWithLength<int> result = IntCache::allocate(width * height);
    for (int row = 0; row < height; ++row)
    {
        for (int column = 0; column < width; ++column)
        {
            initRandom((long long)(x + column), (long long)(z + row));
            int index = column + row * width;
            int value = source.data[index];
            if (!isShallowOcean(value) && nextRandom(13) == 0)
            {
                value |= (nextRandom(15) + 1) << 8 & 0xf00;
            }
            result.data[index] = value;
        }
    }
    return result;
}
