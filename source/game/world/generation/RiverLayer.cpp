#include "game/world/generation/RiverLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/core/PIXNamedEvents.hpp"

RiverLayer::RiverLayer(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

int RiverLayer::riverFilter(int biomeId)
{
    return biomeId >= 2 ? 2 + (biomeId & 1) : biomeId;
}

arrayWithLength<int> RiverLayer::getArea(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    int parentHeight = height + 2;

    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "RiverLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int left = riverFilter(parentArea.data[j + 0 + (i + 1) * parentWidth]);
            int right = riverFilter(parentArea.data[j + 2 + (i + 1) * parentWidth]);
            int upper = riverFilter(parentArea.data[j + 1 + (i + 0) * parentWidth]);
            int lower = riverFilter(parentArea.data[j + 1 + (i + 2) * parentWidth]);
            int centre = riverFilter(parentArea.data[j + 1 + (i + 1) * parentWidth]);

            if (left == centre && upper == centre && right == centre && lower == centre)
            {
                area.data[j + i * width] = -1;
            }
            else
            {
                area.data[j + i * width] = Biome::RIVER->id;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
