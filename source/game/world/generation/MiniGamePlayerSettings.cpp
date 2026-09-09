#include "game/world/generation/MiniGamePlayerSettings.hpp"
#include "game/minigame/MiniGameDef.hpp"

static MiniGamePlayerSettings getMiniGamePlayerSettings()
{
    MiniGamePlayerSettings settings;
    MiniGameDef::GetPlayerSettings(settings);
    return settings;
}

MiniGamePlayerSettings::MiniGamePlayerSettings()
    : mPlayerAbilities()
    , mLivesPerRound(1)
    , mLootSetId(0)
    , mRandomLootSet(false)
    , mTeamCount(0)
    , mTeamMaxSize(1)
    , mTeamBasedSpawn(false)
    , mCheckpointBasedSpawn(false)
    , mShowNametags(false)
    , mTakeEverythingEnabled(true)
    , mChestRefillEnabled(true)
    , mCanTntDestroyBlocks()
    , mMapSizeOverride()
    , mRandomMapSize(false)
    , mArenaSizeMultiplier()
    , mLayerCount()
    , mRoundCount(0)
    , mSmallInventoryEnabled(true)
    , mNaturalRegeneration(true)
    , mFoodBalance(0)
    , mSmallSkinsEnabled(true)
    , mRoundLength(0)
    , mMaxPlayers(8)
    , mFriendlyFire(false)
    , mTeamGame(false)
    , mSpectatorParticipation(false)
{
}

MiniGamePlayerSettings::~MiniGamePlayerSettings()
{
}

bool MiniGamePlayerSettings::operator==(const MiniGamePlayerSettings& other) const
{
    return Get(eCustomGameModeHostOption_All) == other.Get(eCustomGameModeHostOption_All)
        && mPlayerAbilities == other.mPlayerAbilities
        && mLivesPerRound == other.mLivesPerRound
        && mLootSetId == other.mLootSetId
        && mRandomLootSet == other.mRandomLootSet
        && mTeamCount == other.mTeamCount
        && mTeamMaxSize == other.mTeamMaxSize
        && mTeamBasedSpawn == other.mTeamBasedSpawn
        && mCheckpointBasedSpawn == other.mCheckpointBasedSpawn
        && mShowNametags == other.mShowNametags
        && mTakeEverythingEnabled == other.mTakeEverythingEnabled
        && mChestRefillEnabled == other.mChestRefillEnabled
        && ((!mCanTntDestroyBlocks.isSet() && !other.mCanTntDestroyBlocks.isSet())
            || mCanTntDestroyBlocks.get() == other.mCanTntDestroyBlocks.get())
        && ((!mMapSizeOverride.isSet() && !other.mMapSizeOverride.isSet())
            || mMapSizeOverride.get() == other.mMapSizeOverride.get())
        && mRandomMapSize == other.mRandomMapSize
        && ((!mArenaSizeMultiplier.isSet() && !other.mArenaSizeMultiplier.isSet())
            || mArenaSizeMultiplier.get() == other.mArenaSizeMultiplier.get())
        && ((!mLayerCount.isSet() && !other.mLayerCount.isSet())
            || mLayerCount.get() == other.mLayerCount.get())
        && mRoundCount == other.mRoundCount
        && mSmallInventoryEnabled == other.mSmallInventoryEnabled
        && mNaturalRegeneration == other.mNaturalRegeneration
        && mFoodBalance == other.mFoodBalance
        && mSmallSkinsEnabled == other.mSmallSkinsEnabled
        && mRoundLength == other.mRoundLength
        && mMaxPlayers == other.mMaxPlayers
        && mFriendlyFire == other.mFriendlyFire
        && mTeamGame == other.mTeamGame
        && mSpectatorParticipation == other.mSpectatorParticipation;
}

GameModeOptions GetSetting__tm__67_15GameModeOptionsXCM22MiniGamePlayerSettings15GameModeOptionsL_1_1__Fv_Z1Z()
{
    return getMiniGamePlayerSettings();
}

unsigned int GetSetting__tm__38_UiXCM22MiniGamePlayerSettingsUiL_2_25__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mLootSetId;
}

unsigned char GetSetting__tm__36_bXCM22MiniGamePlayerSettingsbL_2_43__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mShowNametags;
}

unsigned int GetSetting__tm__38_UiXCM22MiniGamePlayerSettingsUiL_2_21__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mLivesPerRound;
}

unsigned int GetSetting__tm__38_UiXCM22MiniGamePlayerSettingsUiL_2_33__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mTeamCount;
}

unsigned char GetSetting__tm__36_bXCM22MiniGamePlayerSettingsbL_2_41__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mTeamBasedSpawn;
}

unsigned char GetSetting__tm__36_bXCM22MiniGamePlayerSettingsbL_2_42__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mCheckpointBasedSpawn;
}

unsigned char GetSetting__tm__37_bXCM22MiniGamePlayerSettingsbL_3_103__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mSpectatorParticipation;
}

unsigned char GetSetting__tm__36_bXCM22MiniGamePlayerSettingsbL_2_81__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mSmallInventoryEnabled;
}

unsigned char GetSetting__tm__36_bXCM22MiniGamePlayerSettingsbL_2_44__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mTakeEverythingEnabled;
}

unsigned char GetSetting__tm__36_bXCM22MiniGamePlayerSettingsbL_2_45__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mChestRefillEnabled;
}

Abilities GetSetting__tm__53_9AbilitiesXCM22MiniGamePlayerSettings9AbilitiesL_1_5__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mPlayerAbilities;
}

maybe<bool> GetSetting__tm__66_14maybe__tm__2_bXCM22MiniGamePlayerSettings14maybe__tm__2_bL_2_46__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mCanTntDestroyBlocks;
}

int GetSetting__tm__36_iXCM22MiniGamePlayerSettingsiL_2_85__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mRoundCount;
}

int GetSetting__tm__36_iXCM22MiniGamePlayerSettingsiL_2_77__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mFoodBalance;
}

maybe<unsigned int> GetSetting__tm__68_15maybe__tm__3_UiXCM22MiniGamePlayerSettings15maybe__tm__3_UiL_2_69__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mLayerCount;
}

maybe<float> GetSetting__tm__66_14maybe__tm__2_fXCM22MiniGamePlayerSettings14maybe__tm__2_fL_2_61__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mArenaSizeMultiplier;
}

maybe<int> GetSetting__tm__66_14maybe__tm__2_iXCM22MiniGamePlayerSettings14maybe__tm__2_iL_2_49__Fv_Z1Z()
{
    return getMiniGamePlayerSettings().mMapSizeOverride;
}
