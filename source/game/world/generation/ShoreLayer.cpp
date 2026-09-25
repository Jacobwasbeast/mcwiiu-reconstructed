#include "game/world/generation/ShoreLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/MesaBiome.hpp"

ShoreLayer::ShoreLayer(long long seed, boost::shared_ptr<Layer> parentLayer)
    : Layer(seed)
{
    parent = parentLayer;
}

ShoreLayer::~ShoreLayer()
{
}

bool ShoreLayer::isJungleCompatible(int biomeId)
{
    if (Biome::getBiome(biomeId) != NULL
        && Biome::getBiome(biomeId)->getBaseClass() == Biome::eBiomeBaseClass_Jungle)
    {
        return true;
    }

    if ((biomeId < 4 || (biomeId > 5 && (biomeId < 21 || biomeId > 23))) && !isOcean(biomeId))
    {
        return false;
    }

    return true;
}

void ShoreLayer::replaceIfNeighborOcean(arrayWithLength<int> parentArea,
                                        arrayWithLength<int> area, int x, int z,
                                        int width, int centre, int replacement)
{
    if (!isOcean(centre))
    {
        int parentWidth = width + 2;
        int north = parentArea.data[x + 1 + z * parentWidth];
        int east = parentArea.data[x + 2 + (z + 1) * parentWidth];
        int west = parentArea.data[x + (z + 1) * parentWidth];
        int south = parentArea.data[x + 1 + (z + 2) * parentWidth];

        if (isOcean(north) || isOcean(east) || isOcean(west) || isOcean(south))
        {
            area.data[x + z * width] = replacement;
            return;
        }
    }

    area.data[x + z * width] = centre;
}

bool ShoreLayer::isMesa(int biomeId)
{
    Biome* biome = Biome::getBiome(biomeId);
    return biome != NULL && dynamic_cast<MesaBiome*>(biome) != NULL;
}

arrayWithLength<int> ShoreLayer::getArea(int x, int z, int width, int height)
{
    int parentWidth = width + 2;
    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, parentWidth, height + 2);

    PIXBeginNamedEvent(0.0f, "ShoreLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int localZ = 0; localZ < height; localZ++)
    {
        for (int localX = 0; localX < width; localX++)
        {
            initRandom((long long)(localX + x), (long long)(localZ + z));

            int centre = parentArea.data[localX + 1 + (localZ + 1) * parentWidth];
            Biome* biome = Biome::getBiome(centre);

            if (centre == Biome::MUSHROOM_ISLAND->id)
            {
                int north = parentArea.data[localX + 1 + localZ * parentWidth];
                int east = parentArea.data[localX + 2 + (localZ + 1) * parentWidth];
                int west = parentArea.data[localX + (localZ + 1) * parentWidth];
                int south = parentArea.data[localX + 1 + (localZ + 2) * parentWidth];

                if (isShallowOcean(north) || isShallowOcean(east) || isShallowOcean(west) || isShallowOcean(south))
                {
                    area.data[localX + localZ * width] = Biome::MUSHROOM_ISLAND_SHORE->id;
                }
                else
                {
                    area.data[localX + localZ * width] = centre;
                }
            }
            else if (biome != NULL && biome->getBaseClass() == Biome::eBiomeBaseClass_Jungle)
            {
                int north = parentArea.data[localX + 1 + localZ * parentWidth];
                int east = parentArea.data[localX + 2 + (localZ + 1) * parentWidth];
                int west = parentArea.data[localX + (localZ + 1) * parentWidth];
                int south = parentArea.data[localX + 1 + (localZ + 2) * parentWidth];

                if (isJungleCompatible(north) && isJungleCompatible(east)
                    && isJungleCompatible(west) && isJungleCompatible(south))
                {
                    if (isOcean(north) || isOcean(east) || isOcean(west) || isOcean(south))
                    {
                        area.data[localX + localZ * width] = Biome::BEACH->id;
                    }
                    else
                    {
                        area.data[localX + localZ * width] = centre;
                    }
                }
                else
                {
                    area.data[localX + localZ * width] = Biome::JUNGLE_EDGE->id;
                }
            }
            else if (centre == Biome::EXTREME_HILLS->id
                     || centre == Biome::EXTREME_HILLS_PLUS_TREES->id
                     || centre == Biome::EXTREME_HILLS_EDGE->id)
            {
                replaceIfNeighborOcean(parentArea, area, localX, localZ, width, centre, Biome::STONE_BEACH->id);
            }
            else if (biome != NULL && biome->isSnowCovered())
            {
                replaceIfNeighborOcean(parentArea, area, localX, localZ, width, centre, Biome::COLD_BEACH->id);
            }
            else if (centre == Biome::MESA->id || centre == Biome::MESA_ROCK->id)
            {
                int north = parentArea.data[localX + 1 + localZ * parentWidth];
                int east = parentArea.data[localX + 2 + (localZ + 1) * parentWidth];
                int west = parentArea.data[localX + (localZ + 1) * parentWidth];
                int south = parentArea.data[localX + 1 + (localZ + 2) * parentWidth];

                if (!isOcean(north) && !isOcean(east) && !isOcean(west) && !isOcean(south)
                    && !(isMesa(north) && isMesa(east) && isMesa(west) && isMesa(south)))
                {
                    area.data[localX + localZ * width] = Biome::DESERT->id;
                }
                else
                {
                    area.data[localX + localZ * width] = centre;
                }
            }
            else if (!isOcean(centre) && centre != Biome::RIVER->id && centre != Biome::SWAMPLAND->id)
            {
                int north = parentArea.data[localX + 1 + localZ * parentWidth];
                int east = parentArea.data[localX + 2 + (localZ + 1) * parentWidth];
                int west = parentArea.data[localX + (localZ + 1) * parentWidth];
                int south = parentArea.data[localX + 1 + (localZ + 2) * parentWidth];

                if (isOcean(north) || isOcean(east) || isOcean(west) || isOcean(south))
                {
                    area.data[localX + localZ * width] = Biome::BEACH->id;
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
