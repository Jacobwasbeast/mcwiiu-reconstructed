#include "game/world/generation/SmoothLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

SmoothLayer::SmoothLayer(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

SmoothLayer::~SmoothLayer()
{
}

arrayWithLength<int> SmoothLayer::getArea(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    int parentHeight = height + 2;

    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "SmoothLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int west = parentArea.data[j + (i + 1) * parentWidth];
            int east = parentArea.data[j + 2 + (i + 1) * parentWidth];
            int north = parentArea.data[j + 1 + i * parentWidth];
            int south = parentArea.data[j + 1 + (i + 2) * parentWidth];
            int centre = parentArea.data[j + 1 + (i + 1) * parentWidth];

            if (west == east && north == south)
            {
                initRandom(x + j, z + i);

                if (nextRandom(2) == 0)
                {
                    centre = west;
                }
                else
                {
                    centre = north;
                }
            }
            else
            {
                if (west == east)
                {
                    centre = west;
                }

                if (north == south)
                {
                    centre = north;
                }
            }

            area.data[j + i * width] = centre;
        }
    }

    PIXEndNamedEvent();

    return area;
}
