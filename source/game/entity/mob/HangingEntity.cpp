#include "game/entity/mob/HangingEntity.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Rotation.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/DiodeBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/CMinecraftApp.hpp"

uint32_t gHangingEntityRenderCount = 0;

HangingEntity::HangingEntity(Level* level)
    : Entity(level, true), mTickCounter(0), mPos(), mDirection(NULL)
{
    _init(level);
}

HangingEntity::HangingEntity(Level* level, const BlockPos& pos)
    : Entity(level, true), mTickCounter(0), mPos(), mDirection(NULL)
{
    _init(level);
    mPos = pos;
}

HangingEntity::~HangingEntity()
{
}

void HangingEntity::_init(Level* level)
{
    mTickCounter = 0;
    mDirection = NULL;
    mPos = BlockPos();
    setSize(0.5f, 0.5f);
}

void HangingEntity::defineSynchedData()
{
}

void HangingEntity::setDirection(const Direction* direction)
{
    mDirection = direction;
    if (direction != NULL)
    {
        mYRot = (float)direction->get2DDataValue() * 90.0f;
        mYRotO = mYRot;
    }
    recalculateBoundingBox();
}

double HangingEntity::offs(int value)
{
    return (value & 31) == 0 ? 0.5 : 0.0;
}

void HangingEntity::recalculateBoundingBox()
{
    if (mDirection == NULL)
        return;

    double x = (double)mPos.x + 0.5;
    double y = (double)mPos.y + 0.5;
    double z = (double)mPos.z + 0.5;
    double width = (double)offs(getWidth());
    double height = (double)offs(getHeight());
    const Direction* side = mDirection->getCounterClockWise();
    const Vec3i normal = mDirection->getNormal();
    const Vec3i sideNormal = side->getNormal();

    x += (double)sideNormal.x * width;
    y += (double)normal.y * height;
    z += (double)sideNormal.z * width;
    mX = x;
    mY = y;
    mZ = z;

    double depth = 0.03125;
    if (mDirection->getAxis() == Direction::Axis::X)
        setBoundingBox(AABB::newTemp(x - depth, y - height * 0.5, z - width * 0.5,
                                     x + depth, y + height * 0.5, z + width * 0.5));
    else
        setBoundingBox(AABB::newTemp(x - width * 0.5, y - height * 0.5, z - depth,
                                     x + width * 0.5, y + height * 0.5, z + depth));
}

void HangingEntity::tick()
{
    mXo = mX;
    mYo = mY;
    mZo = mZ;
    ++mTickCounter;
    if (mTickCounter == 100)
    {
        mTickCounter = 0;
        if ((mLevel == NULL || !mLevel->mIsClientSide) && !survives())
        {
            remove();
            dropItem(shared_from_this());
        }
    }
}

bool HangingEntity::survives()
{
    if (mLevel == NULL || mDirection == NULL)
        return false;
    if (!mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox, false, false, false)->empty())
        return false;

    int width = getWidth() / 16;
    int height = getHeight() / 16;
    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;
    const Direction* horizontal = mDirection->getCounterClockWise();
    BlockPos anchor = mPos.relative(mDirection->getOpposite());
    MutableBlockPos check;
    int xStart = -(width - 1) / 2;
    int yStart = -(height - 1) / 2;
    int y;
    for (y = 0; y < height; ++y)
    {
        int x;
        for (x = 0; x < width; ++x)
        {
            check.set(anchor);
            check.move(horizontal, x + xStart);
            check.move(Direction::UP, y + yStart);
            const BlockState* state = mLevel->getBlockState(check);
            if (!state->getMaterial()->isSolid() && !DiodeBlock::isDiode(state))
                return false;
        }
    }
    HangingEntitySelector selector;
    return mLevel->getEntities(shared_from_this(), mBoundingBox, &selector, NULL)->empty();
}

bool HangingEntity::isPickable()
{
    return true;
}

bool HangingEntity::skipAttackInteraction(boost::shared_ptr<Entity> entity)
{
    if (entity == NULL)
        return false;
    if ((entity->GetType() & 0x0C800000ULL) != 0x0C800000ULL)
        return false;

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    EntityDamageSource* source = DamageSource::CreatePlayerAttack(player);
    bool skipped = hurt(source, 0.0f);
    delete source;
    return skipped;
}

const Direction* HangingEntity::getDirection()
{
    return mDirection;
}

bool HangingEntity::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;
    boost::shared_ptr<Entity> attacker = source == NULL ? boost::shared_ptr<Entity>() : source->getEntity();
    if (attacker != NULL && (attacker->GetType() & 0x0C800000ULL) == 0x0C800000ULL)
    {
        if (!boost::static_pointer_cast<Player>(attacker)->isAllowedToHurtEntity(shared_from_this()))
            return false;
    }
    if (!mRemoved && !mLevel->mIsClientSide)
    {
        remove();
        markHurt();
        dropItem(attacker);
    }
    return true;
}

void HangingEntity::move(MoverType type, double x, double y, double z, bool pushedByPiston)
{
    if ((x * x + y * y + z * z) > 0.0 && !mRemoved)
    {
        remove();
        dropItem(shared_from_this());
    }
}

void HangingEntity::push(double x, double y, double z)
{
    move(eMoverType_Self, x, y, z, false);
}

void HangingEntity::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putByte(L"Facing", (unsigned char)(mDirection == NULL ? 0 : mDirection->get2DDataValue()));
    tag->putInt(L"TileX", mPos.x);
    tag->putInt(L"TileY", mPos.y);
    tag->putInt(L"TileZ", mPos.z);
}

void HangingEntity::readAdditionalSaveData(CompoundTag* tag)
{
    mPos = BlockPos(tag->getInt(L"TileX"), tag->getInt(L"TileY"), tag->getInt(L"TileZ"));
    setDirection(Direction::from2DDataValue((int)tag->getByte(L"Facing")));
}

boost::shared_ptr<ItemEntity> HangingEntity::spawnAtLocation(not_null_ptr<ItemInstance> itemInstance, float yOffset)
{
    return Entity::spawnAtLocation(itemInstance, yOffset);
}

bool HangingEntity::repositionEntityAfterLoad()
{
    return false;
}

void HangingEntity::setPos(double x, double y, double z)
{
    BlockPos old = mPos;
    mX = x;
    mY = y;
    mZ = z;
    mPos = BlockPos(x, y, z);
    if (!mPos.equals(old))
    {
        recalculateBoundingBox();
        mHasImpulse = true;
    }
}

BlockPos HangingEntity::getPos()
{
    return mPos;
}

float HangingEntity::rotate(Rotation* rotation)
{
    if (mDirection != NULL && mDirection->getAxis() != Direction::Axis::Y)
        setDirection(rotation->rotate(mDirection));
    float result = Mth::wrapDegrees(mYRot);
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90: return result + 90.0f;
    case Rotation::eTYPE_Clockwise180: return result + 180.0f;
    case Rotation::eTYPE_CounterClockwise90: return result + 270.0f;
    default: return result;
    }
}

float HangingEntity::mirror(Mirror* mirror)
{
    return rotate(mirror->getRotation(mDirection));
}

void HangingEntity::thunderHit(const LightningBolt* lightningBolt)
{
}

bool HangingEntity::shouldRender(double distanceSqr)
{
    if (gHangingEntityRenderCount < 100 / g_pApp->GetLocalPlayerCount())
        return Entity::shouldRender(distanceSqr);
    return false;
}

bool HangingEntity::shouldRenderAtSqrDistance(double distanceSqr)
{
    return distanceSqr < 29584.0;
}

unsigned long long HangingEntity::GetType()
{
    return 0x0A080000ULL;
}

bool HangingEntity::HangingEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    return (entity->GetType() & 0x0A080000ULL) == 0x0A080000ULL;
}

HangingEntity::HangingEntitySelector::~HangingEntitySelector()
{
}
