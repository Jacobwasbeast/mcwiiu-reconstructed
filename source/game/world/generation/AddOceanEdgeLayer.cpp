#include "game/world/generation/AddOceanEdgeLayer.hpp"

#include "game/core/IntCache.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

AddOceanEdgeLayer::AddOceanEdgeLayer(long long seed, boost::shared_ptr<Layer> parent, bool useOceanTemperatures)
    : Layer(seed)
{
    this->parent = parent;
    (void)useOceanTemperatures;
}

AddOceanEdgeLayer::~AddOceanEdgeLayer()
{
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

            if (centre == 40)
            {
                if (north == 46 || east == 46 || west == 46 || south == 46)
                {
                    centre = 0;
                }
            }
            else if (centre == 46)
            {
                if (north == 40 || east == 40 || west == 40 || south == 40)
                {
                    centre = 0;
                }
            }

            area.data[column + row * width] = centre;
        }
    }

    PIXEndNamedEvent();

    return area;
}
