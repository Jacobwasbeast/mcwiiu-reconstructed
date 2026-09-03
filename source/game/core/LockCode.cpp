#include "game/core/LockCode.hpp"

#include "game/data/CompoundTag.hpp"

static LockCode* NO_LOCK = new LockCode(L"");
static const std::wstring LOCK_TAG = L"Lock";

LockCode::LockCode(const std::wstring& key)
    : mKey(key)
{
}

LockCode::~LockCode()
{
}

bool LockCode::isEmpty()
{
    if (mKey.compare(L"") != 0 && !mKey.empty())
    {
        return false;
    }

    return true;
}

std::wstring LockCode::getKey()
{
    return mKey;
}

void LockCode::addTag(CompoundTag* tag)
{
    tag->putString(LOCK_TAG, mKey);
}

LockCode* LockCode::fromTag(CompoundTag* tag)
{
    if (tag->contains(LOCK_TAG, 8))
    {
        return new LockCode(tag->getString(LOCK_TAG));
    }

    return NO_LOCK;
}
