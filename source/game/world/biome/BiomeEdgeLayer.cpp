#include "game/world/biome/BiomeEdgeLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/core/PIXNamedEvents.hpp"

BiomeEdgeLayer::BiomeEdgeLayer(long long seed, boost::shared_ptr<Layer> parent)
    : Layer(seed)
{
    this->parent = parent;
}

BiomeEdgeLayer::~BiomeEdgeLayer()
{
}

bool BiomeEdgeLayer::isValidTemperatureEdge(int firstBiomeId, int secondBiomeId)
{
    if (Layer::isSame(firstBiomeId, secondBiomeId))
    {
        return true;
    }

    Biome* firstBiome = Biome::getBiome(firstBiomeId);
    Biome* secondBiome = Biome::getBiome(secondBiomeId);

    if (firstBiome == NULL || secondBiome == NULL)
    {
        return false;
    }

    Biome::TemperatureCategory firstCategory = firstBiome->getTemperatureCategory();
    Biome::TemperatureCategory secondCategory = secondBiome->getTemperatureCategory();

    return firstCategory == secondCategory
        || firstCategory == Biome::eTemperatureCategory_Medium
        || secondCategory == Biome::eTemperatureCategory_Medium;
}

bool BiomeEdgeLayer::checkEdge(arrayWithLength<int> parentArea,
                               arrayWithLength<int> area,
                               int x,
                               int z,
                               int width,
                               int centre,
                               int edge,
                               int replacement)
{
    if (!Layer::isSame(centre, edge))
    {
        return false;
    }

    int parentWidth = width + 2;
    int row = x + (z + 1) * parentWidth;
    int west = parentArea[row];
    int south = parentArea[x + (z + 2) * parentWidth + 1];
    int east = parentArea[row + 2];

    if (isValidTemperatureEdge(parentArea[x + z * parentWidth + 1], edge)
        && isValidTemperatureEdge(east, edge)
        && isValidTemperatureEdge(west, edge)
        && isValidTemperatureEdge(south, edge))
    {
        area[x + z * width] = centre;
    }
    else
    {
        area[x + z * width] = replacement;
    }

    return true;
}

bool BiomeEdgeLayer::checkEdgeStrict(arrayWithLength<int> parentArea,
                                     arrayWithLength<int> area,
                                     int x,
                                     int z,
                                     int width,
                                     int centre,
                                     int edge,
                                     int replacement)
{
    if (centre == edge)
    {
        int parentWidth = width + 2;
        int row = x + (z + 1) * parentWidth;
        int west = parentArea[row];
        int east = parentArea[row + 2];
        int south = parentArea[x + (z + 2) * parentWidth + 1];

        if (Layer::isSame(parentArea[x + z * parentWidth + 1], edge)
            && Layer::isSame(east, edge)
            && Layer::isSame(west, edge)
            && Layer::isSame(south, edge))
        {
            area[x + z * width] = centre;
        }
        else
        {
            area[x + z * width] = replacement;
        }

        return true;
    }

    return false;
}

arrayWithLength<int> BiomeEdgeLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, width + 2, height + 2);

    PIXBeginNamedEvent(0.0f, "BiomeEdgeLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);
    int parentWidth = width + 2;

    for (int localZ = 0; localZ < height; localZ++)
    {
        for (int localX = 0; localX < width; localX++)
        {
            initRandom((long long)(x + localX), (long long)(z + localZ));

            int centre = parentArea.data[localX + 1 + (localZ + 1) * parentWidth];

            if (checkEdge(parentArea,
                          area,
                          localX,
                          localZ,
                          width,
                          centre,
                          Biome::EXTREME_HILLS->id,
                          Biome::EXTREME_HILLS_EDGE->id))
            {
                continue;
            }

            if (checkEdgeStrict(parentArea,
                                area,
                                localX,
                                localZ,
                                width,
                                centre,
                                Biome::MESA_ROCK->id,
                                Biome::MESA->id))
            {
                continue;
            }

            if (checkEdgeStrict(parentArea,
                                area,
                                localX,
                                localZ,
                                width,
                                centre,
                                Biome::MESA_CLEAR_ROCK->id,
                                Biome::MESA->id))
            {
                continue;
            }

            if (checkEdgeStrict(parentArea,
                                area,
                                localX,
                                localZ,
                                width,
                                centre,
                                Biome::MEGA_TAIGA->id,
                                Biome::TAIGA->id))
            {
                continue;
            }

            int north = parentArea.data[localX + 1 + localZ * parentWidth];
            int south = parentArea.data[localX + 1 + (localZ + 2) * parentWidth];
            int east = parentArea.data[localX + 2 + (localZ + 1) * parentWidth];
            int west = parentArea.data[localX + (localZ + 1) * parentWidth];

            if (centre == Biome::DESERT->id)
            {
                if (north == Biome::ICE_PLAINS->id
                    || east == Biome::ICE_PLAINS->id
                    || west == Biome::ICE_PLAINS->id
                    || south == Biome::ICE_PLAINS->id)
                {
                    area.data[localX + localZ * width] = Biome::EXTREME_HILLS_PLUS_TREES->id;
                }
                else
                {
                    area.data[localX + localZ * width] = centre;
                }
            }
            else if (centre == Biome::SWAMPLAND->id)
            {
                if (north == Biome::DESERT->id
                    || east == Biome::DESERT->id
                    || west == Biome::DESERT->id
                    || south == Biome::DESERT->id
                    || north == Biome::COLD_TAIGA->id
                    || east == Biome::COLD_TAIGA->id
                    || west == Biome::COLD_TAIGA->id
                    || south == Biome::COLD_TAIGA->id
                    || north == Biome::ICE_PLAINS->id
                    || east == Biome::ICE_PLAINS->id
                    || west == Biome::ICE_PLAINS->id
                    || south == Biome::ICE_PLAINS->id)
                {
                    area.data[localX + localZ * width] = Biome::PLAINS->id;
                }
                else if (north == Biome::JUNGLE->id
                         || south == Biome::JUNGLE->id
                         || east == Biome::JUNGLE->id
                         || west == Biome::JUNGLE->id)
                {
                    area.data[localX + localZ * width] = Biome::JUNGLE_EDGE->id;
                }
                else
                {
                    area.data[localX + localZ * width] = centre;
                }
            }
            else
            {
                area.data[localX + localZ * width] = centre;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
