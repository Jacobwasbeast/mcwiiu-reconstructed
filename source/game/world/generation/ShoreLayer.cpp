#include "game/world/generation/ShoreLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/world/biome/Biome.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

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
    Biome* biome = Biome::getBiome(biomeId);

    if (biome != NULL && biome->getBiomeType() == Biome::eBiomeType_Jungle)
        return true;

    if ((biomeId < 4 || (biomeId > 5 && (biomeId < 21 || biomeId > 23)))
        && !Layer::isOcean(biomeId))
        return false;

    return true;
}

void ShoreLayer::replaceIfNeighborOcean(arrayWithLength<int> parentArea,
                                        arrayWithLength<int> area, int x, int z,
                                        int width, int centre, int replacement)
{
    int parentWidth = width + 2;

    if (!Layer::isOcean(centre))
    {
        int north = parentArea.data[x + (z + 1) * parentWidth];
        int south = parentArea.data[x + 1 + (z + 2) * parentWidth];
        int east = parentArea.data[x + 2 + (z + 1) * parentWidth];
        int west = parentArea.data[x + 1 + z * parentWidth];

        if (Layer::isOcean(north) || Layer::isOcean(south)
            || Layer::isOcean(east) || Layer::isOcean(west))
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
    return biome != NULL && biome->getBiomeType() == Biome::eBiomeType_Mesa;
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

            if (centre == Biome::eBiomeIDs_Ocean)
            {
                int north = parentArea.data[localX + 1 + localZ * parentWidth];
                int south = parentArea.data[localX + 1 + (localZ + 2) * parentWidth];
                int east = parentArea.data[localX + 2 + (localZ + 1) * parentWidth];
                int west = parentArea.data[localX + (localZ + 1) * parentWidth];

                if (Layer::isShallowOcean(north) || Layer::isShallowOcean(south)
                    || Layer::isShallowOcean(east) || Layer::isShallowOcean(west))
                    area.data[localX + localZ * width] = Biome::eBiomeIDs_DeepOcean;
                else
                    area.data[localX + localZ * width] = centre;
                continue;
            }

            if (biome != NULL && biome->getBiomeType() == Biome::eBiomeType_Jungle)
            {
                int north = parentArea.data[localX + 1 + localZ * parentWidth];
                int south = parentArea.data[localX + 1 + (localZ + 2) * parentWidth];
                int east = parentArea.data[localX + 2 + (localZ + 1) * parentWidth];
                int west = parentArea.data[localX + (localZ + 1) * parentWidth];

                if (!isJungleCompatible(north) || !isJungleCompatible(south)
                    || !isJungleCompatible(east) || !isJungleCompatible(west))
                    area.data[localX + localZ * width] = Biome::eBiomeIDs_JungleEdge;
                else if (Layer::isOcean(north) || Layer::isOcean(south)
                         || Layer::isOcean(east) || Layer::isOcean(west))
                    area.data[localX + localZ * width] = Biome::eBiomeIDs_Beach;
                else
                    area.data[localX + localZ * width] = centre;
                continue;
            }

            if (centre == Biome::eBiomeIDs_MushroomIsland
                || centre == Biome::eBiomeIDs_MushroomIslandShore)
            {
                replaceIfNeighborOcean(parentArea, area, localX, localZ, width, centre,
                                       Biome::eBiomeIDs_MushroomIslandShore);
                continue;
            }

            if (biome != NULL && biome->isSnowCovered())
            {
                replaceIfNeighborOcean(parentArea, area, localX, localZ, width, centre,
                                       Biome::eBiomeIDs_ColdBeach);
                continue;
            }

            if (centre == Biome::eBiomeIDs_Desert || centre == Biome::eBiomeIDs_Taiga)
            {
                replaceIfNeighborOcean(parentArea, area, localX, localZ, width, centre,
                                       Biome::eBiomeIDs_Beach);
                continue;
            }

            if (centre == Biome::eBiomeIDs_ExtremeHills
                || centre == Biome::eBiomeIDs_ExtremeHillsEdge)
            {
                int north = parentArea.data[localX + 1 + localZ * parentWidth];
                int south = parentArea.data[localX + 1 + (localZ + 2) * parentWidth];
                int east = parentArea.data[localX + 2 + (localZ + 1) * parentWidth];
                int west = parentArea.data[localX + (localZ + 1) * parentWidth];

                if (Layer::isOcean(north) || Layer::isOcean(south)
                    || Layer::isOcean(east) || Layer::isOcean(west))
                    area.data[localX + localZ * width] = Biome::eBiomeIDs_StoneBeach;
                else
                    area.data[localX + localZ * width] = centre;
                continue;
            }

            if (centre == Biome::eBiomeIDs_Mesa || centre == Biome::eBiomeIDs_MesaRock)
            {
                int north = parentArea.data[localX + 1 + localZ * parentWidth];
                int south = parentArea.data[localX + 1 + (localZ + 2) * parentWidth];
                int east = parentArea.data[localX + 2 + (localZ + 1) * parentWidth];
                int west = parentArea.data[localX + (localZ + 1) * parentWidth];

                if (Layer::isOcean(north) || Layer::isOcean(south)
                    || Layer::isOcean(east) || Layer::isOcean(west))
                {
                    area.data[localX + localZ * width] = Biome::eBiomeIDs_Beach;
                }
                else if (isMesa(north) && isMesa(south) && isMesa(east) && isMesa(west))
                {
                    area.data[localX + localZ * width] = centre;
                }
                else
                {
                    area.data[localX + localZ * width] = Biome::eBiomeIDs_MesaClearRock;
                }
                continue;
            }

            int north = parentArea.data[localX + 1 + localZ * parentWidth];
            int south = parentArea.data[localX + 1 + (localZ + 2) * parentWidth];
            int east = parentArea.data[localX + 2 + (localZ + 1) * parentWidth];
            int west = parentArea.data[localX + (localZ + 1) * parentWidth];

            if (Layer::isOcean(centre)
                || centre == Biome::eBiomeIDs_DeepOcean
                || Layer::isOcean(north) || Layer::isOcean(south)
                || Layer::isOcean(east) || Layer::isOcean(west))
                area.data[localX + localZ * width] = Biome::eBiomeIDs_Beach;
            else
                area.data[localX + localZ * width] = centre;
        }
    }

    PIXEndNamedEvent();
    return area;
}
