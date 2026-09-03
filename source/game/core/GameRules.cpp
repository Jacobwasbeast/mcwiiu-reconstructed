#include "game/core/GameRules.hpp"
#include "game/core/CMinecraftApp.hpp"

GameRules::GameRules()
{
}

GameRules::~GameRules()
{
}

bool GameRules::getBoolean(int rule)
{
    switch (rule)
    {
    case 0:
        return CMinecraftApp::sInstance.GetGameHostOption((eGameHostOption)0xe) != 0;
    case 1:
        return CMinecraftApp::sInstance.GetGameHostOption((eGameHostOption)0x18) != 0;
    case 2:
        return CMinecraftApp::sInstance.GetGameHostOption((eGameHostOption)0x19) != 0;
    case 3:
        return CMinecraftApp::sInstance.GetGameHostOption((eGameHostOption)0x1a) != 0;
    case 4:
        return CMinecraftApp::sInstance.GetGameHostOption((eGameHostOption)0x1b) != 0;
    case 5:
    case 6:
        return CMinecraftApp::sInstance.GetGameHostOption((eGameHostOption)0x1c) != 0;
    case 7:
        return CMinecraftApp::sInstance.GetGameHostOption((eGameHostOption)0x1d) != 0;
    case 8:
        return CMinecraftApp::sInstance.GetGameHostOption((eGameHostOption)0x1e) != 0
            && CMinecraftApp::sInstance.GetGameHostOption((eGameHostOption)0x1f) == 0;
    case 9:
        return true;
    case 0xb:
        return true;
    case 0x12:
        return CMinecraftApp::sInstance.GetGameHostOption((eGameHostOption)0x15) != 0;
    default:
        return false;
    }
}

unsigned int GameRules::getInt(int rule)
{
    if (rule == 0xf)
        return 10;
    if (rule != 0x11)
        return 0;
    return 0x18;
}

void GameRules::set(int rule, bool value)
{
}
