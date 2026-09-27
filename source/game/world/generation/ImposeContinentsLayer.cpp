#include "game/world/generation/ImposeContinentsLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

static const int CONTINENT_MAP_SIZE = 10;

static const int CONTINENT_MAP[CONTINENT_MAP_SIZE][CONTINENT_MAP_SIZE] =
{
    { 1, 1, 1, 1, 0, 0, 0, 1, 1, 1 },
    { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1 },
    { 1, 1, 0, 1, 1, 1, 1, 1, 0, 1 },
    { 1, 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 1, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 0, 1, 1, 1, 1, 1, 0, 1 },
    { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1 }
};

ImposeContinentsLayer::ImposeContinentsLayer(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

ImposeContinentsLayer::~ImposeContinentsLayer()
{
}

arrayWithLength<int> ImposeContinentsLayer::getArea(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    int parentHeight = height + 2;

    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "ImposeContinentsLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int biomeId = parentArea.data[j + 1 + (i + 1) * parentWidth];

            int worldX = j + x;
            int worldZ = i + z;

            if (worldX != 0 || worldZ != 0)
            {
                int mapX = (worldX + (int)baseSeed) % CONTINENT_MAP_SIZE;
                int mapZ = (worldZ + (int)baseSeed) % CONTINENT_MAP_SIZE;

                if (mapX < 0)
                {
                    mapX += CONTINENT_MAP_SIZE;
                }

                if (mapZ < 0)
                {
                    mapZ += CONTINENT_MAP_SIZE;
                }

                if (CONTINENT_MAP[mapZ][mapX] == 0)
                {
                    biomeId = 0;
                }
            }

            area.data[j + i * width] = biomeId;
        }
    }

    PIXEndNamedEvent();

    return area;
}
