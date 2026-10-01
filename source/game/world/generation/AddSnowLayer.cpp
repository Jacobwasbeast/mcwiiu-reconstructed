#include "game/world/generation/AddSnowLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

AddSnowLayer::AddSnowLayer(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

arrayWithLength<int> AddSnowLayer::getArea(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    int parentHeight = height + 2;

    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "AddSnowLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int centre = parentArea.data[j + 1 + (i + 1) * parentWidth];
            initRandom((long long)(x + j), (long long)(z + i));

            if (!isShallowOcean(centre))
            {
                int temperature = nextRandom(6);
                if (temperature == 0)
                {
                    centre = 4;
                }
                else if (temperature <= 1)
                {
                    centre = 3;
                }
                else
                {
                    centre = 1;
                }
            }

            area.data[j + i * width] = centre;
        }
    }

    PIXEndNamedEvent();

    return area;
}
