#ifndef GAME_UI_LAUNCHMOREOPTIONSMENUINITDATA_HPP
#define GAME_UI_LAUNCHMOREOPTIONSMENUINITDATA_HPP

#include <string>
#include <vector>

#include "game/world/generation/MiniGamePlayerSettings.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/world/LevelRules.hpp"

class LaunchMoreOptionsMenuInitData
{
public:
    LaunchMoreOptionsMenuInitData();

    friend class IUIScene_StartGame;
    friend class MiniGameDef;
    friend class UIScene_LoadMenu;
    friend class UIScene_LaunchMoreOptionsMenu;
    friend class UIScene_MapSelectMenu;
    friend class UIScene_BiomePreviewMenu;
    friend class UIScene_InGameSettingsListMenu;
    friend class UIScene_CreateWorldMenu;

private:
    bool mFlag0;
    bool mFlag1;
    bool mFlag2;
    bool mFlag3;
    bool mFlag4;
    bool mFlag5;
    bool mFlag6;
    bool mFlag7;
    bool mFlag8;
    bool mFlag9;
    bool mFlagA;
    bool mFlagB;
    bool mFlagC;
    bool mFlagD;
    bool mFlagE;
    bool mFlagF;
    bool mFlag10;
    bool mFlag11;
    bool mFlag12;
    bool mFlag13;
    bool mFlag14;
    bool mFlag15;
    bool mFlag16;
    bool mFlag17;
    int mValue18;
    int mValue1C;
    std::wstring mDisplayName;
    bool mFlag40;
    int mValue44;
    bool mFlag48;
    int mValue4C;
    int mValue50;
    bool mFlag54;
    bool mFlag55;
    bool mFlag56;
    bool mFlag57;
    bool mFlag58;
    int mValue5C;
    int mValue60;
    int mValue64;
    bool mFlag68;
    MiniGamePlayerSettings mPlayerSettings;
    int mValueB8;
    bool mFlagBC;
    bool mFlagBD;
    int mValueC0;
    bool mFlagD0;
    bool mFlagD2;
    unsigned char mFlagD4;
    int mValueD8;
    int mValueDC;
    SuperflatConfig mSuperflatConfig;
    std::vector<LevelRules::LevelRulesetGroup*> mLevelRulesetGroups;
    int mValue13C;
};

#endif
