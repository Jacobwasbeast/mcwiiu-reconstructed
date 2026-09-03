#ifndef GAME_WORLD_LEVELDATA_DATAWALKER_HPP
#define GAME_WORLD_LEVELDATA_DATAWALKER_HPP

class CompoundTag;
class DataFixer;

class LEVELDATA_DataWalker
{
public:
    virtual ~LEVELDATA_DataWalker();
    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);
};

#endif
