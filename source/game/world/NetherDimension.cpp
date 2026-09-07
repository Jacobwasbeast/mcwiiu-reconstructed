#include "game/world/NetherDimension.hpp"

#include <cmath>

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/FixedBiomeSource.hpp"

#include "game/world/NetherFlatLevelSource.hpp"
#include "game/world/NetherLevelSource.hpp"

NetherDimension::~NetherDimension()
{
}

void NetherDimension::init()
{
    mBiomeSource = new FixedBiomeSource(Biome::HELL, 4.5f);
    mHasSkyLight = true;
    mUltraWarm = true;
}

Vec3* NetherDimension::getFogColor(float celestialAngle, float timeOfDay)
{
    return Vec3::newTemp(0.2, 0.03, 0.03);
}

void NetherDimension::updateLightRamp()
{
    const float base = 1.0f;
    const float scale = 15.0f;
    const float ambient = 3.0f;
    for (int i = 0; i < 16; ++i)
    {
        float light = base - (float)i / scale;
        mBrightnessRamp[i] = (base - light) / (light * ambient + base) * 0.9f + 0.1f;
    }
}

ChunkGenerator* NetherDimension::createRandomLevelGenerator() const
{
    LevelData* levelData = mLevel->getLevelData();
    if (mLevelType == LevelType::FLAT)
        return new NetherFlatLevelSource(mLevel, levelData->isGenerateMapFeatures(), mLevel->getSeed());
    return new NetherLevelSource(mLevel, levelData->isGenerateMapFeatures(), mLevel->getSeed());
}

bool NetherDimension::isNaturalDimension()
{
    return false;
}

bool NetherDimension::isValidSpawn(int x, int z) const
{
    return false;
}

double NetherDimension::getTimeOfDay(long long time, float partialTick) const
{
    return 0.5;
}

bool NetherDimension::mayRespawn() const
{
    return false;
}

bool NetherDimension::isFoggyAt(int x, int z)
{
    return true;
}

WorldBorder* NetherDimension::createWorldBorder(Level* level)
{
    return new NetherDimensionWorldBorder(level);
}

const DimensionType* NetherDimension::getType() const
{
    return DimensionType::NETHER;
}

int NetherDimension::getXZSize()
{
    LevelData* levelData = mLevel->getLevelData();
    return static_cast<int>(ceilf(static_cast<float>(levelData->getXZSize()) /
                                  static_cast<float>(levelData->getHellScale())));
}

AABB* NetherDimension::getPlayerConstraints(const BlockPos& pos)
{
    AABB* bounds = Dimension::getPlayerConstraints(pos);
    bounds->mMaxY = 127.0;
    return bounds;
}

int NetherDimension::getBlockLightColour()
{
    return static_cast<int>(Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_BlockLight_HellDimension));
}

NetherDimension::NetherDimensionWorldBorder::NetherDimensionWorldBorder(Level* level)
    : WorldBorder(level)
{
}

NetherDimension::NetherDimensionWorldBorder::~NetherDimensionWorldBorder()
{
}

double NetherDimension::NetherDimensionWorldBorder::getCenterX()
{
    return WorldBorder::getCenterX() * 0.125;
}

double NetherDimension::NetherDimensionWorldBorder::getCenterZ()
{
    return WorldBorder::getCenterZ() * 0.125;
}
