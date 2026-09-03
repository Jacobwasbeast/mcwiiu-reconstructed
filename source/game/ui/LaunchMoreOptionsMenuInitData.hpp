#ifndef GAME_UI_LAUNCHMOREOPTIONSMENUINITDATA_HPP
#define GAME_UI_LAUNCHMOREOPTIONSMENUINITDATA_HPP

#include <string>
#include <vector>

#include "game/world/generation/MiniGamePlayerSettings.hpp"
#include "game/core/SuperflatConfig.hpp"

namespace LevelRules
{
class LevelRulesetGroup;
}

class LaunchMoreOptionsMenuInitData
{
public:
    LaunchMoreOptionsMenuInitData();

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
    unsigned char mPadding17;
    int mValue18;
    int mValue1C;
    std::wstring mDisplayName;
    short mValue24;
    unsigned char mPadding26[18];
    int mValue38;
    int mValue3C;
    bool mFlag40;
    unsigned char mPadding41[3];
    int mValue44;
    bool mFlag48;
    unsigned char mPadding49[3];
    int mValue4C;
    int mValue50;
    bool mFlag54;
    bool mFlag55;
    bool mFlag56;
    bool mFlag57;
    bool mFlag58;
    unsigned char mPadding59[3];
    int mValue5C;
    int mValue60;
    int mValue64;
    bool mFlag68;
    unsigned char mPadding69[3];
    MiniGamePlayerSettings mPlayerSettings;
    bool mFlagD4;
    unsigned char mPaddingD5[3];
    int mValueD8;
    int mValueDC;
    SuperflatConfig mSuperflatConfig;
    std::vector<LevelRules::LevelRulesetGroup*> mLevelRulesetGroups;
    int mValue13C;
};

#endif
