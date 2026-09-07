#include "game/world/TheEndDimension.hpp"

#include <cmath>
#include <cstring>
#include <limits>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EndDragonFight.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/TheEndLevelSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/FixedBiomeSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ChunkPos.hpp"

static ChunkPos gTheEndCentralArea(0, 0);
static ChunkPos gTheEndOuterArea(100, 0);
static bool gTheEndSpawnPosInitialized = false;
static bool gTheEndLegacySpawnPosInitialized = false;
static BlockPos gTheEndSpawnPos(0, 0, 0);
static BlockPos gTheEndLegacySpawnPos(0, 0, 0);

int aaDistance(int x, int z, ChunkPos* pos);

int aaDistance(int x, int z, ChunkPos* pos)
{
    const int xDistance = Mth::abs(x - pos->x);
    const int zDistance = Mth::abs(z - pos->z);
    return xDistance > zDistance ? xDistance : zDistance;
}

TheEndDimension::TheEndDimension()
    : Dimension(), mDragonFight(NULL), mChunkData(NULL), mModernEnd(true)
{
    const int size = getXZSize();
    mChunkData = new unsigned char[size * size];
    std::memset(mChunkData, 0xff, size * size);
}

TheEndDimension::~TheEndDimension()
{
    delete mDragonFight;
    delete[] mChunkData;
}

EndDragonFight* TheEndDimension::getDragonFight()
{
    return mDragonFight;
}

int TheEndDimension::getXZSize()
{
    return mModernEnd ? 128 : 18;
}

void TheEndDimension::init()
{
    mBiomeSource = new FixedBiomeSource(Biome::THE_END, 0.5f);
    LevelData* levelData = mLevel->getLevelData();
    mModernEnd = levelData->getModernEnd();
    if (!mLevel->mIsClientSide)
    {
        CompoundTag* data = levelData->getDimensionData(DimensionType::THE_END);
        mDragonFight = new EndDragonFight(static_cast<ServerLevel*>(mLevel), data);
    }
    else
    {
        mDragonFight = NULL;
    }
    theApp.SetExtraGameData(static_cast<eGameExtraData>(3), mModernEnd);
}

ChunkGenerator* TheEndDimension::createRandomLevelGenerator() const
{
    LevelData* levelData = mLevel->getLevelData();
    if (mModernEnd)
    {
        return new TheEndLevelSource(mLevel, levelData->isGenerateMapFeatures(),
                                     mLevel->getSeed(), BlockPos(0, 0, 0));
    }
    return new TheEndLevelSource(mLevel, levelData->isGenerateMapFeatures(),
                                 mLevel->getSeed(), *getSpawnPos());
}

double TheEndDimension::getTimeOfDay(long long time, float partialTick) const
{
    return 0.5;
}

float* TheEndDimension::getSunriseColor(float celestialAngle, float timeOfDay)
{
    return NULL;
}

Vec3* TheEndDimension::getFogColor(float celestialAngle, float timeOfDay)
{
    float brightness = Mth::clamp(Mth::cos(celestialAngle * 6.2831855f) * 2.0f + 0.5f,
                                  0.0f, 1.0f);
    float scale = brightness * 2.0f + 0.15f;
    return Vec3::newTemp(0.627451f * scale, 0.5019608f * scale, scale);
}

bool TheEndDimension::hasGround() { return false; }
bool TheEndDimension::mayRespawn() const { return false; }
bool TheEndDimension::isNaturalDimension() { return false; }
double TheEndDimension::getCloudHeight() { return 8.0; }

bool TheEndDimension::isValidSpawn(int x, int z) const
{
    BlockPos pos(x, 0, z);
    const BlockState* state = mLevel->getTopBlockState(pos);
    return state->getMaterial()->isSolid();
}

BlockPos* TheEndDimension::getSpawnPos() const
{
    if (!gTheEndSpawnPosInitialized)
    {
        gTheEndSpawnPosInitialized = true;
        gTheEndSpawnPos = BlockPos(60, 50, 60);
    }
    if (!gTheEndLegacySpawnPosInitialized)
    {
        gTheEndLegacySpawnPosInitialized = true;
        gTheEndLegacySpawnPos = BlockPos(100, 50, 0);
    }
    return mModernEnd ? &gTheEndSpawnPos : &gTheEndLegacySpawnPos;
}

int TheEndDimension::getSpawnYPosition() { return 50; }
bool TheEndDimension::isFoggyAt(int x, int z) { return false; }

const DimensionType* TheEndDimension::getType() const
{
    return DimensionType::THE_END;
}

void TheEndDimension::saveData()
{
    CompoundTag* data = new CompoundTag();
    if (mDragonFight != NULL)
        data->put(L"DragonFight", mDragonFight->saveData());
    mLevel->getLevelData()->setDimensionData(DimensionType::THE_END, data);
}

void TheEndDimension::tick()
{
    if (mDragonFight != NULL)
        mDragonFight->tick();
}

ChunkPos* TheEndDimension::closestAreaToPoint(int x, int z)
{
    unsigned int centralDistance = aaDistance(x, z, &gTheEndCentralArea);
    unsigned int outerDistance = aaDistance(x, z, &gTheEndOuterArea);
    return outerDistance < centralDistance ? &gTheEndOuterArea : &gTheEndCentralArea;
}

AABB* TheEndDimension::getPlayerConstraints(const BlockPos& pos)
{
    if (mModernEnd)
    {
        BlockPos origin = closestAreaToPoint(pos.x >> 4, pos.z >> 4)->getWorldPosition();
        return AABB::newTemp(-96.0, -std::numeric_limits<double>::max(), -96.0,
                             96.0, std::numeric_limits<double>::max(), 96.0)->move(origin);
    }
    AABB* bounds = Dimension::getPlayerConstraints(pos);
    bounds->set(bounds->mMinX, -std::numeric_limits<double>::max(), bounds->mMinZ,
                bounds->mMaxX, std::numeric_limits<double>::max(), bounds->mMaxZ);
    return bounds;
}

bool TheEndDimension::containsChunk(int x, int z)
{
    bool result = Dimension::containsChunk(x, z);
    if (!result || !mModernEnd)
        return result;
    const int size = getXZSize();
    const int index = (x + size / 2) * size + z + size / 2;
    unsigned char& cached = mChunkData[index];
    if (cached == 0xff)
    {
        ChunkPos* area = closestAreaToPoint(x, z);
        cached = (x - area->x >= -6 && x - area->x < 6 &&
                  z - area->z >= -6 && z - area->z < 6) ? 1 : 0;
    }
    return cached == 1;
}

int TheEndDimension::getBlockLightColour()
{
    return static_cast<int>(Minecraft::GetInstance()->getColourTable()->getColour(
        eMinecraftColour_BlockLight_EndDimension));
}
