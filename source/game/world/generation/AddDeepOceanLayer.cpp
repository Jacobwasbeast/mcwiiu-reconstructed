#include "game/world/generation/AddDeepOceanLayer.hpp"

#include "game/core/IntCache.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

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

    PIXBeginNamedEvent(0.0f, "AddDeepOceanLayer::getArea");

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
                if (centre == 0)
                {
                    centre = 24;
                }
                else if (centre == 40)
                {
                    centre = 41;
                }
                else if (centre == 42)
                {
                    centre = 43;
                }
                else if (centre == 44)
                {
                    centre = 45;
                }
                else if (centre == 46)
                {
                    centre = 47;
                }
            }

            area.data[j + i * width] = centre;
        }
    }

    PIXEndNamedEvent();
    return area;
}
