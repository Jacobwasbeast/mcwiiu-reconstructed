#ifndef GAME_CORE_KEYEDDATAWALKER_HPP
#define GAME_CORE_KEYEDDATAWALKER_HPP

#include "game/util/ResourceLocation.hpp"

class CompoundTag;
class DataFixer;

class KeyedDataWalker
{
public:
    explicit KeyedDataWalker(unsigned long long type);
    virtual ~KeyedDataWalker();

    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);

protected:
    virtual CompoundTag* doWalk(DataFixer* fixer, CompoundTag* tag, int version) = 0;

    ResourceLocation mKey;
};

#endif
