#ifndef GAME_CORE_ANONYMOUSDATAWALKER_HPP
#define GAME_CORE_ANONYMOUSDATAWALKER_HPP

class CompoundTag;
class DataFixer;

class AnonymousDataWalker
{
public:
    virtual ~AnonymousDataWalker();
    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);
};

#endif
