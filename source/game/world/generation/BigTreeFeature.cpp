#include "game/world/generation/BigTreeFeature.hpp"

#include <algorithm>
#include <cmath>

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

BigTreeFeature::FoliageCoords::FoliageCoords(const BlockPos& pos, int branchBase)
    : BlockPos(pos), mBranchBase(branchBase)
{
}

int BigTreeFeature::FoliageCoords::getBranchBase()
{
    return mBranchBase;
}

BigTreeFeature::BigTreeFeature(bool doBlockNotify)
    : AbstractTreeFeature(doBlockNotify, 5, Blocks::log->defaultBlockState(),
                          Blocks::leaves->defaultBlockState(), false),
      mRandom(), mBasePos(), mOriginY(0), mOriginZ(0), mHeightLimit(0), mHeight(0),
      mScaleWidth(0.618), mBranchSlope(0.381), mBranchDensity(1.0), mLeafDensity(0.7),
      mTrunkWidth(1), mLeafDistanceLimit(12), mFoliageHeight(4), mFoliageCoords()
{
}

BigTreeFeature::~BigTreeFeature()
{
}

double BigTreeFeature::treeShape(int height)
{
    double remaining = static_cast<double>(mHeightLimit - height);
    double limit = static_cast<double>(mHeightLimit) * 0.3;
    if (remaining < limit)
    {
        return -1.0;
    }
    double branchHeight = static_cast<double>(mHeightLimit) * 0.5;
    double distance = branchHeight - remaining;
    if (distance == 0.0)
    {
        return branchHeight;
    }
    if (std::fabs(distance) < branchHeight)
    {
        return std::sqrt(branchHeight * branchHeight - distance * distance) * 0.5;
    }
    return 0.0;
}

int BigTreeFeature::getSteps(const BlockPos& pos)
{
    return std::max(std::abs(pos.x), std::max(std::abs(pos.y), std::abs(pos.z)));
}

int BigTreeFeature::checkLine(const BlockPos& from, const BlockPos& to)
{
    BlockPos delta = to - from;
    int steps = getSteps(delta);
    if (steps == 0)
    {
        return isFree(mLevel->getBlock(from)) ? -1 : 0;
    }
    double dx = static_cast<double>(delta.x) / steps;
    double dy = static_cast<double>(delta.y) / steps;
    double dz = static_cast<double>(delta.z) / steps;
    for (int i = 0; i <= steps; ++i)
    {
        BlockPos pos = from.offset(dx * i + 0.5, dy * i + 0.5, dz * i + 0.5);
        if (!isFree(mLevel->getBlock(pos)))
        {
            return i;
        }
    }
    return -1;
}

void BigTreeFeature::prepare()
{
    mHeight = static_cast<int>(mHeightLimit * mScaleWidth);
    if (mHeight >= mHeightLimit)
    {
        mHeight = mHeightLimit - 1;
    }
    double radius = mLeafDensity * mHeightLimit / 13.0;
    int clusters = static_cast<int>(1.0 + radius * radius);
    if (clusters < 1)
    {
        clusters = 1;
    }
    mFoliageCoords.clear();
    mFoliageCoords.push_back(FoliageCoords(mBasePos.above(mHeightLimit - mFoliageHeight),
                                           mOriginY + mHeight));
    for (int layer = mHeightLimit - mFoliageHeight; layer >= 0; --layer)
    {
        double shape = treeShape(layer);
        if (shape < 0.0)
        {
            continue;
        }
        for (int count = clusters; count > 0; --count)
        {
            double angle = mRandom.nextFloat() * 6.2831855;
            double distance = mBranchDensity * shape * (mRandom.nextFloat() + 0.328);
            BlockPos branch = mBasePos.offset(distance * std::sin(angle), static_cast<double>(layer),
                                              distance * std::cos(angle));
            if (checkLine(branch, branch.above(mFoliageHeight)) == -1)
            {
                int branchY = static_cast<int>(-std::sqrt(static_cast<double>(
                    (branch.x - mBasePos.x) * (branch.x - mBasePos.x) +
                    (branch.z - mBasePos.z) * (branch.z - mBasePos.z))) * mBranchSlope +
                    mOriginY + mHeight);
                BlockPos trunk = mBasePos.above(branchY - mOriginY);
                if (checkLine(trunk, branch) == -1)
                {
                    mFoliageCoords.push_back(FoliageCoords(branch, branchY));
                }
            }
        }
    }
}

void BigTreeFeature::crossection(const BlockPos& pos, float radius, const BlockState* state)
{
    int range = static_cast<int>(radius + 0.618);
    double radiusSquared = radius * radius;
    for (int x = -range; x <= range; ++x)
    {
        for (int z = -range; z <= range; ++z)
        {
            if ((x + 0.5) * (x + 0.5) + (z + 0.5) * (z + 0.5) <= radiusSquared)
            {
                BlockPos target = pos.offset(x, 0, z);
                Block* block = mLevel->getBlock(target);
                if (block == Blocks::AIR || block == Blocks::leaves || block == Blocks::leaves2)
                {
                    setBlock(mLevel, target, state, 0);
                }
            }
        }
    }
}

double BigTreeFeature::foliageShape(int layer)
{
    if (layer < 0 || layer >= mFoliageHeight)
    {
        return -1.0;
    }
    if (layer != 0 && layer != mFoliageHeight - 1)
    {
        return 3.0;
    }
    return 2.0;
}

void BigTreeFeature::foliageCluster(const BlockPos& pos)
{
    for (int layer = 0; layer < mFoliageHeight; ++layer)
    {
        crossection(pos.above(layer), static_cast<float>(foliageShape(layer)),
                    Blocks::leaves->defaultBlockState());
    }
}

int BigTreeFeature::getLogAxis(const BlockPos& from, const BlockPos& to)
{
    int dx = std::abs(to.x - from.x);
    int dz = std::abs(to.z - from.z);
    if (std::max(dx, dz) < 1)
    {
        return 0;
    }
    return dx >= dz ? 1 : 2;
}

void BigTreeFeature::limb(const BlockPos& from, const BlockPos& to, Block* block)
{
    BlockPos delta = to - from;
    int steps = getSteps(delta);
    if (steps < 0)
    {
        return;
    }
    double dx = static_cast<double>(delta.x) / steps;
    double dy = static_cast<double>(delta.y) / steps;
    double dz = static_cast<double>(delta.z) / steps;
    const BlockState* state = block->defaultBlockState();
    for (int i = 0; i <= steps; ++i)
    {
        setBlock(mLevel, from.offset(dx * i + 0.5, dy * i + 0.5, dz * i + 0.5), state, 0);
    }
}

void BigTreeFeature::makeFoliage()
{
    for (std::vector<FoliageCoords>::iterator it = mFoliageCoords.begin(); it != mFoliageCoords.end(); ++it)
    {
        foliageCluster(*it);
    }
}

bool BigTreeFeature::trimBranches(int branchBase)
{
    return static_cast<double>(mHeightLimit) * 0.2 <= static_cast<double>(branchBase);
}

void BigTreeFeature::makeTrunk()
{
    limb(mBasePos, mBasePos.above(mHeight), Blocks::log);
    if (mTrunkWidth == 2)
    {
        limb(mBasePos.east(), mBasePos.east().above(mHeight), Blocks::log);
        limb(mBasePos.east().south(), mBasePos.east().south().above(mHeight), Blocks::log);
        limb(mBasePos.south(), mBasePos.south().above(mHeight), Blocks::log);
    }
}

void BigTreeFeature::makeBranches()
{
    for (std::vector<FoliageCoords>::iterator it = mFoliageCoords.begin(); it != mFoliageCoords.end(); ++it)
    {
        BlockPos branchBase(mBasePos.x, it->getBranchBase(), mBasePos.z);
        if (branchBase != *it && trimBranches(it->getBranchBase() - mOriginY))
        {
            limb(branchBase, *it, Blocks::log);
        }
    }
}

void BigTreeFeature::init()
{
    mFoliageHeight = 5;
}

bool BigTreeFeature::checkLocation()
{
    Block* block = mLevel->getBlock(mBasePos.below());
    if (block == Blocks::grass || block == Blocks::dirt || block == Blocks::farmland)
    {
        int obstruction = checkLine(mBasePos, mBasePos.above(mHeightLimit - 1));
        if (obstruction == -1)
        {
            return true;
        }
        if (obstruction > 5)
        {
            mHeightLimit = obstruction;
            return true;
        }
    }
    return false;
}

bool BigTreeFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    mLevel = level;
    mOriginY = origin.y;
    mOriginZ = origin.z;
    mBasePos = origin;
    mRandom.setSeed(random.nextLong());
    if (mHeightLimit == 0)
    {
        mHeightLimit = random.nextInt(mLeafDistanceLimit) + 5;
    }
    if (!checkLocation())
    {
        return false;
    }
    prepare();
    makeFoliage();
    makeTrunk();
    makeBranches();
    return true;
}
