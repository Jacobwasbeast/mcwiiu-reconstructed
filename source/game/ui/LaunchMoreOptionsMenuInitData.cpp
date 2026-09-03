#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"

LaunchMoreOptionsMenuInitData::LaunchMoreOptionsMenuInitData()
    : mFlag0(false),
      mFlag1(false),
      mFlag2(false),
      mFlag3(false),
      mFlag4(false),
      mFlag5(false),
      mFlag6(false),
      mFlag7(false),
      mFlag8(false),
      mFlag9(false),
      mFlagA(false),
      mFlagB(false),
      mFlagC(false),
      mFlagD(false),
      mFlagE(false),
      mFlagF(false),
      mFlag10(false),
      mFlag11(false),
      mFlag12(false),
      mFlag13(false),
      mFlag14(false),
      mFlag15(false),
      mFlag16(false),
      mPadding17(0),
      mValue18(-1),
      mValue1C(0),
      mDisplayName(L"buckets_"),
      mValue24(0),
      mPadding26(),
      mValue38(0),
      mValue3C(7),
      mFlag40(true),
      mPadding41(),
      mValue44(3),
      mFlag48(false),
      mPadding49(),
      mValue4C(0),
      mValue50(0),
      mFlag54(false),
      mFlag55(false),
      mFlag56(false),
      mFlag57(false),
      mFlag58(false),
      mPadding59(),
      mValue5C(15),
      mValue60(0),
      mValue64(0),
      mFlag68(false),
      mPadding69(),
      mPlayerSettings(),
      mFlagD4(false),
      mPaddingD5(),
      mValueD8(0),
      mValueDC(0),
      mSuperflatConfig(),
      mLevelRulesetGroups(),
      mValue13C(0)
{
    mFlag0 = true;
    mFlag2 = true;
    mFlag8 = true;
    mFlagA = true;
    mFlagB = true;
    mFlag11 = true;
    mFlag12 = true;
    mFlag13 = true;
    mFlag14 = true;
    mFlag15 = true;
    mFlag16 = true;
    mSuperflatConfig.makePreset(DefaultPresets::CLASSIC);
}
