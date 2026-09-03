#include "game/world/block/LockableContainerBlockEntity.hpp"

#include "game/core/LockCode.hpp"
#include "game/data/CompoundTag.hpp"

LockableContainerBlockEntity::LockableContainerBlockEntity()
{
    mLockKeyCode = NULL;
}

LockableContainerBlockEntity::~LockableContainerBlockEntity()
{
}

std::wstring LockableContainerBlockEntity::getDisplayName()
{
    if (hasCustomName())
    {
        return getCustomName();
    }
    return getName();
}

bool LockableContainerBlockEntity::isLocked()
{
    return mLockKeyCode != NULL && !mLockKeyCode->isEmpty();
}

void LockableContainerBlockEntity::setLocked(LockCode* lockCode)
{
    mLockKeyCode = lockCode;
}

LockCode* LockableContainerBlockEntity::getLockKeyCode()
{
    return mLockKeyCode;
}

void LockableContainerBlockEntity::unlock()
{
    mLockKeyCode = NULL;
}

void LockableContainerBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mLockKeyCode = LockCode::fromTag(tag);
}

void LockableContainerBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    if (mLockKeyCode != NULL)
    {
        mLockKeyCode->addTag(tag);
    }
}
