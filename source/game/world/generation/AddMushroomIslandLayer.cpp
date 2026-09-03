#include "game/world/generation/AddMushroomIslandLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/world/biome/Biome.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

AddMushroomIslandLayer::AddMushroomIslandLayer(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

AddMushroomIslandLayer::~AddMushroomIslandLayer()
{
}

arrayWithLength<int> AddMushroomIslandLayer::getArea(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    int parentHeight = height + 2;

    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "AddMushroomIslandLayer::getArea");

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

            initRandom((long long)(x + j), (long long)(z + i));

            if (isShallowOcean(centre) && isShallowOcean(northWest) && isShallowOcean(northEast)
                && isShallowOcean(southWest) && isShallowOcean(southEast) && nextRandom(100) == 0)
            {
                area.data[j + i * width] = Biome::MUSHROOM_ISLAND->id;
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
