#include "game/world/generation/GrowMushroomIslandLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

GrowMushroomIslandLayer::GrowMushroomIslandLayer(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

GrowMushroomIslandLayer::~GrowMushroomIslandLayer()
{
}

arrayWithLength<int> GrowMushroomIslandLayer::getArea(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, parentWidth, height + 2);

    PIXBeginNamedEvent(0.0f, "GrowMushroomIslandLayer::getArea");

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

            if (northWest == 14 || northEast == 14 || southWest == 14 || southEast == 14)
            {
                area.data[j + i * width] = 14;
            }
            else
            {
                area.data[j + i * width] = centre;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
