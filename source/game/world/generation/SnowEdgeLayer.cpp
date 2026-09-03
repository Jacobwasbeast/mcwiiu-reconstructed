#include "game/world/generation/SnowEdgeLayer.hpp"

#include "game/core/IntCache.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

SnowEdgeLayer::SnowEdgeLayer(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

SnowEdgeLayer::~SnowEdgeLayer()
{
}

arrayWithLength<int> SnowEdgeLayer::getArea(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    int parentHeight = height + 2;

    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "SnowEdgeLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            initRandom((long long)(j + x), (long long)(i + z));

            int centre = parentArea.data[j + 1 + (i + 1) * parentWidth];

            if (centre == 1)
            {
                int north = parentArea.data[j + 1 + i * parentWidth];
                int east = parentArea.data[j + 2 + (i + 1) * parentWidth];
                int west = parentArea.data[j + (i + 1) * parentWidth];
                int south = parentArea.data[j + 1 + (i + 2) * parentWidth];

                if (north == 4 || east == 4 || west == 4 || south == 4)
                {
                    centre = 3;
                }
            }

            area.data[j + i * width] = centre;
        }
    }

    PIXEndNamedEvent();

    return area;
}
