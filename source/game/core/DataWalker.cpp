#include "game/core/DataWalker.hpp"

#include "game/data/CompoundTag.hpp"

bool DataWalker::CheckID(CompoundTag* tag, const std::wstring& id)
{
    return tag->getString(std::wstring(L"id")) == id;
}
