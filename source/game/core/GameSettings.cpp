#include "game/core/GameSettings.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/DLCSkinFile.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/core/StatsCounter.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/CProfile.hpp"
extern CStorage* gStorage;

GameSettings* gGameSettings[4] = { NULL, NULL, NULL, NULL };
unsigned char gDefaultActionMappings[3][25];
unsigned char gSecondaryActionMappings[3][25];
bool gHalloweenEvent;
bool gFestiveEvent;
bool gSeasonalEvent;
extern CInput* g_pInput;

extern "C" void DefineActions()
{
    static const unsigned char defaultMappings[3][25] =
    {
        { 2, 0xc, 0xb, 10, 9, 4, 1, 0xd, 3, 0xe, 5, 6, 7, 8, 0, 0, 10, 9, 0xb, 1, 3, 1, 3, 1, 4 },
        { 9, 0xb, 0xc, 5, 6, 4, 1, 0xe, 3, 10, 5, 6, 7, 8, 0, 0, 6, 6, 0xc, 1, 3, 1, 3, 1, 4 },
        { 0xc, 0xb, 2, 5, 6, 4, 1, 10, 3, 0xe, 5, 6, 7, 8, 0, 0, 5, 6, 2, 1, 3, 1, 3, 1, 4 }
    };
    static const unsigned char secondaryMappings[3][25] =
    {
        { 2, 0xc, 0xb, 10, 9, 4, 1, 8, 3, 7, 0, 0, 0, 0, 0, 0, 10, 9, 0xb, 1, 3, 1, 3, 1, 4 },
        { 9, 0xb, 0xc, 5, 6, 4, 1, 8, 3, 10, 0, 0, 0, 0, 0, 0, 6, 6, 0xc, 1, 3, 1, 3, 1, 4 },
        { 0xc, 0xb, 2, 5, 6, 4, 1, 10, 3, 7, 0, 0, 0, 0, 0, 0, 5, 6, 2, 1, 3, 1, 3, 1, 4 }
    };

    int controller;
    int action;
    for (controller = 0; controller < 3; ++controller)
    {
        for (action = 0; action < 25; ++action)
        {
            GameSettings::SetDefaultActionMapping((unsigned char)controller, action, defaultMappings[controller][action]);
            GameSettings::SetSecondaryActionMapping((unsigned char)controller, action, secondaryMappings[controller][action]);
        }
    }

    static const unsigned char movementMasks[2][2] =
    {
        { 2, 1 },
        { 1, 2 }
    };
    static const unsigned char actionIndices[4] = { EControllerActions_MenuA, EControllerActions_MenuOk,
                                                    EControllerActions_MenuB, EControllerActions_MenuCancel };
    static const unsigned int actionMasks[32] =
    {
        4, 8, 0x200400, 0x100800, 0x9000, 0x6000, 0x800000, 0x400000,
        0x40, 0x80, 0x10, 0x20, 0x20000, 0x10000, 0x80000, 0x40000,
        2, 0x200000, 0x100000, 0x8000, 0x4000, 0x80000, 0x40000, 0x20000,
        0x10000, 0x800000, 0x400000, 0x40, 0x80, 8, 0x10, 1
    };
    static const unsigned char actionMapIndices[32] =
    {
        EControllerActions_MenuX,
        EControllerActions_MenuY,
        EControllerActions_MenuUp,
        EControllerActions_MenuDown,
        EControllerActions_MenuLeft,
        EControllerActions_MenuRight,
        EControllerActions_MenuPageUp,
        EControllerActions_MenuPageDown,
        EControllerActions_MenuRightScroll,
        EControllerActions_MenuLeftScroll,
        EControllerActions_MenuStart,
        EControllerActions_MenuBack,
        EControllerActions_MenuOtherStickUp,
        EControllerActions_MenuOtherStickDown,
        EControllerActions_MenuOtherStickLeft,
        EControllerActions_MenuOtherStickRight,
        EControllerActions_Jump,
        EControllerActions_MoveForward,
        EControllerActions_MoveBackward,
        EControllerActions_MoveLeft,
        EControllerActions_MoveRight,
        EControllerActions_LookUp,
        EControllerActions_LookDown,
        EControllerActions_LookLeft,
        EControllerActions_LookRight,
        EControllerActions_Use,
        EControllerActions_Action,
        EControllerActions_CycleHeldItemRight,
        EControllerActions_CycleHeldItemLeft,
        EControllerActions_Inventory,
        EControllerActions_Pause,
        EControllerActions_Drop
    };

    for (controller = 0; controller < 2; ++controller)
    {
        const unsigned char* controllerMovementMasks =
            movementMasks[g_pInput->IsCircleCrossSwapped() != 0 ? 1 : 0];
        g_pInput->SetGameJoypadMaps(controller, actionIndices[0], controllerMovementMasks[0]);
        g_pInput->SetGameJoypadMaps(controller, actionIndices[1], controllerMovementMasks[0]);
        g_pInput->SetGameJoypadMaps(controller, actionIndices[2], controllerMovementMasks[1]);
        g_pInput->SetGameJoypadMaps(controller, actionIndices[3], controllerMovementMasks[1]);
        for (action = 0; action < 32; ++action)
        {
            g_pInput->SetGameJoypadMaps(controller, actionMapIndices[action], actionMasks[action]);
        }
        if (controller == 0)
        {
            g_pInput->SetGameJoypadMaps(0, EControllerActions_Sneak, 0x100);
            g_pInput->SetGameJoypadMaps(0, EControllerActions_ChangeCameraMode, 0x200);
            g_pInput->SetGameJoypadMaps(0, EControllerActions_MenuStickPress, 0x200);
            g_pInput->SetGameJoypadMaps(0, EControllerActions_MenuOtherStickPress, 0x100);
        }
        else
        {
            g_pInput->SetGameJoypadMaps(controller, EControllerActions_Sneak, 0x800);
            g_pInput->SetGameJoypadMaps(controller, EControllerActions_ChangeCameraMode, 0x400);
            g_pInput->SetGameJoypadMaps(controller, EControllerActions_MenuStickPress, 0x200);
            g_pInput->SetGameJoypadMaps(controller, EControllerActions_MenuOtherStickPress, 0x100);
        }
        g_pInput->SetGameJoypadMaps(controller, EControllerActions_Crafting, 4);
        g_pInput->SetGameJoypadMaps(controller, EControllerActions_Scoreboard, 0x20);
        g_pInput->SetGameJoypadMaps(controller, EControllerActions_FlyLeft, 0x1000);
        g_pInput->SetGameJoypadMaps(controller, EControllerActions_FlyRight, 0x2000);
        g_pInput->SetGameJoypadMaps(controller, EControllerActions_FlyUp, 0x400);
        g_pInput->SetGameJoypadMaps(controller, EControllerActions_FlyDown, 0x800);
        g_pInput->SetGameJoypadMaps(controller, 0x39, 0x1000);
        g_pInput->SetGameJoypadMaps(controller, 0x3a, 0x2000);
        g_pInput->SetGameJoypadMaps(controller, 0x3b, 0x400);
        g_pInput->SetGameJoypadMaps(controller, 0x3c, 0x800);
    }
}

static GameSettings* settings(int profile)
{
    if (profile == 255)
    {
        profile = gProfile != NULL ? gProfile->GetPrimaryPad() : 0;
        if (profile < 0)
            profile = 0;
    }
    if (profile < 0 || profile >= 4) return 0;
    return gGameSettings[profile];
}

uint32_t GameSettings::GetGameSettings(int profile, eGameSetting setting)
{
    if (profile == 255)
        profile = gProfile->GetPrimaryPad();
    GameSettings* s = settings(profile);
    if (s == NULL)
        return 0;
    switch (setting)
    {
    case eGameSetting_MusicVolume:
        return s->mMusicVolume;
    case eGameSetting_SoundVolume:
        return s->mSoundVolume;
    case eGameSetting_Gamma:
        return s->mGamma;
    case eGameSetting_Difficulty:
        return s->mPackedSettings & 3;
    case eGameSetting_GameSensitivity:
        return s->mGameSensitivity;
    case eGameSetting_InterfaceSensitivity:
        return s->mInterfaceSensitivity;
    case eGameSetting_ViewBob:
        return (s->mPackedSettings >> 2) & 1;
    case eGameSetting_ControlScheme:
        return (s->mPackedSettings >> 4) & 3;
    case eGameSetting_InvertLook:
        return (s->mPackedSettings >> 6) & 1;
    case eGameSetting_SouthPaw:
        return (s->mPackedSettings >> 7) & 1;
    case eGameSetting_SecondaryControls:
        return (s->mSettingFlags >> 23) & 1;
    case eGameSetting_SplitScreenVertical:
        return (s->mPackedSettings >> 8) & 1;
    case eGameSetting_DisplayMii:
        return (s->mPackedSettings >> 3) & 1;
    case eGameSetting_Autosave:
        return (s->mPackedSettings >> 11) & 15;
    case eGameSetting_SplitscreenNicknames:
        return (s->mPackedSettings >> 9) & 1;
    case eGameSetting_Hints:
        return (s->mPackedSettings >> 10) & 1;
    case eGameSetting_InterfaceOpacity:
        return s->mInterfaceOpacity;
    case eGameSetting_Tooltips:
        return s->mPackedSettings >> 15;
    case eGameSetting_Clouds:
        return s->mSettingFlags & 1;
    case eGameSetting_Online:
        return (s->mSettingFlags >> 1) & 1;
    case eGameSetting_InviteOnly:
        return (s->mSettingFlags >> 2) & 1;
    case eGameSetting_FriendsOfFriends:
        return (s->mSettingFlags >> 3) & 1;
    case eGameSetting_DisplayUpdateMessage:
        return (s->mSettingFlags >> 4) & 3;
    case eGameSetting_DisplayHUD:
        return (s->mSettingFlags >> 7) & 1;
    case eGameSetting_DisplayHand:
        return (s->mSettingFlags >> 8) & 1;
    case eGameSetting_CustomSkinAnim:
        return (s->mSettingFlags >> 9) & 1;
    case eGameSetting_DeathMessages:
        return (s->mSettingFlags >> 10) & 1;
    case eGameSetting_HUDSize:
        return (s->mSettingFlags >> 11) & 3;
    case eGameSetting_HUDSizeSplitscreen:
        return (s->mSettingFlags >> 13) & 3;
    case eGameSetting_AnimatedCharacter:
        return (s->mSettingFlags >> 15) & 1;
    case eGameSetting_EULAAccepted:
        return (s->mSettingFlags >> 16) & 1;
    case eGameSetting_Reserved:
        return (s->mSettingFlags >> 17) & 1;
    case eGameSetting_ClassicCrafting:
        return (s->mSettingFlags >> 18) & 1;
    case eGameSetting_CaveSounds:
        return (s->mSettingFlags >> 19) & 1;
    case eGameSetting_GameChat:
        return (s->mSettingFlags >> 20) & 1;
    case eGameSetting_MinecartSounds:
        return (s->mSettingFlags >> 21) & 1;
    case eGameSetting_PublicMiniGamePrompts:
        return (s->mSettingFlags >> 24) & 3;
    case eGameSetting_UpdateMessageSeen:
        return (s->mSettingFlags >> 27) & 1;
    case eGameSetting_DisplayGameMessages:
        return (s->mSettingFlags >> 28) & 1;
    case eGameSetting_DisplaySaveIcon:
        return (s->mSettingFlags >> 29) & 1;
    case eGameSetting_FlyingViewRolling:
        return (s->mSettingFlags >> 30) & 1;
    case eGameSetting_ControlMappings:
        return 0;
    case eGameSetting_ShowGlideGhost:
        return (s->mSettingFlags >> 22) & 1;
    case eGameSetting_ShowGlideGhostPath:
        return s->mSettingFlags >> 31;
    case eGameSetting_AutoJump:
        return (s->mSettingFlags >> 26) & 1;
    case eGameSetting_SkinAdjustments:
        return s->mSkinFlags & 15;
    default:
        return 0;
    }
}
uint8_t GameSettings::GetActionMapping(int profile, int action)
{
    GameSettings* current = settings(profile);
    if (current == NULL || action < 0 || action >= 25)
    {
        return 0;
    }
    return current->mActionMappings[action];
}
void GameSettings::SetActionMapping(int profile, int action, unsigned char value)
{
    GameSettings* current = settings(profile);
    if (current != NULL && action >= 0 && action < 25)
    {
        current->mActionMappings[action] = value;
    }
}

void GameSettings::UpdateActionMapping(int profile, int scheme, bool swapPrimaryButtons)
{
    for (int action = 0; action < 25; ++action)
    {
        unsigned char value = g_pInput->IsPadClassicController(profile)
            ? gSecondaryActionMappings[scheme][action]
            : gDefaultActionMappings[scheme][action];
        if (swapPrimaryButtons)
        {
            if (value == 1)
                value = 2;
            else if (value == 2)
                value = 1;
        }
        SetActionMapping(profile, action, value);
    }
}

uint32_t GameSettings::GetInputButton(unsigned int value)
{
    if (value < 3)
        return value;
    static const unsigned int buttons[15] =
    {
        0, 0, 0, 4, 8, 0x1000, 0x2000, 0x400,
        0x800, 0x40, 0x80, 0x400000, 0x800000, 0x100, 0x200
    };
    return value < 15 ? buttons[value] : 0;
}
uint32_t GameSettings::GetActionVal(unsigned char value) { return GetInputButton(value & 15) | GetInputButton((value >> 4) & 255); }
void GameSettings::ActionGameSettings(int profile, eGameSetting setting)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    switch (setting)
    {
    case eGameSetting_MusicVolume:
        if (profile == gProfile->GetPrimaryPad())
            minecraft->mOptions->set(Options::Option::getItem(0),
                static_cast<float>(settings(profile)->mMusicVolume) / 100.0f);
        return;
    case eGameSetting_SoundVolume:
        if (profile == gProfile->GetPrimaryPad())
            minecraft->mOptions->set(Options::Option::getItem(1),
                static_cast<float>(settings(profile)->mSoundVolume) / 100.0f);
        return;
    case eGameSetting_Gamma:
        if (profile == gProfile->GetPrimaryPad())
        {
            const float gamma = static_cast<float>(settings(profile)->mGamma) * 327.68f;
            gRenderer->UpdateGamma(static_cast<unsigned short>(static_cast<int>(gamma) & 0xffff));
        }
        return;
    case eGameSetting_Difficulty:
        if (profile == gProfile->GetPrimaryPad())
        {
            minecraft->mOptions->toggle(Options::Option::getItem(9), settings(profile)->mPackedSettings & 3);
            g_pApp->SetGameHostOption(eGameHostOption_Difficulty, GetGameSettings(profile, eGameSetting_Difficulty));
        }
        return;
    case eGameSetting_GameSensitivity:
        minecraft->mOptions->set(Options::Option::getItem(3),
            static_cast<float>(settings(profile)->mGameSensitivity) / 100.0f);
        return;
    case eGameSetting_SouthPaw:
        {
            const bool swapAnalogSticks = (settings(profile)->mPackedSettings & 0x80) != 0;
            if (!swapAnalogSticks)
            {
                g_pInput->SetJoypadStickAxisMap(profile, 0, 0);
                g_pInput->SetJoypadStickAxisMap(profile, 1, 1);
                g_pInput->SetJoypadStickAxisMap(profile, 2, 2);
                g_pInput->SetJoypadStickAxisMap(profile, 3, 3);
                g_pInput->SetJoypadStickTriggerMap(profile, 0, 0);
                g_pInput->SetJoypadStickTriggerMap(profile, 1, 1);
            }
            else
            {
                g_pInput->SetJoypadStickAxisMap(profile, 0, 2);
                g_pInput->SetJoypadStickAxisMap(profile, 1, 3);
                g_pInput->SetJoypadStickAxisMap(profile, 2, 0);
                g_pInput->SetJoypadStickAxisMap(profile, 3, 1);
                g_pInput->SetJoypadStickTriggerMap(profile, 0, 1);
                g_pInput->SetJoypadStickTriggerMap(profile, 1, 0);
            }
        }
        return;
    case eGameSetting_SplitScreenVertical:
        if (profile == gProfile->GetPrimaryPad())
            minecraft->updatePlayerViewportAssignments();
        return;
    case eGameSetting_InterfaceOpacity:
        gConsoleUIController.RefreshTooltips(static_cast<unsigned int>(profile));
        return;
    case eGameSetting_Tooltips:
        {
            GameSettings* current = settings(profile);
            gConsoleUIController.SetEnableTooltips(
                static_cast<unsigned int>(profile), current != NULL && (current->mPackedSettings & 0x8000) != 0);
        }
        return;
    case eGameSetting_ControlMappings:
        {
            const int scheme = static_cast<int>(GetGameSettings(profile, eGameSetting_ControlScheme));
            const bool swapPrimaryButtons = GetGameSettings(profile, eGameSetting_SecondaryControls) != 0;
            if (scheme > 0 || swapPrimaryButtons)
            {
                UpdateActionMapping(profile, scheme, swapPrimaryButtons);
                SetGameSettings(profile, eGameSetting_ControlScheme, 0);
                SetGameSettings(profile, eGameSetting_SecondaryControls, 0);
            }
            for (int action = 0; action < 25; ++action)
            {
                g_pInput->SetGameJoypadMaps(profile, static_cast<unsigned char>(EControllerActions_Jump + action),
                    GetActionVal(GetActionMapping(profile, action)));
            }
        }
        return;
    case eGameSetting_SplitscreenNicknames:
        {
            if (minecraft->mLocalPlayers[0])
                gConsoleUIController.DisplayGamertag(0, minecraft->mLocalPlayers[0]->mViewport != 0);
            if (minecraft->mLocalPlayers[1])
                gConsoleUIController.DisplayGamertag(1, minecraft->mLocalPlayers[1]->mViewport != 0);
            if (minecraft->mLocalPlayers[2])
                gConsoleUIController.DisplayGamertag(2, minecraft->mLocalPlayers[2]->mViewport != 0);
            if (minecraft->mLocalPlayers[3])
                gConsoleUIController.DisplayGamertag(3, minecraft->mLocalPlayers[3]->mViewport != 0);
        }
        return;
    default:
        return;
    }
}
void GameSettings::SetSettingsChanged(int profile) { GameSettings* s = settings(profile); if (s) s->mChanged = 1; }
template <typename T>
void setGameSettingsHelper(int profile, eGameSetting setting, unsigned char value, T* field, T mask, unsigned int shift)
{
    const T oldValue = *field;
    const T newValue = static_cast<T>(value << shift);
    if ((mask & (oldValue ^ newValue)) != 0)
    {
        *field = static_cast<T>((oldValue & ~mask) | newValue);
        if (setting > 3 || profile == gProfile->GetPrimaryPad())
            GameSettings::ActionGameSettings(profile, setting);
        GameSettings::SetSettingsChanged(profile);
    }
}

void GameSettings::SetGameSettings(int profile, eGameSetting setting, unsigned char value)
{
    GameSettings* gameSettings = gGameSettings[profile];
    switch (setting)
    {
    case eGameSetting_MusicVolume:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mMusicVolume, 0xff, 0);
        break;
    case eGameSetting_SoundVolume:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mSoundVolume, 0xff, 0);
        break;
    case eGameSetting_Gamma:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mGamma, 0xff, 0);
        break;
    case eGameSetting_Difficulty:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x3, 0);
        break;
    case eGameSetting_GameSensitivity:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mGameSensitivity, 0xff, 0);
        break;
    case eGameSetting_InterfaceSensitivity:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mInterfaceSensitivity, 0xff, 0);
        break;
    case eGameSetting_ViewBob:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x4, 2);
        break;
    case eGameSetting_ControlScheme:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x30, 4);
        break;
    case eGameSetting_InvertLook:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x40, 6);
        break;
    case eGameSetting_SouthPaw:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x80, 7);
        break;
    case eGameSetting_SecondaryControls:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x800000, 23);
        break;
    case eGameSetting_SplitScreenVertical:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x100, 8);
        break;
    case eGameSetting_DisplayMii:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x8, 3);
        break;
    case eGameSetting_Autosave:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x7800, 11);
        break;
    case eGameSetting_SplitscreenNicknames:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x200, 9);
        break;
    case eGameSetting_Hints:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x400, 10);
        break;
    case eGameSetting_InterfaceOpacity:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mInterfaceOpacity, 0xff, 0);
        break;
    case eGameSetting_Tooltips:
        setGameSettingsHelper<unsigned short>(profile, setting, value, &gameSettings->mPackedSettings, 0x8000, 15);
        break;
    case eGameSetting_Clouds:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x1, 0);
        break;
    case eGameSetting_Online:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x2, 1);
        break;
    case eGameSetting_InviteOnly:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x4, 2);
        break;
    case eGameSetting_FriendsOfFriends:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x8, 3);
        break;
    case eGameSetting_DisplayUpdateMessage:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x30, 4);
        break;
    case eGameSetting_DisplayHUD:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x80, 7);
        break;
    case eGameSetting_DisplayHand:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x100, 8);
        break;
    case eGameSetting_CustomSkinAnim:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x200, 9);
        break;
    case eGameSetting_DeathMessages:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x400, 10);
        break;
    case eGameSetting_HUDSize:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x1800, 11);
        break;
    case eGameSetting_HUDSizeSplitscreen:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x6000, 13);
        break;
    case eGameSetting_AnimatedCharacter:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x8000, 15);
        break;
    case eGameSetting_EULAAccepted:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x10000, 16);
        break;
    case eGameSetting_Reserved:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x20000, 17);
        break;
    case eGameSetting_ClassicCrafting:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x40000, 18);
        break;
    case eGameSetting_CaveSounds:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x80000, 19);
        break;
    case eGameSetting_GameChat:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x100000, 20);
        break;
    case eGameSetting_MinecartSounds:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x200000, 21);
        break;
    case eGameSetting_PublicMiniGamePrompts:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x3000000, 24);
        break;
    case eGameSetting_UpdateMessageSeen:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x8000000, 27);
        break;
    case eGameSetting_DisplayGameMessages:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x10000000, 28);
        break;
    case eGameSetting_DisplaySaveIcon:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x20000000, 29);
        break;
    case eGameSetting_FlyingViewRolling:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x40000000, 30);
        break;
    case eGameSetting_ControlMappings:
        break;
    case eGameSetting_ShowGlideGhost:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x400000, 22);
        break;
    case eGameSetting_ShowGlideGhostPath:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x80000000, 31);
        break;
    case eGameSetting_AutoJump:
        setGameSettingsHelper<unsigned int>(profile, setting, value, &gameSettings->mSettingFlags, 0x4000000, 26);
        break;
    case eGameSetting_SkinAdjustments:
        setGameSettingsHelper<unsigned char>(profile, setting, value, &gameSettings->mSkinFlags, 0xf, 0);
        break;
    default:
        break;
    }
}

uint8_t GameSettings::GetMinecraftLanguage(int profile)
{
    GameSettings* s = settings(profile);
    return s ? s->mMinecraftLanguage : 0;
}

uint8_t GameSettings::GetMinecraftLocale(int profile)
{
    GameSettings* s = settings(profile);
    return s ? s->mMinecraftLocale : 0;
}

void GameSettings::SetDefaultActionMapping(unsigned char scheme, int action, unsigned char value)
{
    gDefaultActionMappings[scheme][action] = value;
}

void GameSettings::SetSecondaryActionMapping(unsigned char scheme, int action, unsigned char value)
{
    gSecondaryActionMappings[scheme][action] = value;
}
void GameSettings::SetCustomisableControllerMapping(int profile, GameSettings* settings)
{
    for (int action = 0; action < 25; ++action)
    {
        settings->mActionMappings[action] = g_pInput->IsPadClassicController(profile)
            ? gSecondaryActionMappings[0][action]
            : gDefaultActionMappings[0][action];
    }
}
void GameSettings::updateToLatestVersionFrom(unsigned short version, int profile)
{
    switch (version)
    {
    case 10: mHostOptionVersion = 0xff; updateToLatestVersionFrom(11, profile); break;
    case 11: mSettingFlags &= 0xfffeffffu; updateToLatestVersionFrom(12, profile); break;
    case 12: mMinecraftLanguage = 0; updateToLatestVersionFrom(13, profile); break;
    case 13: case 14: case 15: case 16: mSettingFlags |= 0x80000u; updateToLatestVersionFrom(17, profile); break;
    case 17: mSettingFlags &= 0xffffffbfu; updateToLatestVersionFrom(18, profile); break;
    case 18: case 19: case 20: mSettingFlags |= 0x200000u; updateToLatestVersionFrom(21, profile); break;
    case 21: case 22: mSettingFlags |= 0x3000000u; mHostOptionVersion = 0xff; updateToLatestVersionFrom(24, profile); break;
    case 23: mHostOptionVersion = 0xc0; updateToLatestVersionFrom(24, profile); break;
    case 24: mSettingFlags |= 0x30000000u; updateToLatestVersionFrom(25, profile); break;
    case 25: updateToLatestVersionFrom(27, profile); break;
    case 26: mSettingFlags &= 0xbfffffffU; updateToLatestVersionFrom(27, profile); break;
    case 27: case 28: SetCustomisableControllerMapping(profile, this); updateToLatestVersionFrom(29, profile); break;
    case 29: case 30: mSettingFlags |= 0x80400000u; updateToLatestVersionFrom(30, profile); break;
    case 31: mSettingFlags &= 0xfbffffffu; mHostOptionVersion = 0xff; updateToLatestVersionFrom(32, profile); break;
    case 32: mInterfaceSensitivity = 9; updateToLatestVersionFrom(33, profile); break;
    case 33: case 34: mSettingFlags |= 0x30u; break;
    default: break;
    }
}

uint32_t GameSettings::OldProfileVersionCallback(void*, unsigned char* data, unsigned short version, int profile)
{
    if (version < 15)
    {
        StatsCounter* stats = Minecraft::GetInstance()->mStats[profile];
        stats->clear();
        stats->parseOld(version, data);
        stats->save(profile, true, data);
    }
    reinterpret_cast<GameSettings*>(data)->updateToLatestVersionFrom(version, profile);
    return 0;
}
void GameSettings::SetDefaultControls(C4JStorage::PROFILESETTINGS* values, int profile, bool changed)
{
    SetGameSettings(profile, eGameSetting_InvertLook,
        static_cast<unsigned char>(values->values[0] != 0));
    SetGameSettings(profile, eGameSetting_SouthPaw, values->flags);

    for (int action = 0; action < 25; ++action)
    {
        unsigned char value = g_pInput->IsPadClassicController(profile)
            ? gSecondaryActionMappings[0][action]
            : gDefaultActionMappings[0][action];
        SetActionMapping(profile, action, value);
        g_pInput->SetGameJoypadMaps(profile, static_cast<unsigned char>(EControllerActions_Jump + action),
                                    GetActionVal(value));
    }
    settings(profile)->mChanged = static_cast<unsigned char>(changed);
}
void GameSettings::SetDefaultGameOptions(C4JStorage::PROFILESETTINGS* values, int profile, bool changed)
{
    SetGameSettings(profile, eGameSetting_MusicVolume, 100);
    SetGameSettings(profile, eGameSetting_SoundVolume, 100);
    SetGameSettings(profile, eGameSetting_Gamma, 0x32);

    if (Minecraft::GetInstance()->mLevel == NULL)
    {
        SetGameSettings(profile, eGameSetting_Difficulty, 1);
    }

    SetGameSettings(profile, eGameSetting_GameSensitivity, 100);
    SetGameSettings(profile, eGameSetting_ViewBob, 1);
    SetGameSettings(profile, eGameSetting_ControlScheme, 0);
    SetGameSettings(profile, eGameSetting_SplitScreenVertical, 0);
    SetGameSettings(profile, eGameSetting_DisplayMii, 1);
    SetGameSettings(profile, eGameSetting_InterfaceSensitivity, 100);
    SetGameSettings(profile, eGameSetting_SplitscreenNicknames, 1);
    SetGameSettings(profile, eGameSetting_Hints, 1);
    SetGameSettings(profile, eGameSetting_Autosave, 2);
    SetGameSettings(profile, eGameSetting_Tooltips, 1);
    SetGameSettings(profile, eGameSetting_InterfaceOpacity, 0x50);
    SetGameSettings(profile, eGameSetting_Clouds, 1);
    SetGameSettings(profile, eGameSetting_Online, 1);
    SetGameSettings(profile, eGameSetting_InviteOnly, 0);
    SetGameSettings(profile, eGameSetting_FriendsOfFriends, 0);
    SetGameSettings(profile, eGameSetting_DisplayHUD, 1);
    SetGameSettings(profile, eGameSetting_DisplayHand, 1);
    SetGameSettings(profile, eGameSetting_CustomSkinAnim, 1);
    SetGameSettings(profile, eGameSetting_DeathMessages, 1);
    SetGameSettings(profile, eGameSetting_HUDSize, 1);
    SetGameSettings(profile, eGameSetting_HUDSizeSplitscreen, 2);
    SetGameSettings(profile, eGameSetting_AnimatedCharacter, 1);

    GameSettings* current = settings(profile);
    for (int i = 0; i < 10; ++i)
    {
        current->mFavoriteSkins[i] = 0xffffffffu;
    }
    current->mFavoriteSkinsPos = 0;
    current->mHiddenMashupWorlds = 0xffffffffu;

    SetGameSettings(profile, eGameSetting_EULAAccepted, 0);
    if (g_pApp->GetGameStarted() == 0)
    {
        current->mMinecraftLocale = 0;
        current->mMinecraftLanguage = 0;
    }

    MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(eMiniGameId_Battle, true);
    const MiniGamePlayerSettings playerSettings = miniGame->GetDefaultPlayerSettings();
    current->mGameModeOptions = playerSettings.mHostOptions.Get(eCustomGameModeHostOption_All);

    SetGameSettings(profile, eGameSetting_ClassicCrafting, 0);
    SetGameSettings(profile, eGameSetting_CaveSounds, 1);
    SetGameSettings(profile, eGameSetting_MinecartSounds, 1);
    SetGameSettings(profile, eGameSetting_GameChat, 0);
    current->mBattleMiniGameMap = 0xffffffffu;
    SetGameSettings(profile, eGameSetting_SecondaryControls, 0);
    SetGameSettings(profile, eGameSetting_PublicMiniGamePrompts, 3);
    SetGameSettings(profile, eGameSetting_DisplayGameMessages, 1);
    SetGameSettings(profile, eGameSetting_DisplaySaveIcon, 1);
    SetGameSettings(profile, eGameSetting_FlyingViewRolling, 0);
    SetGameSettings(profile, eGameSetting_ShowGlideGhost, 1);
    SetGameSettings(profile, eGameSetting_ShowGlideGhostPath, 1);
    SetGameSettings(profile, eGameSetting_AutoJump, 0);
    current->mSkinFlags = 9;
    current->mGlideMiniGameMap = 0xffffffffu;
    SetDefaultControls(values, profile, changed);
}
void GameSettings::InitGameSettings()
{
    for (int profile = 0; profile < 4; ++profile)
    {
        gGameSettings[profile] = reinterpret_cast<GameSettings*>(gStorage->GetGameDefinedProfileData(profile));
        *reinterpret_cast<unsigned char*>(gGameSettings[profile]) = 0;
        SetDefaultGameOptions(gStorage->GetDashboardProfileSettings(profile), profile, false);
    }
}

void GameSettings::ClearGameSettingsChangedFlag(int profile)
{
    GameSettings* s = settings(profile);
    if (s)
    {
        s->mChanged = 0;
    }
}
void GameSettings::ApplyGameSettingsChanged(int profile)
{
    ActionGameSettings(profile, eGameSetting_MusicVolume);
    ActionGameSettings(profile, eGameSetting_SoundVolume);
    ActionGameSettings(profile, eGameSetting_Gamma);
    ActionGameSettings(profile, eGameSetting_Difficulty);
    ActionGameSettings(profile, eGameSetting_GameSensitivity);
    ActionGameSettings(profile, eGameSetting_ViewBob);
    ActionGameSettings(profile, eGameSetting_ControlScheme);
    ActionGameSettings(profile, eGameSetting_InvertLook);
    ActionGameSettings(profile, eGameSetting_SouthPaw);
    ActionGameSettings(profile, eGameSetting_SecondaryControls);
    ActionGameSettings(profile, eGameSetting_SplitScreenVertical);
    ActionGameSettings(profile, eGameSetting_DisplayMii);
    ActionGameSettings(profile, eGameSetting_InterfaceSensitivity);
    ActionGameSettings(profile, eGameSetting_SplitscreenNicknames);
    ActionGameSettings(profile, eGameSetting_Hints);
    ActionGameSettings(profile, eGameSetting_InterfaceOpacity);
    ActionGameSettings(profile, eGameSetting_Tooltips);
    ActionGameSettings(profile, eGameSetting_Clouds);
    ActionGameSettings(profile, eGameSetting_DisplayHUD);
    ActionGameSettings(profile, eGameSetting_DisplayHand);
    ActionGameSettings(profile, eGameSetting_CustomSkinAnim);
    ActionGameSettings(profile, eGameSetting_DeathMessages);
    ActionGameSettings(profile, eGameSetting_HUDSize);
    ActionGameSettings(profile, eGameSetting_HUDSizeSplitscreen);
    ActionGameSettings(profile, eGameSetting_AnimatedCharacter);
    ActionGameSettings(profile, eGameSetting_EULAAccepted);
    ActionGameSettings(profile, eGameSetting_DisplayGameMessages);
    ActionGameSettings(profile, eGameSetting_DisplaySaveIcon);
    ActionGameSettings(profile, eGameSetting_FlyingViewRolling);
    ActionGameSettings(profile, eGameSetting_ShowGlideGhost);
    ActionGameSettings(profile, eGameSetting_ShowGlideGhostPath);
    ActionGameSettings(profile, eGameSetting_ControlMappings);
}

void GameSettings::CheckGameSettingsChanged(bool changed, int profile)
{
    if (profile == 255)
    {
        int i;
        for (i = 0; i < 4; ++i)
        {
            CheckGameSettingsChanged(changed, i);
        }
        return;
    }
    GameSettings* s = settings(profile);
    if (s && s->mChanged)
    {
        ApplyGameSettingsChanged(profile);
    }
}
uint32_t GameSettings::GetInputButton(int profile, bool secondary, EControllerActions action)
{
    const unsigned char mapping = GetActionMapping(profile, action - EControllerActions_Jump);
    return GetInputButton(secondary ? (mapping >> 4) : (mapping & 0x0f));
}

void GameSettings::SetPlayerCape(int profile, unsigned int id, bool changed)
{
    const int resolvedProfile = profile == 255 ? (gProfile != NULL ? gProfile->GetPrimaryPad() : 0) : profile;
    GameSettings* s = settings(resolvedProfile);
    if (s && resolvedProfile >= 0 && resolvedProfile < 4)
    {
        unsigned int old = s->mCapeId;
        s->mCapeId = id;
        if (changed)
        {
            s->mChanged = 1;
        }
        Minecraft* minecraft = Minecraft::GetInstance();
        if (minecraft != NULL && minecraft->mLocalPlayers[resolvedProfile])
        {
            minecraft->mLocalPlayers[resolvedProfile]->setCustomCape(id);
            if (!changed)
                minecraft->mLocalPlayers[resolvedProfile]->setCustomCape(old);
        }
        if (!changed)
        {
            s->mCapeId = old;
        }
    }
}
void GameSettings::SetPlayerCape(int profile, const std::wstring& path)
{
    SetPlayerCape(profile, Player::getCapeIdFromPath(path), true);
}
std::wstring GameSettings::GetPlayerCapeName(int profile)
{
    GameSettings* s = settings(profile);
    return s ? Player::getCapePathFromId(s->mCapeId) : std::wstring();
}
uint32_t GameSettings::IsAllowedSkin(int profile)
{
    GameSettings* settings = ::settings(profile);
    if (static_cast<int32_t>(settings->mSkinId) >= 0)
        return 1;

    wchar_t skinPath[256];
    swprintf(skinPath, 256, L"dlcskin%08d.png", settings->mSkinId & 0x7fffffff);
    DLCPack* pack = g_pApp->mDLCManager.getPackContainingSkin(skinPath);
    if (pack != NULL)
    {
        DLCSkinFile* skinFile = static_cast<DLCSkinFile*>(pack->getSkinFile(skinPath));
        bool smallSkinBlocked = false;
        const bool skinIsFree = skinFile->getParameterAsBool(DLCManager::eDLCParameterType_Free);
        const bool licensed = pack->hasPurchasedPack();
        if (Minecraft::GetInstance()->GetMiniGame()->AllowSmallSkins() &&
            (skinFile->getSkinAdjustments().mGameFlags & 1) != 0)
            smallSkinBlocked = true;
        if ((skinIsFree || licensed) && !smallSkinBlocked)
            return 1;
    }
    return 0;
}
uint32_t GameSettings::GetPlayerCapeId(int profile)
{
    GameSettings* s = settings(profile);
    if (s && IsAllowedSkin(profile))
        return s->mCapeId;
    return 0;
}

void GameSettings::SetPlayerSkin(int profile, unsigned int id, bool changed)
{
    const int resolvedProfile = profile == 255 ? (gProfile != NULL ? gProfile->GetPrimaryPad() : 0) : profile;
    GameSettings* s = settings(resolvedProfile);
    if (s && resolvedProfile >= 0 && resolvedProfile < 4)
    {
        unsigned int old = s->mSkinId;
        s->mSkinId = id;
        if (changed)
        {
            s->mChanged = 1;
        }
        Minecraft* minecraft = Minecraft::GetInstance();
        if (minecraft != NULL && minecraft->mLocalPlayers[resolvedProfile])
        {
            minecraft->mLocalPlayers[resolvedProfile]->setCustomSkin(id);
            if (!changed)
                minecraft->mLocalPlayers[resolvedProfile]->setCustomSkin(old);
        }
        if (!changed)
        {
            s->mSkinId = old;
        }
    }
}
void GameSettings::SetPlayerSkin(int profile, const std::wstring& path)
{
    SetPlayerSkin(profile, CMinecraftApp::getSkinIdFromPath(path), true);
}
std::wstring GameSettings::GetPlayerSkinName(int profile)
{
    GameSettings* s = settings(profile);
    return s ? CMinecraftApp::getSkinPathFromId(s->mSkinId) : std::wstring();
}
uint32_t GameSettings::GetPlayerSkinId(int profile)
{
    GameSettings* s = settings(profile);
    return s && IsAllowedSkin(profile) ? s->mSkinId : 0;
}

void GameSettings::SetPlayerFavoriteSkin(int profile, int index, unsigned int id)
{
    GameSettings* s = settings(profile);
    if (s && index >= 0 && index < 10)
    {
        s->mChanged = 1;
        s->mFavoriteSkins[index] = id;
    }
}

uint32_t GameSettings::GetPlayerFavoriteSkin(int profile, int index)
{
    GameSettings* s = settings(profile);
    return s && index >= 0 && index < 10 ? s->mFavoriteSkins[index] : 0;
}

uint8_t GameSettings::GetPlayerFavoriteSkinsPos(int profile)
{
    GameSettings* s = settings(profile);
    return s ? s->mFavoriteSkinsPos : 0;
}

uint32_t GameSettings::GetPlayerFavoriteSkinsPos(int profile, unsigned int id)
{
    int i;
    for (i = 0; i < 10; ++i)
    {
        if (GetPlayerFavoriteSkin(profile, i) == id)
        {
            return i;
        }
    }
    return 0;
}

void GameSettings::SetPlayerFavoriteSkinsPos(int profile, int value)
{
    GameSettings* s = settings(profile);
    if (s)
    {
        s->mChanged = 1;
        s->mFavoriteSkinsPos = (unsigned char)value;
    }
}

uint32_t GameSettings::GetPlayerFavoriteSkinsCount(int profile)
{
    GameSettings* s = settings(profile);
    if (!s)
    {
        return 0;
    }
    int i;
    for (i = 0; i < 10; ++i)
    {
        if (s->mFavoriteSkins[i] == 0xffffffffu)
        {
            return i;
        }
    }
    return 10;
}

void GameSettings::ValidateFavoriteSkins(int profile)
{
    GameSettings* s = settings(profile);
    if (!s)
    {
        return;
    }
    uint32_t count = GetPlayerFavoriteSkinsCount(profile);
    uint32_t validCount = 0;
    uint32_t i;
    for (i = 0; i < count; ++i)
    {
        uint32_t skinId = GetPlayerFavoriteSkin(profile, (int)i);
        bool isDefaultSkin = false;
        bool hasPurchasedPack = false;
        DLCFile* skinFile = g_pApp->mDLCManager.getSkinFile((int)skinId, isDefaultSkin, hasPurchasedPack);
        if (isDefaultSkin ||
            (skinFile != NULL && (hasPurchasedPack || static_cast<DLCSkinFile*>(skinFile)->isFree())))
        {
            s->mFavoriteSkins[validCount] = skinId;
            ++validCount;
        }
    }
    for (i = validCount; i < 10; ++i)
    {
        s->mFavoriteSkins[i] = 0xffffffffu;
    }
}

void GameSettings::HideMashupPackWorld(int profile, unsigned int id)
{
    GameSettings* s = settings(profile);
    if (s)
    {
        s->mChanged = 1;
        s->mHiddenMashupWorlds &= ~(1u << ((id - 0x400) & 31));
    }
}

void GameSettings::SetBattleMiniGameMapSelection(int profile, unsigned int value)
{
    GameSettings* s = settings(profile);
    if (s)
    {
        s->mChanged = 1;
        s->mBattleMiniGameMap = value;
    }
}
uint32_t GameSettings::GetBattleMiniGameMapSelection(int profile) { GameSettings* s = settings(profile); return s ? s->mBattleMiniGameMap : 0; }
void GameSettings::SetGlideMiniGameMapSelection(int profile, unsigned int value) { GameSettings* s = settings(profile); if (s) { s->mChanged = 1; s->mGlideMiniGameMap = value; } }
uint32_t GameSettings::GetGlideMiniGameMapSelection(int profile) { GameSettings* s = settings(profile); return s ? s->mGlideMiniGameMap : 0; }
uint32_t GameSettings::GetTumbleMiniGameMapSelection()
{
    if (gHalloweenEvent)
        return 2;
    if (gFestiveEvent)
        return 4;
    return gSeasonalEvent ? 8 : 1;
}
void GameSettings::SetMinecraftLanguage(int profile, unsigned char value) { GameSettings* s = settings(profile); if (s) { s->mChanged = 1; s->mMinecraftLanguage = value; } }
void GameSettings::SetMinecraftLocale(int profile, unsigned char value) { GameSettings* s = settings(profile); if (s) { s->mChanged = 1; s->mMinecraftLocale = value; } }
void GameSettings::SetGameSettings(int profile, eCustomGameModeHostOption option, unsigned int value)
{
    GameSettings* s = settings(profile);
    if (s && option == eCustomGameModeHostOption_All)
    {
        s->mChanged = 1;
        s->mGameModeOptions = value;
    }
}
uint32_t GameSettings::GetGameSettings(eGameSetting setting)
{
    return GetGameSettings(gProfile->GetPrimaryPad(), setting);
}
uint32_t GameSettings::GetGameSettingsDebugMask(int, bool) { return 0; }
uint32_t GameSettings::IsDebugSettingEnabled(eDebugSetting, int) { return 0; }
void GameSettings::SetGameSettingsDebugMask(int, unsigned int) { }
uint32_t GameSettings::GetLoggingMask()
{
    int profile = gProfile->GetPrimaryPad();
    return gGameSettings[profile]->mUnknownA0;
}
void GameSettings::SetSpecialTutorialCompletionFlag(int profile, int flag)
{
    GameSettings* s = settings(profile);
    if (s && flag >= 0 && flag < 32)
    {
        s->mSpecialTutorialFlags |= 1u << flag;
    }
}

void GameSettings::SetTutorialCompletionFlag(int profile, int flag)
{
    GameSettings* s = settings(profile);
    if (s && flag >= 0 && flag < 32)
    {
        s->mTutorialFlags |= 1u << (flag ^ 7);
        s->mChanged = 1;
    }
}

bool GameSettings::GetTutorialCompletionFlag(int profile, int flag)
{
    GameSettings* s = settings(profile);
    return s && flag >= 0 && flag < 32 && (s->mTutorialFlags & (1u << (flag ^ 7))) != 0;
}
