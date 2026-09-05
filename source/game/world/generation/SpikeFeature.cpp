#include "game/world/generation/SpikeFeature.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/MutableBlockPos.hpp"

class EnderCrystal : public Entity
{
public:
    static boost::shared_ptr<EnderCrystal> create(Level* level);
    void setBeamTarget(BlockPos* target);
};

SpikeFeature::SpikeFeature()
    : Feature(false),
      mCrystalInvulnerable(false),
      mSpike(NULL),
      mCrystalBeamTarget(NULL)
{
}

SpikeFeature::~SpikeFeature()
{
    delete mCrystalBeamTarget;
}

void SpikeFeature::setSpike(EndSpike* spike)
{
    mSpike = spike;
}

void SpikeFeature::setCrystalInvulnerable(bool invulnerable)
{
    mCrystalInvulnerable = invulnerable;
}

bool SpikeFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (mSpike == NULL)
    {
        return false;
    }

    int radius = (int)mSpike->getRadius();
    int height = (int)mSpike->getHeight();
    BlockPos from(origin.x - radius, 1, origin.z - radius);
    BlockPos to(origin.x + radius, height + 10, origin.z + radius);
    Iterator<MutableBlockPos*>* positions = BlockPos::mutablesBetween(from, to);

    while (positions->hasNext())
    {
        MutableBlockPos* position = positions->next();
        if (position->distSqr((double)origin.x, (double)position->y,
                              (double)origin.z) <= (double)(radius * radius + 1) &&
            position->y < height)
        {
            setBlock(level, *position, Blocks::end_stone->defaultBlockState(), 0);
        }
        else if (position->y > 65)
        {
            setBlock(level, *position, Blocks::AIR->defaultBlockState(), 0);
        }
    }
    delete positions;

    if (mSpike->isGuarded())
    {
        int x;
        int z;
        for (x = -2; x <= 2; ++x)
        {
            for (z = -2; z <= 2; ++z)
            {
                int y;
                if (x == -2 || x == 2 || z == -2 || z == 2)
                {
                    for (y = 0; y <= 3; ++y)
                    {
                        BlockPos cage(origin.x + x, height + y, origin.z + z);
                        setBlock(level, cage, Blocks::obsidian->defaultBlockState(), 0);
                    }
                }
                else
                {
                    BlockPos cage(origin.x + x, height + 3, origin.z + z);
                    setBlock(level, cage, Blocks::obsidian->defaultBlockState(), 0);
                }
            }
        }
    }

    boost::shared_ptr<EnderCrystal> crystal = EnderCrystal::create(level);
    if (mCrystalBeamTarget == NULL)
    {
        crystal->setBeamTarget(NULL);
    }
    else
    {
        crystal->setBeamTarget(new BlockPos(*mCrystalBeamTarget));
    }
    crystal->setInvulnerable(mCrystalInvulnerable);
    crystal->moveTo((double)origin.x + 0.5, (double)height + 1.0,
                    (double)origin.z + 0.5, random.nextFloat() * 360.0f, 3.0f);
    level->addEntity(boost::shared_ptr<Entity>(crystal));

    BlockPos base(origin.x, height, origin.z);
    setBlock(level, base, Blocks::bedrock->defaultBlockState(), 0);
    delete mSpike;
    mSpike = NULL;
    return true;
}

void SpikeFeature::setCrystalBeamTarget(const BlockPos& pos)
{
    delete mCrystalBeamTarget;
    mCrystalBeamTarget = new BlockPos(pos);
}

SpikeFeature::EndSpike::EndSpike()
    : mCenterX(0),
      mCenterZ(0),
      mRadius(0),
      mHeight(0),
      mGuarded(false),
      mTopBoundingBox(NULL)
{
}

SpikeFeature::EndSpike::EndSpike(const EndSpike& other)
    : mCenterX(other.mCenterX),
      mCenterZ(other.mCenterZ),
      mRadius(other.mRadius),
      mHeight(other.mHeight),
      mGuarded(other.mGuarded),
      mTopBoundingBox(AABB::newPermanent(other.mTopBoundingBox))
{
}

SpikeFeature::EndSpike::EndSpike(int centerX, int centerZ, int radius, int height, bool guarded)
    : mCenterX(centerX),
      mCenterZ(centerZ),
      mRadius(radius),
      mHeight(height),
      mGuarded(guarded),
      mTopBoundingBox(AABB::newPermanent((double)(centerX - radius), 0.0,
                                         (double)(centerZ - radius),
                                         (double)(centerX + radius), 256.0,
                                         (double)(centerZ + radius)))
{
}

SpikeFeature::EndSpike::~EndSpike()
{
    delete mTopBoundingBox;
}

unsigned int SpikeFeature::EndSpike::getRadius()
{
    return (unsigned int)mRadius;
}

unsigned int SpikeFeature::EndSpike::getHeight()
{
    return (unsigned int)mHeight;
}

unsigned char SpikeFeature::EndSpike::isGuarded()
{
    return (unsigned char)mGuarded;
}

unsigned int SpikeFeature::EndSpike::getCenterX()
{
    return (unsigned int)mCenterX;
}

unsigned int SpikeFeature::EndSpike::getCenterZ()
{
    return (unsigned int)mCenterZ;
}

 AABB* SpikeFeature::EndSpike::getTopBoundingBox()
{
    return mTopBoundingBox;
}
