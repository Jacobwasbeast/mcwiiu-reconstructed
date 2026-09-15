#include "game/core/DLCGameRules.hpp"

DLCGameRules::DLCGameRules(const std::wstring& path)
    : DLCFile(DLCManager::eDLCType_GameRules, path)
{
}

DLCGameRules::~DLCGameRules() = default;
