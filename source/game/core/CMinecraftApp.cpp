#include "game/core/CMinecraftApp.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/PlatformTiming.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "platform/wiiu/cCommerceNintendoWiiU.hpp"
#include "game/core/ArchiveFile.hpp"
#include "game/core/File.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/client/render/PlayerModel.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/Player.hpp"
#include "platform/wiiu/CStorage.hpp"
#include <sstream>
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/MinecartHopper.hpp"
#include "game/core/L10N.hpp"
#include "game/client/audio/ConsoleSoundEngine.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "game/core/GameType.hpp"
#include "game/core/Compression.hpp"
#include "game/core/GameModeTransitionParams.hpp"
#include "game/core/GhostController.hpp"
#include "game/util/Random.hpp"
#include "game/core/StringUtils.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/item/FireworksRecipe.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/chunk/LevelChunk.hpp"

struct SKIN_BOX
{
    int id;
    float x;
    float y;
    float z;
    float width;
    float height;
    float depth;
    float xTexOffs;
    float yTexOffs;
    float delta;
    unsigned int armourFlags;
    unsigned int mirror;
};
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/core/Achievement.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/IUIScene_DLCUpsell.hpp"
#include "game/core/MOJJANG_DATA.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/GameRuleManager.hpp"
#include "game/core/Gui.hpp"
#include "game/core/Stat.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/item/Container.hpp"
#include "game/item/Inventory.hpp"
#include "game/core/Merchant.hpp"
#include "game/ui/UIScene_PauseMenu.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/SignBlockEntity.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "platform/wiiu/PlayerUID.hpp"
#include "platform/wiiu/cNintendoDLCInfo.hpp"
#include <algorithm>
#include <boost/pointer_cast.hpp>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <cstdlib>
#include <map>
#include <sstream>
#include <vector>

void Sleep(unsigned int milliseconds);

unsigned int GetTickCount();

static boost::unordered_map<PlayerUID, MOJJANG_DATA*> g_mojangData;
unsigned int CMinecraftApp::sPreviousSignInUsersMask = 0;
CConsoleMinecraftApp gApp;
CMinecraftApp* g_pApp = &gApp;
static Random sTipRandom;
static bool s_pendingDlcInstallTip;

namespace
{

struct BANNEDLISTDATA
{
    PlayerUID playerUid;
    char levelId[14];
    char padding[2];
};

struct ContainerMenuInitData
{
    boost::shared_ptr<Container> inventory;
    boost::shared_ptr<Container> container;
    int player;
    bool splitscreen;
};

struct HorseMenuInitData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Container> container;
    boost::shared_ptr<AbstractHorse> horse;
    int player;
    bool splitscreen;
};

struct InventoryContainerMenuInitData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Container> container;
    int player;
    bool splitscreen;
};

struct ContainerPairMenuInitData
{
    boost::shared_ptr<Container> inventory;
    boost::shared_ptr<Container> container;
    int player;
    bool splitscreen;
};

struct TradingMenuInitData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Merchant> merchant;
    Level* level;
    int player;
    bool splitscreen;
};

struct SignEntryMenuInitData
{
    boost::shared_ptr<SignBlockEntity> sign;
    int player;
};

struct LocalPlayerMenuInitData
{
    boost::shared_ptr<LocalPlayer> localPlayer;
    bool creative;
    int player;
    bool splitscreen;
};

struct EnchantingMenuInitData
{
    boost::shared_ptr<Inventory> inventory;
    Level* level;
    int x;
    int y;
    int z;
    int playerIndex;
    bool splitscreen;
    std::wstring name;
};

struct FireworksMenuInitData
{
    boost::shared_ptr<LocalPlayer> localPlayer;
    bool splitscreen;
    int player;
    int x;
    int y;
    int z;
    unsigned char flag;
};

struct RepairingMenuInitData
{
    boost::shared_ptr<Inventory> inventory;
    Level* level;
    int x;
    int y;
    int z;
    int player;
    bool splitscreen;
};

struct WritingBookMenuInitData
{
    boost::shared_ptr<ItemInstance> item;
    int player;
    bool signing;
};

std::vector<BANNEDLISTDATA*>& BannedLevelListAt(CMinecraftApp* minecraftApp, int player)
{
    std::vector<BANNEDLISTDATA*>** lists =
        reinterpret_cast<std::vector<BANNEDLISTDATA*>**>(reinterpret_cast<char*>(minecraftApp) + 0x1ac);
    return *lists[player];
}

int DisconnectMessageForReason(unsigned int reason)
{
    switch (reason)
    {
    case 7: return 0xad3e1729;
    case 8: return 0xf3a95051;
    case 0xe: return 0xb59cada7;
    case 0xf: return 0xce44990b;
    case 0x14: return 0xfb7f0496;
    case 0x15: return 0x4d669d55;
    default: return 0x10f4b2d5;
    }
}

bool IsLicenseExemptTexturePack(const std::wstring& folderName)
{
    static const wchar_t* kNames[] = {
        L"Battle & Beasts", L"Battle & Beasts 2", L"Festive", L"Fantasy_Texture_Pack",
        L"City_Texture_Pack", L"Natural_Texture_Pack", L"Mario"
    };
    for (int i = 0; i < 7; ++i)
    {
        if (folderName == kNames[i])
            return true;
    }
    return false;
}
}

static PlayerUID GetTutorialPlayerUid(const Player* player)
{
    if (player == NULL)
        return PlayerUID(0);
    const char* base = reinterpret_cast<const char*>(player);
    PlayerUID primary;
    PlayerUID alternate;
    std::memcpy(&primary, base + 0x784, sizeof(PlayerUID));
    std::memcpy(&alternate, base + 0x798, sizeof(PlayerUID));
    return primary != PlayerUID(0) ? primary : alternate;
}
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
      mRequiredTexturePackID(0), mFirstMiniGameType(eMiniGameId_Lobby),
      mMediaArchive(NULL), mSaveNotificationCount(0), mDLCDownloadQueueResetFlag(false)
{
    InitializeCriticalSection(&mPendingActionsMutex);
    InitializeCriticalSection(&mSaveNotificationMutex);
    InitializeCriticalSection(&mActionMutex);
    InitializeCriticalSection(&mTerrainFeaturePositionsMutex);
    InitializeCriticalSection(&mMemoryTextureMutex);
    InitializeCriticalSection(&mMemoryTPDMutex);
    InitializeCriticalSection(&mAdditionalModelPartsMutex);
    InitializeCriticalSection(&mAdditionalSkinBoxesMutex);
    InitializeCriticalSection(&mSkinAdjustmentsMutex);
    InitializeCriticalSection(&mDLCRetrieveMutex);
    InitializeCriticalSection(&mDLCDownloadQueueMutex);
    SetBanListCheck(0, false);
    SetBanListCheck(1, false);
    SetBanListCheck(2, false);
    SetBanListCheck(3, false);
    std::fill(mOpacityTimer, mOpacityTimer + 4, 0);
    std::fill(mOptionsCallbackStatus, mOptionsCallbackStatus + 4, 0);
    std::fill(mPlayerIds, mPlayerIds + 8, 0);
    std::fill(mPlayerPrivileges, mPlayerPrivileges + 8, 0);
    std::fill(mPlayerGameTypes, mPlayerGameTypes + 8, 0);
    std::fill(mBanListRead, mBanListRead + 4, false);
    std::fill(mBanListBuffers, mBanListBuffers + 4, static_cast<unsigned char*>(NULL));
    std::memset(mUniqueMapName, 0, sizeof(mUniqueMapName));
    for (int player = 0; player < 4; ++player)
        mCachedSignInUIDs[player] = PlayerUID(0);
    LocaleAndLanguageInit();
}
CMinecraftApp::~CMinecraftApp()
{
    delete mMediaArchive;
    mMediaArchive = NULL;
    for (int player = 0; player < 4; ++player)
    {
        if (mBanListBuffers[player] != NULL)
        {
            delete[] mBanListBuffers[player];
            mBanListBuffers[player] = NULL;
        }
    }
    DeleteCriticalSection(&mSaveNotificationMutex);
    DeleteCriticalSection(&mActionMutex);
    DeleteCriticalSection(&mPendingActionsMutex);
    DeleteCriticalSection(&mTerrainFeaturePositionsMutex);
    DeleteCriticalSection(&mMemoryTextureMutex);
    DeleteCriticalSection(&mMemoryTPDMutex);
    DeleteCriticalSection(&mAdditionalModelPartsMutex);
    DeleteCriticalSection(&mAdditionalSkinBoxesMutex);
    DeleteCriticalSection(&mSkinAdjustmentsMutex);
}
void CMinecraftApp::SetGameHostOption(unsigned int& options, eGameHostOption option, unsigned int value)
{
    switch (static_cast<unsigned int>(option))
    {
    case 0: options = (options & ~3U) | (value & 3U); break;
    case 3: options = value ? (options | 4U) : (options & ~4U); break;
    case 4: options = value ? (options | 8U) : (options & ~8U); break;
    case 6: options = (options & ~0x30U) | ((value & 3U) << 4); break;
    case 7: options = value ? (options | 0x40U) : (options & ~0x40U); break;
    case 8: options = value ? (options | 0x80U) : (options & ~0x80U); break;
    case 9: options = value ? (options | 0x100U) : (options & ~0x100U); break;
    case 10: options = value ? (options | 0x200U) : (options & ~0x200U); break;
    case 11: options = value ? (options | 0x400U) : (options & ~0x400U); break;
    case 12: options = value ? (options | 0x800U) : (options & ~0x800U); break;
    case 13: options = value ? (options | 0x1000U) : (options & ~0x1000U); break;
    case 14: options = value ? (options | 0x2000U) : (options & ~0x2000U); break;
    case 15: options = value ? (options | 0x1c000U) : (options & ~0x1c000U); break;
    case 16: options = value ? (options | 0x4000U) : (options & ~0x4000U); break;
    case 17: options = value ? (options | 0x8000U) : (options & ~0x8000U); break;
    case 18: options = value ? (options | 0x10000U) : (options & ~0x10000U); break;
    case 20: options = (options & ~0x700000U) | ((value & 7U) << 20); break;
    case 21: options = value ? (options & ~0x80000000U) : (options | 0x80000000U); break;
    case 22: options = value ? (options | 0x40000U) : (options & ~0x40000U); break;
    case 23: options = value ? (options | 0x80000U) : (options & ~0x80000U); break;
    case 24: options = value ? (options & ~0x800000U) : (options | 0x800000U); break;
    case 25: options = value ? (options | 0x1000000U) : (options & ~0x1000000U); break;
    case 26: options = value ? (options & ~0x2000000U) : (options | 0x2000000U); break;
    case 27: options = value ? (options & ~0x4000000U) : (options | 0x4000000U); break;
    case 28: options = value ? (options & ~0x8000000U) : (options | 0x8000000U); break;
    case 29: options = value ? (options & ~0x10000000U) : (options | 0x10000000U); break;
    case 30: options = value ? (options & ~0x20000000U) : (options | 0x20000000U); break;
    case 31: options = value ? (options | 0x40000000U) : (options & ~0x40000000U); break;
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
    std::wstring path = L"Common/Media/MediaWiiU.arc";
    if (!path.empty())
    {
        File file(path);
        mMediaArchive = new ArchiveFile(file);
    }
}
arrayWithLength<unsigned char> CMinecraftApp::getArchiveFile(const std::wstring& firstArgument, bool secondArgument)
{
    if (secondArgument == 0)
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        if (minecraft != 0 && minecraft->mTexturePackRepository != 0)
        {
            TexturePack* selected = minecraft->mTexturePackRepository->getSelected();
            if (selected != 0 && selected->hasData() && selected->getArchiveFile() != 0 &&
                selected->getArchiveFile()->hasFile(firstArgument))
                return selected->getArchiveFile()->getFile(firstArgument);
        }
    }
    if (mMediaArchive != 0 && mMediaArchive->hasFile(firstArgument))
        return mMediaArchive->getFile(firstArgument);
    return arrayWithLength<unsigned char>();
}
bool CMinecraftApp::hasArchiveFile(const std::wstring& firstArgument, bool secondArgument)
{
    if (secondArgument == 0)
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        if (minecraft != 0 && minecraft->mTexturePackRepository != 0)
        {
            TexturePack* selected = minecraft->mTexturePackRepository->getSelected();
            if (selected != 0 && selected->getArchiveFile() != 0 &&
                selected->getArchiveFile()->hasFile(firstArgument))
                return true;
        }
    }
    return mMediaArchive != 0 && mMediaArchive->hasFile(firstArgument);
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
    if (static_cast<unsigned int>(player) >= 4U)
        return;

    const unsigned int actionValue = static_cast<unsigned int>(action);
    unsigned int& currentAction = mXuiActions[player].action;
    void*& currentParam = mXuiActions[player].param;

    if ((IsActionPending(player, action) == 0 ||
         (actionValue != 0x10U && actionValue != 3U && actionValue != 8U)) &&
        (currentAction != 0x19U || actionValue != 0x26U) &&
        (currentAction == 0U || actionValue != 7U) &&
        (currentAction != 0x11U || actionValue != 0x10U))
    {
        if (actionValue == 0U)
        {
            std::vector<XuiActionData>& pending = mPendingActions[player];
            if (!pending.empty())
            {
                EnterCriticalSection(&mPendingActionsMutex);
                currentAction = pending.front().action;
                currentParam = pending.front().param;
                pending.erase(pending.begin());
                LeaveCriticalSection(&mPendingActionsMutex);
                return;
            }
        }
        else if (currentAction != 0U &&
                 !isValidStateTransition(static_cast<eXuiAction>(currentAction), action))
        {
            EnterCriticalSection(&mPendingActionsMutex);
            XuiActionData entry;
            entry.action = actionValue;
            entry.param = param;
            mPendingActions[player].push_back(entry);
            LeaveCriticalSection(&mPendingActionsMutex);
            return;
        }
        currentAction = actionValue;
        currentParam = param;
    }
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
    if (firstArgument < 0 || firstArgument >= 4 || !mBanListRead[firstArgument])
        return;
    mBanListRead[firstArgument] = false;
    SetBanListCheck(firstArgument, false);
    mBanListEntries[firstArgument].clear();
    if (mBanListBuffers[firstArgument] != NULL)
    {
        delete[] mBanListBuffers[firstArgument];
        mBanListBuffers[firstArgument] = NULL;
    }
}
void CMinecraftApp::ClearAndResetDLCDownloadQueue()
{
    EnterCriticalSection(&mDLCDownloadQueueMutex);
    for (std::vector<_DLCRequest*>::iterator it = mDLCDownloadQueue.begin();
         it != mDLCDownloadQueue.end(); ++it)
        delete *it;
    mDLCDownloadQueue.clear();
    mDLCDownloadQueueResetFlag = true;
    LeaveCriticalSection(&mDLCDownloadQueueMutex);
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
    EnterCriticalSection(&mSaveNotificationMutex);
    const int previousCount = mSaveNotificationCount;
    ++mSaveNotificationCount;
    if (previousCount == 0 &&
        CGameNetworkManager::sInstance.IsInSession() &&
        !CGameNetworkManager::sInstance.IsLeavingGame() &&
        gMinecraftServer != NULL)
    {
        gMinecraftServer->broadcastStartSavingPacket();
        if (CGameNetworkManager::sInstance.IsLocalGame() &&
            CGameNetworkManager::sInstance.GetPlayerCount() == 1)
        {
            SetXuiServerAction(GetPrimaryUserIndex(), static_cast<eXuiServerAction>(5),
                               reinterpret_cast<void*>(1));
        }
    }
    LeaveCriticalSection(&mSaveNotificationMutex);
}
void CMinecraftApp::LeaveSaveNotificationSection()
{
    EnterCriticalSection(&mSaveNotificationMutex);
    mSaveNotificationCount = mSaveNotificationCount - 1;
    if (mSaveNotificationCount == 0 &&
        CGameNetworkManager::sInstance.IsInSession() &&
        !CGameNetworkManager::sInstance.IsLeavingGame() &&
        gMinecraftServer != NULL)
    {
        gMinecraftServer->broadcastStopSavingPacket();
        if (CGameNetworkManager::sInstance.IsLocalGame() &&
            CGameNetworkManager::sInstance.GetPlayerCount() == 1)
        {
            SetXuiServerAction(GetPrimaryUserIndex(), static_cast<eXuiServerAction>(5),
                               reinterpret_cast<void*>(0));
        }
    }
    LeaveCriticalSection(&mSaveNotificationMutex);
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
    ESen_UpsellOutcome outcome;
    if (response == static_cast<eUpsellResponse>(1))
        outcome = static_cast<ESen_UpsellOutcome>(3);
    else if (response == static_cast<eUpsellResponse>(2))
        outcome = static_cast<ESen_UpsellOutcome>(1);
    else
        outcome = static_cast<ESen_UpsellOutcome>(2);

    const int playerId = gProfile->GetPrimaryPad();
    telemetryManagerInstance->RecordUpsellResponded(playerId, eSen_UpsellID_FullGame, 1, outcome);
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
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mPlayer == NULL)
    {
        gConsoleUIController.NavigateToHomeMenu(false);
        cCommerceNintendoWiiU* commerce = gApp.GetCommerceInstance();
        if (commerce != NULL)
        {
            commerce->SetNetworkConnected(false);
            commerce->ClearShoppingCatalogRetrieved();
        }
        if (button == 0xff)
            button = gProfile->GetPrimaryPad();
        g_pApp->SetAction(button, static_cast<eXuiAction>(0x13), NULL);
    }
    else
    {
        g_pApp->SetAction(minecraft->mPlayer->mUserIndex, static_cast<eXuiAction>(0x12), NULL);
    }

    ClientMasterGameMode* gameMode = minecraft->GetClientMasterGameMode();
    if (gameMode != NULL)
        gameMode->OnExitedGame();
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
    Minecraft* minecraft = Minecraft::GetInstance();
    if (result == static_cast<C4JStorage::EMessageResult>(4))
    {
        const int userIndex = minecraft->mPlayer->mUserIndex;
        if (gProfile->IsSignedInLive(userIndex))
            gProfile->DisplayFullVersionPurchase(false, userIndex, 1);
    }
    else
    {
        telemetryManagerInstance->RecordUpsellResponded(button, eSen_UpsellID_FullGame, 1,
                                              static_cast<ESen_UpsellOutcome>(2));
    }
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
    Minecraft* minecraft = Minecraft::GetInstance();
    int userIndex = minecraft->mPlayer->mUserIndex;
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);

    if (result == static_cast<C4JStorage::EMessageResult>(4))
    {
        if (gProfile->IsSignedInLive(userIndex))
        {
            gProfile->DisplayFullVersionPurchase(false, userIndex, 1);
            app->SetAction(userIndex, static_cast<eXuiAction>(5), NULL);
        }
    }
    else
    {
        telemetryManagerInstance->RecordUpsellResponded(button, eSen_UpsellID_FullGame, 1,
                                              static_cast<ESen_UpsellOutcome>(2));
        app->SetAction(userIndex, static_cast<eXuiAction>(5), NULL);
    }
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
    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();

    Minecraft* minecraft = Minecraft::GetInstance();
    int dimension = -1;
    const bool isClient = minecraft->isClientSide();
    const bool inSession = CGameNetworkManager::sInstance.IsInSession();

    if (!isClient && !inSession)
    {
        if (context == NULL)
            goto waitForSessionEnd;

        if (minecraft->mGui != NULL)
            minecraft->mGui->displayClientMessage(DisconnectMessageForReason(g_pApp->GetDisconnectReason()), 0);

        if (!CGameNetworkManager::sInstance.IsInSession())
            goto waitForSessionEnd;
    }
    else
    {
        if (context == NULL)
        {
            if (minecraft->mGui != NULL)
                minecraft->mGui->displayClientMessage(0x3e0b4895, 0);

            for (int playerIndex = 0; playerIndex < 2; ++playerIndex)
            {
                boost::shared_ptr<MultiplayerLocalPlayer>& localPlayer = minecraft->mLocalPlayers[playerIndex];
                if (localPlayer)
                    localPlayer->SetWaitingForScoreboard(true);
            }
        }
        else
        {
            if (minecraft->mGui != NULL)
                minecraft->mGui->displayClientMessage(DisconnectMessageForReason(g_pApp->GetDisconnectReason()), 0);

            boost::shared_ptr<MultiplayerLocalPlayer>& firstPlayer = minecraft->mLocalPlayers[0];
            if (firstPlayer)
                firstPlayer->SetWaitingForScoreboard(false);

            boost::shared_ptr<MultiplayerLocalPlayer>& secondPlayer = minecraft->mLocalPlayers[1];
            if (secondPlayer)
            {
                secondPlayer->SetWaitingForScoreboard(false);
                MinecraftServer::HaltServer(true);
                CGameNetworkManager::sInstance.LeaveGame(false);
                goto waitForSessionEnd;
            }
        }

        MinecraftServer::HaltServer(true);
        CGameNetworkManager::sInstance.LeaveGame(false);
    }

waitForSessionEnd:
    while (CGameNetworkManager::sInstance.IsInSession())
        Sleep(1);

    while (gConsoleUIController.IsReloadingSkin())
        Sleep(1);

    boost::shared_ptr<Player> emptyPlayer;
    minecraft->setLevel(NULL, dimension, emptyPlayer, false, true);
    gApp.mGameRuleManager.unloadCurrentGameRules();
    return 0;
}
uint32_t CMinecraftApp::TrialOverReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    int userIndex = minecraft->mPlayer->mUserIndex;
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);

    if (result == static_cast<C4JStorage::EMessageResult>(4))
    {
        if (!gProfile->IsSignedInLive(userIndex))
            app->SetAction(userIndex, static_cast<eXuiAction>(0xe), NULL);
        else
            gProfile->DisplayFullVersionPurchase(false, userIndex, 1);
    }
    else
    {
        telemetryManagerInstance->RecordUpsellResponded(button, eSen_UpsellID_FullGame, 1,
                                              static_cast<ESen_UpsellOutcome>(2));
        app->SetAction(userIndex, static_cast<eXuiAction>(0xe), NULL);
    }
    return 0;
}
uint32_t CMinecraftApp::UnlockFullInviteReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(4))
    {
        if (gProfile->GetLiveConnectionStatus() != 0)
            gProfile->DisplayFullVersionPurchase(false, button, 1);
    }
    else
    {
        telemetryManagerInstance->RecordUpsellResponded(button, eSen_UpsellID_FullGame, 1,
                                              static_cast<ESen_UpsellOutcome>(2));
    }
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
int CMinecraftApp::ExitAndJoinFromInviteAndSaveReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(5))
    {
        TexturePack* selected = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
        if (selected != TexturePackRepository::defaultTexturePack)
        {
            DLCPack* pack = selected->getDLCPack();
            if (pack != NULL && !pack->hasPurchasedPack())
            {
                int parameters[2] = { static_cast<int>(0x4fcbc7e7), static_cast<int>(0xb9a1bf4d) };
                gConsoleUIController.RequestAlertMessage(
                    static_cast<int>(0x3bb3096e), static_cast<int>(0xd52c2e3), parameters, 2,
                    static_cast<unsigned int>(button),
                    UIScene_PauseMenu::WarningTrialTexturePackReturned, context, NULL, true);
                return 0;
            }
        }

        gApp.setIgnoreMainMenuInputDuringInvite(true);
        g_pApp->SetChangingSessionType(true);
        g_pApp->SetAction(button, static_cast<eXuiAction>(0x16), NULL);
    }
    return 0;
}
int CMinecraftApp::ExitAndJoinFromInviteDeclineSaveReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(5))
    {
        if (gMinecraftServer != NULL)
            *reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(gMinecraftServer) + 0x148) = 0;
        g_pApp->SetAction(button, static_cast<eXuiAction>(0x16), NULL);
    }
    return 0;
}
uint32_t CMinecraftApp::ExitAndJoinFromInviteSaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(5))
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        TexturePack* selected = minecraft->mTexturePackRepository->getSelected();
        if (selected != TexturePackRepository::defaultTexturePack)
        {
            DLCPack* pack = selected->getDLCPack();
            if (pack != NULL && !pack->hasPurchasedPack())
            {
                int parameters[2] = { static_cast<int>(0x4fcbc7e7), static_cast<int>(0xb9a1bf4d) };
                gConsoleUIController.RequestAlertMessage(
                    static_cast<int>(0x3bb3096e), static_cast<int>(0xd52c2e3), parameters, 2,
                    static_cast<unsigned int>(button),
                    UIScene_PauseMenu::WarningTrialTexturePackReturned, context, NULL, true);
                return 0;
            }
        }

        bool saveExists = false;
        gStorage->DoesSaveExist(&saveExists);
        if (!saveExists)
        {
            gApp.setIgnoreMainMenuInputDuringInvite(true);
            g_pApp->SetChangingSessionType(true);
            g_pApp->SetAction(button, static_cast<eXuiAction>(0x16), NULL);
        }
        else
        {
            int parameters[2] = { static_cast<int>(0x4fcbc7e7), static_cast<int>(0xb9a1bf4d) };
            gConsoleUIController.RequestErrorMessage(
                static_cast<int>(0x3143fcab), static_cast<int>(0xd66b8839), parameters, 2,
                static_cast<unsigned int>(gProfile->GetPrimaryPad()),
                ExitAndJoinFromInviteAndSaveReturned, context, NULL, true);
        }
    }
    else if (result == static_cast<C4JStorage::EMessageResult>(6))
    {
        int parameters[2] = { static_cast<int>(0x4fcbc7e7), static_cast<int>(0xb9a1bf4d) };
        gConsoleUIController.RequestErrorMessage(
            static_cast<int>(0xa3d4a110), static_cast<int>(0xbb7e4722), parameters, 2,
            static_cast<unsigned int>(gProfile->GetPrimaryPad()),
            ExitAndJoinFromInviteDeclineSaveReturned, context, NULL, true);
    }
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
    SetExtraGameData(mExtraGameData, type, value);
}
void CMinecraftApp::SetTutorialMode(bool tutorial, const boost::shared_ptr<Player>& player)
{
    if (!player)
    {
        SetExtraGameData(static_cast<eGameExtraData>(1), tutorial ? 1U : 0U);
        mTutorialModes[PlayerUID(0)] = tutorial;
        return;
    }
    mTutorialModes[GetTutorialPlayerUid(player.get())] = tutorial;
}
uint32_t CMinecraftApp::GetXuiAction(int32_t firstArgument)
{
    return mXuiActions[firstArgument].action;
}
uint32_t CMinecraftApp::RemoteSaveThreadProc(void* context)
{
    (void)context;

    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();
    FireworksRecipe::UseDefaultThreadStorage();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_RemoteSave);
    Block::CreateNewThreadStorage();

    Minecraft* minecraft = Minecraft::GetInstance();
    ProgressRenderer* progressRenderer = minecraft->mProgressRenderer;
    progressRenderer->progressStartNoAbort(static_cast<int>(0x5aaa2e05));
    progressRenderer->progressStage(-1);
    progressRenderer->progressStagePercentage(0);

    while (g_pApp->GetGameStarted() == 0)
    {
        const int player = gProfile->GetPrimaryPad();
        if (g_pApp->GetXuiAction(player) != 0x1cU)
            break;
        minecraft->tickAllConnections();
        Sleep(100);
    }

    const int player = gProfile->GetPrimaryPad();
    if (g_pApp->GetXuiAction(player) != 0x1cU)
        return 0x4c7U;

    g_pApp->SetAction(player, static_cast<eXuiAction>(0), NULL);
    gConsoleUIController.UpdatePlayerBasePositions();
    Block::ReleaseThreadStorage();
    ElementStorage::ReleaseThreadStorage();
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
    GameModeTransitionParams* params = static_cast<GameModeTransitionParams*>(context);
    const unsigned int texturePackId = params->mTexturePackId;
    const bool waitForRoundStart = params->mWaitForRoundStart;
    delete params;

    Minecraft* minecraft = Minecraft::GetInstance();
    g_pApp->SetGameStarted(false);
    minecraft->mGameRenderer->DisableUpdateThread(3);

    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    phys::Vec2::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();
    Block::CreateNewThreadStorage();
    FireworksRecipe::UseDefaultThreadStorage();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_GameModeTransition);

    ClientMasterGameMode* masterGameMode = minecraft->GetClientMasterGameMode();
    std::wstring transitionOverride = masterGameMode->GetTransitionTextOverride();

    std::vector<std::wstring> progressStrings;
    if (!transitionOverride.empty())
        progressStrings.push_back(transitionOverride);
    progressStrings.push_back(L10N::GetString(0xbc34c612));
    progressStrings.push_back(L"TPACK");

    ProgressRenderer* progressRenderer = minecraft->mProgressRenderer;
    progressRenderer->progressStartNoAbort(progressStrings[0]);
    progressRenderer->progressStage(progressStrings[1]);

    progressRenderer->progressStagePercentage(0);
    Sleep(100);
    progressRenderer->progressStagePercentage(10);

    TexturePackRepository* texturePackRepository = minecraft->mTexturePackRepository;
    TexturePack* previousPack = texturePackRepository->getSelected();
    TexturePack* previousPackWithData = NULL;
    if (previousPack != NULL && previousPack->hasData())
        previousPackWithData = previousPack;

    progressRenderer->progressStagePercentage(20);

    if (texturePackRepository->selectTexturePackById(texturePackId, false))
    {
        SetupMiniGameMusic();
        if (previousPackWithData != NULL)
        {
            previousPackWithData->loadData();
            if (gStorage != NULL)
                gStorage->UnmountInstalledDLC("TPACK");
        }
        else if (previousPack != TexturePackRepository::defaultTexturePack)
        {
            if (gStorage != NULL)
                gStorage->UnmountInstalledDLC("TPACK");
        }
    }

    int progress = 30;
    int progressAccumulator = 0;
    bool exitWaitLoop = false;
    bool setGameStartedWhenReady = true;

    for (;;)
    {
        if (waitForRoundStart && !exitWaitLoop &&
            CGameNetworkManager::sInstance.IsInSession() &&
            !CGameNetworkManager::sInstance.IsLeavingGame())
        {
            for (;;)
            {
                masterGameMode->IsRoundStarting();
                gConsoleUIController.IsExpectingOrReloadingSkin();
                CGameNetworkManager::sInstance.IsInSession();
                minecraft->tickAllConnections();
                if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
                {
                    GhostController* ghostController = minecraft->GetGhostController();
                    if (ghostController != NULL)
                        ghostController->tick();
                }
                Sleep(100);
                progressAccumulator = (progressAccumulator + 100) % 1000;
                if (progressAccumulator == 0 && progress < 90)
                {
                    progress += 10;
                    progressRenderer->progressStagePercentage(progress);
                }

                if (exitWaitLoop)
                    break;

                if (!masterGameMode->IsRoundStarting() &&
                    !masterGameMode->ServerIsWaitingOnUsLoading())
                {
                    if (!g_pApp->IsActionPending(0, static_cast<eXuiAction>(3)) &&
                        (!g_pApp->IsActionPending(0, static_cast<eXuiAction>(0x10)) ||
                         CGameNetworkManager::sInstance.IsLocalGame()))
                    {
                        break;
                    }
                    setGameStartedWhenReady = false;
                    exitWaitLoop = true;
                }
                else
                {
                    exitWaitLoop = true;
                }

                if (!gConsoleUIController.IsExpectingOrReloadingSkin())
                    goto complete_transition;
            }
            continue;
        }

        if (!gConsoleUIController.IsExpectingOrReloadingSkin())
            goto complete_transition;

        masterGameMode->IsRoundStarting();
        gConsoleUIController.IsExpectingOrReloadingSkin();
        CGameNetworkManager::sInstance.IsInSession();
        minecraft->tickAllConnections();
        if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
        {
            GhostController* ghostController = minecraft->GetGhostController();
            if (ghostController != NULL)
                ghostController->tick();
        }
        Sleep(100);
        progressAccumulator = (progressAccumulator + 100) % 1000;
        if (progressAccumulator == 0 && progress < 90)
        {
            progress += 10;
            progressRenderer->progressStagePercentage(progress);
        }
    }

complete_transition:
    progressRenderer->progressStagePercentage(90);

    if (!CGameNetworkManager::sInstance.IsHost())
    {
        const int primaryPad = gProfile->GetPrimaryPad();
        if (!gProfile->IsSignedIn(primaryPad))
            setGameStartedWhenReady = false;
    }

    if (CGameNetworkManager::sInstance.IsInSession() &&
        !CGameNetworkManager::sInstance.IsLeavingGame() &&
        setGameStartedWhenReady)
    {
        g_pApp->SetGameStarted(true);
    }

    minecraft->mGameRenderer->EnableUpdateThread(3);
    g_pApp->SetAction(gProfile->GetPrimaryPad(), static_cast<eXuiAction>(0), NULL);

    if (g_pApp->GetGameStarted() == 0 || !waitForRoundStart)
        goto cleanup;

    for (;;)
    {
        if (!CGameNetworkManager::sInstance.IsInSession() ||
            CGameNetworkManager::sInstance.IsLeavingGame())
        {
            goto cleanup;
        }
        Sleep(100);
        if (masterGameMode->IsRoundStarting() ||
            g_pApp->IsActionPending(0, static_cast<eXuiAction>(3)))
        {
            goto cleanup;
        }
        if (g_pApp->IsActionPending(0, static_cast<eXuiAction>(0x10)) &&
            !CGameNetworkManager::sInstance.IsLocalGame())
        {
            Block::ReleaseThreadStorage();
            ElementStorage::ReleaseThreadStorage();
            return 0;
        }
    }

cleanup:
    Block::ReleaseThreadStorage();
    ElementStorage::ReleaseThreadStorage();
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
    case 4: return L"Tutorial World";
    case 0x200: return L"Skin Pack 1";
    case 0x201: return L"Skin Pack 2";
    case 0x400: return L"Mass Effect";
    case 0x401: return L"Skyrim";
    case 0x402: return L"Halo";
    case 0x405: return L"Festive";
    case 0x406: return L"LittleBigPlanet";
    case 0x407: return L"Greek Mythology";
    case 0x408: return L"Norse Mythology";
    case 0x409: return L"Halloween 2015";
    case 0x40a: return L"Mario";
    case 0x40c: return L"Adventure Time";
    case 0x40e: return L"Pirates Of The Caribbean";
    case 0x40f: return L"Egyptian Mythology";
    case 0x4f4: return L"Chinese Mythology";
    case 0x4f5: return L"Fallout";
    case 0x4f9: return L"The Nightmare Before Christmas";
    case 0x801: case 0x1000801: return L"Plastic";
    case 0x802: return L"Candy";
    case 0x803: return L"Fantasy";
    case 0x804: return L"Halloween Texture Pack";
    case 0x805: return L"Natural";
    case 0x806: case 0x1000806: return L"City";
    case 0x807: return L"Cartoon";
    case 0x808: case 0x1000808: return L"Steampunk";
    case 0x809: return L"Pattern";
    case 0x812: return L"Super Cute Texture Pack";
    case 0x1001: return L"Battle Map Pack 0";
    case 0x1002: return L"Battle Map Pack 1";
    case 0x1003: return L"Battle Map Pack 2";
    case 0x1004: return L"Battle Map Pack 3";
    case 0x1005: return L"Battle Map Pack 4";
    case 0x1007: return L"MG02 Map Pack 0";
    case 0x1009: return L"MG02 Map Pack 1";
    case 0x100a: return L"MG04_TestWorldPack";
    case 0x100b: return L"Halloween Battle Map";
    case 0x100c: return L"Festive Battle Map";
    case 0x100e: return L"Fallout Battle Map Pack";
    case 0x100f: return L"Glide Map Pack 0";
    case 0x1011: return L"Glide Beasts Track Pack";
    case 0x1012: return L"Glide Giants Track Pack";
    case 0x1013: return L"Glide Myths Track Pack";
    case 0x1014: return L"MG02 Map Pack 2";
    case 0x1015: return L"MG02 Map Pack 3";
    default: return std::wstring();
    }
}
std::wstring CMinecraftApp::getRootPath(unsigned int contentType, bool create, bool mounted, std::wstring suffix)
{
    std::wstring path = suffix;
    if (create)
    {
        bool useDLCFolder;
        if (contentType < 0x400)
        {
            if (contentType < 0x222)
                useDLCFolder = (contentType == 4) || (contentType == 0x207) || (contentType == 0x216);
            else
                useDLCFolder = (contentType < 0x227) || (contentType == 0x22c) || (contentType == 0x230) || (contentType == 0x233);
        }
        else if (contentType < 0x4f4)
        {
            if (contentType <= 0x402)
                useDLCFolder = true;
            else if (contentType <= 0x404)
                useDLCFolder = false;
            else if (contentType <= 0x40a)
                useDLCFolder = true;
            else if (contentType == 0x40c)
                useDLCFolder = true;
            else if (contentType <= 0x40d)
                useDLCFolder = false;
            else if (contentType <= 0x40f)
                useDLCFolder = true;
            else
                useDLCFolder = false;
        }
        else
        {
            useDLCFolder = (contentType <= 0x4f5) || (contentType == 0x4f9) || (contentType == 0x804) || (contentType == 0x812);
        }

        if (useDLCFolder)
        {
            const std::wstring name = getDLCFolderName(contentType);
            if (contentType == 0x207 || contentType == 0x216 || contentType == 0x804 ||
                (contentType >= 0x222 && contentType < 0x227) || contentType == 0x22c || contentType == 0x230 ||
                contentType == 0x233)
                path = std::wstring(L"WiiU\\CU\\DLC\\") + name;
            else
                path = std::wstring(L"WiiU\\DLC\\") + name;
        }

        File candidateFile(path);
        if (!candidateFile.exists())
            path = suffix;
    }
    return path + (mounted ? std::wstring(L"\\Data\\") : std::wstring(L"\\"));
}
std::wstring CMinecraftApp::getFilePath(unsigned int contentType, std::wstring name, bool mounted, std::wstring suffix)
{
    const std::wstring primary = getRootPath(contentType, true, mounted, suffix) + name;
    File primaryFile(primary);
    if (primaryFile.exists())
        return primary;
    return getRootPath(contentType, false, true, suffix) + name;
}
uint64_t CMinecraftApp::HandleDLC(DLCPack* pack)
{
    unsigned int numItemsAdded = 0;
    std::vector<std::string> files;
    gStorage->GetMountedDLCFileList("DLCDrive", files);
    if (!files.empty())
    {
        for (std::vector<std::string>::const_iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt)
        {
            const std::string& filePath = *fileIt;
            File file(convStringToWstring(filePath));
            const std::wstring fileName = file.getName();
            const unsigned int packId = mDLCManager.retrievePackIDFromDLCDataFile(filePath);
            DLC_INFO* info = static_cast<CConsoleMinecraftApp*>(this)->GetDLCInfoInstance()->GetDLCInfoForPackID(packId);
            if (info == NULL || info->PackType != 5)
            {
                const unsigned int license = info != NULL ? info->PackID : 0;
                mDLCManager.readDLCDataFile(numItemsAdded, filePath, pack, license, false);
            }
            else
            {
                const std::wstring tpack(L"tpack");
                const std::wstring dataPath = getFilePath(info->PackID, fileName, false, tpack);
                File packFile(dataPath);
                if (!packFile.exists())
                    mDLCManager.readDLCDataFile(numItemsAdded, filePath, pack, info->PackID, false);
                else
                    mDLCManager.readDLCDataFile(numItemsAdded, dataPath, pack, info->PackID, false);
            }
        }
    }

    if (numItemsAdded == 0)
        mDLCManager.removePack(pack);

    return 0;
}
int CMinecraftApp::DLCMountedCallback(void* context, int result, unsigned int contentType, unsigned int index)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    if (contentType != 0)
    {
        ++app->mDLCManager.m_numCorruptPacks;
        ++app->mDLCInstallIndex;
        app->MountNextDLC(result);
        return 0;
    }

    CONTENT_DATA content = *reinterpret_cast<CONTENT_DATA*>(gStorage->GetDLC(app->mDLCInstallIndex));
    const std::wstring folderName = convStringToWstring(std::string(content.name));
    DLCPack* existingPack = app->mDLCManager.getPack(folderName);
    cNintendoDLCInfo* dlcInfo = static_cast<CConsoleMinecraftApp*>(app)->GetDLCInfoInstance();

    if (existingPack != NULL && !existingPack->IsCorrupt())
    {
        const unsigned int licenseMask = dlcInfo->GetDLCPurchased(content.name);
        existingPack->SetDLCMountIndex(app->mDLCInstallIndex);
        existingPack->SetDLCDeviceID(content.state);
        existingPack->updateLicenseMask(licenseMask);
        gStorage->UnmountInstalledDLC(NULL);
    }
    else
    {
        if (existingPack != NULL)
            app->mDLCManager.removePack(existingPack);

        unsigned int licenseMask = dlcInfo->GetDLCPurchased(content.name);
        if (IsLicenseExemptTexturePack(folderName))
            licenseMask = 1U;

        DLCPack* pack = new DLCPack(folderName, licenseMask);
        pack->SetDLCMountIndex(app->mDLCInstallIndex);
        pack->SetDLCDeviceID(content.state);
        app->mDLCManager.addPack(pack);
        app->HandleDLC(pack);
        gStorage->UnmountInstalledDLC(NULL);
    }

    ++app->mDLCInstallIndex;
    app->MountNextDLC(result);
    return 0;
}
void CMinecraftApp::MountNextDLC(int32_t firstArgument)
{
    while (mDLCInstallIndex < mDLCInstallCount)
    {
        const int result = gStorage->MountInstalledDLC(firstArgument, mDLCInstallIndex,
                                                        DLCMountedCallback, this, NULL);
        if (result == 0x3e5)
            return;
        ++mDLCInstallIndex;
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
    static const struct { int id; const wchar_t* tag; } languageEntries[] = {
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
    for (const auto& languageEntry : languageEntries)
    {
        mLocaleToString[languageEntry.id] = languageEntry.tag;
        mStringToLocale[languageEntry.tag] = languageEntry.id;
    }
}
void CMinecraftApp::HandleButtonPresses()
{
}
uint64_t CMinecraftApp::LoadInventoryMenu(int player, boost::shared_ptr<LocalPlayer> localPlayer, bool creative)
{
    LocalPlayerMenuInitData* data = new LocalPlayerMenuInitData();
    data->localPlayer = localPlayer;
    data->creative = creative;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(0xb), data,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint64_t CMinecraftApp::LoadCreativeMenu(int player, boost::shared_ptr<LocalPlayer> localPlayer, bool creative)
{
    LocalPlayerMenuInitData* data = new LocalPlayerMenuInitData();
    data->localPlayer = localPlayer;
    data->creative = creative;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(0x2a), data,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint64_t CMinecraftApp::LoadEnchantingMenu(int player, boost::shared_ptr<Inventory> inventory, int x, int y, int z, Level* level, const std::wstring& name)
{
    EnchantingMenuInitData* data = new EnchantingMenuInitData();
    data->inventory = inventory;
    data->level = level;
    data->x = x;
    data->y = y;
    data->z = z;
    data->playerIndex = player;
    data->name = name;
    data->splitscreen = GetLocalPlayerCount() > 1;

    gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(0x2e), data,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint64_t CMinecraftApp::LoadFurnaceMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container)
{
    InventoryContainerMenuInitData* data = new InventoryContainerMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(8), data,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint64_t CMinecraftApp::LoadBrewingStandMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container)
{
    InventoryContainerMenuInitData* data = new InventoryContainerMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(0x2f), data,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint64_t CMinecraftApp::LoadContainerMenu(int player, boost::shared_ptr<Container> inventory, boost::shared_ptr<Container> container)
{
    ContainerMenuInitData* data = new ContainerMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    EUIScene scene = static_cast<EUIScene>(9);
    if (data->splitscreen && container->getContainerSize() >= 0x1c)
        scene = static_cast<EUIScene>(10);

    gConsoleUIController.NavigateToScene(player, scene, data, static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint64_t CMinecraftApp::LoadTrapMenu(int player, boost::shared_ptr<Container> inventory, boost::shared_ptr<Container> container)
{
    ContainerPairMenuInitData* data = new ContainerPairMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(0xc), data,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint32_t CMinecraftApp::LoadCrafting2x2Menu(int player, boost::shared_ptr<LocalPlayer> localPlayer)
{
    if (GameSettings::GetGameSettings(player, static_cast<eGameSetting>(0x20)) != 0)
        return static_cast<uint32_t>(LoadInventoryMenu(player, localPlayer, false));

    char* data = new char[0x20];
    std::memset(data, 0, 0x20);
    new (data) boost::shared_ptr<LocalPlayer>(localPlayer);
    *reinterpret_cast<int*>(data + 8) = 0;
    *reinterpret_cast<unsigned char*>(data + 12) = GetLocalPlayerCount() > 1 ? 1 : 0;
    *reinterpret_cast<int*>(data + 16) = player;

    return gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(6), data,
                                                static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
}
uint64_t CMinecraftApp::LoadCrafting3x3Menu(int player, boost::shared_ptr<LocalPlayer> localPlayer, int x, int y, int z)
{
    char* data = new char[0x20];
    std::memset(data, 0, 0x20);
    new (data) boost::shared_ptr<LocalPlayer>(localPlayer);

    const bool splitscreen = GetLocalPlayerCount() > 1;
    EUIScene scene;
    if (GameSettings::GetGameSettings(player, static_cast<eGameSetting>(0x20)) == 0)
    {
        *reinterpret_cast<int*>(data + 8) = 1;
        *reinterpret_cast<unsigned char*>(data + 12) = splitscreen ? 1 : 0;
        *reinterpret_cast<int*>(data + 16) = player;
        *reinterpret_cast<int*>(data + 20) = x;
        *reinterpret_cast<int*>(data + 24) = y;
        *reinterpret_cast<int*>(data + 28) = z;
        scene = static_cast<EUIScene>(7);
    }
    else
    {
        *reinterpret_cast<unsigned char*>(data + 8) = splitscreen ? 1 : 0;
        *reinterpret_cast<int*>(data + 12) = player;
        *reinterpret_cast<int*>(data + 16) = x;
        *reinterpret_cast<int*>(data + 20) = y;
        *reinterpret_cast<int*>(data + 24) = z;
        data[28] = 0;
        scene = static_cast<EUIScene>(0x38);
    }

    gConsoleUIController.NavigateToScene(player, scene, data, static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint64_t CMinecraftApp::LoadFireworksMenu(int player, boost::shared_ptr<LocalPlayer> localPlayer, int x, int y, int z)
{
    FireworksMenuInitData* data = new FireworksMenuInitData();
    data->localPlayer = localPlayer;
    data->flag = 1;
    data->player = player;
    data->x = x;
    data->y = y;
    data->z = z;
    data->splitscreen = GetLocalPlayerCount() > 1;

    gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(0x38), data,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint32_t CMinecraftApp::LoadSignEntryMenu(int player, boost::shared_ptr<SignBlockEntity> sign)
{
    SignEntryMenuInitData* data = new SignEntryMenuInitData();
    data->sign = sign;
    data->player = player;

    const uint32_t result = gConsoleUIController.NavigateToScene(
        player, static_cast<EUIScene>(0x1d), data, static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    delete data;
    return result;
}
uint64_t CMinecraftApp::LoadRepairingMenu(int player, boost::shared_ptr<Inventory> inventory, Level* level, int x, int y, int z)
{
    RepairingMenuInitData* data = new RepairingMenuInitData();
    data->inventory = inventory;
    data->level = level;
    data->x = x;
    data->y = y;
    data->z = z;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(0x33), data,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint64_t CMinecraftApp::LoadTradingMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Merchant> merchant, Level* level, const std::wstring& name)
{
    TradingMenuInitData* data = new TradingMenuInitData();
    data->inventory = inventory;
    data->merchant = merchant;
    data->level = level;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(0x32), data,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint32_t CMinecraftApp::LoadShulkerBoxMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container)
{
    boost::shared_ptr<Container> inventoryContainer(inventory, static_cast<Container*>(inventory.get()));
    return static_cast<uint32_t>(LoadContainerMenu(player, inventoryContainer, container));
}
uint64_t CMinecraftApp::LoadHopperMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container)
{
    InventoryContainerMenuInitData* data = new InventoryContainerMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(0x35), data,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint32_t CMinecraftApp::LoadHopperMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<MinecartHopper> hopper)
{
    InventoryContainerMenuInitData* data = new InventoryContainerMenuInitData();
    data->inventory = inventory;
    data->container = boost::dynamic_pointer_cast<Container>(hopper);
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return static_cast<uint32_t>(gConsoleUIController.NavigateToScene(
        player, static_cast<EUIScene>(0x35), data, static_cast<EUILayer>(6), static_cast<EUIGroup>(7)));
}
uint64_t CMinecraftApp::LoadHorseMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container, boost::shared_ptr<AbstractHorse> horse)
{
    HorseMenuInitData* data = new HorseMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->horse = horse;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(0x37), data,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint64_t CMinecraftApp::LoadBeaconMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container)
{
    InventoryContainerMenuInitData* data = new InventoryContainerMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    gConsoleUIController.NavigateToScene(player, static_cast<EUIScene>(0x36), data,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return 0;
}
uint32_t CMinecraftApp::LoadWritingBookMenu(int player, not_null_ptr<ItemInstance> item, bool signing)
{
    WritingBookMenuInitData* data = new WritingBookMenuInitData();
    data->item = item;
    data->player = player;
    data->signing = signing;

    const uint32_t result = gConsoleUIController.NavigateToScene(
        player, static_cast<EUIScene>(0x3d), data, static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    delete data;
    return result;
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
    if (mTutorialModes.find(uid) != mTutorialModes.end())
        return mTutorialModes[uid];
    return mTutorialModes[PlayerUID(0)];
}
uint8_t CMinecraftApp::GetTutorialMode(const boost::shared_ptr<Player>& player)
{
    uint8_t tutorial;
    if (player)
    {
        PlayerUID uid = GetTutorialPlayerUid(player.get());
        if (mTutorialModes.find(uid) != mTutorialModes.end())
        {
            tutorial = mTutorialModes[uid];
            return tutorial;
        }
    }
    tutorial = mTutorialModes[PlayerUID(0)];
    return tutorial;
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
    unsigned int availableSlots = 0;
    for (int player = 0; player < 4; ++player)
    {
        if (g_pInput->IsPadConnected(player) != 0 || gProfile->IsSignedIn(player))
            ++availableSlots;
    }
    if (g_pInput->IsLocalMultiplayerAvailable() != 0 && availableSlots > 1)
        return 1;
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
    const int playerId = gProfile->GetPrimaryPad();
    gConsoleUIController.SetMenuDisplayed(playerId, focussed);
}
uint32_t CMinecraftApp::DebugInputCallback(void* context)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    app->mDebugInputEnabled = !app->mDebugInputEnabled;
    return 0;
}
void CMinecraftApp::SetDebugSequence(const char* sequence)
{
    if (g_pInput != NULL)
        g_pInput->SetDebugSequence(sequence, reinterpret_cast<CInput::InputCallback>(DebugInputCallback), this);
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
    const int userIndex = static_cast<int>(firstArgument);
    if (gProfile->GetLiveConnectionStatus() != 0 && !gProfile->IsSignedInLive(userIndex))
    {
        IUIScene_DLCUpsell* scene =
            reinterpret_cast<IUIScene_DLCUpsell*>(static_cast<uintptr_t>(fourthArgument));
        scene->ShowNotOnlineDialog(userIndex);
    }
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
    if (mPrimaryPlayerUID == PlayerUID(0) || uid == PlayerUID(0))
        return false;
    return gProfile->AreXUIDSEqual(uid, mPrimaryPlayerUID);
}
bool CMinecraftApp::isXuidDeadmau5(PlayerUID uid)
{
    boost::unordered_map<PlayerUID, MOJJANG_DATA*>::const_iterator found = g_mojangData.find(uid);
    if (found != g_mojangData.end() && found->second != NULL && found->second->field0 == 5)
        return true;
    return false;
}
void CMinecraftApp::AddMemoryTextureFile(const std::wstring& path, unsigned char* data, unsigned int size)
{
    EnterCriticalSection(&mMemoryTextureMutex);
    boost::unordered_map<std::wstring, MEMDATA*>::iterator found = mMemoryTextureFiles.find(path);
    if (found == mMemoryTextureFiles.end())
    {
        MEMDATA* entry = new MEMDATA();
        entry->data = data;
        entry->size = size;
        entry->refCount = 1;
        mMemoryTextureFiles[path] = entry;
    }
    else
    {
        MEMDATA* entry = found->second;
        if (entry->size == 0 && size != 0)
        {
            if (entry->data != NULL)
                delete[] entry->data;
            entry->data = data;
            entry->size = size;
        }
        ++entry->refCount;
    }
    LeaveCriticalSection(&mMemoryTextureMutex);
}
void CMinecraftApp::GetMemFileDetails(const std::wstring& firstArgument, unsigned char** secondArgument, unsigned int* thirdArgument)
{
    EnterCriticalSection(&mMemoryTextureMutex);
    boost::unordered_map<std::wstring, MEMDATA*>::iterator found = mMemoryTextureFiles.find(firstArgument);
    if (found != mMemoryTextureFiles.end())
    {
        *secondArgument = found->second->data;
        *thirdArgument = found->second->size;
    }
    LeaveCriticalSection(&mMemoryTextureMutex);
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
    EnterCriticalSection(&mMemoryTPDMutex);
    boost::unordered_map<int, MEMDATA*>::iterator found = mMemoryTPDFiles.find(type);
    if (found != mMemoryTPDFiles.end())
    {
        *data = found->second->data;
        *size = found->second->size;
    }
    LeaveCriticalSection(&mMemoryTPDMutex);
}
bool CMinecraftApp::DefaultCapeExists()
{
    static const wchar_t kSpecialCapePath[] = L"Special_Cape.png";
    const std::wstring path(kSpecialCapePath, wcslen(kSpecialCapePath));
    EnterCriticalSection(&mMemoryTextureMutex);
    const bool exists = mMemoryTextureFiles.find(path) != mMemoryTextureFiles.end();
    LeaveCriticalSection(&mMemoryTextureMutex);
    return exists;
}
uint64_t CMinecraftApp::RemoveLevelFromBannedLevelList(int player, PlayerUID uid, char* levelId)
{
    std::vector<BANNEDLISTDATA*>& bannedList = BannedLevelListAt(this, player);
    for (std::vector<BANNEDLISTDATA*>::iterator it = bannedList.begin(); it != bannedList.end();)
    {
        BANNEDLISTDATA* entry = *it;
        if (entry == NULL)
        {
            ++it;
            continue;
        }

        if (!IsEqualXUID(entry->playerUid, uid) || std::strcmp(entry->levelId, levelId) != 0)
        {
            ++it;
            continue;
        }

        telemetryManagerInstance->RecordUnBanLevel(player);
        it = bannedList.erase(it);
    }

    if (mBanListBuffers[player] != NULL)
    {
        delete[] mBanListBuffers[player];
        mBanListBuffers[player] = NULL;
    }

    if (!bannedList.empty())
    {
        const size_t bufferSize = bannedList.size() * sizeof(BANNEDLISTDATA);
        mBanListBuffers[player] = new unsigned char[bufferSize];
        for (size_t i = 0; i < bannedList.size(); ++i)
            std::memcpy(mBanListBuffers[player] + i * sizeof(BANNEDLISTDATA), bannedList[i], sizeof(BANNEDLISTDATA));
    }

    return 0;
}
void CMinecraftApp::SetUniqueMapName(char* name)
{
    std::memcpy(mUniqueMapName, name, sizeof(mUniqueMapName));
}
uint64_t CMinecraftApp::getArchiveFileSize(const std::wstring& path)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != NULL && minecraft->mTexturePackRepository != NULL)
    {
        TexturePack* selected = minecraft->mTexturePackRepository->getSelected();
        if (selected != NULL && selected->hasValidData())
        {
            ArchiveFile* archive = selected->getArchiveFile();
            if (archive != NULL && archive->hasFile(path))
                return archive->getFileSize(path);
        }
    }
    if (mMediaArchive != NULL)
        return mMediaArchive->getFileSize(path);
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
    if (result == static_cast<C4JStorage::EMessageResult>(4) && gProfile->IsValidPlayer(button))
    {
        if (!gProfile->IsSignedInLive(button))
        {
            int parameter = static_cast<int>(0xb9a1bf4d);
            gConsoleUIController.RequestErrorMessage(
                static_cast<int>(0x8a21894b), static_cast<int>(0xc34c0b77), &parameter, 1,
                static_cast<unsigned int>(button), NULL, NULL, NULL, true);
        }
        else
        {
            DLCPack* pack = Minecraft::GetInstance()->mTexturePackRepository->getSelected()->getDLCPack();
            unsigned int packId = pack->GetPackId();
            if (!gApp.GetCommerceInstance()->OwnsBundleContainingPack(packId))
            {
                int parameter = static_cast<int>(0xb9a1bf4d);
                gConsoleUIController.RequestErrorMessage(
                    static_cast<int>(0x9da04a9e), static_cast<int>(0xcba57cf3), &parameter, 1,
                    static_cast<unsigned int>(button), NULL, NULL, NULL, true);
            }
        }
    }
    return 0;
}
uint32_t CMinecraftApp::tipFiltered(int32_t firstArgument)
{
    return 0;
}
uint32_t CMinecraftApp::GetNextTip(bool firstTipRequest, EMiniGameId secondArgument)
{
    if (s_pendingDlcInstallTip && gProfile->GetSignedInUsersMask() != 0)
    {
        s_pendingDlcInstallTip = false;
        if (DLCInstallProcessCompleted() == 0 ||
            mDLCManager.getPackCount(DLCManager::eDLCType_Skin) == 0)
        {
            return 0x64a70fa4U;
        }
    }

    if (!firstTipRequest)
    {
        mTipRotationIndex = (mTipRotationIndex + 1) % 3;
        return mTipData[mTipRotationIndex == 2 ? 3 : mTipRotationIndex].getNextTip();
    }
    if (secondArgument == eMiniGameId_Battle)
        return mTipData[2].getNextTip();
    if (secondArgument == eMiniGameId_Tumble)
        return mTipData[3].getNextTip();
    return mTipData[4].getNextTip();
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
std::wstring CMinecraftApp::GetActionReplacement(int32_t controllerIndex, uint8_t actionMask)
{
    uint32_t primaryButton = g_pInput->GetGameJoypadMaps(controllerIndex, actionMask);
    if (actionMask >= 0x20 && actionMask < 0x39)
    {
        primaryButton = static_cast<uint32_t>(GameSettings::GetInputButton(
            controllerIndex, false, static_cast<EControllerActions>(actionMask)));
        const uint32_t secondaryButton = static_cast<uint32_t>(GameSettings::GetInputButton(
            controllerIndex, true, static_cast<EControllerActions>(actionMask)));
        if (secondaryButton != 0)
        {
            const std::wstring secondary =
                GetSingleActionReplacement(controllerIndex, secondaryButton);
            const std::wstring primary = GetSingleActionReplacement(controllerIndex, primaryButton);
            wchar_t replacement[256];
            swprintf(replacement, sizeof(replacement) / sizeof(replacement[0]), L"%ls+%ls",
                     secondary.c_str(), primary.c_str());
            return std::wstring(replacement);
        }
    }

    const std::wstring primary = GetSingleActionReplacement(controllerIndex, primaryButton);
    wchar_t replacement[256];
    swprintf(replacement, sizeof(replacement) / sizeof(replacement[0]), L"%ls", primary.c_str());
    return std::wstring(replacement);
}
std::wstring CMinecraftApp::GetVKReplacement(int32_t player, uint32_t key, bool fullSize)
{
    const wchar_t* action = NULL;
    if (key >= 0x5820 && key <= 0x5827)
        action = L"LeftStickNavigate";
    else if (key >= 0x5830 && key <= 0x5837)
        action = L"RightStickButton";
    else
    {
        switch (key)
        {
        case 0x5800: action = g_pInput->IsCircleCrossSwapped() ? L"ActionRight" : L"ActionBottom"; break;
        case 0x5801: action = g_pInput->IsCircleCrossSwapped() ? L"ActionBottom" : L"ActionRight"; break;
        case 0x5802: action = L"ActionLeft"; break;
        case 0x5803: action = L"ActionTop"; break;
        case 0x5804: action = L"ShoulderR"; break;
        case 0x5805: action = L"ShoulderL"; break;
        case 0x5806: action = L"TriggerL"; break;
        case 0x5807: action = L"TriggerR"; break;
        }
    }
    if (action == NULL)
        return std::wstring(L" ") + L10N::GetString(0x211cd1ad);

    const bool small = !fullSize && gApp.GetLocalPlayerCount() > 1;
    const wchar_t* controller = L"";
    if (g_pInput->IsPadDRCController(player) || player == 4)
        controller = L"_CTRL_1";
    else if (g_pInput->IsPadClassicController(player))
        controller = L"_CTRL_2";
    const int imageSize = small ? 20 : 30;
    wchar_t replacement[128];
    swprintf(replacement, sizeof(replacement) / sizeof(replacement[0]),
             L"<img src=\"%ls%ls%ls\" align=\"middle\" height=\"%d\" width=\"%d\"/>",
             action, controller, small ? L"_Small" : L"", imageSize, imageSize);
    return std::wstring(replacement);
}
std::wstring CMinecraftApp::GetIconReplacement(uint32_t firstArgument)
{
    if (firstArgument != 1 && firstArgument != 3 && firstArgument != 4)
        return std::wstring();
    const wchar_t* format = firstArgument == 4
        ? L"<img src=\"Icon_Armor\" align=\"middle\" height=\"%d\" width=\"%d\"/>"
        : L"<img src=\"Icon_Shank\" align=\"middle\" height=\"%d\" width=\"%d\"/>";
    wchar_t replacement[128];
    int screenWidth, screenHeight;
    gConsoleUIController.getRenderDimensions(C4JRender::eViewportType_FullScreen, screenWidth, screenHeight);
    const int size = screenWidth < 1920 ? 22 : 33;
    swprintf(replacement, sizeof(replacement) / sizeof(replacement[0]), format, size, size);
    if (firstArgument == 3)
        return std::wstring(replacement) + replacement + replacement;
    return std::wstring(replacement);
}
std::wstring CMinecraftApp::FormatHTMLString(int player, const std::wstring& value, int shadowColour)
{
    std::wstring result = replaceAll(value, L"{*B*}", L"<br />");
    const wchar_t* titleTokens[] = { L"{*T1*}", L"{*T2*}", L"{*T3*}", L"{*ETB*}", L"{*ETW*}" };
    wchar_t colour[64];
    for (int i = 0; i < 5; ++i)
    {
        swprintf(colour, sizeof(colour) / sizeof(colour[0]),
                 i < 3 ? L"<font color=\"#%08x\">" : L"</font><font color=\"#%08x\">",
                 GetHTMLColour(static_cast<eMinecraftColour>(0x1b1 + i)));
        result = replaceAll(result, titleTokens[i], colour);
    }
    result = replaceAll(result, L"{*EF*}", L"</font>");

    const wchar_t* colourTokens[] = {
        L"{*C0*}", L"{*C1*}", L"{*C2*}", L"{*C3*}",
        L"{*C4*}", L"{*C5*}", L"{*C6*}", L"{*C7*}",
        L"{*C8*}", L"{*C9*}", L"{*CA*}", L"{*CB*}",
        L"{*CC*}", L"{*CD*}", L"{*CE*}", L"{*CF*}"
    };
    for (int i = 0; i < 16; ++i)
    {
        swprintf(colour, sizeof(colour) / sizeof(colour[0]),
                 L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
                 GetHTMLColour(static_cast<eMinecraftColour>(0x191 + i)),
                 static_cast<unsigned int>(shadowColour));
        result = replaceAll(result, colourTokens[i], colour);
    }

    const bool swapSticks = GameSettings::GetGameSettings(player, static_cast<eGameSetting>(9)) != 0;
    result = replaceAll(result, L"{*CONTROLLER_ACTION_MOVE*}", GetActionReplacement(player, swapSticks ? 0x1d : 0x1b));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_LOOK*}", GetActionReplacement(player, swapSticks ? 0x1b : 0x1d));

    struct ControllerToken
    {
        const wchar_t* token;
        unsigned int action;
        bool virtualKey;
    };
    const ControllerToken controllerTokens[] = {
        { L"{*CONTROLLER_MENU_NAVIGATE*}", 0x5823, true },
        { L"{*CONTROLLER_VK_A*}", 0x5800, true },
        { L"{*CONTROLLER_VK_B*}", 0x5801, true },
        { L"{*CONTROLLER_VK_X*}", 0x5802, true },
        { L"{*CONTROLLER_VK_Y*}", 0x5803, true },
        { L"{*CONTROLLER_VK_LB*}", 0x5805, true },
        { L"{*CONTROLLER_VK_RB*}", 0x5804, true },
        { L"{*CONTROLLER_VK_LS*}", 0x5820, true },
        { L"{*CONTROLLER_VK_RS*}", 0x5830, true },
        { L"{*CONTROLLER_VK_LT*}", 0x5806, true },
        { L"{*CONTROLLER_VK_RT*}", 0x5807, true },
        { L"{*CONTROLLER_ACTION_JUMP*}", 0x20, false },
        { L"{*CONTROLLER_ACTION_SNEAK*}", 0x27, false },
        { L"{*CONTROLLER_ACTION_USE*}", 0x21, false },
        { L"{*CONTROLLER_ACTION_ACTION*}", 0x22, false },
        { L"{*CONTROLLER_ACTION_LEFT_SCROLL*}", 0x23, false },
        { L"{*CONTROLLER_ACTION_RIGHT_SCROLL*}", 0x24, false },
        { L"{*CONTROLLER_ACTION_INVENTORY*}", 0x25, false },
        { L"{*CONTROLLER_ACTION_CRAFTING*}", 0x28, false },
        { L"{*CONTROLLER_ACTION_DROP*}", 0x26, false },
        { L"{*CONTROLLER_ACTION_CAMERA*}", 0x29, false },
        { L"{*CONTROLLER_ACTION_MENU_PAGEDOWN*}", 9, false },
        { L"{*CONTROLLER_ACTION_DISMOUNT*}", 0x27, false },
        { L"{*CONTROLLER_ACTION_DPAD_UP*}", 0x2c, false },
        { L"{*CONTROLLER_ACTION_DPAD_DOWN*}", 0x2d, false },
        { L"{*CONTROLLER_ACTION_DPAD_RIGHT*}", 0x2b, false },
        { L"{*CONTROLLER_ACTION_DPAD_LEFT*}", 0x2a, false },
        { L"{*CONTROLLER_ACTION_PICKBLOCK*}", 0x2f, false },
        { L"{*CONTROLLER_ACTION_SPECTATE_PREV*}", 0x30, false },
        { L"{*CONTROLLER_ACTION_SPECTATE_NEXT*}", 0x31, false },
        { L"{*CONTROLLER_ACTION_SPECTATE_NOISE*}", 0x32, false },
        { L"{*CONTROLLER_ACTION_SPECTATE_CANCEL*}", 0x33, false },
        { L"{*CONTROLLER_ACTION_READY*}", 0x34, false },
        { L"{*CONTROLLER_ACTION_VOTESELECT*}", 0x35, false },
        { L"{*CONTROLLER_ACTION_RESTART_SECTION*}", 0x36, false },
        { L"{*CONTROLLER_ACTION_RESTART_RACE*}", 0x37, false },
        { L"{*CONTROLLER_ACTION_LOOK_BEHIND*}", 0x38, false },
        { L"{*CONTROLLER_VK_START*}", 0x5814, true },
    };
    for (const ControllerToken& token : controllerTokens)
    {
        const std::wstring replacement = token.virtualKey
            ? GetVKReplacement(player, token.action, false)
            : GetActionReplacement(player, static_cast<unsigned char>(token.action));
        result = replaceAll(result, token.token, replacement);
    }

    result = replaceAll(result, L"{*ICON_SHANK_01*}", GetIconReplacement(1));
    result = replaceAll(result, L"{*ICON_SHANK_03*}", GetIconReplacement(3));
    result = replaceAll(result, L"{*ICON_ARMOR_01*}", GetIconReplacement(4));
    result = replaceAll(result, L"{*CONTROLLER_VK_BACK*}", L10N::GetString(0x3d497286));
    result = replaceAll(result, L"{*PLATFORM_NAME*}", L10N::GetString(0xed3c4bfb));
    extern unsigned int XGetLanguage();
    const unsigned int language = XGetLanguage();
    if (language == 2 || language == 7 || language == 8)
        result = replaceAll(result, L"&nbsp;", L"");
    return result;
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
    (void)players;

    bool tutorialMode = false;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != NULL && minecraft->isTutorial())
        tutorialMode = true;

    const bool customGame =
        minecraft != NULL && minecraft->GetMiniGame() != NULL && minecraft->GetMiniGame()->IsCustomGame();
    bool allowAchievementRange = true;
    if (stat != NULL && stat->isAchievement())
    {
        const unsigned int achievementId = static_cast<Achievement*>(stat)->getAchievementID();
        if (achievementId > 0x3b &&
            (achievementId < 0x42 || (achievementId > 0x4c && achievementId < 0x54)))
        {
            allowAchievementRange = false;
            if (!customGame)
            {
                if (!CGameNetworkManager::sInstance.IsInPublicJoinableGame() &&
                    minecraft != NULL && minecraft->GetMiniGame() != NULL &&
                    minecraft->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
                {
                    goto hostOptionCheck;
                }
            }
            allowAchievementRange = true;
        }
    }

hostOptionCheck:
    if ((GetGameHostOption(static_cast<eGameHostOption>(10)) != 0 ||
         GetGameHostOption(static_cast<eGameHostOption>(0x12)) != 0 ||
         GetGameHostOption(static_cast<eGameHostOption>(0x11)) != 0 ||
         GetGameHostOption(static_cast<eGameHostOption>(0x10)) != 0 ||
         GetGameHostOption(static_cast<eGameHostOption>(0x17)) != 0 ||
         (GetGameHostOption(static_cast<eGameHostOption>(0x18)) == 0 &&
          (GetGameHostOption(static_cast<eGameHostOption>(0x19)) != 0 ||
           GetGameHostOption(static_cast<eGameHostOption>(0x1a)) == 0)) ||
         (GetGameHostOption(static_cast<eGameHostOption>(0x15)) == 0 &&
          GetGameHostOption(static_cast<eGameHostOption>(0x1e)) == 0 && !tutorialMode)) &&
        (GetGameHostOption(static_cast<eGameHostOption>(0x1f)) == 0 || !allowAchievementRange))
    {
        return 0;
    }

    return 1;
}
void CMinecraftApp::SetBaseSaveName(const std::wstring& name)
{
    if (name.empty())
    {
        mCurrentImageTextData.flags &= ~0x40000000U;
        std::memset(mCurrentImageTextData.values, 0, sizeof(mCurrentImageTextData.values));
        return;
    }
    mCurrentImageTextData.flags |= 0x40000000U;
    std::memset(mCurrentImageTextData.values, 0, sizeof(mCurrentImageTextData.values));
    char* destination = reinterpret_cast<char*>(mCurrentImageTextData.values);
    char* source = CStorage::wstringtochararray(name);
    std::strncpy(destination, source, 0x14);
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
static bool ParseImageTextHexValue(const char* text, unsigned int* value)
{
    std::istringstream stream(text);
    stream >> std::hex >> *value;
    return !stream.fail();
}

void CMinecraftApp::ParseImageTextData(const unsigned char* image, unsigned int size, PngTagData* data)
{
    std::memset(data, 0, sizeof(PngTagData));
    const char* cursor = reinterpret_cast<const char*>(image);
    const char* end = cursor + size;

    while (cursor < end)
    {
        char key[80];
        std::memset(key, 0, sizeof(key));
        char* keyOut = key;
        unsigned int keyLength = 0;
        while (*cursor != '\0' && cursor < end && keyLength < 0x4fU)
        {
            *keyOut++ = *cursor++;
            ++keyLength;
        }
        if (cursor >= end)
            break;
        ++cursor;

        if (std::strcmp(key, "4J_SEED") == 0)
        {
            char seedText[23];
            std::memset(seedText, 0, sizeof(seedText));
            std::strncpy(seedText, cursor, 0x16);
            const char* seedEnd = seedText;
            while (*seedEnd != '\0')
                ++seedEnd;
            const unsigned int seedLength = static_cast<unsigned int>(seedEnd - seedText);
            if (seedLength > 0x16U)
            {
                cursor += 0x17;
            }
            else
            {
                cursor += seedLength + 1;
            }
            data->flags |= 0x80000000U;
            const long long seed = std::strtoll(seedText, NULL, 10);
            data->width = static_cast<unsigned int>(static_cast<unsigned long long>(seed) >> 32);
            data->height = static_cast<unsigned int>(seed);
        }
        else if (std::strcmp(key, "4J_HOSTOPTIONS") == 0)
        {
            char valueText[12];
            std::memset(valueText, 0, sizeof(valueText));
            unsigned int valueLength = 0;
            while (*cursor != '\0' && cursor < end && valueLength < 8U)
            {
                valueText[valueLength++] = *cursor++;
            }
            data->flags |= 0x20000000U;
            ParseImageTextHexValue(valueText, &data->values[5]);
            if (cursor < end)
                ++cursor;
        }
        else if (std::strcmp(key, "4J_TEXTUREPACK") == 0)
        {
            char valueText[12];
            std::memset(valueText, 0, sizeof(valueText));
            unsigned int valueLength = 0;
            while (*cursor != '\0' && cursor < end && valueLength < 8U)
            {
                valueText[valueLength++] = *cursor++;
            }
            data->flags |= 0x10000000U;
            ParseImageTextHexValue(valueText, &data->values[6]);
            if (cursor < end)
                ++cursor;
        }
        else if (std::strcmp(key, "4J_EXTRADATA") == 0)
        {
            char valueText[12];
            std::memset(valueText, 0, sizeof(valueText));
            unsigned int valueLength = 0;
            while (*cursor != '\0' && cursor < end && valueLength < 8U)
            {
                valueText[valueLength++] = *cursor++;
            }
            data->flags |= 0x08000000U;
            ParseImageTextHexValue(valueText, &data->values[7]);
            if (cursor < end)
                ++cursor;
        }
        else if (std::strcmp(key, "4J_EXPLOREDCHUNKS") == 0)
        {
            char valueText[41];
            std::memset(valueText, 0, sizeof(valueText));
            unsigned int valueLength = 0;
            while (*cursor != '\0' && cursor < end && valueLength < 0x28U)
            {
                valueText[valueLength++] = *cursor++;
            }
            data->flags |= 0x04000000U;
            std::sscanf(valueText, "%08x%08x%08x%08x%08x",
                        &data->values[8], &data->values[9], &data->values[10],
                        &data->values[11], &data->values[12]);
            if (cursor < end)
                ++cursor;
        }
        else if (std::strcmp(key, "4J_BASESAVENAME") == 0)
        {
            char valueText[21];
            std::memset(valueText, 0, sizeof(valueText));
            unsigned int valueLength = 0;
            while (*cursor != '\0' && cursor < end && valueLength < 0x14U)
            {
                valueText[valueLength++] = *cursor++;
            }
            data->flags |= 0x40000000U;
            char* destination = reinterpret_cast<char*>(data->values);
            std::strncpy(destination, valueText, 0x14);
            if (cursor < end)
                ++cursor;
        }
        else if (std::strcmp(key, "4J_#LOADS") == 0)
        {
            char valueText[12];
            std::memset(valueText, 0, sizeof(valueText));
            unsigned int valueLength = 0;
            while (*cursor != '\0' && cursor < end && valueLength < 8U)
            {
                valueText[valueLength++] = *cursor++;
            }
            data->flags |= 0x02000000U;
            ParseImageTextHexValue(valueText, &data->dataSize);
            if (cursor < end)
                ++cursor;
        }
        else if (cursor < end)
        {
            ++cursor;
        }
    }
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
std::wstring CMinecraftApp::getEntityName(uint64_t entityIdentifier, int32_t nameMode)
{
    return 0;
}
uint32_t CMinecraftApp::DLCOffersReturned(uint32_t firstArgument, uint32_t secondArgument, uint32_t thirdArgument, int32_t fourthArgument)
{
    CMinecraftApp* app = reinterpret_cast<CMinecraftApp*>(static_cast<uintptr_t>(firstArgument));
    EnterCriticalSection(&app->mDLCRetrieveMutex);
    for (std::vector<_DLCRequest*>::iterator it = app->mDLCDownloadQueue.begin();
         it != app->mDLCDownloadQueue.end(); ++it)
    {
        if ((*it)->mContentId == thirdArgument)
        {
            (*it)->mStatus = 2U;
            app->mDLCOffersRetrievedValue = secondArgument;
            break;
        }
    }
    LeaveCriticalSection(&app->mDLCRetrieveMutex);
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
    const EPlayerModelType modelType = static_cast<EPlayerModelType>(
        Player::GetModelTypeFromAnimBitmask(adjustments.mAnimFlags));
    Minecraft* minecraft = Minecraft::GetInstance();
    PlayerModel* model = NULL;
    if (minecraft != NULL)
    {
        EntityRenderDispatcher* dispatcher = minecraft->getEntityRenderDispatcher();
        if (dispatcher != NULL)
        {
            PlayerRenderer* renderer = dispatcher->getPlayerRenderer(modelType);
            if (renderer != NULL)
                model = renderer->getModel();
        }
    }

    std::vector<ModelPart*>* modelParts = new std::vector<ModelPart*>();
    std::vector<SKIN_BOX*>* skinBoxes = new std::vector<SKIN_BOX*>();

    if (model != NULL && count != 0)
    {
        for (unsigned int index = 0; index < count; ++index)
        {
            SKIN_BOX* box = boxes + index;
            model->SetSkinAdjustments(adjustments);
            ModelPart* part = model->AddOrRetrievePart(box, skinId);
            modelParts->push_back(part);
            skinBoxes->push_back(box);
        }
    }

    EnterCriticalSection(&mAdditionalModelPartsMutex);
    EnterCriticalSection(&mAdditionalSkinBoxesMutex);
    mAdditionalModelParts[skinId] = modelParts;
    mAdditionalSkinBoxes[skinId] = skinBoxes;
    LeaveCriticalSection(&mAdditionalSkinBoxesMutex);
    LeaveCriticalSection(&mAdditionalModelPartsMutex);
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
void CMinecraftApp::getLocale(std::vector<std::wstring>& localeNames)
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
        localeNames.push_back(mLocaleToString[localeIds[index]]);
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
void CMinecraftApp::CaptureScreenshot(int32_t playerNumber)
{
    (void)playerNumber;
}
void CMinecraftApp::RunFrame()
{
    /* The Wii U symbol is an intentional no-op in this build. */
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
}
void CMinecraftApp::SetSkinAdjustments(uint32_t id, _SkinAdjustments value)
{
    EnterCriticalSection(&mSkinAdjustmentsMutex);
    mSkinAdjustments[id] = value;
    LeaveCriticalSection(&mSkinAdjustmentsMutex);
}
_SkinAdjustments CMinecraftApp::GetSkinAdjustments(uint32_t id)
{
    _SkinAdjustments result;
    EnterCriticalSection(&mSkinAdjustmentsMutex);
    boost::unordered_map<unsigned int, _SkinAdjustments>::const_iterator found = mSkinAdjustments.find(id);
    if (found != mSkinAdjustments.end())
        result = found->second;
    LeaveCriticalSection(&mSkinAdjustmentsMutex);
    return result;
}
