#include "game/world/generation/EndCityFeature.hpp"

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/Rotation.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/world/Level.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/world/TheEndLevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/core/EndCityPieces.hpp"

extern CMinecraftApp* g_pApp;

EndCityFeature::EndCityFeature(TheEndLevelSource* source)
    : StructureFeature(), mSource(source)
{
}

std::wstring EndCityFeature::getFeatureName()
{
    return L"EndCity";
}

bool EndCityFeature::isFeatureChunk(int chunkX, int chunkZ, bool)
{
    ChunkPos* area = TheEndDimension::closestAreaToPoint(chunkX, chunkZ);
    if (area->x == TheEndDimension::AREAS[1].x && area->z == TheEndDimension::AREAS[1].z)
    {
        return chunkX == area->x && chunkZ == area->z;
    }
    if (area->x == TheEndDimension::AREAS[0].x && area->z == TheEndDimension::AREAS[0].z)
    {
        return false;
    }
    Random& chunkRandom = *level->getRandomFor(area->x, area->z, 0x9E7F71);
    return chunkX == area->x && chunkZ == area->z && chunkRandom.nextDouble() < 0.33;
}

int EndCityFeature::getYPositionForFeature(int chunkX, int chunkZ,
                                           TheEndLevelSource* source)
{
    Random random(chunkX + chunkZ * 10387313);
    Rotation* rotation = Rotation::VALUES[random.nextInt(4)];
    arrayWithLength<unsigned short> elements(65536, true);
    ChunkPrimer primer(true, elements);
    source->prepareHeights(chunkX, chunkZ, &primer);
    int offset = 5;
    int heights[4];
    if (rotation == Rotation::CLOCKWISE_90)
    {
        heights[0] = primer.getHighestNonAirPos(7, 7);
        heights[1] = primer.getHighestNonAirPos(7, 12);
        heights[2] = primer.getHighestNonAirPos(2, 7);
        heights[3] = primer.getHighestNonAirPos(2, 12);
    }
    else if (rotation == Rotation::CLOCKWISE_180)
    {
        heights[0] = primer.getHighestNonAirPos(7, 7);
        heights[1] = primer.getHighestNonAirPos(7, 2);
        heights[2] = primer.getHighestNonAirPos(2, 7);
        heights[3] = primer.getHighestNonAirPos(2, 2);
    }
    else
    {
        if (rotation == Rotation::COUNTERCLOCKWISE_90)
            offset = -5;
        heights[0] = primer.getHighestNonAirPos(7, 7);
        heights[1] = primer.getHighestNonAirPos(7, offset + 7);
        heights[2] = primer.getHighestNonAirPos(12, 7);
        heights[3] = primer.getHighestNonAirPos(12, offset + 7);
    }
    int minimum = heights[0] < heights[1] ? heights[0] : heights[1];
    int otherMinimum = heights[2] < heights[3] ? heights[2] : heights[3];
    return minimum < otherMinimum ? minimum : otherMinimum;
}

StructureStart* EndCityFeature::createStructureStart(int chunkX, int chunkZ)
{
    return new EndCityStart(level, mSource, random, chunkX, chunkZ);
}

BlockPos* EndCityFeature::getNearestGeneratedFeature(Level* levelIn, const BlockPos& pos,
                                                       bool skipExistingChunks)
{
    level = levelIn;
    return StructureFeature::findNearestFeaturePositionBySpacing(
        levelIn, this, pos, 10, 4, 0x9E7F71, true, 100, skipExistingChunks);
}

EndCityFeature::EndCityStart::EndCityStart(Level* levelIn, TheEndLevelSource* source,
                                           Random& random, int chunkX, int chunkZ)
    : StructureStart(chunkX, chunkZ)
{
    g_pApp->AddTerrainFeaturePosition((_eTerrainFeatureType)8,
                                       (chunkX * 16 + 8) / 16,
                                       (chunkZ * 16 + 8) / 16);
    create(levelIn, source, random, chunkX, chunkZ);
}

EndCityFeature::EndCityStart::EndCityStart()
    : StructureStart()
{
}

EndCityFeature::EndCityStart::~EndCityStart()
{
}

void EndCityFeature::EndCityStart::create(Level* levelIn, TheEndLevelSource* source,
                                          Random& random, int chunkX, int chunkZ)
{
    Rotation* rotation = Rotation::getRandom(random);
    int y = EndCityFeature::getYPositionForFeature(chunkX, chunkZ, source);
    BlockPos position(chunkX * 16 + 8, y, chunkZ * 16 + 8);
    LevelStorage* storage = levelIn->getLevelStorage();
    EndCityPieces::startHouseTower(storage->getStructureManager(), position, rotation,
                                    *this, random);
    calculateBoundingBox();
    mValid = true;
}

bool EndCityFeature::EndCityStart::isValid()
{
    return mValid;
}

EStructureStart EndCityFeature::EndCityStart::GetType()
{
    return eStructureStart_EndCity;
}
