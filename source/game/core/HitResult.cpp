#include "game/core/HitResult.hpp"
#include <sstream>


HitResult::HitResult(Vec3* location, const Direction* direction, const BlockPos& blockPos)
    : BlockPos(), mType(BLOCK), mDirection(direction), mLocation(NULL), mEntity()
{
    x = blockPos.x;
    y = blockPos.y;
    z = blockPos.z;
    mLocation = Vec3::newTemp(location);
}

HitResult::HitResult(Vec3* location, const Direction* direction)
    : BlockPos(), mType(BLOCK), mDirection(direction), mLocation(Vec3::newTemp(location)), mEntity()
{
}

HitResult::HitResult(boost::shared_ptr<Entity> entity)
    : BlockPos(), mType(ENTITY), mLocation(NULL), mEntity(entity)
{
    BlockPos entityBlock(entity);
    mLocation = Vec3::newTemp(entityBlock.x, entityBlock.y, entityBlock.z);
}

HitResult::HitResult(Type type, Vec3* location, const Direction* direction, const BlockPos& blockPos)
    : BlockPos(), mType(type), mDirection(direction), mLocation(NULL), mEntity()
{
    x = blockPos.x;
    y = blockPos.y;
    z = blockPos.z;
    mLocation = Vec3::newTemp(location);
}

HitResult::HitResult(boost::shared_ptr<Entity> entity, Vec3* location)
    : BlockPos(), mType(ENTITY), mLocation(location), mEntity(entity)
{
}

BlockPos HitResult::getBlockPos() const
{
    return BlockPos(x, y, z);
}

double HitResult::distanceTo(boost::shared_ptr<Entity> entity) const
{
    BlockPos entityBlock(entity);
    Vec3* entityLocation = Vec3::newTemp(entityBlock.x, entityBlock.y, entityBlock.z);
    return mLocation->distanceTo(entityLocation);
}

std::wstring HitResult::toString() const
{
    std::wstringstream result;
    result << L"HitResult{type=" << static_cast<int>(mType)
           << L", blockPos=" << x << L"," << y << L"," << z
           << L", direction=" << mDirection << L", location=" << mLocation
           << L", entity=" << mEntity.get() << L"}";
    return result.str();
}
