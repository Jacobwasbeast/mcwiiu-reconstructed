#include "game/data/DataFix.hpp"

DataFix::~DataFix()
{
}

bool DataFix::CheckID(CompoundTag* tag, const std::wstring& id)
{
    return tag->getString(L"id") == id;
}
