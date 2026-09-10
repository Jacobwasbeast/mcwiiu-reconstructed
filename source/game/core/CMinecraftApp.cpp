#include "game/core/CMinecraftApp.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CInput.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/L10N.hpp"
#include "game/client/audio/ConsoleSoundEngine.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "game/core/GameType.hpp"
#include "game/util/Random.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <cstdlib>
#include <map>
#include <sstream>

struct _LARGE_INTEGER
{
    long long QuadPart;
};

void QueryPerformanceCounter(_LARGE_INTEGER* value);
void QueryPerformanceFrequency(_LARGE_INTEGER* value);
unsigned int GetTickCount();

static std::map<unsigned int, _SkinAdjustments> g_skinAdjustments;
static boost::unordered_map<PlayerUID, MOJJANG_DATA*> g_mojangData;
unsigned int CMinecraftApp::sPreviousSignInUsersMask = 0;
CConsoleMinecraftApp gApp;
CMinecraftApp* g_pApp = &gApp;
static Random sTipRandom;
static TIPSTRUCT sGeneralTips[] = {
    {0, 0xE4D51ED8}, {0, 0x56DC8E13}, {0, 0x30DA13AA}, {0, 0xA2E182E5}, {0, 0xEEE677B7},
    {0, 0xC8E3FD4E}, {0, 0xDAC3C5F9}, {0, 0xB4C14B90}, {0, 0x19F43D28}, {0, 0x3FF6B791},
    {0, 0x65F931FA}, {0, 0x8BFBAC63}, {0, 0xB1FE26CC}, {0, 0xD800A135}, {0, 0xFE031B9E},
    {0, 0x24059607}, {0, 0xE9E069E0}, {0, 0x0FE2E449}, {0, 0x0A78FA5B}, {0, 0xBE740589},
    {0, 0x98718B20}, {0, 0xA282E3FF}, {0, 0x7C806996}, {0, 0x567DEF2D}, {0, 0x307B74C4},
    {0, 0x3A8CCDA3}, {0, 0x148A533A}, {0, 0xF9FD6E2A}, {0, 0x1FFFE893}, {0, 0xADF87958},
    {0, 0xD3FAF3C1}, {0, 0x920757CE}, {0, 0xB809D237}, {0, 0x6C04DD65}, {0, 0xC9E99AE2},
    {0, 0xEFEC154B}, {0, 0xEA822B5D}, {0, 0xC47FB0F4}, {0, 0x3687202F}, {0, 0x1084A5C6},
    {0, 0x527841B9}, {0, 0x2C75C750}, {0, 0x9E7D368B}, {0, 0x787ABC22}, {0, 0x1A95FEA5},
    {0, 0xF493843C}, {0, 0xDA069F2C}
};
static TIPSTRUCT sAlternateTips[] = {
    {0, 0x39F3BCDA}, {0, 0x13F14271}, {0, 0xEDEEC808}, {0, 0xF80020E7}, {0, 0xABFB2C15},
    {0, 0x85F8B1AC}, {0, 0x2FE263FB}, {0, 0x09DFE992}, {0, 0x22D3FFFA}, {0, 0x48D67A63},
    {0, 0xD6CF0B28}, {0, 0xFCD18591}, {0, 0xBADDE99E}, {0, 0xE0E06407}, {0, 0x6ED8F4CC},
    {0, 0x94DB6F35}, {0, 0xF2C02CB2}, {0, 0x18C2A71B}, {0, 0x520926A9}, {0, 0x2C06AC40},
    {0, 0x9E0E1B7B}, {0, 0x780BA112}, {0, 0xEA13104D}, {0, 0xC41095E4}
};
static TIPSTRUCT sMiniGameTips1[] = {
    {0, 0x6F523BBE}, {0, 0xA85A4173}, {0, 0x8257C70A}, {0, 0xE0684AD1},
    {0, 0x88938BD8}, {0, 0x3440F824}
};
static TIPSTRUCT sMiniGameTips2[] = {
    {0, 0xDBA695E0}, {0, 0x4DAE051B}, {0, 0x27AB8AB2}, {0, 0x99B2F9ED},
    {0, 0x73B07F84}, {0, 0xE5B7EEBF}, {0, 0xBFB57456}, {0, 0x31BCE391}
};
static TIPSTRUCT sMiniGameTipsOther[] = {
    {0, 0x87A99AF1}, {0, 0xADAC155A}, {0, 0xD3AE8FC3}, {0, 0xF9B10A2C}, {0, 0x1FB38495},
    {0, 0x45B5FEFE}, {0, 0x31934D40}, {0, 0x5795C7A9}, {0, 0x7222ACB9}, {0, 0x4C203250},
    {0, 0xBE27A18B}, {0, 0x98252722}, {0, 0x0A2C965D}
};

uint32_t DeviceFullErrorReturn__FPv(void* context)
{
    (void)context;
    uint32_t optionsCallbackStatus = g_pApp->GetOptionsCallbackStatus(0);
    if ((optionsCallbackStatus == 4 || optionsCallbackStatus == 6) &&
        GameSettings::GetGameSettings(0, static_cast<eGameSetting>(0x1e)) != 0)
    {
        gConsoleUIController.NavigateToScene(0, static_cast<EUIScene>(2), NULL,
                                              static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    }
    else
    {
        gConsoleUIController.NavigateToScene(0, static_cast<EUIScene>(0x4d), NULL,
                                              static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    }
    return 0;
}

CMinecraftApp::PngTagData::PngTagData()
    : flags(0), imageType(0), width(0), height(0), dataSize(0)
{
    std::fill(values, values + 13, 0);
}
CMinecraftApp::_TipData::_TipData() : m_index(0), m_count(0), m_tips(0) {}
CMinecraftApp::_TipData::_TipData(TIPSTRUCT* tips, unsigned int count)
    : m_index(0), m_count(count), m_tips(tips)
{
    unsigned int first = tips == sGeneralTips ? 1 : 0;
    for (unsigned int index = first; index < count; ++index)
        tips[index].sortValue = sTipRandom.nextInt();
    if (count > 1)
        std::qsort(tips + 1, count - 1, sizeof(TIPSTRUCT), CMinecraftApp::TipsSortFunction);
}
unsigned int CMinecraftApp::_TipData::getNextTip()
{
    unsigned int tip;
    do
    {
        if (m_index >= m_count)
            m_index = 0;
        tip = m_tips[m_index++].tipId;
    }
    while (CMinecraftApp::tipFiltered(tip));
    return tip;
}
CMinecraftApp::CMinecraftApp()
    : mPrimaryPlayerUID(0), mDebugUIFocussed(false), mGameStarted(false), mAppPaused(false),
      mChangingSessionType(false), mReallyChangingSessionType(false), mGameMode(0), mHostOptions(0),
      mLoadSavesFromFolderEnabled(false), mDisconnectReason(0), mGlobalXuiAction(0),
      mLiveLinkRequired(false), mDebugInputEnabled(false), mTrialTimerStart(0.0f),
      mTrialPausedTime(0.0f), mLastPerformanceCounter(0), mAccumulatedPerformanceCounter(0),
      mAppTime(0.0f), mFrameTime(0.0f), mPerformanceCounterScale(0.0f),
      mDLCInstallProcessCompleted(false), mDLCInstallPending(false), mDLCInstallCount(0),
      mDLCInstallIndex(0), mAutosaveTime(0),
      mNewDLCAvailable(false), mDLCOffersRetrievedValue(0), mExtraGameData(0),
      mCorruptSaveDeleted(false), mResetNether(false), mResetEnd(false),
      mRequiredTexturePackID(0), mFirstMiniGameType(static_cast<EMiniGameId>(0))
{
    InitializeCriticalSection(&mPendingActionsMutex);
    InitializeCriticalSection(&mActionMutex);
    InitializeCriticalSection(&mTerrainFeaturePositionsMutex);
    InitializeCriticalSection(&mMemoryTextureMutex);
    InitializeCriticalSection(&mMemoryTPDMutex);
    InitializeCriticalSection(&mAdditionalModelPartsMutex);
    InitializeCriticalSection(&mAdditionalSkinBoxesMutex);
    InitializeCriticalSection(&mDLCRetrieveMutex);
    InitializeCriticalSection(&mDLCDownloadQueueMutex);
    std::fill(mBanListCheck, mBanListCheck + 4, false);
    std::fill(mOpacityTimer, mOpacityTimer + 4, 0);
    std::fill(mOptionsCallbackStatus, mOptionsCallbackStatus + 4, 0);
    std::fill(mPlayerIds, mPlayerIds + 8, 0);
    std::fill(mPlayerPrivileges, mPlayerPrivileges + 8, 0);
    std::fill(mPlayerGameTypes, mPlayerGameTypes + 8, 0);
    for (int player = 0; player < 4; ++player)
        mCachedSignInUIDs[player] = PlayerUID(0);
}
CMinecraftApp::~CMinecraftApp()
{
    DeleteCriticalSection(&mActionMutex);
    DeleteCriticalSection(&mPendingActionsMutex);
    DeleteCriticalSection(&mTerrainFeaturePositionsMutex);
    DeleteCriticalSection(&mMemoryTextureMutex);
    DeleteCriticalSection(&mMemoryTPDMutex);
    DeleteCriticalSection(&mAdditionalModelPartsMutex);
    DeleteCriticalSection(&mAdditionalSkinBoxesMutex);
}
void CMinecraftApp::SetGameHostOption(unsigned int& options, eGameHostOption option, unsigned int value)
{
    switch (static_cast<unsigned int>(option))
    {
    case 0: options = (options & ~3U) | (value & 3U); break;
    case 3: options = (options & ~4U) | (value & 4U); break;
    case 4: options = (options & ~8U) | (value & 8U); break;
    case 6: options = (options & ~0x30U) | ((value & 3U) << 4); break;
    case 7: options = (options & ~0x40U) | (value & 0x40U); break;
    case 8: options = (options & ~0x80U) | (value & 0x80U); break;
    case 9: options = (options & ~0x100U) | (value & 0x100U); break;
    case 10: options = (options & ~0x200U) | (value & 0x200U); break;
    case 11: options = (options & ~0x400U) | (value & 0x400U); break;
    case 12: options = (options & ~0x800U) | (value & 0x800U); break;
    case 13: options = (options & ~0x1000U) | (value & 0x1000U); break;
    case 14: options = (options & ~0x2000U) | (value & 0x2000U); break;
    case 15: options = (options & ~0x1c000U) | (value & 0x1c000U); break;
    case 16: options = (options & ~0x4000U) | (value & 0x4000U); break;
    case 17: options = (options & ~0x8000U) | (value & 0x8000U); break;
    case 18: options = (options & ~0x10000U) | (value & 0x10000U); break;
    case 20: options = (options & ~0x700000U) | ((value & 7U) << 20); break;
    case 21: options = (options & ~0x80000000U) | ((value ^ 1U) << 31); break;
    case 22: options = (options & ~0x40000U) | (value & 0x40000U); break;
    case 23: options = (options & ~0x80000U) | (value & 0x80000U); break;
    case 24: options = (options & ~0x800000U) | ((value ^ 1U) << 23); break;
    case 25: options = (options & ~0x1000000U) | (value & 0x1000000U); break;
    case 26: options = (options & ~0x2000000U) | ((value ^ 1U) << 25); break;
    case 27: options = (options & ~0x4000000U) | ((value ^ 1U) << 26); break;
    case 28: options = (options & ~0x8000000U) | ((value ^ 1U) << 27); break;
    case 29: options = (options & ~0x10000000U) | ((value ^ 1U) << 28); break;
    case 30: options = (options & ~0x20000000U) | ((value ^ 1U) << 29); break;
    case 31: options = (options & ~0x40000000U) | (value & 0x40000000U); break;
    case 32: options = value; break;
    default: break;
    }
}
void CMinecraftApp::SetGameHostOption(eGameHostOption option, unsigned int value)
{
    SetGameHostOption(mHostOptions, option, value);
}
void CMinecraftApp::SetXuiServerAction(int player, eXuiServerAction action, void* param)
{
    mXuiServerActions[player] = action;
    mXuiServerActionParams[player] = param;
}
uint32_t CMinecraftApp::GetGameHostOption(unsigned int options, eGameHostOption option)
{
    switch (static_cast<unsigned int>(option))
    {
    case 0: return options & 3U; case 3: return options & 4U; case 4: return options & 8U;
    case 5: return (options & 8U) | 0x3c81U |
        ((static_cast<unsigned int>(GameType::SURVIVAL->getId()) & 3U) << 4);
    case 6: return (options >> 4) & 3U; case 7: return options & 0x40U; case 8: return options & 0x80U;
    case 9: return options & 0x100U; case 10: return options & 0x200U; case 11: return options & 0x400U;
    case 12: return options & 0x800U; case 13: return options & 0x1000U; case 14: return options & 0x2000U;
    case 15: return options & 0x1c000U; case 16: return options & 0x4000U; case 17: return options & 0x8000U;
    case 18: return options & 0x10000U; case 20: return (options >> 20) & 7U; case 21: return (options >> 31) ^ 1U;
    case 22: return options & 0x40000U; case 23: return options & 0x80000U; case 24: return ((options >> 23) & 1U) ^ 1U;
    case 25: return options & 0x1000000U; case 26: return ((options >> 25) & 1U) ^ 1U;
    case 27: return ((options >> 26) & 1U) ^ 1U; case 28: return ((options >> 27) & 1U) ^ 1U;
    case 29: return ((options >> 28) & 1U) ^ 1U; case 30: return ((options >> 29) & 1U) ^ 1U;
    case 31: return options & 0x40000000U; case 32: return options; default: return 0;
    }
}
uint32_t CMinecraftApp::GetGameHostOption(eGameHostOption option)
{
    return GetGameHostOption(mHostOptions, option);
}
uint32_t CMinecraftApp::GetOptionsCallbackStatus(int32_t firstArgument)
{
    return mOptionsCallbackStatus[firstArgument];
}
void CMinecraftApp::UpdateTime()
{
    _LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    long long elapsed = counter.QuadPart - mLastPerformanceCounter;
    mAccumulatedPerformanceCounter += elapsed;
    mLastPerformanceCounter = counter.QuadPart;
    mFrameTime = mPerformanceCounterScale * elapsed;
    mAppTime = mPerformanceCounterScale * mAccumulatedPerformanceCounter;
}
uint8_t CMinecraftApp::GetLocalPlayerCount()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft == 0) return 0;
    uint8_t count = 0;
    for (int index = 0; index < 4; ++index)
        if (minecraft->GetPlayerByPlayerIndex(index)) ++count;
    return count;
}
void CMinecraftApp::loadMediaArchive()
{
}
arrayWithLength<unsigned char> CMinecraftApp::getArchiveFile(const std::wstring& firstArgument, bool secondArgument)
{
    return arrayWithLength<unsigned char>();
}
bool CMinecraftApp::hasArchiveFile(const std::wstring& firstArgument, bool secondArgument)
{
    return false;
}
void CMinecraftApp::InitTime()
{
    _LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    mPerformanceCounterScale = 1.0f / frequency.QuadPart;
    _LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    mLastPerformanceCounter = counter.QuadPart;
    mAccumulatedPerformanceCounter = 0;
    mFrameTime = 0.0f;
    mAppTime = 0.0f;
}
uint32_t CMinecraftApp::IsActionPending(int player, eXuiAction action)
{
    if (player < 0 || player >= 4) return 0;
    EnterCriticalSection(&mPendingActionsMutex);
    if (mXuiActions[player].action == static_cast<unsigned int>(action))
    {
        LeaveCriticalSection(&mPendingActionsMutex);
        return 1;
    }
    for (std::vector<XuiActionData>::const_iterator it = mPendingActions[player].begin();
         it != mPendingActions[player].end(); ++it)
        if (it->action == static_cast<unsigned int>(action))
        {
            LeaveCriticalSection(&mPendingActionsMutex);
            return 1;
        }
    LeaveCriticalSection(&mPendingActionsMutex);
    return 0;
}
bool CMinecraftApp::isValidStateTransition(eXuiAction current, eXuiAction next)
{
    const unsigned int from = static_cast<unsigned int>(current);
    const unsigned int to = static_cast<unsigned int>(next);
    if (from < 0x12U)
    {
        if (from == 3U) return to == 0U || to == 4U;
        if (from != 4U)
        {
            if (from == 0x10U)
                return to == 0U || to >= 0x10U && to <= 0x18U;
            if (from != 0x11U) return true;
            return to == 0x12U || to == 0x13U;
        }
    }
    else if (from > 0x13U && from != 0x24U)
    {
        if (from == 0x25U) return to == 0x29U;
        if (from != 0x29U) return true;
        return to == 0U || to == 0x25U;
    }
    return to == 0U;
}
void CMinecraftApp::SetActionInternal(int player, eXuiAction action, void* param)
{
    if (player < 0 || player >= 4) return;
    EnterCriticalSection(&mPendingActionsMutex);
    if (action == eXuiAction_0)
    {
        if (!mPendingActions[player].empty())
        {
            mXuiActions[player] = mPendingActions[player].front();
            mPendingActions[player].erase(mPendingActions[player].begin());
        }
        else
        {
            mXuiActions[player].action = 0;
            mXuiActions[player].param = param;
        }
        LeaveCriticalSection(&mPendingActionsMutex);
        return;
    }
    const unsigned int current = mXuiActions[player].action;
    bool duplicate = current == static_cast<unsigned int>(action);
    for (std::vector<XuiActionData>::const_iterator it = mPendingActions[player].begin();
         !duplicate && it != mPendingActions[player].end(); ++it)
        duplicate = it->action == static_cast<unsigned int>(action);
    if ((duplicate && action != static_cast<eXuiAction>(0x10) &&
         action != static_cast<eXuiAction>(3) && action != static_cast<eXuiAction>(8)) ||
        (current == 0x19U && action == static_cast<eXuiAction>(0x26)) ||
        (current == 0U && action == static_cast<eXuiAction>(7)) ||
        (current == 0x11U && action == static_cast<eXuiAction>(0x10)))
    {
        LeaveCriticalSection(&mPendingActionsMutex);
        return;
    }
    if (current != 0 &&
        !isValidStateTransition(static_cast<eXuiAction>(current), action))
    {
        XuiActionData pending = { static_cast<unsigned int>(action), param };
        mPendingActions[player].push_back(pending);
        LeaveCriticalSection(&mPendingActionsMutex);
        return;
    }
    mXuiActions[player].action = static_cast<unsigned int>(action);
    mXuiActions[player].param = param;
    LeaveCriticalSection(&mPendingActionsMutex);
}
void CMinecraftApp::SetAction(int player, eXuiAction action, void* param)
{
    EnterCriticalSection(&mActionMutex);
    SetActionInternal(player, action, param);
    LeaveCriticalSection(&mActionMutex);
}
void CMinecraftApp::SetBanListCheck(int32_t firstArgument, bool secondArgument)
{
    mBanListCheck[firstArgument] = secondArgument;
}
void CMinecraftApp::InvalidateBannedList(int32_t firstArgument)
{
}
void CMinecraftApp::ClearAndResetDLCDownloadQueue()
{
}
void CMinecraftApp::ClearDLCInstalled()
{
    mDLCInstallProcessCompleted = false;
}
uint8_t CMinecraftApp::GetLiveLinkRequired()
{
    return mLiveLinkRequired;
}
int CMinecraftApp::GetPlayerUIDIdentityField(const PlayerUID& uid)
{
    int value = 0;
    std::memcpy(&value, reinterpret_cast<const unsigned char*>(&uid) + 0x14, sizeof(value));
    return value;
}

void CMinecraftApp::SetPlayerUIDIdentityField(PlayerUID& uid, int value)
{
    std::memcpy(reinterpret_cast<unsigned char*>(&uid) + 0x14, &value, sizeof(value));
}

void CMinecraftApp::SyncCachedPlayerUID(CMinecraftApp* app, int player)
{
    if (gProfile->IsSignedIn(player))
    {
        gProfile->GetXUID(player, &app->mCachedSignInUIDs[player], true);
        return;
    }
    app->mCachedSignInUIDs[player] = PlayerUID(0);
    SetPlayerUIDIdentityField(app->mCachedSignInUIDs[player], 0);
}

bool CMinecraftApp::HasActiveLocalPlayer(int player)
{
    CGameNetworkManager& network = CGameNetworkManager::sInstance;
    if (network.GetLocalPlayerByUserIndex(player) != NULL)
        return true;
    Minecraft* minecraft = Minecraft::GetInstance();
    return minecraft != NULL && minecraft->mLocalPlayers[player].get() != NULL;
}

void CMinecraftApp::HandleSignInPlayerSlot(CMinecraftApp* app, unsigned int player, unsigned int primaryPad,
                                           unsigned int usersMask, unsigned int changedMask, bool identityChanged)
{
    if (player == primaryPad)
        return;

    const unsigned int playerBit = 1u << (player & 0x3f);
    if (identityChanged && GetPlayerUIDIdentityField(app->mCachedSignInUIDs[player]) != 0 &&
        CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(static_cast<int>(player)) != NULL)
    {
        app->SetAction(static_cast<int>(player), static_cast<eXuiAction>(0xb), NULL);
        return;
    }

    if ((changedMask & playerBit) != playerBit)
        return;

    PlayerUID signedInUid(0);
    if ((usersMask & playerBit) != 0)
    {
        gProfile->GetXUID(static_cast<int>(player), &signedInUid, true);
        if (gProfile->AreXUIDSEqual(signedInUid, app->mCachedSignInUIDs[player]))
            return;
    }

    app->InvalidateBannedList(static_cast<int>(player));
    if (HasActiveLocalPlayer(static_cast<int>(player)))
        app->SetAction(static_cast<int>(player), static_cast<eXuiAction>(0xb), NULL);
}

void CMinecraftApp::ClearSignInChangeUsersMask()
{
    sPreviousSignInUsersMask = 0;
}

uint64_t CMinecraftApp::SignInChangeCallback(void* context, bool signedIn, unsigned int userIndex)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    const int primaryPad = gProfile->GetPrimaryPad();
    const unsigned int usersMask = userIndex;

    if (primaryPad != -1)
    {
        if (gProfile->GetLockedProfile() == -1)
        {
            app->InvalidateBannedList(primaryPad);
            if (gStorage != NULL)
                gStorage->CheckForProfileUpdatesRequired();
            app->ClearAndResetDLCDownloadQueue();
            app->ClearDLCInstalled();
        }
        else if (((usersMask & (1u << (primaryPad & 0x3f))) == 0) || signedIn)
        {
            app->SetAction(primaryPad, static_cast<eXuiAction>(8), NULL);
            app->InvalidateBannedList(primaryPad);
            if (gStorage != NULL)
                gStorage->CheckForProfileUpdatesRequired();
            app->ClearAndResetDLCDownloadQueue();
            app->ClearDLCInstalled();
        }
        else
        {
            const unsigned int changedMask = usersMask ^ sPreviousSignInUsersMask;
            CGameNetworkManager& network = CGameNetworkManager::sInstance;

            if (!network.IsInSession())
            {
                if (app->GetLiveLinkRequired() != 0 && !gProfile->AllowedToPlayMultiplayer(primaryPad))
                    app->SetAction(primaryPad, static_cast<eXuiAction>(0x10), NULL);
            }
            else
            {
                bool identityChanged = false;
                for (int player = 0; player < 4; ++player)
                {
                    int currentIdentity = 0;
                    if (gProfile->IsSignedIn(player))
                    {
                        PlayerUID currentUid(0);
                        gProfile->GetXUID(player, &currentUid, true);
                        currentIdentity = GetPlayerUIDIdentityField(currentUid);
                    }
                    const int cachedIdentity = GetPlayerUIDIdentityField(app->mCachedSignInUIDs[player]);
                    if (cachedIdentity != 0 && currentIdentity != 0 && cachedIdentity != currentIdentity)
                        identityChanged = true;
                }

                int playerThreeIdentity = 0;
                if (gProfile->IsSignedIn(3))
                {
                    PlayerUID playerThreeUid(0);
                    gProfile->GetXUID(3, &playerThreeUid, true);
                    playerThreeIdentity = GetPlayerUIDIdentityField(playerThreeUid);
                }

                const int cachedPlayerThreeIdentity = GetPlayerUIDIdentityField(app->mCachedSignInUIDs[3]);
                if ((cachedPlayerThreeIdentity == 0 || playerThreeIdentity == 0 ||
                     cachedPlayerThreeIdentity == playerThreeIdentity))
                {
                    if (identityChanged)
                    {
                        int messageParameter = static_cast<int>(0xb9a1bf4d);
                        gConsoleUIController.RequestErrorMessage(static_cast<int>(0xdad07967),
                                                                 static_cast<int>(0x4eec6392),
                                                                 &messageParameter, 1,
                                                                 static_cast<unsigned int>(primaryPad),
                                                                 NULL, NULL, NULL, true);
                    }
                }
                else
                {
                    identityChanged = true;
                    int messageParameter = static_cast<int>(0xb9a1bf4d);
                    gConsoleUIController.RequestErrorMessage(static_cast<int>(0xdad07967),
                                                             static_cast<int>(0x4eec6392),
                                                             &messageParameter, 1,
                                                             static_cast<unsigned int>(primaryPad),
                                                             NULL, NULL, NULL, true);
                }

                const bool needsLiveLinkAction =
                    !gProfile->AllowedToPlayMultiplayer(primaryPad) && !network.IsLocalGame();

                for (unsigned int player = 0; player < 4; player += 2)
                    HandleSignInPlayerSlot(app, player, static_cast<unsigned int>(primaryPad),
                                           usersMask, changedMask, identityChanged);
                for (unsigned int player = 1; player < 4; player += 2)
                    HandleSignInPlayerSlot(app, player, static_cast<unsigned int>(primaryPad),
                                           usersMask, changedMask, identityChanged);

                if (needsLiveLinkAction)
                    app->SetAction(primaryPad, static_cast<eXuiAction>(0x10), NULL);

                network.HandleSignInChange();
            }

            if (gStorage != NULL)
            {
                for (int player = 0; player < 4; ++player)
                {
                    if (primaryPad == player)
                        continue;
                    const unsigned int playerBit = 1u << (player & 0x3f);
                    if ((changedMask & playerBit) != 0 && (usersMask & playerBit) != 0)
                        gStorage->ReadFromProfile(player, 0);
                }
            }
        }
    }

    sPreviousSignInUsersMask = usersMask;
    for (int player = 0; player < 4; ++player)
        SyncCachedPlayerUID(app, player);

    return 0;
}
void CMinecraftApp::ProfileReadErrorCallback(void* context)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    if (app != NULL)
        app->SetAction(app->GetPrimaryUserIndex(), static_cast<eXuiAction>(0x20), NULL);
}
uint32_t CMinecraftApp::DefaultOptionsCallback(void* context, C4JStorage::PROFILESETTINGS* settings, int profile)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    if (app != NULL)
        app->SetAction(profile, static_cast<eXuiAction>(0x19), settings);
    return 0;
}
uint32_t CMinecraftApp::OptionsDataCallback(void* context, int profile, unsigned short size, C4JStorage::eOptionsCallback status)
{
    return 0;
}
void CMinecraftApp::EnterSaveNotificationSection()
{
}
void CMinecraftApp::LeaveSaveNotificationSection()
{
}
uint8_t CMinecraftApp::GetGameStarted()
{
    return mGameStarted;
}
void CMinecraftApp::SetDefaultHostOptions()
{
    SetGameHostOption(static_cast<eGameHostOption>(0x18), 1);
    SetGameHostOption(static_cast<eGameHostOption>(0x19), 0);
    SetGameHostOption(static_cast<eGameHostOption>(0x1a), 1);
    SetGameHostOption(static_cast<eGameHostOption>(0x1b), 1);
    SetGameHostOption(static_cast<eGameHostOption>(0x1c), 1);
    SetGameHostOption(static_cast<eGameHostOption>(0x1d), 1);
    SetGameHostOption(static_cast<eGameHostOption>(0x1e), 1);
    SetGameHostOption(static_cast<eGameHostOption>(0x15), 1);
}
void CMinecraftApp::InitGameSettings()
{
    GameSettings::InitGameSettings();
    SetDefaultHostOptions();
}
uint64_t CMinecraftApp::UpsellReturnedCallback(void* context, eUpsellType type, eUpsellResponse response, int userIndex)
{
    return 0;
}
int CMinecraftApp::TipsSortFunction(const void* firstTip, const void* secondTip)
{
    const TIPSTRUCT* first = static_cast<const TIPSTRUCT*>(firstTip);
    const TIPSTRUCT* second = static_cast<const TIPSTRUCT*>(secondTip);
    return first->sortValue - second->sortValue;
}
void CMinecraftApp::InitialiseTips()
{
    if (!gProfile->IsFullVersion())
        sGeneralTips[0].tipId = 0xAD79941;
    std::memset(mTipData, 0, sizeof(mTipData));
    mTipData[0] = _TipData(sGeneralTips, sizeof(sGeneralTips) / sizeof(sGeneralTips[0]));
    mTipData[1] = _TipData(sAlternateTips, sizeof(sAlternateTips) / sizeof(sAlternateTips[0]));
    mTipData[2] = _TipData(sMiniGameTips1, sizeof(sMiniGameTips1) / sizeof(sMiniGameTips1[0]));
    mTipData[3] = _TipData(sMiniGameTips2, sizeof(sMiniGameTips2) / sizeof(sMiniGameTips2[0]));
    mTipData[4] = _TipData(sMiniGameTipsOther, sizeof(sMiniGameTipsOther) / sizeof(sMiniGameTipsOther[0]));
    mTipRotationIndex = 0;
}
void CMinecraftApp::SetAppPaused(bool firstArgument)
{
    mAppPaused = firstArgument;
}
uint8_t CMinecraftApp::GetReallyChangingSessionType()
{
    return mReallyChangingSessionType;
}
eXuiAction CMinecraftApp::GetGlobalXuiAction()
{
    return static_cast<eXuiAction>(mGlobalXuiAction);
}
uint32_t CMinecraftApp::GetTMSAction(int32_t firstArgument)
{
    return mTMSActions[firstArgument];
}
uint32_t CMinecraftApp::EthernetDisconnectReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
uint32_t CMinecraftApp::BannedLevelDialogReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    if (result != static_cast<C4JStorage::EMessageResult>(4))
    {
        CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
        if (app != NULL)
        {
            eXuiAction action = static_cast<eXuiAction>(0xb);
            if (button == app->GetPrimaryUserIndex())
                action = static_cast<eXuiAction>(3);
            app->SetAction(button, action, NULL);
        }
    }
    return 0;
}
void CMinecraftApp::SetGlobalXuiAction(eXuiAction action)
{
    mGlobalXuiAction = action;
}
uint8_t CMinecraftApp::GetChangingSessionType()
{
    return mChangingSessionType;
}
uint32_t CMinecraftApp::UnlockFullSaveReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
void CMinecraftApp::SetAutosaveTimerTime()
{
    mAutosaveTime = GetTickCount() +
        GameSettings::GetGameSettings(GetPrimaryUserIndex(), static_cast<eGameSetting>(13)) * 900000;
}
void CMinecraftApp::SetGameStarted(bool firstArgument)
{
    mGameStarted = firstArgument;
    mAppPaused = !firstArgument;
}
uint32_t CMinecraftApp::GetXuiServerAction(int32_t firstArgument)
{
    return mXuiServerActions[firstArgument];
}
uint32_t CMinecraftApp::UnlockFullExitReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
void CMinecraftApp::SetChangingSessionType(bool firstArgument)
{
    mChangingSessionType = firstArgument;
}
uint32_t CMinecraftApp::GetDisconnectReason()
{
    return mDisconnectReason;
}
void CMinecraftApp::SetReallyChangingSessionType(bool firstArgument)
{
    mReallyChangingSessionType = firstArgument;
}
uint32_t CMinecraftApp::SignoutExitWorldThreadProc(void* context)
{
    return 0;
}
uint32_t CMinecraftApp::TrialOverReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
uint32_t CMinecraftApp::UnlockFullInviteReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
uint32_t CMinecraftApp::ExitAndJoinFromInvite(void* context, int button, C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(5))
    {
        CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
        if (app != NULL)
            app->SetAction(button, static_cast<eXuiAction>(0x16), NULL);
    }
    return 0;
}
uint32_t CMinecraftApp::ExitAndJoinFromInviteAndSaveReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
uint32_t CMinecraftApp::ExitAndJoinFromInviteDeclineSaveReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
uint32_t CMinecraftApp::ExitAndJoinFromInviteSaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
void CMinecraftApp::SetExtraGameData(unsigned int& options, eGameExtraData type, unsigned int value) const
{
    switch (static_cast<unsigned int>(type))
    {
    case 0: options = value == 0 ? options & ~1U : options | 1U; break;
    case 1: options = value == 0 ? options & ~2U : options | 2U; break;
    case 2: options = value == 0 ? options & ~4U : options | 4U; break;
    case 3: options = value == 0 ? options & ~8U : options | 8U; break;
    case 4: options = (options & 0xffffU) | ((value << 16) & 0xffff0000U); break;
    case 5: options = (options & 0xfffff80fU) | ((value << 4) & 0x7f0U); break;
    case 6: options = value; break;
    default: break;
    }
}
void CMinecraftApp::SetExtraGameData(eGameExtraData type, unsigned int value)
{
}
void CMinecraftApp::SetTutorialMode(bool tutorial, const boost::shared_ptr<Player>& player)
{
}
uint32_t CMinecraftApp::GetXuiAction(int32_t firstArgument)
{
    return mXuiActions[firstArgument].action;
}
uint32_t CMinecraftApp::RemoteSaveThreadProc(void* context)
{
    return 0;
}
int CMinecraftApp::ExitGameFromRemoteSaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    if (app != NULL && result == static_cast<C4JStorage::EMessageResult>(5))
        app->SetAction(button, static_cast<eXuiAction>(3), NULL);
    else if (result != static_cast<C4JStorage::EMessageResult>(5))
    {
        UIScene* scene = gConsoleUIController.FindScene(static_cast<EUIScene>(4));
        if (scene != NULL)
            static_cast<UIScene_FullscreenProgress*>(scene)->SetWasCancelled(false);
    }
    return 0;
}
uint64_t CMinecraftApp::ExitGameFromRemoteSave(void* context)
{
    int messageParameters[2] = { static_cast<int>(0x4fcbc7e7), static_cast<int>(0xb9a1bf4d) };
    gConsoleUIController.RequestAlertMessage(static_cast<int>(0x2686c8e5),
                                              static_cast<int>(0xc85a78c0),
                                              messageParameters, 2,
                                              static_cast<unsigned int>(gProfile->GetPrimaryPad()),
                                              ExitGameFromRemoteSaveDialogReturned, context,
                                              NULL, true);
    return 0;
}
uint32_t CMinecraftApp::TexturePackDialogReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
void CMinecraftApp::SetupMiniGameMusic()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    unsigned int packId = minecraft->mTexturePackRepository->getSelectedId();
    ClientMasterGameMode* masterGameMode = minecraft->GetClientMasterGameMode();
    const unsigned int themeId = masterGameMode->GetCurrentThemeId();
    const bool waitingForMusicStart = masterGameMode->IsWaitingForMusicStartEvent();
    const MiniGameDef* miniGame = minecraft->GetMiniGame();
    SoundEngine* soundEngine = minecraft->mSoundEngine;

    const int miniGameId = miniGame->GetId();
    unsigned int playMiniGameId = static_cast<unsigned int>(miniGameId);

    if (miniGameId == static_cast<int>(eMiniGameId_Battle))
    {
        if (packId < 0x806U)
        {
            if (packId == 0U)
            {
                if (themeId == 0x11U)
                {
                    soundEngine->SetStreamingSounds(
                        0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20, 0x29,
                        0x2a, 0x2a, 0x3e, 0x44, 0x57);
                }
                else
                {
                    soundEngine->SetStreamingSounds(
                        0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x1c, 0x1f, 0x20, 0x29,
                        0x2a, 0x2a, 0x3e, 0x44, 0x57);
                }
            }
            else if (packId == 0x801U)
            {
                soundEngine->SetStreamingSounds(
                    0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x51, 0x53, 0x20, 0x29, 0x2a,
                    0x2a, 0x3e, 0x44, 0x57);
            }
            else if (packId == 0x803U)
            {
                soundEngine->SetStreamingSounds(
                    0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x45, 0x47, 0x20, 0x29, 0x2a,
                    0x2a, 0x3e, 0x44, 0x57);
            }
            else
            {
                playMiniGameId = static_cast<unsigned int>(miniGame->GetId());
                goto play_streaming;
            }
        }
        else if (packId == 0x806U || packId == 0x1000806U)
        {
            soundEngine->SetStreamingSounds(
                0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x48, 0x4c, 0x20, 0x29, 0x2a,
                0x2a, 0x3e, 0x44, 0x57);
        }
        else if (packId == 0x808U || packId == 0x1000808U)
        {
            soundEngine->SetStreamingSounds(
                0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x4d, 0x50, 0x20, 0x29, 0x2a,
                0x2a, 0x3e, 0x44, 0x57);
        }
        else
        {
            playMiniGameId = static_cast<unsigned int>(miniGame->GetId());
            goto play_streaming;
        }

        playMiniGameId = static_cast<unsigned int>(miniGame->GetId());
    }
    else if (miniGameId == static_cast<int>(eMiniGameId_Glide))
    {
        if (packId == 0U)
        {
            if (themeId < 10U)
            {
                if (1U < themeId)
                {
                    if (3U < themeId)
                    {
                        if (themeId != 4U && themeId != 6U)
                        {
                            playMiniGameId = static_cast<unsigned int>(miniGame->GetId());
                            goto play_streaming;
                        }
                        soundEngine->SetStreamingSounds(
                            0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20,
                            0x29, 0x2d, 0x30, 0x3e, 0x44, 0x57);
                    }
                    else
                    {
                        soundEngine->SetStreamingSounds(
                            0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20,
                            0x29, 0x2a, 0x2c, 0x3e, 0x44, 0x57);
                    }
                }
            }
            else if (themeId < 0xbU)
            {
                soundEngine->SetStreamingSounds(
                    0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20, 0x29,
                    0x2d, 0x30, 0x3e, 0x44, 0x57);
            }
            else if (themeId == 0xbU)
            {
                soundEngine->SetStreamingSounds(
                    0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20, 0x29,
                    0x31, 0x33, 0x3e, 0x44, 0x57);
            }
            else if (themeId == 0xcU)
            {
                soundEngine->SetStreamingSounds(
                    0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20, 0x29,
                    0x34, 0x36, 0x3e, 0x44, 0x57);
            }
            else if (themeId == 0xfU)
            {
                soundEngine->SetStreamingSounds(
                    0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20, 0x29,
                    0x3a, 0x3b, 0x3e, 0x44, 0x57);
            }
            else
            {
                playMiniGameId = static_cast<unsigned int>(miniGame->GetId());
                goto play_streaming;
            }
            playMiniGameId = static_cast<unsigned int>(miniGame->GetId());
        }
        else if (packId == 0x801U)
        {
            soundEngine->SetStreamingSounds(
                0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20, 0x29, 0x37,
                0x39, 0x3e, 0x44, 0x57);
            playMiniGameId = static_cast<unsigned int>(miniGame->GetId());
        }
        else if (packId == 0x803U)
        {
            soundEngine->SetStreamingSounds(
                0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20, 0x29, 0x3c,
                0x3d, 0x3e, 0x44, 0x57);
            playMiniGameId = static_cast<unsigned int>(miniGame->GetId());
        }
        else
        {
            playMiniGameId = static_cast<unsigned int>(miniGame->GetId());
        }
    }

play_streaming:
    if (playMiniGameId != 0U)
    {
        if (playMiniGameId < 4U)
        {
            soundEngine->playStreaming(NULL, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, false, waitingForMusicStart,
                                       ConsoleSoundEngine::eAudioDelayType_None);
            return;
        }
        if (playMiniGameId != static_cast<unsigned int>(eMiniGameId_Build))
        {
            soundEngine->playStreaming(NULL, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, false, false,
                                       ConsoleSoundEngine::eAudioDelayType_Random);
            return;
        }
    }
    soundEngine->playStreaming(NULL, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, false, false,
                               ConsoleSoundEngine::eAudioDelayType_None);
}
uint32_t CMinecraftApp::GameModeTransitionThreadProc(void* context)
{
    return 0;
}
uint64_t CMinecraftApp::GameModeTransitionComplete(void* context)
{
    gConsoleUIController.setCleanupOnReload(false);
    gConsoleUIController.CloseUIScenes(0xff, false);

    const int primaryPad = gProfile->GetPrimaryPad();
    if (g_pApp->GetXuiAction(primaryPad) != 3 &&
        !CGameNetworkManager::sInstance.IsLeavingGame() &&
        context == reinterpret_cast<void*>(1))
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        int playerIndex = 0;
        for (int group = 0; group < 2; ++group)
        {
            for (int slot = 0; slot < 2; ++slot)
            {
                boost::shared_ptr<MultiplayerLocalPlayer>& localPlayer = minecraft->mLocalPlayers[playerIndex];
                if (localPlayer)
                {
                    localPlayer->SetWaitingForScoreboard(true);
                    g_pApp->SetAction(playerIndex, static_cast<eXuiAction>(7), NULL);
                }
                ++playerIndex;
            }
        }
    }
    return 0;
}
void CMinecraftApp::SetTMSAction(int32_t firstArgument, uint32_t secondArgument)
{
    mTMSActions[firstArgument] = secondArgument;
}
uint8_t CMinecraftApp::IsAppPaused()
{
    return mAppPaused;
}
void CMinecraftApp::UpdateTrialPausedTimer()
{
    mTrialPausedTime += mFrameTime;
}
void CMinecraftApp::setLevelGenerationOptions(LevelGenerationOptions* firstArgument)
{
    mGameRuleManager.setLevelGenerationOptions(firstArgument);
}
uint8_t CMinecraftApp::DLCInstallProcessCompleted()
{
    return mDLCInstallProcessCompleted;
}
std::wstring CMinecraftApp::getDLCFolderName(unsigned int dlcId)
{
    switch (dlcId)
    {
    case 4: return L"Tutorial_World";
    case 0x200: return L"Skin_Pack_1";
    case 0x201: return L"Skin_Pack_2";
    case 0x400: return L"Mass_Effect";
    case 0x401: return L"Skyrim";
    case 0x402: return L"Halo";
    case 0x405: return L"Festive";
    case 0x406: return L"LittleBigPlanet";
    case 0x407: return L"Greek_Mythology";
    case 0x408: return L"Norse_Mythology";
    case 0x409: return L"Halloween_2015";
    case 0x40a: return L"Mario";
    case 0x40c: return L"Adventure_Time";
    case 0x40e: return L"Pirates_Of_The_Caribbean";
    case 0x40f: return L"Egyptian_Mythology";
    case 0x4f4: return L"Chinese_Mythology";
    case 0x4f5: return L"Fallout";
    case 0x4f9: return L"The_Nightmare_Before_Christmas";
    case 0x801: case 0x1000801: return L"Plastic";
    case 0x802: return L"Candy";
    case 0x803: return L"Fantasy";
    case 0x804: return L"Halloween_Texture_Pack";
    case 0x805: return L"Natural";
    case 0x806: case 0x1000806: return L"City";
    case 0x807: return L"Cartoon";
    case 0x808: case 0x1000808: return L"Steampunk";
    case 0x809: return L"Pattern";
    case 0x812: return L"Super_Cute_Texture_Pack";
    case 0x1001: return L"Battle_Map_Pack_0";
    case 0x1002: return L"Battle_Map_Pack_1";
    case 0x1003: return L"Battle_Map_Pack_2";
    case 0x1004: return L"Battle_Map_Pack_3";
    case 0x1005: return L"Battle_Map_Pack_4";
    case 0x1007: return L"MG02_Map_Pack_0";
    case 0x1009: return L"MG02_Map_Pack_1";
    case 0x100a: return L"MG04_TestWorldPack";
    case 0x100b: return L"Halloween_Battle_Map";
    case 0x100c: return L"Festive_Battle_Map";
    case 0x100e: return L"Fallout_Battle_Map_Pack";
    case 0x100f: return L"Glide_Map_Pack_0";
    case 0x1011: return L"Glide_Beasts_Track_Pack";
    case 0x1012: return L"Glide_Giants_Track_Pack";
    case 0x1013: return L"Glide_Myths_Track_Pack";
    case 0x1014: return L"MG02_Map_Pack_2";
    case 0x1015: return L"MG02_Map_Pack_3";
    default: return std::wstring();
    }
}
std::wstring CMinecraftApp::getRootPath(unsigned int contentType, bool create, bool mounted, std::wstring suffix)
{
    return std::wstring();
}
std::wstring CMinecraftApp::getFilePath(unsigned int firstArgument, std::wstring secondArgument, bool thirdArgument, std::wstring fourthArgument)
{
    return 0;
}
uint64_t CMinecraftApp::HandleDLC(DLCPack* pack)
{
    return 0;
}
int CMinecraftApp::DLCMountedCallback(void* context, int result, unsigned int contentType, unsigned int index)
{
    return 0;
}
void CMinecraftApp::MountNextDLC(int32_t firstArgument)
{
    if (mDLCInstallIndex < mDLCInstallCount)
    {
        const int result = gStorage->MountInstalledDLC(firstArgument, mDLCInstallIndex,
                                                        DLCMountedCallback, this, NULL);
        if (result == 0x3e5)
            return;
        ++mDLCInstallIndex;
        return;
    }
    mDLCInstallProcessCompleted = true;
    mDLCInstallPending = false;
    gConsoleUIController.HandleDLCMountingComplete();
}
int CMinecraftApp::DLCInstalledCallback(void* context, int result, int index)
{
    if (result != -1)
    {
        g_pApp->mDLCInstallCount = static_cast<unsigned int>(result);
        g_pApp->MountNextDLC(index);
    }
    return 0;
}
uint32_t CMinecraftApp::StartInstallDLCProcess(int32_t firstArgument)
{
    if (DLCInstallProcessCompleted() != 0 || mDLCInstallPending)
        return 0;
    mDLCInstallCount = 0;
    mDLCInstallPending = true;
    mDLCInstallIndex = 0;
    gStorage->GetInstalledDLC(firstArgument, DLCInstalledCallback, this);
    return 1;
}
LevelGenerators* CMinecraftApp::getLevelGenerators()
{
    return mGameRuleManager.getLevelGenerators();
}
void CMinecraftApp::ClearTerrainFeaturePosition()
{
    EnterCriticalSection(&mTerrainFeaturePositionsMutex);
    while (!mTerrainFeaturePositions.empty())
    {
        delete mTerrainFeaturePositions.back();
        mTerrainFeaturePositions.pop_back();
    }
    LeaveCriticalSection(&mTerrainFeaturePositionsMutex);
}
void CMinecraftApp::LocaleAndLanguageInit()
{
    static const struct { int id; const wchar_t* tag; } locales[] = {
        {28, L"zh-CHT"},
        {27, L"cs-CS"},
        {29, L"la-LAS"},
        {0, L"en-EN"},
        {1, L"en-US"},
        {2, L"en-GB"},
        {3, L"en-IE"},
        {4, L"en-AU"},
        {5, L"en-NZ"},
        {6, L"en-CA"},
        {7, L"ja-JP"},
        {8, L"de-DE"},
        {9, L"de-AT"},
        {10, L"fr-FR"},
        {11, L"fr-CA"},
        {12, L"es-ES"},
        {13, L"es-MX"},
        {14, L"it-IT"},
        {15, L"ko-KR"},
        {16, L"pt-PT"},
        {17, L"pt-BR"},
        {18, L"ru-RU"},
        {19, L"nl-NL"},
        {20, L"fi-FI"},
        {21, L"sv-SV"},
        {22, L"da-DA"},
        {23, L"no-NO"},
        {24, L"pl-PL"},
        {25, L"tr-TR"},
        {26, L"el-EL"},
        {30, L"zh-SG"},
        {31, L"zh-CN"},
        {32, L"zh-HK"},
        {33, L"zh-TW"},
        {34, L"nl-BE"},
        {35, L"da-DK"},
        {36, L"fr-BE"},
        {37, L"fr-CH"},
        {38, L"de-CH"},
        {39, L"nb-NO"},
        {40, L"en-GR"},
        {41, L"en-HK"},
        {42, L"en-SA"},
        {43, L"en-HU"},
        {44, L"en-IN"},
        {45, L"en-IL"},
        {46, L"en-SG"},
        {47, L"en-SK"},
        {48, L"en-ZA"},
        {49, L"en-CZ"},
        {50, L"en-AE"},
        {51, L"es-AR"},
        {52, L"es-CL"},
        {53, L"es-CO"},
        {54, L"es-US"},
        {55, L"sv-SE"},
        {56, L"cs-CZ"},
        {57, L"el-GR"},
        {58, L"nn-NO"},
        {59, L"sk-SK"},
        {60, L"zh-HANS"},
        {61, L"zh-HANT"}
    };
    for (const auto& locale : locales)
    {
        mLocaleToString[locale.id] = locale.tag;
        mStringToLocale[locale.tag] = locale.id;
    }
}
void CMinecraftApp::HandleButtonPresses()
{
}
uint64_t CMinecraftApp::LoadInventoryMenu(int player, boost::shared_ptr<LocalPlayer> localPlayer, bool creative)
{
    return 0;
}
uint64_t CMinecraftApp::LoadCreativeMenu(int player, boost::shared_ptr<LocalPlayer> localPlayer, bool creative)
{
    return 0;
}
uint64_t CMinecraftApp::LoadEnchantingMenu(int player, boost::shared_ptr<Inventory> inventory, int x, int y, int z, Level* level, const std::wstring& name)
{
    return 0;
}
uint64_t CMinecraftApp::LoadFurnaceMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container)
{
    return 0;
}
uint64_t CMinecraftApp::LoadBrewingStandMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container)
{
    return 0;
}
uint64_t CMinecraftApp::LoadContainerMenu(int player, boost::shared_ptr<Container> inventory, boost::shared_ptr<Container> container)
{
    return 0;
}
uint64_t CMinecraftApp::LoadTrapMenu(int player, boost::shared_ptr<Container> inventory, boost::shared_ptr<Container> container)
{
    return 0;
}
uint32_t CMinecraftApp::LoadCrafting2x2Menu(int player, boost::shared_ptr<LocalPlayer> localPlayer)
{
    return 0;
}
uint64_t CMinecraftApp::LoadCrafting3x3Menu(int player, boost::shared_ptr<LocalPlayer> localPlayer, int x, int y, int z)
{
    return 0;
}
uint64_t CMinecraftApp::LoadFireworksMenu(int player, boost::shared_ptr<LocalPlayer> localPlayer, int x, int y, int z)
{
    return 0;
}
uint32_t CMinecraftApp::LoadSignEntryMenu(int player, boost::shared_ptr<SignBlockEntity> sign)
{
    return 0;
}
uint64_t CMinecraftApp::LoadRepairingMenu(int player, boost::shared_ptr<Inventory> inventory, Level* level, int x, int y, int z)
{
    return 0;
}
uint64_t CMinecraftApp::LoadTradingMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Merchant> merchant, Level* level, const std::wstring& name)
{
    return 0;
}
uint32_t CMinecraftApp::LoadShulkerBoxMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container)
{
    return 0;
}
uint64_t CMinecraftApp::LoadHopperMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container)
{
    return 0;
}
uint32_t CMinecraftApp::LoadHopperMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<MinecartHopper> hopper)
{
    return 0;
}
uint64_t CMinecraftApp::LoadHorseMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container, boost::shared_ptr<AbstractHorse> horse)
{
    return 0;
}
uint64_t CMinecraftApp::LoadBeaconMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container)
{
    return 0;
}
uint32_t CMinecraftApp::LoadWritingBookMenu(int player, not_null_ptr<ItemInstance> item, bool signing)
{
    return 0;
}
bool CMinecraftApp::IsConsoleChatCensored()
{
    return GameSettings::GetGameSettings(GetPrimaryUserIndex(), static_cast<eGameSetting>(0x22)) == 0;
}
uint32_t CMinecraftApp::IsUUIDLocal(const std::wstring& uuid)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        const boost::shared_ptr<MultiplayerLocalPlayer>& localPlayer =
            minecraft->mLocalPlayers[playerIndex];
        if (localPlayer && localPlayer->getStringUUID() == uuid)
            return 1;
    }
    return 0;
}
uint8_t CMinecraftApp::GetTutorialMode(PlayerUID uid)
{
    return 0;
}
uint8_t CMinecraftApp::GetTutorialMode(const boost::shared_ptr<Player>& player)
{
    return 0;
}
void CMinecraftApp::SetGameMode(eGameMode mode)
{
    mGameMode = mode;
}
void CMinecraftApp::StartReloadingTexturePack()
{
    const int user = GetPrimaryUserIndex();
    eXuiAction action = static_cast<eXuiAction>(0x24);
    if (GetXuiAction(user) == 0x29)
        action = static_cast<eXuiAction>(0x25);
    SetAction(user, action, NULL);
}
void CMinecraftApp::StaticCtor()
{
}
void* CMinecraftApp::GetXuiServerActionParam(int player)
{
    return mXuiServerActionParams[player];
}
void CMinecraftApp::SetDisconnectReason(DisconnectPacket::eDisconnectReason reason)
{
    mDisconnectReason = reason;
}
uint8_t CMinecraftApp::GetLoadSavesFromFolderEnabled()
{
    return mLoadSavesFromFolderEnabled;
}
uint32_t CMinecraftApp::GetOpacityTimer(int32_t firstArgument)
{
    return mOpacityTimer[firstArgument];
}
void CMinecraftApp::SetOpacityTimer(int32_t firstArgument)
{
    mOpacityTimer[firstArgument] = 120;
}
void CMinecraftApp::TickOpacityTimer(int32_t firstArgument)
{
    if (mOpacityTimer[firstArgument] != 0)
        --mOpacityTimer[firstArgument];
}
uint32_t CMinecraftApp::IsLocalMultiplayerAvailable()
{
    return 0;
}
uint64_t CMinecraftApp::NotificationsCallback(void* context, unsigned int notification, unsigned int value)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    NOTIFICATION* entry = new NOTIFICATION();
    entry->type = notification;
    entry->value = value;

    if (notification == 1)
    {
        if (app->GetGameStarted() != 0 && CGameNetworkManager::sInstance.IsInSession())
        {
            Minecraft* minecraft = Minecraft::GetInstance();
            for (int player = 0; player < 4; ++player)
            {
                if (!gProfile->IsSignedIn(player) && minecraft->mLocalPlayers[player] != NULL &&
                    !gConsoleUIController.IsPauseMenuDisplayed(player) &&
                    !gConsoleUIController.IsSceneInStack(player, static_cast<EUIScene>(0x30)))
                {
                    gConsoleUIController.CloseUIScenes(player, false);
                    gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(5), NULL,
                                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
                    if (player == 3)
                    {
                        app->mNotifications.push_back(entry);
                        return 0;
                    }
                }
            }
        }
    }
    else if (notification != 2)
    {
        app->mNotifications.push_back(entry);
        return 0;
    }
    else
    {
        app->ClearDLCInstalled();
        gConsoleUIController.HandleDLCInstalled(gStorage->GetMountedDLCIndex());
    }

    app->mNotifications.push_back(entry);
    return 0;
}
void CMinecraftApp::SetLiveLinkRequired(bool firstArgument)
{
    mLiveLinkRequired = firstArgument;
}
std::wstring CMinecraftApp::GetDebugRootPath()
{
    return L"Special";
}
uint32_t CMinecraftApp::DebugSettingsOn()
{
    return 0;
}
uint32_t CMinecraftApp::DebugArtToolsOn()
{
    return 0;
}
uint32_t CMinecraftApp::DebugDevToolsOn()
{
    return 0;
}
uint8_t CMinecraftApp::DebugUIFocussed()
{
    return mDebugUIFocussed;
}
void CMinecraftApp::SetDebugUIFocussed(bool focussed)
{
    mDebugUIFocussed = focussed;
}
uint32_t CMinecraftApp::DebugInputCallback(void* context)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    app->mDebugInputEnabled = !app->mDebugInputEnabled;
    return 0;
}
void CMinecraftApp::SetDebugSequence(const char* sequence)
{
}
uint8_t CMinecraftApp::DLCInstallPending()
{
    return mDLCInstallPending;
}
void CMinecraftApp::SetNewDLCAvailable(bool firstArgument)
{
    mNewDLCAvailable = firstArgument;
}
uint64_t CMinecraftApp::UpsellDLC(uint32_t firstArgument, uint32_t secondArgument, uint32_t thirdArgument, uint32_t fourthArgument)
{
    return 0;
}
void CMinecraftApp::StoreLaunchData()
{
}
void CMinecraftApp::ExitGame()
{
}
bool CMinecraftApp::isXuidNotch(PlayerUID uid)
{
    return false;
}
bool CMinecraftApp::isXuidDeadmau5(PlayerUID uid)
{
    return 0;
}
void CMinecraftApp::AddMemoryTextureFile(const std::wstring& path, unsigned char* data, unsigned int size)
{
}
void CMinecraftApp::GetMemFileDetails(const std::wstring& firstArgument, unsigned char** secondArgument, unsigned int* thirdArgument)
{
}
bool CMinecraftApp::IsFileInMemoryTextures(const std::wstring& path)
{
    EnterCriticalSection(&mMemoryTextureMutex);
    const bool found = mMemoryTextureFiles.find(path) != mMemoryTextureFiles.end();
    LeaveCriticalSection(&mMemoryTextureMutex);
    return found;
}
void CMinecraftApp::GetTPD(int type, unsigned char** data, unsigned int* size)
{
}
bool CMinecraftApp::DefaultCapeExists()
{
    return false;
}
uint64_t CMinecraftApp::RemoveLevelFromBannedLevelList(int player, PlayerUID uid, char* levelId)
{
    return 0;
}
void CMinecraftApp::SetUniqueMapName(char* name)
{
}
uint64_t CMinecraftApp::getArchiveFileSize(const std::wstring& path)
{
    return 0;
}
void CMinecraftApp::SetTrialTimerStart()
{
    mTrialTimerStart = mAppTime;
    mTrialPausedTime = 0.0f;
}
double CMinecraftApp::getTrialTimer()
{
    return (mAppTime - mTrialTimerStart) - mTrialPausedTime;
}
uint32_t CMinecraftApp::WarningTrialTexturePackReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
uint32_t CMinecraftApp::tipFiltered(int32_t firstArgument)
{
    return 0;
}
uint32_t CMinecraftApp::GetNextTip(bool firstArgument, EMiniGameId secondArgument)
{
    return 0;
}
unsigned int CMinecraftApp::GetHTMLColour(eMinecraftColour colour)
{
    return Minecraft::GetInstance()->getColourTable()->getColour(colour);
}
unsigned int CMinecraftApp::GetHTMLColor(eMinecraftColour colour)
{
    return GetHTMLColour(colour);
}
std::wstring CMinecraftApp::GetSingleActionReplacement(int32_t firstArgument, uint32_t secondArgument)
{
    static const struct ActionIcon { uint32_t mask; const wchar_t* name; } icons[] = {
        {0x00000001, L"ActionBottom"}, {0x00000002, L"ActionRight"},
        {0x00000004, L"ActionLeft"}, {0x00000008, L"ActionTop"},
        {0x0030c000, L"LeftStick"}, {0x000f0000, L"RightStick"},
        {0x00001000, L"DPadLeft"}, {0x00002000, L"DPadRight"},
        {0x00000400, L"DPadUp"}, {0x00000800, L"DPadDown"},
        {0x00800000, L"TriggerL"}, {0x00400000, L"TriggerR"},
        {0x00000040, L"ShoulderR"}, {0x00000080, L"ShoulderL"},
        {0x00000020, L"SystemSelect"}, {0x00000010, L"SystemStart"},
        {0x00000100, L"RightStickButton"}, {0x00000200, L"LeftStickButton"}
    };
    const wchar_t* action = 0;
    for (unsigned int i = 0; i < sizeof(icons) / sizeof(icons[0]); ++i) {
        if ((secondArgument & icons[i].mask) != 0) {
            action = icons[i].name;
            break;
        }
    }
    if (action == 0)
        return std::wstring(L" ") + L10N::GetString(0x211CD1AD);

    const wchar_t* controller = L"_CTRL_1";
    if (firstArgument != 4 && g_pInput != 0 &&
        !g_pInput->IsPadDRCController(firstArgument) &&
        g_pInput->IsPadClassicController(firstArgument))
        controller = L"_CTRL_2";
    const wchar_t* size = GetLocalPlayerCount() > 1 ? L"_Small" : L"";
    wchar_t image[128];
    const int imageSize = GetLocalPlayerCount() > 1 ? 20 : 30;
    swprintf(image, sizeof(image) / sizeof(image[0]),
             L"<img src=\"%ls%ls%ls\" align=\"middle\" height=\"%d\" width=\"%d\"/>",
             action, controller, size, imageSize, imageSize);
    return std::wstring(image);
}
std::wstring CMinecraftApp::GetActionReplacement(int32_t firstArgument, uint8_t secondArgument)
{
    return 0;
}
std::wstring CMinecraftApp::GetVKReplacement(int32_t firstArgument, uint32_t secondArgument, bool thirdArgument)
{
    return 0;
}
std::wstring CMinecraftApp::GetIconReplacement(uint32_t firstArgument)
{
    return 0;
}
std::wstring CMinecraftApp::FormatHTMLString(int player, const std::wstring& value, int flags)
{
    return value;
}
double CMinecraftApp::getAppTime()
{
    return mAppTime;
}
void CMinecraftApp::AddTerrainFeaturePosition(_eTerrainFeatureType type, int x, int z)
{
    EnterCriticalSection(&mTerrainFeaturePositionsMutex);
    for (std::vector<FEATURE_DATA*>::const_iterator it = mTerrainFeaturePositions.begin();
         it != mTerrainFeaturePositions.end(); ++it)
    {
        if ((*it)->type == static_cast<int>(type) && (*it)->x == x && (*it)->z == z)
        {
            LeaveCriticalSection(&mTerrainFeaturePositionsMutex);
            return;
        }
    }
    FEATURE_DATA* data = new FEATURE_DATA;
    data->type = static_cast<int>(type);
    data->x = x;
    data->z = z;
    mTerrainFeaturePositions.push_back(data);
    LeaveCriticalSection(&mTerrainFeaturePositionsMutex);
}
uint32_t CMinecraftApp::GetTerrainFeaturePosition(_eTerrainFeatureType type, int* x, int* z)
{
    EnterCriticalSection(&mTerrainFeaturePositionsMutex);
    for (std::vector<FEATURE_DATA*>::const_iterator it = mTerrainFeaturePositions.begin();
         it != mTerrainFeaturePositions.end(); ++it)
    {
        if ((*it)->type == static_cast<int>(type))
        {
            *x = (*it)->x;
            *z = (*it)->z;
            LeaveCriticalSection(&mTerrainFeaturePositionsMutex);
            return 1;
        }
    }
    LeaveCriticalSection(&mTerrainFeaturePositionsMutex);
    return 0;
}
MOJJANG_DATA* CMinecraftApp::GetMojangDataForXuid(PlayerUID uid)
{
    return g_mojangData[uid];
}
uint8_t CMinecraftApp::GetBanListCheck(int32_t firstArgument)
{
    return mBanListCheck[firstArgument];
}
bool CMinecraftApp::AutosaveDue()
{
    return mAutosaveTime < GetTickCount();
}
uint32_t CMinecraftApp::SecondsToAutosave()
{
    return (mAutosaveTime - GetTickCount()) / 1000;
}
uint32_t CMinecraftApp::GetExtraGameData(unsigned int& options, eGameExtraData type) const
{
    switch (static_cast<unsigned int>(type))
    {
    case 0: return options & 1U;
    case 1: return (options >> 1) & 1U;
    case 2: return (options >> 2) & 1U;
    case 3: return (options >> 3) & 1U;
    case 4: return (options >> 16) & 0xffffU;
    case 5: return (options & 0x7f0U) >> 4;
    case 6: return options;
    default: return 0;
    }
}
uint32_t CMinecraftApp::GetExtraGameData(eGameExtraData type)
{
    return GetExtraGameData(mExtraGameData, type);
}
void CMinecraftApp::SetResetNether(bool firstArgument)
{
    mResetNether = firstArgument;
}
uint8_t CMinecraftApp::GetResetNether()
{
    return mResetNether;
}
void CMinecraftApp::SetResetEnd(bool firstArgument)
{
    mResetEnd = firstArgument;
}
uint8_t CMinecraftApp::GetResetEnd()
{
    return mResetEnd;
}
uint32_t CMinecraftApp::CanRecordStatsAndAchievements(Stat* stat, arrayWithLength<unsigned char> players)
{
    return 0;
}
void CMinecraftApp::SetBaseSaveName(const std::wstring& name)
{
}
void CMinecraftApp::SetCurrentImageTextData(const PngTagData* data)
{
    if (data != 0)
        mCurrentImageTextData = *data;
}
void CMinecraftApp::CurrentImageTextData(PngTagData* data)
{
    *data = mCurrentImageTextData;
    data->flags |= 0x20000000;
    data->values[5] = GetGameHostOption(static_cast<eGameHostOption>(32));
    data->flags |= 0x08000000;
    data->values[7] = GetExtraGameData(static_cast<eGameExtraData>(6));
    data->flags |= 0x10000000;
    data->values[6] = Minecraft::GetInstance()->getCurrentTexturePackId();
}
void CMinecraftApp::ParseImageTextData(const unsigned char* image, unsigned int size, PngTagData* data)
{
}
void CMinecraftApp::GetImageTextData(const unsigned char* image, unsigned int size, PngTagData* data)
{
    static const unsigned char signature[8] =
        { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
    if (size <= sizeof(signature) || std::memcmp(image, signature, sizeof(signature)) != 0)
        return;
    unsigned int offset = sizeof(signature);
    while (offset < size)
    {
        unsigned int chunkSize;
        unsigned int chunkType;
        std::memcpy(&chunkSize, image + offset, sizeof(chunkSize));
        std::memcpy(&chunkType, image + offset + 4, sizeof(chunkType));
        if (chunkType == 0x74455874U)
            ParseImageTextData(image + offset + 8, chunkSize, data);
        offset += chunkSize + 12;
    }
}
unsigned int CMinecraftApp::CreateImageTextData(unsigned char* image, const PngTagData* data)
{
    char* output = static_cast<char*>(static_cast<void*>(image));
    unsigned int length = 0;

    if ((data->flags & 0x80000000) != 0)
    {
        std::strcpy(output + length, "4J_SEED");
        length += 8;
        const long long seed = static_cast<long long>(
            (static_cast<unsigned long long>(data->width) << 32) | data->height);
        std::sprintf(output + length, "%lld", seed);
        length += static_cast<unsigned int>(std::strlen(output + length)) + 1;
    }

    std::strcpy(output + length, "4J_HOSTOPTIONS");
    length += 15;
    std::sprintf(output + length, "%x", data->values[5]);
    length += static_cast<unsigned int>(std::strlen(output + length)) + 1;

    std::strcpy(output + length, "4J_TEXTUREPACK");
    length += 15;
    std::sprintf(output + length, "%x", data->values[6]);
    length += static_cast<unsigned int>(std::strlen(output + length)) + 1;

    std::strcpy(output + length, "4J_EXTRADATA");
    length += 13;
    std::sprintf(output + length, "%x", data->values[7]);
    length += static_cast<unsigned int>(std::strlen(output + length)) + 1;

    if ((data->flags & 0x04000000) != 0)
    {
        std::strcpy(output + length, "4J_EXPLOREDCHUNKS");
        length += 18;
        std::sprintf(output + length, "%08x%08x%08x%08x%08x",
                     data->values[8], data->values[9], data->values[10],
                     data->values[11], data->values[12]);
        length += static_cast<unsigned int>(std::strlen(output + length)) + 1;
    }

    if ((data->flags & 0x40000000) != 0)
    {
        std::strcpy(output + length, "4J_BASESAVENAME");
        length += 16;
        const char* baseSaveName = static_cast<const char*>(static_cast<const void*>(data->values));
        std::strcpy(output + length, baseSaveName);
        length += static_cast<unsigned int>(std::strlen(output + length)) + 1;
    }

    if ((data->flags & 0x02000000) != 0)
    {
        std::strcpy(output + length, "4J_#LOADS");
        length += 10;
        std::sprintf(output + length, "%x", data->dataSize);
        length += static_cast<unsigned int>(std::strlen(output + length)) + 1;
    }

    return length - 1;
}
void CMinecraftApp::processSchematics(LevelChunk* firstArgument)
{
    mGameRuleManager.processSchematics(firstArgument);
}
void CMinecraftApp::processSchematicsLighting(LevelChunk* firstArgument)
{
    mGameRuleManager.processSchematicsLighting(firstArgument);
}
void CMinecraftApp::loadDefaultGameRules()
{
    mGameRuleManager.loadDefaultGameRules();
}
LevelRules* CMinecraftApp::getLevelRules()
{
    return mGameRuleManager.mLevelRules;
}
LevelRuleset* CMinecraftApp::getGameRuleDefinitions()
{
    return mGameRuleManager.getGameRuleDefinitions();
}
LevelGenerationOptions* CMinecraftApp::getLevelGenerationOptions()
{
    return mGameRuleManager.getLevelGenerationOptions();
}
const wchar_t* CMinecraftApp::GetGameRulesString(const std::wstring& firstArgument)
{
    return mGameRuleManager.GetGameRulesString(firstArgument);
}
bool CMinecraftApp::ShouldCollideWith(Block* firstArgument)
{
    LevelGenerationOptions* options = getLevelGenerationOptions();
    if (options == NULL)
        return true;
    return options->getRequiredGameRules()->ShouldCollideWith(firstArgument);
}
void CMinecraftApp::UpdatePlayerInfo(uint8_t firstArgument, int16_t secondArgument, uint32_t thirdArgument, uint8_t fourthArgument)
{
    for (unsigned int i = 0; i < 8; ++i)
    {
        if (mPlayerIds[i] == firstArgument)
        {
            mPlayerIds[i] = 0;
            mPlayerPrivileges[i] = 0;
            mPlayerGameTypes[i] = 0;
        }
    }
    if (secondArgument >= 0 && secondArgument < 8)
    {
        const unsigned int slot = static_cast<unsigned int>(secondArgument);
        mPlayerIds[slot] = firstArgument;
        mPlayerPrivileges[slot] = thirdArgument;
        mPlayerGameTypes[slot] = fourthArgument;
    }
}
uint32_t CMinecraftApp::GetPlayerColour(uint8_t firstArgument)
{
    for (unsigned int i = 0; i < 8; ++i)
        if (mPlayerIds[i] == firstArgument) return i;
    return 0xffffffffU;
}
uint32_t CMinecraftApp::GetPlayerPrivileges(uint8_t firstArgument)
{
    for (unsigned int i = 0; i < 8; ++i)
        if (mPlayerIds[i] == firstArgument) return mPlayerPrivileges[i];
    return 0;
}
uint8_t CMinecraftApp::GetPlayerGameType(uint8_t firstArgument)
{
    for (unsigned int i = 0; i < 8; ++i)
        if (mPlayerIds[i] == firstArgument) return mPlayerGameTypes[i];
    return 0;
}
std::wstring CMinecraftApp::getEntityName(uint64_t firstArgument, int32_t secondArgument)
{
    return 0;
}
uint32_t CMinecraftApp::DLCOffersReturned(uint32_t firstArgument, uint32_t secondArgument, uint32_t thirdArgument, int32_t fourthArgument)
{
    return 0;
}
uint32_t CMinecraftApp::RetrieveNextDLCContent()
{
    if (gProfile == NULL)
        return 1;

    const int primaryPad = gProfile->GetPrimaryPad();
    if (primaryPad == -1 || gProfile->GetLiveConnectionStatus() == 0)
        return 1;

    EnterCriticalSection(&mDLCRetrieveMutex);

    for (std::vector<_DLCRequest*>::iterator it = mDLCDownloadQueue.begin();
         it != mDLCDownloadQueue.end(); ++it)
    {
        if ((*it)->mStatus == 1)
        {
            LeaveCriticalSection(&mDLCRetrieveMutex);
            return 1;
        }
    }

    std::vector<_DLCRequest*>::iterator it = mDLCDownloadQueue.begin();
    while (it != mDLCDownloadQueue.end() && (*it)->mStatus != 0)
        ++it;

    if (it == mDLCDownloadQueue.end())
    {
        LeaveCriticalSection(&mDLCRetrieveMutex);
        return 0;
    }

    _DLCRequest* request = *it;
    if (gStorage == NULL)
    {
        LeaveCriticalSection(&mDLCRetrieveMutex);
        return 1;
    }

    const int result = gStorage->GetDLCOffers(
        primaryPad,
        reinterpret_cast<CStorage::DLCStatusCallback>(DLCOffersReturned),
        this,
        request->mContentId);

    request->mStatus = (result == 5) ? 1U : 2U;
    LeaveCriticalSection(&mDLCRetrieveMutex);
    return 1;
}
void CMinecraftApp::SetCorruptSaveDeleted(bool firstArgument)
{
    mCorruptSaveDeleted = firstArgument;
}
uint8_t CMinecraftApp::GetCorruptSaveDeleted()
{
    return mCorruptSaveDeleted;
}
void CMinecraftApp::SetAdditionalSkinBoxes(unsigned int skinId, SKIN_BOX* boxes, unsigned int count, _SkinAdjustments adjustments)
{
}
std::vector<ModelPart*>* CMinecraftApp::SetAdditionalSkinBoxes(unsigned int skinId, std::vector<SKIN_BOX*>* boxes, _SkinAdjustments adjustments)
{
    return 0;
}
std::vector<ModelPart*>* CMinecraftApp::GetAdditionalModelParts(unsigned int firstArgument)
{
    EnterCriticalSection(&mAdditionalModelPartsMutex);
    boost::unordered_map<unsigned int, std::vector<ModelPart*>*>::iterator it =
        mAdditionalModelParts.find(firstArgument);
    std::vector<ModelPart*>* result = it == mAdditionalModelParts.end() ? NULL : it->second;
    LeaveCriticalSection(&mAdditionalModelPartsMutex);
    return result;
}
std::vector<SKIN_BOX*>* CMinecraftApp::GetAdditionalSkinBoxes(unsigned int skinId)
{
    EnterCriticalSection(&mAdditionalSkinBoxesMutex);
    boost::unordered_map<unsigned int, std::vector<SKIN_BOX*>*>::iterator it =
        mAdditionalSkinBoxes.find(skinId);
    std::vector<SKIN_BOX*>* result = it == mAdditionalSkinBoxes.end() ? NULL : it->second;
    LeaveCriticalSection(&mAdditionalSkinBoxesMutex);
    return result;
}
unsigned int CMinecraftApp::getSkinIdFromPath(const std::wstring& path)
{
    if (path.length() < 14)
        return 0;

    const bool dlcSkin = path.compare(0, 3, L"dlc") == 0;
    std::wstring idText = path.substr(7);
    const std::wstring::size_type extension = idText.find(L'.');
    idText = idText.substr(0, extension);

    unsigned int id = 0;
    std::wstringstream stream(idText);
    if (dlcSkin)
        stream >> std::dec >> id;
    else
        stream >> std::hex >> id;
    return id | (dlcSkin ? 0x80000000U : 0U);
}
std::wstring CMinecraftApp::getSkinPathFromId(unsigned int id)
{
    wchar_t path[256];
    if (static_cast<int>(id) < 0)
        std::swprintf(path, 256, L"dlcskin%08d.png", id & 0x7fffffffU);
    else if ((id & 0x7fffffe0U) == 0)
        std::swprintf(path, 256, L"defskin%08X.png", id & 0x1fU);
    else
        std::swprintf(path, 256, L"ugcskin%08X.png", id);
    return std::wstring(path);
}
uint32_t CMinecraftApp::GetTMSGlobalFileListRead()
{
    return 1;
}
uint32_t CMinecraftApp::GetTMSDLCInfoRead()
{
    return 1;
}
uint32_t CMinecraftApp::GetTMSXUIDsFileRead()
{
    return 1;
}
void CMinecraftApp::SetRequiredTexturePackID(uint32_t firstArgument)
{
    mRequiredTexturePackID = firstArgument;
}
void CMinecraftApp::GetFileFromTPD(eTPDFileType type, unsigned char* source, unsigned int sourceSize, unsigned char** data, unsigned int* size)
{
    *data = 0;
    *size = 0;
}
void CMinecraftApp::getLocale(std::vector<std::wstring>& locale)
{
    extern unsigned int XGetLanguage();
    extern int XGetLocale();

    std::vector<int> localeIds;
    const unsigned int language = XGetLanguage();
    bool finalizeEarly = false;

    if (language < 0xF1)
    {
        switch (language)
        {
        case 0:
            break;
        case 1:
        {
            unsigned int localeValue = static_cast<unsigned int>(XGetLocale());
            if (localeValue < 0x18)
            {
                if (localeValue != 1 && localeValue != 5 && localeValue != 9)
                {
                    if (localeValue < 0xE)
                        break;
                    if (localeValue > 0x12)
                    {
                        localeIds.push_back(1);
                        localeIds.push_back(0);
                        finalizeEarly = true;
                        break;
                    }
                }
            }
            else if (localeValue != 0x18 &&
                (localeValue < 0x1C ||
                 ((localeValue > 0x1E && localeValue != 0x23) &&
                  (localeValue < 0x29 || localeValue > 0x2B))))
            {
                break;
            }
            if (!finalizeEarly)
                localeIds.push_back(2);
            break;
        }
        case 2:
            localeIds.push_back(7);
            break;
        case 3:
        {
            const int localeValue = XGetLocale();
            if (localeValue == 2)
                localeIds.push_back(9);
            else if (localeValue == 0x21)
                localeIds.push_back(0x26);
            else
            {
                localeIds.push_back(8);
                break;
            }
            localeIds.push_back(8);
            break;
        }
        case 4:
        {
            const int localeValue = XGetLocale();
            if (localeValue == 3)
                localeIds.push_back(0x24);
            else if (localeValue == 5)
                localeIds.push_back(0xB);
            else if (localeValue == 0x21)
                localeIds.push_back(0x25);
            else
            {
                localeIds.push_back(10);
                break;
            }
            localeIds.push_back(10);
            break;
        }
        case 5:
        {
            const unsigned int localeValue = static_cast<unsigned int>(XGetLocale());
            if (localeValue < 0x24)
            {
                if (localeValue != 6 && localeValue != 8 && localeValue != 0x16)
                {
                    localeIds.push_back(0xC);
                    break;
                }
                localeIds.push_back(0x1D);
                localeIds.push_back(0xD);
            }
            else if (localeValue == 0x24 || localeValue == 0x28 || localeValue == 0xF0)
            {
                localeIds.push_back(0x1D);
                localeIds.push_back(0xD);
            }
            localeIds.push_back(0xC);
            break;
        }
        case 6:
            localeIds.push_back(0xE);
            break;
        case 7:
            localeIds.push_back(0xF);
            break;
        case 8:
        {
            const int localeValue = XGetLocale();
            if (localeValue == 0xF)
            {
                localeIds.push_back(0x20);
                localeIds.push_back(0x21);
            }
            else if (localeValue == 0x22)
            {
                localeIds.push_back(0x21);
                localeIds.push_back(0x20);
            }
            else
            {
                localeIds.push_back(0x3D);
                localeIds.push_back(0x1C);
                break;
            }
            localeIds.push_back(0x3D);
            localeIds.push_back(0x1C);
            break;
        }
        case 9:
            if (XGetLocale() == 4)
                localeIds.push_back(0x11);
            localeIds.push_back(0x10);
            break;
        case 10:
            break;
        case 0xB:
            localeIds.push_back(0x18);
            break;
        case 0xC:
            localeIds.push_back(0x12);
            break;
        case 0xD:
            localeIds.push_back(0x15);
            localeIds.push_back(0x37);
            break;
        case 0xE:
            localeIds.push_back(0x19);
            break;
        case 0xF:
            localeIds.push_back(0x27);
            localeIds.push_back(0x17);
            localeIds.push_back(0x3A);
            break;
        case 0x10:
            if (XGetLocale() == 3)
                localeIds.push_back(0x22);
            localeIds.push_back(0x13);
            break;
        case 0x11:
            localeIds.push_back(0x3C);
            break;
        default:
            break;
        }
    }
    else if (language < 0xF2)
    {
        localeIds.push_back(0x14);
    }
    else if (language == 0xF2)
    {
        localeIds.push_back(0x1A);
        localeIds.push_back(0x39);
        localeIds.push_back(0x28);
        localeIds.push_back(2);
    }
    else if (language < 0xF4)
    {
        localeIds.push_back(0x16);
        localeIds.push_back(0x23);
    }
    else if (language == 0xF4)
    {
        localeIds.push_back(0x38);
        localeIds.push_back(0x31);
    }
    else
    {
        localeIds.push_back(1);
        localeIds.push_back(0);
        finalizeEarly = true;
    }

    if (!finalizeEarly)
    {
        localeIds.push_back(1);
        localeIds.push_back(0);
    }

    for (size_t index = 0; index < localeIds.size(); ++index)
    {
        locale.push_back(mLocaleToString[localeIds[index]]);
    }
}
void CMinecraftApp::SetFirstMiniGameType(EMiniGameId type)
{
    mFirstMiniGameType = type;
}
EMiniGameId CMinecraftApp::GetFirstMiniGameType()
{
    return mFirstMiniGameType;
}
void CMinecraftApp::ProcessFakePadValueA()
{
    int wordBase = 0;
    for (int pad = 0; pad < 4; ++pad)
    {
        unsigned int high = mFakePadPending[wordBase + 2];
        unsigned int low = mFakePadPending[wordBase + 3];
        for (int bit = 0; bit < 64; ++bit)
        {
            bool set = bit < 32 ? (low & (1u << bit)) != 0
                                  : (high & (1u << (bit - 32))) != 0;
            if (set)
            {
                g_pInput->SetFakePadValue(pad, static_cast<unsigned char>(bit));
                g_pInput->SetGameJoypadMaps(pad, 0x20, 0);
            }
        }
        mFakePadPending[wordBase + 2] = 0;
        mFakePadPending[wordBase + 3] = 0;
        wordBase += 2;
    }
}
uint32_t CMinecraftApp::IsVoiceChatEnabled()
{
    return GetGameStarted() != 0 &&
        GameSettings::GetGameSettings(GetPrimaryUserIndex(), static_cast<eGameSetting>(0x22)) != 0;
}
void CMinecraftApp::CaptureScreenshot(int32_t firstArgument)
{
}
void CMinecraftApp::RunFrame()
{
}
void CMinecraftApp::HandleXuiActions()
{
    static unsigned int actionTicks = 0;
    ++actionTicks;
    if (GetGameStarted() != 0)
    {
        if (IsAppPaused() != 0)
            actionTicks = 0;
        else if (actionTicks > 0x7fffffffU)
            actionTicks = 1;
    }
    else
    {
        actionTicks = 0;
    }
}
void CMinecraftApp::FatalLoadError(const wchar_t* message, ...)
{
    if (message == 0)
        return;
}
void CMinecraftApp::SetSkinAdjustments(uint32_t id, _SkinAdjustments value)
{
    g_skinAdjustments[id] = value;
}
_SkinAdjustments CMinecraftApp::GetSkinAdjustments(uint32_t id)
{
    std::map<unsigned int, _SkinAdjustments>::const_iterator found = g_skinAdjustments.find(id);
    if (found != g_skinAdjustments.end())
        return found->second;
    return _SkinAdjustments();
}
