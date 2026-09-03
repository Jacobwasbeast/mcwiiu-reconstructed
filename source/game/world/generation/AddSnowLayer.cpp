#include "game/world/generation/AddSnowLayer.hpp"

#include "game/core/IntCache.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

AddSnowLayer::AddSnowLayer(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

AddSnowLayer::~AddSnowLayer()
{
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
        for (int j = 0; j < width - 1; j += 2)
        {
            int first = parentArea.data[j + 1 + (i + 1) * parentWidth];
            initRandom((long long)(x + j), (long long)(z + i));

            if (!isShallowOcean(first))
            {
                int randomValue = nextRandom(6);
                if (randomValue == 0)
                {
                    first = 4;
                }
                else if (randomValue == 1)
                {
                    first = 3;
                }
                else
                {
                    first = 1;
                }
            }

            int second = parentArea.data[j + 2 + (i + 1) * parentWidth];
            initRandom((long long)(x + j + 1), (long long)(z + i));

            if (!isShallowOcean(second))
            {
                int randomValue = nextRandom(6);
                if (randomValue == 0)
                {
                    second = 4;
                }
                else if (randomValue == 1)
                {
                    second = 3;
                }
                else
                {
                    second = 1;
                }
            }

            area.data[j + i * width] = first;
            area.data[j + 1 + i * width] = second;
        }

        if ((width & 1) != 0)
        {
            int last = parentArea.data[width + (i + 1) * parentWidth];
            initRandom((long long)(x + width - 1), (long long)(z + i));

            if (!isShallowOcean(last))
            {
                int randomValue = nextRandom(6);
                if (randomValue == 0)
                {
                    last = 4;
                }
                else if (randomValue == 1)
                {
                    last = 3;
                }
                else
                {
                    last = 1;
                }
            }

            area.data[width - 1 + i * width] = last;
        }
    }

    PIXEndNamedEvent();

    return area;
}
