#ifndef GAME_SERVER_SERVERPLAYERDATAWALKER_L0_ADDWALKERS_HPP
#define GAME_SERVER_SERVERPLAYERDATAWALKER_L0_ADDWALKERS_HPP

#include "game/core/DataWalker.hpp"

class CompoundTag;
class DataFixer;

class ServerPlayerDataWalker__L0__addWalkers__12ServerPlayerSFP14DataFixerUpper : public DataWalker
{
public:
    virtual ~ServerPlayerDataWalker__L0__addWalkers__12ServerPlayerSFP14DataFixerUpper();
    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);
};

#endif
