#ifndef GAME_WORLD_GENERATION_PLAYERDATAWALKER_HPP
#define GAME_WORLD_GENERATION_PLAYERDATAWALKER_HPP

#include <string>
#include <vector>

class CompoundTag;
class DataFixer;

class PlayerDataWalker
{
public:
    PlayerDataWalker(std::vector<std::wstring>* itemFields);
    virtual ~PlayerDataWalker();

    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);

private:
    std::vector<std::wstring>* mItemFields;
};

#endif
