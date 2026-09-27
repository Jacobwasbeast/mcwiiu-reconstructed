#include "game/world/generation/RemoveTooMuchOceanLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

RemoveTooMuchOceanLayer::RemoveTooMuchOceanLayer(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

RemoveTooMuchOceanLayer::~RemoveTooMuchOceanLayer()
{
}

arrayWithLength<int> RemoveTooMuchOceanLayer::getArea(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    int parentHeight = height + 2;

    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "RemoveTooMuchOceanLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int north = parentArea.data[j + 1 + i * parentWidth];
            int east = parentArea.data[j + 2 + (i + 1) * parentWidth];
            int west = parentArea.data[j + (i + 1) * parentWidth];
            int south = parentArea.data[j + 1 + (i + 2) * parentWidth];
            int centre = parentArea.data[j + 1 + (i + 1) * parentWidth];

            area.data[j + i * width] = centre;

            initRandom(x + j, z + i);

            if (isShallowOcean(centre) && isShallowOcean(north) && isShallowOcean(east)
                && isShallowOcean(west) && isShallowOcean(south) && nextRandom(2) == 0)
            {
                area.data[j + i * width] = 1;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
