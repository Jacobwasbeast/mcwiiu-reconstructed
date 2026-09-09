#include "game/world/generation/IcebergFeature.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

IcebergFeature::IcebergFeature()
    : Feature(false), mSnowOnTop(false), mIsEllipse(false), mAngle(3.1415927410125732),
      mHeight(0), mRadius(0), mLargestOffset(0), mIcebergState(NULL)
{
}

IcebergFeature::~IcebergFeature()
{
}

void IcebergFeature::setSnowOnTop(bool snowOnTop)
{
    mSnowOnTop = snowOnTop;
}

int IcebergFeature::heightDependentRadiusEllipse(int height, int totalHeight, int radius)
{
    return (int)std::ceil((1.0f - (float)height * (float)height / (float)totalHeight) * (float)radius * 0.5f);
}

int IcebergFeature::heightDependentRadiusRound(Random& random, int height, int totalHeight, int radius)
{
    float denominator = (float)totalHeight * (1.0f - random.nextFloat());
    float value = (float)height * (float)height / denominator;
    if (random.nextInt(5) + 15 < totalHeight && height < random.nextInt(6) + 3)
    {
        height = height / 2;
        value = (float)height / (denominator * 0.4f);
    }
    return (int)std::ceil((1.0f - value) * (float)radius * 0.5f);
}

int IcebergFeature::heightDependentRadiusSteep(Random& random, int height, int totalHeight, int radius)
{
    const double randomFactor = (double)random.nextFloat();
    const double value = (double)height / ((double)totalHeight * (randomFactor * 0.5 + 1.0));
    return (int)std::ceil((1.0 - value) * (double)radius * 0.5);
}

void IcebergFeature::initEllipse(Random& random)
{
    mIsEllipse = random.nextDouble() > 0.8;
    if (mIsEllipse)
    {
        mAngle = random.nextDouble() * 2.0 * 3.1415927410125732;
        mRadius = 15 - mLargestOffset - random.nextInt(4);
        mLargestOffset = random.nextInt(3) + 3;
    }
}

double IcebergFeature::signedDistanceEllipse(int x, int z, const BlockPos& origin, int radius, int height, double angle)
{
    const double dx = (double)(x - origin.x);
    const double dz = (double)(z - origin.z);
    const double cosine = std::cos(angle);
    const double sine = std::sin(angle);
    const double first = -(dz * sine - dx * cosine) / (double)radius;
    const double second = (dz * cosine + dx * sine) / (double)height;
    return first * first + second * second - 1.0;
}

int IcebergFeature::getEllipseC(int height, int totalHeight)
{
    if (height < 1 || totalHeight - height > 3)
        return mLargestOffset;
    return mLargestOffset + totalHeight - height - 4;
}

double IcebergFeature::signedDistanceCircle(int x, int z, const BlockPos& origin, int radius, Random& random)
{
    const float r = (float)radius;
    const float dx = (float)(x - origin.x);
    const float dz = (float)(z - origin.z);
    const double wobble = (double)Mth::clamp(random.nextFloat(), 0.2f, 0.8f);
    return (double)(10.0f * (float)wobble / r + dx * dx + dz * dz - r * r);
}

void IcebergFeature::setIcebergBlock(const BlockPos& pos, Level* level, Random& random, int height, int totalHeight)
{
    Block* block = level->getBlockState(pos)->getBlock();
    if (!isIcebergBlock(block) && block != Blocks::WATER && block != Blocks::snow_layer)
        return;

    bool replace = true;
    if (mIsEllipse && random.nextDouble() <= 0.05)
        replace = false;
    int divisor = mIsEllipse ? 3 : 2;
    if (mSnowOnTop && block != Blocks::ice && (height <= totalHeight * 0.6 + random.nextInt(std::max(1, totalHeight / divisor))) && replace)
        setBlock(level, pos, Blocks::snow->defaultBlockState(), 0);
    else
        setBlock(level, pos, mIcebergState, 0);
}

void IcebergFeature::generateIcebergBlock(Level* level, Random& random, const BlockPos& origin, int height, int x, int y, int z, int radius, int verticalRadius)
{
    BlockPos local(x, y, z);
    const double distance = mIsEllipse
        ? signedDistanceEllipse(x, z, local, verticalRadius, getEllipseC(y, height), mAngle)
        : signedDistanceCircle(x, z, local, verticalRadius, random);
    if (distance >= 0.0)
        return;
    const BlockPos target = origin.offset(x, y, z);
    if (level->isEmptyBlock(target) || isIcebergBlock(level->getBlockState(target)->getBlock()))
        setIcebergBlock(target, level, random, height - y, height);
}

bool IcebergFeature::isIcebergBlock(Block* block)
{
    return block == Blocks::ice || block == Blocks::packed_ice || block == Blocks::blue_ice;
}

bool IcebergFeature::belowIsAir(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos.below())->getBlock() == Blocks::AIR;
}

void IcebergFeature::smooth(Level* level, const BlockPos& origin, int radius, int height)
{
    const int extent = mIsEllipse ? mRadius : height / 2;
    for (int x = -extent; x <= extent; ++x)
        for (int z = -extent; z <= extent; ++z)
        {
            BlockPos pos = origin.offset(x, 0, z);
            Block* block = level->getBlockState(pos)->getBlock();
            if (isIcebergBlock(block) && belowIsAir(level, pos))
            {
                setBlock(level, pos, Blocks::snow->defaultBlockState(), 0);
                setBlock(level, pos.above(), Blocks::snow_layer->defaultBlockState(), 0);
            }
        }
}

void IcebergFeature::removeFloatingSnowLayer(Level* level, const BlockPos& pos)
{
    if (level->getBlockData(pos.x, pos.y + 1, pos.z) == 78)
        setBlock(level, pos.above(), Blocks::AIR->defaultBlockState(), 0);
}

void IcebergFeature::carve(int x, int z, const BlockPos& offset, Level* level, bool replaceWithWater, double angle, const BlockPos& origin)
{
    const int vertical = z + mHeight / 3 + 1;
    for (int y = -vertical; y <= vertical; ++y)
    {
        if (signedDistanceEllipse(x, y, origin, vertical, mHeight, angle) >= 0.0)
            continue;
        BlockPos pos = offset.offset(x, z, y);
        Block* block = level->getBlockState(pos)->getBlock();
        if (isIcebergBlock(block) || block == Blocks::snow_layer)
        {
            if (replaceWithWater)
                setBlock(level, pos, Blocks::WATER->defaultBlockState(), 0);
            else
            {
                setBlock(level, pos, Blocks::AIR->defaultBlockState(), 0);
                removeFloatingSnowLayer(level, pos);
            }
        }
    }
}

void IcebergFeature::generateCutOut(Random& random, Level* level, int radius, int height, const BlockPos& origin)
{
    int directionX = random.nextBoolean() ? -1 : 1;
    int directionZ = random.nextBoolean() ? -1 : 1;
    int centre = height / 2;
    int x = random.nextInt(std::max(1, centre - 2));
    int z = random.nextInt(std::max(1, centre - 2));
    if (random.nextBoolean()) x = centre + 1 - random.nextInt(std::max(1, height - centre - 1));
    if (random.nextBoolean()) z = centre + 1 - random.nextInt(std::max(1, height - centre - 1));
    BlockPos offset = BlockPos::ZERO.offset(directionX * x, 0, directionZ * z);
    const double angle = mIsEllipse ? mAngle + 1.5707963705062866 : random.nextDouble() * 2.0 * 3.1415927410125732;
    for (int y = 0; y < height - 3; ++y)
        carve(y, heightDependentRadiusRound(random, y, height, radius), origin, level, false, angle, offset);
    int side = -1;
    for (int y = 1; random.nextInt(5) - height < side; ++y, --side)
        carve(y, heightDependentRadiusSteep(random, y, height, radius), origin, level, true, angle, offset);
}

bool IcebergFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    setSnowOnTop(random.nextDouble() > 0.7);
    initEllipse(random);
    mIcebergState = random.nextDouble() <= 0.01 ? Blocks::blue_ice->defaultBlockState() : Blocks::packed_ice->defaultBlockState();
    BlockPos base(origin.x, level->getSeaLevel(), origin.z);
    int height = (mIsEllipse ? random.nextInt(4) + 6 : random.nextInt(11) + 3);
    if (!mIsEllipse && random.nextDouble() > 0.9)
        height += random.nextInt(15) + 7;
    int radius = std::min(18, height + random.nextInt(11));
    mHeight = height;
    mRadius = mIsEllipse ? mRadius : 11;
    for (int y = 0; y < height; ++y)
        for (int x = -mRadius; x <= mRadius; ++x)
            for (int z = -mRadius; z <= mRadius; ++z)
            {
                const int extent = mIsEllipse ? heightDependentRadiusEllipse(y, height, radius) : heightDependentRadiusRound(random, y, height, radius);
                if (std::abs(x) < extent && std::abs(z) < extent)
                    generateIcebergBlock(level, random, base, height, x, y, z, radius, extent);
            }
    smooth(level, base, radius, height);
    if (random.nextDouble() > (mIsEllipse ? 0.1 : 0.9))
        generateCutOut(random, level, radius, height, base);
    return true;
}
