#ifndef GAME_CORE_DLCGAMERULES_HPP
#define GAME_CORE_DLCGAMERULES_HPP

#include <string>

#include "game/core/DLCFile.hpp"

class DLCGameRules : public DLCFile
{
public:
    DLCGameRules(const std::wstring& path) : DLCFile(DLCManager::eDLCType_GameRules, path) {}
    virtual ~DLCGameRules();
};

#endif
