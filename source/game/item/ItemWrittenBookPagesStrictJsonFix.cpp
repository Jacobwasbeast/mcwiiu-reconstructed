#include "game/item/ItemWrittenBookPagesStrictJsonFix.hpp"

#include "game/data/CompoundTag.hpp"

ItemWrittenBookPagesStrictJsonFix::~ItemWrittenBookPagesStrictJsonFix()
{
}

int ItemWrittenBookPagesStrictJsonFix::getVersion()
{
    return 165;
}

CompoundTag* ItemWrittenBookPagesStrictJsonFix::fix(CompoundTag* tag)
{
    return tag;
}
