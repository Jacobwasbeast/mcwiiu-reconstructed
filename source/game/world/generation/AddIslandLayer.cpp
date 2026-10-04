#include "game/world/generation/AddIslandLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

AddIslandLayer::AddIslandLayer(long long seed, boost::shared_ptr<Layer> parentLayer)
    : Layer(seed)
{
    parent = parentLayer;
}

arrayWithLength<int> AddIslandLayer::getArea(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    int parentHeight = height + 2;
    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "AddIslandLayer::getArea");
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

            initRandom((long long)(x + j), (long long)(z + i));

            if (!isShallowOcean(centre)
                || (isShallowOcean(northWest) && isShallowOcean(northEast)
                    && isShallowOcean(southWest) && isShallowOcean(southEast)))
            {
                if (isShallowOcean(centre)
                    || (!isShallowOcean(northWest) && !isShallowOcean(northEast)
                        && !isShallowOcean(southWest) && !isShallowOcean(southEast))
                    || nextRandom(5) != 0)
                {
                    area.data[j + i * width] = centre;
                }
                else if (isShallowOcean(northWest))
                {
                    if (centre == 4)
                    {
                        area.data[j + i * width] = centre;
                    }
                    else
                    {
                        area.data[j + i * width] = northWest;
                    }
                }
                else if (isShallowOcean(northEast))
                {
                    if (centre == 4)
                    {
                        area.data[j + i * width] = centre;
                    }
                    else
                    {
                        area.data[j + i * width] = northEast;
                    }
                }
                else if (isShallowOcean(southWest))
                {
                    if (centre == 4)
                    {
                        area.data[j + i * width] = centre;
                    }
                    else
                    {
                        area.data[j + i * width] = southWest;
                    }
                }
                else if (isShallowOcean(southEast))
                {
                    if (centre == 4)
                    {
                        area.data[j + i * width] = centre;
                    }
                    else
                    {
                        area.data[j + i * width] = southEast;
                    }
                }
            }
            else
            {
                int selected = 1;
                int nonOceanCount = 1;

                if (!isShallowOcean(northWest))
                {
                    nonOceanCount = 2;
                    if (nextRandom(1) == 0)
                    {
                        selected = northWest;
                    }
                }

                if (!isShallowOcean(northEast))
                {
                    if (nextRandom(nonOceanCount) == 0)
                    {
                        selected = northEast;
                    }
                    nonOceanCount++;
                }

                if (!isShallowOcean(southWest))
                {
                    if (nextRandom(nonOceanCount) == 0)
                    {
                        selected = southWest;
                    }
                    nonOceanCount++;
                }

                if (!isShallowOcean(southEast) && nextRandom(nonOceanCount) == 0)
                {
                    selected = southEast;
                }

                if (nextRandom(3) == 0 || selected == 4)
                {
                    area.data[j + i * width] = selected;
                }
                else
                {
                    area.data[j + i * width] = 0;
                }
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
