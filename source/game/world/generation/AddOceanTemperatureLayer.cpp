#include "game/world/generation/AddOceanTemperatureLayer.hpp"

#include "game/core/IntCache.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

AddOceanTemperatureLayer::AddOceanTemperatureLayer(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

AddOceanTemperatureLayer::~AddOceanTemperatureLayer()
{
}

arrayWithLength<int> AddOceanTemperatureLayer::getArea(int x, int z, int width, int height)
{
    PIXBeginNamedEvent(0.0f, "AddOceanTemperatureLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            initRandom((long long)(j + x), (long long)(i + z));

            float temperature = (float)nextRandom(100) / 100.0f;
            if (temperature < 0.075f)
            {
                area.data[j + i * width] = 40;
            }
            else if (temperature < 0.4f)
            {
                area.data[j + i * width] = 42;
            }
            else if (temperature < 0.675f)
            {
                area.data[j + i * width] = 0;
            }
            else if (temperature < 0.95f)
            {
                area.data[j + i * width] = 44;
            }
            else
            {
                area.data[j + i * width] = 46;
            }
        }
    }

    PIXEndNamedEvent();
    return area;
}
