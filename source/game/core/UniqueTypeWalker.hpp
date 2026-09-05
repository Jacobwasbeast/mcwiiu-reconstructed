#ifndef GAME_CORE_UNIQUETYPEWALKER_HPP
#define GAME_CORE_UNIQUETYPEWALKER_HPP

#include "game/util/ResourceLocation.hpp"

class CompoundTag;
class DataFixer;

class UniqueTypeWalker
{
public:
    explicit UniqueTypeWalker(const ResourceLocation& key);
    virtual ~UniqueTypeWalker();

    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);

private:
    ResourceLocation mKey;
};

#endif
