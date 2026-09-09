#include "game/world/generation/WallGenerator.hpp"

#include <vector>

#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/core/GenerationWriteTask.hpp"

WallGenerator::Wall::Wall(AABB* area, bool internal, bool boundary)
    : mArea(new AABB(*area))
    , mInternal(internal)
    , mBoundary(boundary)
{
}

WallGenerator::Wall::~Wall()
{
    delete mArea;
}

WallGenerator::WallGenerator(Random random, std::vector<AABB*>* areas,
                             std::vector<AABB*>* walls)
    : MapGenerator(random)
    , mAreas(*areas)
    , mBounds(NULL)
{
    std::vector<AABB*>::iterator area = mAreas.begin();
    while (area != mAreas.end())
    {
        if (mBounds == NULL)
        {
            mBounds = *area;
        }
        else
        {
            mBounds = mBounds->minmax(*area);
        }
        ++area;
    }

    if (walls != NULL)
    {
        std::vector<AABB*>::iterator wall = walls->begin();
        while (wall != walls->end())
        {
            mBounds = mBounds->minmax(*wall);
            mWalls.push_back(new Wall(*wall, true, false));
            ++wall;
        }
    }
    mBounds = mBounds->grow(2.0, 1.0, 2.0);
}

WallGenerator::~WallGenerator()
{
    std::vector<Wall*>::iterator wall = mWalls.begin();
    while (wall != mWalls.end())
    {
        delete *wall;
        ++wall;
    }
}

void WallGenerator::GetInternalWalls(std::vector<AABB*>* walls)
{
    std::vector<Wall*>::iterator wall = mWalls.begin();
    while (wall != mWalls.end())
    {
        if ((*wall)->mInternal)
        {
            walls->push_back((*wall)->mArea);
        }
        ++wall;
    }
}

bool WallGenerator::IsPointInternal(int x, int z, int size)
{
    return x > 0 && x < size && z > 0 && z < size;
}

void WallGenerator::StartGeneration(Level* level)
{
    AABB* grown = mBounds->grow(2.0, 1.0, 2.0);
    BlockPos min(grown->mMinX, grown->mMinY, grown->mMinZ);
    BlockPos max(grown->mMaxX, grown->mMaxY, grown->mMaxZ);
    BlockTransaction* transaction = BlockTransaction::CreateTransaction(min, max);

    const int size = static_cast<int>((mBounds->mMaxX - mBounds->mMinX) * 0.5);
    int x = 0;
    int z = 0;
    while (x < 3)
    {
        z = 0;
        while (z < 3)
        {
            bool internal = IsPointInternal(x, z, 2);
            AABB* center = AABB::newTemp(mBounds->mMinX + x * size,
                                         mBounds->mMinY,
                                         mBounds->mMinZ + z * size,
                                         mBounds->mMinX + (x + 1) * size,
                                         mBounds->mMaxY,
                                         mBounds->mMinZ + (z + 1) * size);
            mWalls.push_back(new Wall(center, internal, true));

            if (x + 1 < 3)
            {
                bool boundary = internal || IsPointInternal(x + 1, z, 2);
                AABB* side = AABB::newTemp(mBounds->mMinX + (x + 1) * size,
                                            mBounds->mMinY,
                                            mBounds->mMinZ + z * size + 1.0,
                                            mBounds->mMinX + (x + 1) * size + 1.0,
                                            mBounds->mMaxY,
                                            mBounds->mMinZ + (z + 1) * size + 1.0);
                mWalls.push_back(new Wall(side, boundary, false));
            }
            if (z + 1 < 3)
            {
                bool boundary = internal || IsPointInternal(x, z + 1, 2);
                AABB* side = AABB::newTemp(mBounds->mMinX + x * size,
                                            mBounds->mMinY,
                                            mBounds->mMinZ + (z + 1) * size + 1.0,
                                            mBounds->mMinX + (x + 1) * size + 1.0,
                                            mBounds->mMaxY,
                                            mBounds->mMinZ + (z + 1) * size + 1.0);
                mWalls.push_back(new Wall(side, boundary, false));
            }
            ++z;
        }
        ++x;
    }

    std::vector<AABB*>::iterator area = mAreas.begin();
    while (area != mAreas.end())
    {
        SetArea(level, transaction, *area, Blocks::stone->defaultBlockState());
        ++area;
    }

    std::vector<Wall*>::iterator wall = mWalls.begin();
    while (wall != mWalls.end())
    {
        const BlockState* state = ((*wall)->mBoundary && (*wall)->mInternal)
            ? Blocks::stone->defaultBlockState() : Blocks::AIR->defaultBlockState();
        SetArea(level, transaction, (*wall)->mArea, state);
        ++wall;
    }

    BlockPos origin(static_cast<int>(grown->mMinX), static_cast<int>(grown->mMinY),
                    static_cast<int>(grown->mMinZ));
    mGenerationWriteTasks.push_back(new GenerationWriteTask(level, transaction, origin, NULL));
}

AABB* WallGenerator::GetSpawnArea() const
{
    return mAreas.empty() ? NULL : mAreas.front();
}

void WallGenerator::CleanUp(Level*)
{
}

AABB* WallGenerator::GetLayerBB(unsigned int)
{
    return 0;
}

int WallGenerator::GetNumberOfLayers()
{
    return 0;
}

void WallGenerator::GetLayerExtents(unsigned int, float*, float*)
{
}
