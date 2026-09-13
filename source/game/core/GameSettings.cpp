#include "game/core/GameSettings.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/DLCSkinFile.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CStorage.hpp"
extern CStorage* gStorage;

GameSettings* gGameSettings[4] = { NULL, NULL, NULL, NULL };
unsigned char gDefaultActionMappings[4][25];
unsigned char gSecondaryActionMappings[4][25];
bool gTumbleMapFlags[3];
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
    static const unsigned char actionIndices[4] = { 0, 0x14, 1, 0x15 };
    static const unsigned int actionMasks[32] =
    {
        4, 8, 0x200400, 0x100800, 0x9000, 0x6000, 0x800000, 0x400000,
        0x40, 0x80, 0x10, 0x20, 0x20000, 0x10000, 0x80000, 0x40000,
        2, 0x200000, 0x100000, 0x8000, 0x4000, 0x80000, 0x40000, 0x20000,
        0x10000, 0x800000, 0x400000, 0x40, 0x80, 8, 0x10, 1
    };
    static const unsigned char actionMapIndices[32] =
    {
        2, 3, 4, 5, 7, 6, 8, 9, 10, 0xb, 0x12, 0x13, 0xe, 0xf, 0x10, 0x11,
        0x20, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x21, 0x22,
        0x24, 0x23, 0x25, 0x16, 0x26
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
            g_pInput->SetGameJoypadMaps(0, 0x27, 0x100);
            g_pInput->SetGameJoypadMaps(0, 0x29, 0x200);
            g_pInput->SetGameJoypadMaps(0, 0xc, 0x200);
            g_pInput->SetGameJoypadMaps(0, 0xd, 0x100);
        }
        else
        {
            g_pInput->SetGameJoypadMaps(controller, 0x27, 0x800);
            g_pInput->SetGameJoypadMaps(controller, 0x29, 0x400);
            g_pInput->SetGameJoypadMaps(controller, 0xc, 0x200);
            g_pInput->SetGameJoypadMaps(controller, 0xd, 0x100);
        }
        g_pInput->SetGameJoypadMaps(controller, 0x28, 4);
        g_pInput->SetGameJoypadMaps(controller, 0x17, 0x20);
        g_pInput->SetGameJoypadMaps(controller, 0x2a, 0x1000);
        g_pInput->SetGameJoypadMaps(controller, 0x2b, 0x2000);
        g_pInput->SetGameJoypadMaps(controller, 0x2c, 0x400);
        g_pInput->SetGameJoypadMaps(controller, 0x2d, 0x800);
        g_pInput->SetGameJoypadMaps(controller, 0x39, 0x1000);
        g_pInput->SetGameJoypadMaps(controller, 0x3a, 0x2000);
        g_pInput->SetGameJoypadMaps(controller, 0x3b, 0x400);
        g_pInput->SetGameJoypadMaps(controller, 0x3c, 0x800);
    }
}

static GameSettings* settings(int profile)
{
    if (profile == 255) profile = 0;
    if (profile < 0 || profile >= 4) return 0;
    return gGameSettings[profile];
}

uint32_t GameSettings::GetGameSettings(int profile, eGameSetting setting)
{
    if (profile == 255)
        profile = gProfile->GetPrimaryPad();
    GameSettings* s = settings(profile); if (!s) return 0;
    switch ((int)setting) {
    case 0: return s->mYAxis; case 1: return s->mSensitivity; case 2: return s->mVibration;
    case 3: return s->mOptionFlags & 3; case 4: return s->mSwapSticks; case 5: return s->mLanguage;
    case 6: return (s->mOptionFlags >> 2) & 1; case 7: return (s->mOptionFlags >> 4) & 3;
    case 8: return (s->mOptionFlags >> 6) & 1; case 9: return (s->mOptionFlags >> 7) & 1;
    case 10: return (s->mDebugMask >> 23) & 1; case 11: return (s->mOptionFlags >> 8) & 1;
    case 12: return (s->mOptionFlags >> 3) & 1; case 13: return (s->mOptionFlags >> 11) & 15;
    case 14: return (s->mOptionFlags >> 9) & 1; case 15: return (s->mOptionFlags >> 10) & 1;
    case 16: return s->mLocale; case 17: return s->mOptionFlags >> 15;
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
    case 40: return (s->mDebugMask >> 30) & 1; case 41: return 0;
    case 42: return (s->mDebugMask >> 22) & 1; case 43: return s->mDebugMask >> 31;
    case 44: return (s->mDebugMask >> 26) & 1; case 45: return s->mSkinFlags & 15;
    default: return 0;
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

void GameSettings::UpdateActionMapping(int profile, int group, bool secondary)
{
    int i;
    for (i = 0; i < 5; ++i)
    {
        int action = group * 5 + i;
        unsigned char value = secondary ? gSecondaryActionMappings[profile][action] : gDefaultActionMappings[profile][action];
        if (secondary && value == 1)
        {
            value = 2;
        }
        SetActionMapping(profile, action, value);
    }
}

uint32_t GameSettings::GetInputButton(unsigned int value)
{
    static const unsigned int buttons[15] =
    {
        0, 1, 2, 0x100, 0x200, 0x400, 0x800, 0x1000,
        0x2000, 0x4000, 0x8000, 0x10000, 0x20000, 0x40000, 0x80000
    };
    return value < 15 ? buttons[value] : 0;
}
uint32_t GameSettings::GetActionVal(unsigned char value) { return GetInputButton(value & 15) | GetInputButton((value >> 4) & 255); }
void GameSettings::ActionGameSettings(int profile, eGameSetting setting)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    switch (static_cast<int>(setting))
    {
    case 0:
        if (profile == gProfile->GetPrimaryPad())
            minecraft->mOptions->set(Options::Option::getItem(0),
                static_cast<float>(settings(profile)->mYAxis) / 100.0f);
        return;
    case 1:
        if (profile == gProfile->GetPrimaryPad())
            minecraft->mOptions->set(Options::Option::getItem(1),
                static_cast<float>(settings(profile)->mSensitivity) / 100.0f);
        return;
    case 2:
        if (profile == gProfile->GetPrimaryPad())
        {
            const float gamma = static_cast<float>(settings(profile)->mVibration) * 327.68f;
            gRenderer->UpdateGamma(static_cast<unsigned short>(static_cast<int>(gamma) & 0xffff));
        }
        return;
    case 3:
        if (profile == gProfile->GetPrimaryPad())
        {
            minecraft->mOptions->toggle(Options::Option::getItem(9), settings(profile)->mOptionFlags & 3);
            g_pApp->SetGameHostOption(static_cast<eGameHostOption>(0), GetGameSettings(profile, (eGameSetting)3));
        }
        return;
    case 4:
        minecraft->mOptions->set(Options::Option::getItem(3),
            static_cast<float>(settings(profile)->mSwapSticks) / 100.0f);
        return;
    case 9:
        {
            const bool swapAnalogSticks = (settings(profile)->mOptionFlags & 0x80) != 0;
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
    case 0xb:
        if (profile == gProfile->GetPrimaryPad())
            minecraft->updatePlayerViewportAssignments();
        return;
    case 0x10:
        gConsoleUIController.RefreshTooltips(static_cast<unsigned int>(profile));
        return;
    case 0x11:
        {
            GameSettings* current = settings(profile);
            gConsoleUIController.SetEnableTooltips(
                static_cast<unsigned int>(profile), current != NULL && (current->mOptionFlags & 0x8000) != 0);
        }
        return;
    case 0x29:
        {
            const int group = static_cast<int>(GetGameSettings(profile, (eGameSetting)7));
            const bool secondary = GetGameSettings(profile, (eGameSetting)10) != 0;
            if (group > 0 || secondary)
            {
                UpdateActionMapping(profile, group, secondary);
                SetGameSettings(profile, (eGameSetting)7, 0);
                SetGameSettings(profile, (eGameSetting)10, 0);
            }
            for (int action = 0; action < 25; ++action)
            {
                g_pInput->SetGameJoypadMaps(profile, static_cast<unsigned char>(action + 0x20),
                    GetActionVal(GetActionMapping(profile, action)));
            }
        }
        return;
    case 0x0e:
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
void GameSettings::SetGameSettings(int profile, eGameSetting setting, unsigned char value)
{
    GameSettings* s = settings(profile);
    if (!s)
        return;

    unsigned int mask = 0;
    unsigned int shift = 0;
    unsigned char* byteValue = NULL;
    bool optionFlags = false;
    switch (static_cast<int>(setting))
    {
    case 0: byteValue = &s->mYAxis; mask = 0xff; break;
    case 1: byteValue = &s->mSensitivity; mask = 0xff; break;
    case 2: byteValue = &s->mVibration; mask = 0xff; break;
    case 3: optionFlags = true; mask = 3; break;
    case 4: byteValue = &s->mSwapSticks; mask = 0xff; break;
    case 5: byteValue = &s->mLanguage; mask = 0xff; break;
    case 6: optionFlags = true; shift = 2; mask = 4; break;
    case 7: optionFlags = true; shift = 4; mask = 0x30; break;
    case 8: optionFlags = true; shift = 6; mask = 0x40; break;
    case 9: optionFlags = true; shift = 7; mask = 0x80; break;
    case 10: shift = 23; mask = 0x800000; break;
    case 0xb: optionFlags = true; shift = 8; mask = 0x100; break;
    case 0xc: optionFlags = true; shift = 3; mask = 8; break;
    case 0xd: optionFlags = true; shift = 11; mask = 0x7800; break;
    case 0xe: optionFlags = true; shift = 9; mask = 0x200; break;
    case 0xf: optionFlags = true; shift = 10; mask = 0x400; break;
    case 0x10: byteValue = &s->mLocale; mask = 0xff; break;
    case 0x11: optionFlags = true; shift = 15; mask = 0x8000; break;
    case 0x12: shift = 0; mask = 1; break;
    case 0x13: shift = 1; mask = 2; break;
    case 0x14: shift = 2; mask = 4; break;
    case 0x15: shift = 3; mask = 8; break;
    case 0x16: shift = 4; mask = 0x30; break;
    case 0x17: shift = 7; mask = 0x80; break;
    case 0x18: shift = 8; mask = 0x100; break;
    case 0x19: shift = 9; mask = 0x200; break;
    case 0x1a: shift = 10; mask = 0x400; break;
    case 0x1b: shift = 11; mask = 0x1800; break;
    case 0x1c: shift = 13; mask = 0x6000; break;
    case 0x1d: shift = 15; mask = 0x8000; break;
    case 0x1e: shift = 16; mask = 0x10000; break;
    case 0x1f: shift = 17; mask = 0x20000; break;
    case 0x20: shift = 18; mask = 0x40000; break;
    case 0x21: shift = 19; mask = 0x80000; break;
    case 0x22: shift = 20; mask = 0x100000; break;
    case 0x23: shift = 21; mask = 0x200000; break;
    case 0x24: shift = 24; mask = 0x3000000; break;
    case 0x25: shift = 27; mask = 0x8000000; break;
    case 0x26: shift = 28; mask = 0x10000000; break;
    case 0x27: shift = 29; mask = 0x20000000; break;
    case 0x28: shift = 30; mask = 0x40000000; break;
    case 0x29: return;
    case 0x2a: shift = 22; mask = 0x400000; break;
    case 0x2b: shift = 31; mask = 0x80000000; break;
    case 0x2c: shift = 26; mask = 0x4000000; break;
    case 0x2d: byteValue = &s->mSkinFlags; mask = 0xf; break;
    default: return;
    }

    if (byteValue != NULL)
    {
        const unsigned int oldValue = *byteValue;
        if ((mask & (oldValue ^ value)) == 0)
            return;
        *byteValue = static_cast<unsigned char>((oldValue & ~mask) | value);
    }
    else if (optionFlags)
    {
        const unsigned int oldValue = s->mOptionFlags;
        const unsigned int shifted = static_cast<unsigned int>(value) << shift;
        if ((mask & (oldValue ^ (shifted & 0xffff))) == 0)
            return;
        s->mOptionFlags = static_cast<unsigned short>((oldValue & ~mask) | shifted);
    }
    else
    {
        const unsigned int oldValue = s->mDebugMask;
        const unsigned int shifted = static_cast<unsigned int>(value) << shift;
        if ((mask & (oldValue ^ shifted)) == 0)
            return;
        s->mDebugMask = (oldValue & ~mask) | shifted;
    }
    if (static_cast<int>(setting) > 3 || profile == gProfile->GetPrimaryPad())
        ActionGameSettings(profile, setting);
    SetSettingsChanged(profile);
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

void GameSettings::SetDefaultActionMapping(unsigned char controller, int action, unsigned char value)
{
    if (controller < 4 && action >= 0 && action < 25)
    {
        gDefaultActionMappings[controller][action] = value;
    }
}

void GameSettings::SetSecondaryActionMapping(unsigned char controller, int action, unsigned char value)
{
    if (controller < 4 && action >= 0 && action < 25)
    {
        gSecondaryActionMappings[controller][action] = value;
    }
}
void GameSettings::SetCustomisableControllerMapping(int profile, GameSettings* source)
{
    if (!source || profile < 0 || profile >= 4)
    {
        return;
    }
    int i;
    for (i = 0; i < 25; ++i)
    {
        gDefaultActionMappings[profile][i] = source->mActionMappings[i];
    }
}
uint64_t GameSettings::updateToLatestVersionFrom(unsigned short version, int profile)
{
    switch (version)
    {
    case 10: mHostOptionVersion = 0xff; updateToLatestVersionFrom(11, profile); break;
    case 11: mDebugMask &= 0xfffeffffu; updateToLatestVersionFrom(12, profile); break;
    case 12: mMinecraftLanguage = 0; updateToLatestVersionFrom(13, profile); break;
    case 13: case 14: case 15: case 16: mDebugMask |= 0x80000u; updateToLatestVersionFrom(17, profile); break;
    case 17: mDebugMask &= 0xffffffbfu; updateToLatestVersionFrom(18, profile); break;
    case 18: case 19: case 20: mDebugMask |= 0x200000u; updateToLatestVersionFrom(21, profile); break;
    case 21: case 22: mDebugMask |= 0x3000000u; mHostOptionVersion = 0xff; updateToLatestVersionFrom(24, profile); break;
    case 23: mHostOptionVersion = 0xc0; updateToLatestVersionFrom(24, profile); break;
    case 24: mDebugMask |= 0x30000000u; updateToLatestVersionFrom(25, profile); break;
    case 25: updateToLatestVersionFrom(27, profile); break;
    case 26: mDebugMask &= 0xbfffffffU; updateToLatestVersionFrom(27, profile); break;
    case 27: case 28: SetCustomisableControllerMapping(profile, this); updateToLatestVersionFrom(29, profile); break;
    case 29: case 30: mDebugMask |= 0x80400000u; updateToLatestVersionFrom(30, profile); break;
    case 31: mDebugMask &= 0xfbffffffu; mHostOptionVersion = 0xff; updateToLatestVersionFrom(32, profile); break;
    case 32: mLanguage = 9; updateToLatestVersionFrom(33, profile); break;
    case 33: case 34: mDebugMask |= 0x30u; break;
    default: break;
    }
    return 0;
}

uint32_t GameSettings::OldProfileVersionCallback(void*, unsigned char* data, unsigned short version, int profile)
{
    GameSettings* settings = ::settings(profile);
    if (settings != NULL) settings->updateToLatestVersionFrom(version, profile);
    return 0;
}
void GameSettings::SetDefaultControls(C4JStorage::PROFILESETTINGS* values, int profile, bool changed)
{
    SetGameSettings(profile, (eGameSetting)8,
        static_cast<unsigned char>(values->values[0] != 0));
    SetGameSettings(profile, (eGameSetting)9, values->flags);

    const int controller = g_pInput->IsPadDRCController(profile) != 0 ? 1 : 0;
    for (int group = 0; group < 5; ++group)
    {
        const int firstAction = group * 5;
        for (int slot = 0; slot < 5; ++slot)
        {
            const int action = firstAction + slot;
            const unsigned char value = gDefaultActionMappings[controller][action];
            SetActionMapping(profile, action, value);
            g_pInput->SetGameJoypadMaps(
                profile, static_cast<unsigned char>(0x20 + action), GetActionVal(value));
        }
    }
    settings(profile)->mChanged = static_cast<unsigned char>(changed);
}
void GameSettings::SetDefaultGameOptions(C4JStorage::PROFILESETTINGS* values, int profile, bool changed)
{
    SetGameSettings(profile, (eGameSetting)0, 100);
    SetGameSettings(profile, (eGameSetting)1, 100);
    SetGameSettings(profile, (eGameSetting)2, 0x32);

    if (Minecraft::GetInstance()->mLevel == NULL)
    {
        SetGameSettings(profile, (eGameSetting)3, 1);
    }

    SetGameSettings(profile, (eGameSetting)4, 100);
    SetGameSettings(profile, (eGameSetting)6, 1);
    SetGameSettings(profile, (eGameSetting)7, 0);
    SetGameSettings(profile, (eGameSetting)0xb, 0);
    SetGameSettings(profile, (eGameSetting)0xc, 1);
    SetGameSettings(profile, (eGameSetting)5, 100);
    SetGameSettings(profile, (eGameSetting)0xe, 1);
    SetGameSettings(profile, (eGameSetting)0xf, 1);
    SetGameSettings(profile, (eGameSetting)0xd, 2);
    SetGameSettings(profile, (eGameSetting)0x11, 1);
    SetGameSettings(profile, (eGameSetting)0x10, 0x50);
    SetGameSettings(profile, (eGameSetting)0x12, 1);
    SetGameSettings(profile, (eGameSetting)0x13, 1);
    SetGameSettings(profile, (eGameSetting)0x14, 0);
    SetGameSettings(profile, (eGameSetting)0x15, 0);
    SetGameSettings(profile, (eGameSetting)0x17, 1);
    SetGameSettings(profile, (eGameSetting)0x18, 1);
    SetGameSettings(profile, (eGameSetting)0x19, 1);
    SetGameSettings(profile, (eGameSetting)0x1a, 1);
    SetGameSettings(profile, (eGameSetting)0x1b, 1);
    SetGameSettings(profile, (eGameSetting)0x1c, 2);
    SetGameSettings(profile, (eGameSetting)0x1d, 1);

    GameSettings* current = settings(profile);
    for (int i = 0; i < 10; ++i)
    {
        current->mFavoriteSkins[i] = 0xffffffffu;
    }
    current->mFavoriteSkinsPos = 0;
    current->mHiddenMashupWorlds = 0xffffffffu;

    SetGameSettings(profile, (eGameSetting)0x1e, 0);
    if (g_pApp->GetGameStarted() == 0)
    {
        current->mMinecraftLocale = 0;
        current->mMinecraftLanguage = 0;
    }

    MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(1), true);
    const MiniGamePlayerSettings playerSettings = miniGame->GetDefaultPlayerSettings();
    current->mGameModeOptions = playerSettings.Get(eCustomGameModeHostOption_All);

    SetGameSettings(profile, (eGameSetting)0x20, 0);
    SetGameSettings(profile, (eGameSetting)0x21, 1);
    SetGameSettings(profile, (eGameSetting)0x23, 1);
    SetGameSettings(profile, (eGameSetting)0x22, 0);
    current->mBattleMiniGameMap = 0xffffffffu;
    SetGameSettings(profile, (eGameSetting)10, 0);
    SetGameSettings(profile, (eGameSetting)0x24, 3);
    SetGameSettings(profile, (eGameSetting)0x26, 1);
    SetGameSettings(profile, (eGameSetting)0x27, 1);
    SetGameSettings(profile, (eGameSetting)0x28, 0);
    SetGameSettings(profile, (eGameSetting)0x2a, 1);
    SetGameSettings(profile, (eGameSetting)0x2b, 1);
    SetGameSettings(profile, (eGameSetting)0x2c, 0);
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
    ActionGameSettings(profile, (eGameSetting)0);
    ActionGameSettings(profile, (eGameSetting)1);
    ActionGameSettings(profile, (eGameSetting)2);
    ActionGameSettings(profile, (eGameSetting)3);
    ActionGameSettings(profile, (eGameSetting)4);
    ActionGameSettings(profile, (eGameSetting)6);
    ActionGameSettings(profile, (eGameSetting)7);
    ActionGameSettings(profile, (eGameSetting)8);
    ActionGameSettings(profile, (eGameSetting)9);
    ActionGameSettings(profile, (eGameSetting)10);
    ActionGameSettings(profile, (eGameSetting)0xb);
    ActionGameSettings(profile, (eGameSetting)0xc);
    ActionGameSettings(profile, (eGameSetting)5);
    ActionGameSettings(profile, (eGameSetting)0xe);
    ActionGameSettings(profile, (eGameSetting)0xf);
    ActionGameSettings(profile, (eGameSetting)0x10);
    ActionGameSettings(profile, (eGameSetting)0x11);
    ActionGameSettings(profile, (eGameSetting)0x12);
    ActionGameSettings(profile, (eGameSetting)0x17);
    ActionGameSettings(profile, (eGameSetting)0x18);
    ActionGameSettings(profile, (eGameSetting)0x19);
    ActionGameSettings(profile, (eGameSetting)0x1a);
    ActionGameSettings(profile, (eGameSetting)0x1b);
    ActionGameSettings(profile, (eGameSetting)0x1c);
    ActionGameSettings(profile, (eGameSetting)0x1d);
    ActionGameSettings(profile, (eGameSetting)0x1e);
    ActionGameSettings(profile, (eGameSetting)0x26);
    ActionGameSettings(profile, (eGameSetting)0x27);
    ActionGameSettings(profile, (eGameSetting)0x28);
    ActionGameSettings(profile, (eGameSetting)0x2a);
    ActionGameSettings(profile, (eGameSetting)0x2b);
    ActionGameSettings(profile, (eGameSetting)0x29);
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
uint64_t GameSettings::GetInputButton(int profile, bool secondary, EControllerActions action)
{
    return GetInputButton(GetActionMapping(profile, (int)action + (secondary ? 0 : 0x20)));
}

void GameSettings::SetPlayerCape(int profile, unsigned int id, bool changed)
{
    GameSettings* s = settings(profile);
    if (s)
    {
        unsigned int old = s->mCapeId;
        s->mCapeId = id;
        if (changed)
        {
            s->mChanged = 1;
        }
        else
        {
            s->mCapeId = old;
        }
    }
}
void GameSettings::SetPlayerCape(int profile, const std::wstring&) { SetPlayerCape(profile, 0, true); }
std::wstring GameSettings::GetPlayerCapeName(int profile)
{
    return Player::getCapePathFromId(gGameSettings[profile]->mCapeId);
}
uint32_t GameSettings::IsAllowedSkin(int profile)
{
    GameSettings* settings = ::settings(profile);
    if (settings == NULL)
        return 0;

    const int32_t skinId = static_cast<int32_t>(settings->mSkinId);
    if (skinId >= 0)
        return 1;

    if (g_pApp == NULL)
        return 0;
    const std::wstring path = CMinecraftApp::getSkinPathFromId(settings->mSkinId);
    DLCPack* pack = g_pApp->mDLCManager.getPackContainingSkin(path);
    if (pack == NULL)
        return 0;
    DLCSkinFile* skin = dynamic_cast<DLCSkinFile*>(pack->getSkinFile(path));
    if (skin == NULL || (!pack->hasPurchasedPack() && !skin->isFree()))
        return 0;
    Minecraft* minecraft = Minecraft::GetInstance();
    return minecraft == NULL || minecraft->GetMiniGame() == NULL ||
           minecraft->GetMiniGame()->AllowSmallSkins() ||
           skin->getSkinAdjustments()->mXMLVersion != 2;
}
uint32_t GameSettings::GetPlayerCapeId(int profile)
{
    if (IsAllowedSkin(profile))
        return gGameSettings[profile]->mCapeId;
    return 0;
}

void GameSettings::SetPlayerSkin(int profile, unsigned int id, bool changed)
{
    GameSettings* s = settings(profile);
    if (s)
    {
        unsigned int old = s->mSkinId;
        s->mSkinId = id;
        if (changed)
        {
            s->mChanged = 1;
        }
        else
        {
            s->mSkinId = old;
        }
    }
}
void GameSettings::SetPlayerSkin(int profile, const std::wstring&) { SetPlayerSkin(profile, 0, true); }
std::wstring GameSettings::GetPlayerSkinName(int profile)
{
    return CMinecraftApp::getSkinPathFromId(gGameSettings[profile]->mSkinId);
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
    int count = GetPlayerFavoriteSkinsCount(profile);
    int i;
    for (i = count; i < 10; ++i)
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
uint32_t GameSettings::GetTumbleMiniGameMapSelection() { if (gTumbleMapFlags[0]) return 2; if (gTumbleMapFlags[1]) return 4; return gTumbleMapFlags[2] ? 8 : 1; }
void GameSettings::SetMinecraftLanguage(int profile, unsigned char value) { GameSettings* s = settings(profile); if (s) { s->mChanged = 1; s->mMinecraftLanguage = value; } }
void GameSettings::SetMinecraftLocale(int profile, unsigned char value) { GameSettings* s = settings(profile); if (s) { s->mChanged = 1; s->mMinecraftLocale = value; } }
uint64_t GameSettings::SetGameSettings(int profile, eCustomGameModeHostOption option, unsigned int value)
{
    GameSettings* s = settings(profile);
    if (s && option == eCustomGameModeHostOption_All)
    {
        s->mChanged = 1;
        s->mGameModeOptions = value;
    }
    return (uint64_t)(uint32_t)profile << 32;
}
uint64_t GameSettings::GetGameSettings(eGameSetting setting) { return (uint64_t)GetGameSettings(0, setting) << 32; }
uint32_t GameSettings::GetGameSettingsDebugMask(int, bool) { return 0; }
uint32_t GameSettings::IsDebugSettingEnabled(eDebugSetting, int) { return 0; }
void GameSettings::SetGameSettingsDebugMask(int, unsigned int) { }
uint32_t GameSettings::GetLoggingMask()
{
    int profile = g_pApp->GetPrimaryUserIndex();
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
