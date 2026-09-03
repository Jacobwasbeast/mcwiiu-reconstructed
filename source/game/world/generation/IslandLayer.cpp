#include "game/world/generation/IslandLayer.hpp"

#include "game/core/IntCache.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

IslandLayer::IslandLayer(long long seed)
    : Layer(seed)
{
}

IslandLayer::~IslandLayer()
{
}

arrayWithLength<int> IslandLayer::getArea(int x, int z, int width, int height)
{
    PIXBeginNamedEvent(0.0f, "IslandLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            initRandom((long long)(j + x), (long long)(i + z));

            area.data[j + i * width] = nextRandom(10) == 0 ? 1 : 0;
        }
    }

    if (x > -width && x <= 0 && z > -height && z <= 0)
    {
        area.data[-x + -z * width] = 1;
    }

    PIXEndNamedEvent();

    return area;
}
