#include "game/world/generation/OceanMixerLayer.hpp"

#include "game/core/IntCache.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

OceanMixerLayer::OceanMixerLayer(long long seed, boost::shared_ptr<Layer> landLayer, boost::shared_ptr<Layer> oceanLayer)
    : Layer(seed),
      landLayer(landLayer),
      oceanLayer(oceanLayer)
{
}

OceanMixerLayer::~OceanMixerLayer()
{
}

void OceanMixerLayer::init(long long seed)
{
    landLayer->init(seed);
    oceanLayer->init(seed);

    Layer::init(seed);
}

arrayWithLength<int> OceanMixerLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> landArea = landLayer->getArea(x, z, width, height);
    arrayWithLength<int> oceanArea = oceanLayer->getArea(x, z, width, height);

    PIXBeginNamedEvent(0.0f, "OceanMixerLayer::getArea");

    int count = width * height;

    arrayWithLength<int> area = IntCache::allocate(count);

    for (int i = 0; i < count; i++)
    {
        if (isOcean(landArea.data[i]) && isOcean(oceanArea.data[i]))
        {
            int oceanBiomeId = oceanArea.data[i];

            if (landArea.data[i] == 24)
            {
                switch (oceanBiomeId)
                {
                case 0:
                    oceanBiomeId = 24;
                    break;

                case 42:
                    oceanBiomeId = 43;
                    break;

                case 44:
                    oceanBiomeId = 45;
                    break;

                case 46:
                    oceanBiomeId = 47;
                    break;
                }
            }

            area.data[i] = oceanBiomeId;
        }
        else
        {
            area.data[i] = landArea.data[i];
        }
    }

    PIXEndNamedEvent();

    return area;
}
