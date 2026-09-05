#include "game/core/GameSettings.hpp"

extern GameSettings gGameSettings[4];
extern unsigned char gDefaultActionMappings[4][25];
extern unsigned char gSecondaryActionMappings[4][25];
extern bool gTumbleMapFlags[3];

static GameSettings* settings(int profile)
{
    if (profile == 255) profile = 0;
    if (profile < 0 || profile >= 4) return 0;
    return &gGameSettings[profile];
}

uint32_t GameSettings::GetGameSettings(int profile, eGameSetting setting)
{
    GameSettings* s = settings(profile); if (!s) return 0;
    switch ((int)setting) {
    case 0: return s->mYAxis; case 1: return s->mSensitivity; case 2: return s->mVibration;
    case 3: return s->mOptionFlags & 3; case 4: return s->mSwapSticks; case 5: return s->mLanguage;
    case 6: return (s->mOptionFlags >> 2) & 1; case 7: return (s->mOptionFlags >> 4) & 3;
    case 8: return (s->mOptionFlags >> 6) & 1; case 9: return (s->mOptionFlags >> 7) & 1;
    case 10: return (s->mDebugMask >> 23) & 1; case 11: return (s->mOptionFlags >> 8) & 1;
    case 12: return (s->mOptionFlags >> 3) & 1; case 13: return (s->mOptionFlags >> 11) & 15;
    case 14: return (s->mOptionFlags >> 9) & 1; case 15: return (s->mOptionFlags >> 10) & 1;
    case 16: return s->mLanguage; case 17: return s->mOptionFlags >> 15;
    case 18: return s->mDebugMask & 1; case 19: return (s->mDebugMask >> 1) & 1;
    case 20: return (s->mDebugMask >> 2) & 1; case 21: return (s->mDebugMask >> 3) & 1;
    case 22: return (s->mDebugMask >> 4) & 3; case 23: return (s->mDebugMask >> 7) & 1;
    case 24: return (s->mDebugMask >> 8) & 1; case 25: return (s->mDebugMask >> 9) & 1;
    case 26: return (s->mDebugMask >> 10) & 1; case 27: return (s->mDebugMask >> 11) & 3;
    case 28: return (s->mDebugMask >> 13) & 3; case 29: return (s->mDebugMask >> 15) & 1;
    case 30: return (s->mDebugMask >> 16) & 1; case 31: return (s->mDebugMask >> 17) & 1;
    case 32: return (s->mDebugMask >> 18) & 1; case 33: return (s->mDebugMask >> 19) & 1;
    case 34: return (s->mDebugMask >> 20) & 1; case 35: return (s->mDebugMask >> 21) & 1;
    case 36: return (s->mDebugMask >> 24) & 3; case 37: return (s->mDebugMask >> 27) & 1;
    case 38: return (s->mDebugMask >> 28) & 1; case 39: return (s->mDebugMask >> 29) & 1;
    case 40: return (s->mDebugMask >> 30) & 1; case 41: return s->mDebugMask >> 31;
    case 42: return (s->mDebugMask >> 22) & 1; case 43: return s->mDebugMask >> 31;
    case 44: return (s->mDebugMask >> 26) & 1; case 45: return s->mSkinFlags & 15;
    default: return 0;
    }
}
uint8_t GameSettings::GetActionMapping(int profile, int action) { return profile >= 0 && profile < 4 && action >= 0 && action < 25 ? gDefaultActionMappings[profile][action] : 0; }
void GameSettings::SetActionMapping(int profile, int action, unsigned char value) { if (profile >= 0 && profile < 4 && action >= 0 && action < 25) gDefaultActionMappings[profile][action] = value; }
void GameSettings::UpdateActionMapping(int profile, int group, bool secondary) { int i; for (i = 0; i < 5; ++i) { int action = group * 5 + i; unsigned char value = secondary ? gSecondaryActionMappings[profile][action] : gDefaultActionMappings[profile][action]; if (secondary && value == 1) value = 2; SetActionMapping(profile, action, value); } }
uint32_t GameSettings::GetInputButton(unsigned int value) { static const unsigned int buttons[15] = { 0, 1, 2, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000, 0x10000, 0x20000, 0x40000, 0x80000 }; return value < 15 ? buttons[value] : 0; }
uint32_t GameSettings::GetActionVal(unsigned char value) { return GetInputButton(value & 15) | GetInputButton((value >> 4) & 255); }
void GameSettings::ActionGameSettings(int, eGameSetting) {}
void GameSettings::SetSettingsChanged(int profile) { GameSettings* s = settings(profile); if (s) s->mChanged = 1; }
void GameSettings::SetGameSettings(int profile, eGameSetting setting, unsigned char value) { GameSettings* s = settings(profile); if (!s) return; switch ((int)setting) { case 0: s->mYAxis = value; break; case 1: s->mSensitivity = value; break; case 2: s->mVibration = value; break; case 4: s->mSwapSticks = value; break; case 5: s->mLanguage = value; break; case 45: s->mSkinFlags = (s->mSkinFlags & 0xf0) | (value & 15); break; default: { unsigned int bit = 1u << ((int)setting & 31); s->mDebugMask = (s->mDebugMask & ~bit) | (value ? bit : 0); } } SetSettingsChanged(profile); }
uint8_t GameSettings::GetMinecraftLanguage(int profile) { GameSettings* s = settings(profile); return s ? s->mMinecraftLanguage : 0; }
uint8_t GameSettings::GetMinecraftLocale(int profile) { GameSettings* s = settings(profile); return s ? s->mMinecraftLocale : 0; }
void GameSettings::SetDefaultActionMapping(unsigned char controller, int action, unsigned char value) { if (controller < 4 && action >= 0 && action < 25) gDefaultActionMappings[controller][action] = value; }
void GameSettings::SetSecondaryActionMapping(unsigned char controller, int action, unsigned char value) { if (controller < 4 && action >= 0 && action < 25) gSecondaryActionMappings[controller][action] = value; }
void GameSettings::SetCustomisableControllerMapping(int profile, GameSettings* source) { if (!source || profile < 0 || profile >= 4) return; int i; for (i = 0; i < 25; ++i) gDefaultActionMappings[profile][i] = ((unsigned char*)source)[0xa4 + i]; }
uint64_t GameSettings::updateToLatestVersionFrom(unsigned short, int) { return 0; }
uint32_t GameSettings::OldProfileVersionCallback(void*, unsigned char*, unsigned short, int) { return 0; }
void GameSettings::SetDefaultControls(C4JStorage::PROFILESETTINGS* values, int profile, bool) { if (values) { SetGameSettings(profile, (eGameSetting)0, (unsigned char)(values->values[0] != 0)); SetGameSettings(profile, (eGameSetting)1, (unsigned char)values->values[1]); SetGameSettings(profile, (eGameSetting)2, (unsigned char)values->values[2]); } }
void GameSettings::SetDefaultGameOptions(C4JStorage::PROFILESETTINGS*, int profile, bool) { SetGameSettings(profile, (eGameSetting)0, 1); SetGameSettings(profile, (eGameSetting)1, 1); SetGameSettings(profile, (eGameSetting)2, 1); }
void GameSettings::InitGameSettings() { int i; for (i = 0; i < 4; ++i) SetDefaultGameOptions(0, i, false); }
void GameSettings::ClearGameSettingsChangedFlag(int profile) { GameSettings* s = settings(profile); if (s) s->mChanged = 0; }
void GameSettings::ApplyGameSettingsChanged(int profile) { ActionGameSettings(profile, (eGameSetting)0); ActionGameSettings(profile, (eGameSetting)1); ActionGameSettings(profile, (eGameSetting)2); ClearGameSettingsChangedFlag(profile); }
void GameSettings::CheckGameSettingsChanged(bool changed, int profile) { if (profile == 255) { int i; for (i = 0; i < 4; ++i) CheckGameSettingsChanged(changed, i); return; } GameSettings* s = settings(profile); if (s && s->mChanged) { ApplyGameSettingsChanged(profile); } }
uint64_t GameSettings::GetInputButton(int profile, bool secondary, EControllerActions action) { return GetInputButton(GetActionMapping(profile, (int)action + (secondary ? 0 : 0x20))); }
void GameSettings::SetPlayerCape(int profile, unsigned int id, bool changed) { GameSettings* s = settings(profile); if (s) { unsigned int old = s->mCapeId; s->mCapeId = id; if (changed) s->mChanged = 1; else s->mCapeId = old; } }
void GameSettings::SetPlayerCape(int profile, const std::wstring&) { SetPlayerCape(profile, 0, true); }
std::wstring GameSettings::GetPlayerCapeName(int) { return std::wstring(); }
uint32_t GameSettings::IsAllowedSkin(int) { return 1; }
uint32_t GameSettings::GetPlayerCapeId(int profile) { GameSettings* s = settings(profile); return s && IsAllowedSkin(profile) ? s->mCapeId : 0; }
void GameSettings::SetPlayerSkin(int profile, unsigned int id, bool changed) { GameSettings* s = settings(profile); if (s) { unsigned int old = s->mSkinId; s->mSkinId = id; if (changed) s->mChanged = 1; else s->mSkinId = old; } }
void GameSettings::SetPlayerSkin(int profile, const std::wstring&) { SetPlayerSkin(profile, 0, true); }
std::wstring GameSettings::GetPlayerSkinName(int) { return std::wstring(); }
uint32_t GameSettings::GetPlayerSkinId(int profile) { GameSettings* s = settings(profile); return s && IsAllowedSkin(profile) ? s->mSkinId : 0; }
void GameSettings::SetPlayerFavoriteSkin(int profile, int index, unsigned int id) { GameSettings* s = settings(profile); if (s && index >= 0 && index < 10) { s->mChanged = 1; s->mFavoriteSkins[index] = id; } }
uint32_t GameSettings::GetPlayerFavoriteSkin(int profile, int index) { GameSettings* s = settings(profile); return s && index >= 0 && index < 10 ? s->mFavoriteSkins[index] : 0; }
uint8_t GameSettings::GetPlayerFavoriteSkinsPos(int profile) { GameSettings* s = settings(profile); return s ? s->mFavoriteSkinsPos : 0; }
uint32_t GameSettings::GetPlayerFavoriteSkinsPos(int profile, unsigned int id) { int i; for (i = 0; i < 10; ++i) if (GetPlayerFavoriteSkin(profile, i) == id) return i; return 0; }
void GameSettings::SetPlayerFavoriteSkinsPos(int profile, int value) { GameSettings* s = settings(profile); if (s) { s->mChanged = 1; s->mFavoriteSkinsPos = (unsigned char)value; } }
uint32_t GameSettings::GetPlayerFavoriteSkinsCount(int profile) { GameSettings* s = settings(profile); if (!s) return 0; int i; for (i = 0; i < 10; ++i) if (s->mFavoriteSkins[i] == 0xffffffffu) return i; return 10; }
void GameSettings::ValidateFavoriteSkins(int profile) { GameSettings* s = settings(profile); if (!s) return; int count = GetPlayerFavoriteSkinsCount(profile); int i; for (i = count; i < 10; ++i) s->mFavoriteSkins[i] = 0xffffffffu; }
void GameSettings::HideMashupPackWorld(int profile, unsigned int id) { GameSettings* s = settings(profile); if (s) { s->mChanged = 1; s->mHiddenMashupWorlds &= ~(1u << ((id - 0x400) & 31)); } }
void GameSettings::SetBattleMiniGameMapSelection(int profile, unsigned int value) { GameSettings* s = settings(profile); if (s) { s->mChanged = 1; s->mBattleMiniGameMap = value; } }
uint32_t GameSettings::GetBattleMiniGameMapSelection(int profile) { GameSettings* s = settings(profile); return s ? s->mBattleMiniGameMap : 0; }
void GameSettings::SetGlideMiniGameMapSelection(int profile, unsigned int value) { GameSettings* s = settings(profile); if (s) { s->mChanged = 1; s->mGlideMiniGameMap = value; } }
uint32_t GameSettings::GetGlideMiniGameMapSelection(int profile) { GameSettings* s = settings(profile); return s ? s->mGlideMiniGameMap : 0; }
uint32_t GameSettings::GetTumbleMiniGameMapSelection() { if (gTumbleMapFlags[0]) return 2; if (gTumbleMapFlags[1]) return 4; return gTumbleMapFlags[2] ? 8 : 1; }
void GameSettings::SetMinecraftLanguage(int profile, unsigned char value) { GameSettings* s = settings(profile); if (s) { s->mChanged = 1; s->mMinecraftLanguage = value; } }
void GameSettings::SetMinecraftLocale(int profile, unsigned char value) { GameSettings* s = settings(profile); if (s) { s->mChanged = 1; s->mMinecraftLocale = value; } }
uint64_t GameSettings::SetGameSettings(int profile, eCustomGameModeHostOption option, unsigned int value) { GameSettings* s = settings(profile); if (s && option == eCustomGameModeHostOption_All) { s->mChanged = 1; s->mGameModeOptions = value; } return (uint64_t)(uint32_t)profile << 32; }
uint64_t GameSettings::GetGameSettings(eGameSetting setting) { return (uint64_t)GetGameSettings(0, setting) << 32; }
uint32_t GameSettings::GetGameSettingsDebugMask(int profile, bool) { GameSettings* s = settings(profile); return s ? s->mDebugMask : 0; }
uint32_t GameSettings::IsDebugSettingEnabled(eDebugSetting setting, int profile) { return (GetGameSettingsDebugMask(profile, false) & (1u << ((int)setting & 31))) != 0; }
void GameSettings::SetGameSettingsDebugMask(int profile, unsigned int value) { GameSettings* s = settings(profile); if (s) s->mDebugMask = value; }
uint32_t GameSettings::GetLoggingMask() { return GetGameSettingsDebugMask(0, false); }
void GameSettings::SetSpecialTutorialCompletionFlag(int profile, int flag) { GameSettings* s = settings(profile); if (s && flag >= 0 && flag < 32) s->mSpecialTutorialFlags |= 1u << flag; }
void GameSettings::SetTutorialCompletionFlag(int profile, int flag) { GameSettings* s = settings(profile); if (s && flag >= 0 && flag < 32) { s->mTutorialFlags |= 1u << (flag ^ 7); s->mChanged = 1; } }
bool GameSettings::GetTutorialCompletionFlag(int profile, int flag) { GameSettings* s = settings(profile); return s && flag >= 0 && flag < 32 && (s->mTutorialFlags & (1u << (flag ^ 7))) != 0; }
