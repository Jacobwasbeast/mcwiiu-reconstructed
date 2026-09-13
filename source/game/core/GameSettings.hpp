#ifndef GAME_CORE_GAMESETTINGS_HPP
#define GAME_CORE_GAMESETTINGS_HPP
#include <stdint.h>
#include <string>
#include "platform/wiiu/C4JStorage.hpp"
#include "game/core/GameModeOptions.hpp"
enum eGameSetting { eGameSetting_YAxis = 0, eGameSetting_SkinAdjustments = 0x2d };
enum EControllerActions : int { EControllerActions_First = 0 };
enum eDebugSetting { eDebugSetting_First = 0 };
class GameSettings
{
    unsigned char mChanged, mYAxis, mSensitivity, mSwapSticks, mVibration, mSkinFlags;
    unsigned short mOptionFlags;
    unsigned int mUnknown0x0C, mTutorialFlags, mUnknown0x10, mUnknown0x14, mUnknown0x18, mUnknown0x1c;
    unsigned int mUnknown0x20, mUnknown0x24, mUnknown0x28, mUnknown0x2c, mUnknown0x30, mUnknown0x34, mUnknown0x38, mUnknown0x3c, mUnknown0x40, mUnknown0x44, mUnknown0x48;
    unsigned int mSkinId;
    unsigned char mLanguage, mLocale;
    unsigned short mReserved;
    unsigned int mDebugMask, mSpecialTutorialFlags, mCapeId, mFavoriteSkins[10];
    unsigned char mFavoriteSkinsPos, mFavoriteSkinsCount;
    unsigned short mReserved2;
    unsigned int mHiddenMashupWorlds;
    unsigned char mMinecraftLanguage, mMinecraftLocale;
    unsigned short mReserved3;
    unsigned int mGameModeOptions;
    unsigned char mHostOptionVersion, mReserved4[3];
    unsigned int mBattleMiniGameMap, mUnknownA0;
    unsigned char mActionMappings[25];
    unsigned char mUnknownBD, mUnknownBE, mUnknownBF;
    unsigned int mGlideMiniGameMap;
public:
    static uint32_t GetGameSettings(int, eGameSetting);
    static uint8_t GetActionMapping(int, int);
    static void SetActionMapping(int, int, unsigned char);
    static void UpdateActionMapping(int, int, bool);
    static uint32_t GetInputButton(unsigned int);
    static uint32_t GetActionVal(unsigned char);
    static void ActionGameSettings(int, eGameSetting);
    static void SetSettingsChanged(int);
    static void SetGameSettings(int, eGameSetting, unsigned char);
    static uint8_t GetMinecraftLanguage(int);
    static uint8_t GetMinecraftLocale(int);
    static void SetDefaultActionMapping(unsigned char, int, unsigned char);
    static void SetSecondaryActionMapping(unsigned char, int, unsigned char);
    static void SetCustomisableControllerMapping(int, GameSettings*);
    uint64_t updateToLatestVersionFrom(unsigned short, int);
    static uint32_t OldProfileVersionCallback(void*, unsigned char*, unsigned short, int);
    static void SetDefaultControls(C4JStorage::PROFILESETTINGS*, int, bool);
    static void SetDefaultGameOptions(C4JStorage::PROFILESETTINGS*, int, bool);
    static void InitGameSettings();
    static void ClearGameSettingsChangedFlag(int);
    static void ApplyGameSettingsChanged(int);
    static void CheckGameSettingsChanged(bool, int);
    static uint64_t GetInputButton(int, bool, EControllerActions);
    static void SetPlayerCape(int, unsigned int, bool);
    static void SetPlayerCape(int, const std::wstring&);
    static std::wstring GetPlayerCapeName(int);
    static uint32_t IsAllowedSkin(int);
    static uint32_t GetPlayerCapeId(int);
    static void SetPlayerSkin(int, unsigned int, bool);
    static void SetPlayerSkin(int, const std::wstring&);
    static std::wstring GetPlayerSkinName(int);
    static uint32_t GetPlayerSkinId(int);
    static void SetPlayerFavoriteSkin(int, int, unsigned int);
    static uint32_t GetPlayerFavoriteSkin(int, int);
    static uint8_t GetPlayerFavoriteSkinsPos(int);
    static uint32_t GetPlayerFavoriteSkinsPos(int, unsigned int);
    static void SetPlayerFavoriteSkinsPos(int, int);
    static uint32_t GetPlayerFavoriteSkinsCount(int);
    static void ValidateFavoriteSkins(int);
    static void HideMashupPackWorld(int, unsigned int);
    static void SetBattleMiniGameMapSelection(int, unsigned int);
    static uint32_t GetBattleMiniGameMapSelection(int);
    static void SetGlideMiniGameMapSelection(int, unsigned int);
    static uint32_t GetGlideMiniGameMapSelection(int);
    static uint32_t GetTumbleMiniGameMapSelection();
    static void SetMinecraftLanguage(int, unsigned char);
    static void SetMinecraftLocale(int, unsigned char);
    static uint64_t SetGameSettings(int, eCustomGameModeHostOption, unsigned int);
    static uint64_t GetGameSettings(eGameSetting);
    static uint32_t GetGameSettingsDebugMask(int, bool);
    static uint32_t IsDebugSettingEnabled(eDebugSetting, int);
    static void SetGameSettingsDebugMask(int, unsigned int);
    static uint32_t GetLoggingMask();
    static void SetSpecialTutorialCompletionFlag(int, int);
    static void SetTutorialCompletionFlag(int, int);
    static bool GetTutorialCompletionFlag(int, int);
    static void ActionDebugMask(int, bool);
    GameSettings();
    ~GameSettings();
    static void EnableMashupPackWorlds(int);
    static uint32_t GetMashupPackWorlds(int);
    static uint64_t GetGameSettings(int, eCustomGameModeHostOption);
    static void SetLoggingMask(unsigned int);
    static void ResetAllTutorialCompletionFlags(int);
};
#endif
