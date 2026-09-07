#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
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
CConsoleMinecraftApp gApp;
CMinecraftApp* g_pApp = &gApp;
static Random sTipRandom;

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
    unsigned int first = tips == 0 ? 1 : 0;
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
      mChangingSessionType(false), mReallyChangingSessionType(false), mGameMode(0),
      mLoadSavesFromFolderEnabled(false), mDisconnectReason(0), mGlobalXuiAction(0),
      mLiveLinkRequired(false), mDebugInputEnabled(false), mTrialTimerStart(0.0f),
      mTrialPausedTime(0.0f), mLastPerformanceCounter(0), mAccumulatedPerformanceCounter(0),
      mAppTime(0.0f), mFrameTime(0.0f), mPerformanceCounterScale(0.0f),
      mDLCInstallProcessCompleted(false), mDLCInstallPending(false), mAutosaveTime(0),
      mNewDLCAvailable(false),
      mCorruptSaveDeleted(false), mResetNether(false), mResetEnd(false),
      mRequiredTexturePackID(0), mFirstMiniGameType(static_cast<EMiniGameId>(0))
{
    std::fill(mBanListCheck, mBanListCheck + 4, false);
    std::fill(mOpacityTimer, mOpacityTimer + 4, 0);
    std::fill(mOptionsCallbackStatus, mOptionsCallbackStatus + 4, 0);
}
CMinecraftApp::~CMinecraftApp() {}
void CMinecraftApp::SetGameHostOption(unsigned int& options, eGameHostOption option, unsigned int value)
{
}
void CMinecraftApp::SetGameHostOption(eGameHostOption option, unsigned int value)
{
}
void CMinecraftApp::SetXuiServerAction(int player, eXuiServerAction action, void* param)
{
    mXuiServerActions[player] = action;
    mXuiServerActionParams[player] = param;
}
uint32_t CMinecraftApp::GetGameHostOption(unsigned int options, eGameHostOption option)
{
    return 0;
}
uint32_t CMinecraftApp::GetGameHostOption(eGameHostOption option)
{
    return 0;
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
    return 0;
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
    return 0;
}
bool CMinecraftApp::isValidStateTransition(eXuiAction current, eXuiAction next)
{
    return false;
}
void CMinecraftApp::SetActionInternal(int player, eXuiAction action, void* param)
{
}
void CMinecraftApp::SetAction(int player, eXuiAction action, void* param)
{
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
uint64_t CMinecraftApp::SignInChangeCallback(void* context, bool signedIn, unsigned int userIndex)
{
    return 0;
}
void CMinecraftApp::ProfileReadErrorCallback(void* context)
{
}
uint32_t CMinecraftApp::DefaultOptionsCallback(void* context, C4JStorage::PROFILESETTINGS* settings, int profile)
{
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
}
void CMinecraftApp::InitGameSettings()
{
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
unsigned int CMinecraftApp::SetExtraGameData(unsigned int& options, eGameExtraData type, unsigned int value) const
{
    return 0;
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
uint32_t CMinecraftApp::ExitGameFromRemoteSaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
uint64_t CMinecraftApp::ExitGameFromRemoteSave(void* context)
{
    return 0;
}
uint32_t CMinecraftApp::TexturePackDialogReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
void CMinecraftApp::SetupMiniGameMusic()
{
}
uint32_t CMinecraftApp::GameModeTransitionThreadProc(void* context)
{
    return 0;
}
uint64_t CMinecraftApp::GameModeTransitionComplete(void* context)
{
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
    return 0;
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
uint32_t CMinecraftApp::DLCMountedCallback(void* context, int result, unsigned int contentType, unsigned int index)
{
    return 0;
}
void CMinecraftApp::MountNextDLC(int32_t firstArgument)
{
}
uint32_t CMinecraftApp::DLCInstalledCallback(void* context, int result, int index)
{
    return 0;
}
uint32_t CMinecraftApp::StartInstallDLCProcess(int32_t firstArgument)
{
    return 0;
}
LevelGenerators* CMinecraftApp::getLevelGenerators()
{
    return mGameRuleManager.getLevelGenerators();
}
void CMinecraftApp::ClearTerrainFeaturePosition()
{
}
void CMinecraftApp::LocaleAndLanguageInit()
{
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
    return false;
}
uint32_t CMinecraftApp::IsUUIDLocal(const std::wstring& uuid)
{
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
    return 0;
}
void CMinecraftApp::SetLiveLinkRequired(bool firstArgument)
{
    mLiveLinkRequired = firstArgument;
}
std::wstring CMinecraftApp::GetDebugRootPath()
{
    return 0;
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
    return false;
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
    return 0;
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
}
uint32_t CMinecraftApp::GetTerrainFeaturePosition(_eTerrainFeatureType type, int* x, int* z)
{
    return 0;
}
MOJJANG_DATA* CMinecraftApp::GetMojangDataForXuid(PlayerUID uid)
{
    return 0;
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
    return 0;
}
uint32_t CMinecraftApp::GetExtraGameData(eGameExtraData type)
{
    return 0;
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
}
uint32_t CMinecraftApp::GetPlayerColour(uint8_t firstArgument)
{
    return 0;
}
uint32_t CMinecraftApp::GetPlayerPrivileges(uint8_t firstArgument)
{
    return 0;
}
uint8_t CMinecraftApp::GetPlayerGameType(uint8_t firstArgument)
{
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
    return 0;
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
    return 0;
}
std::vector<SKIN_BOX*>* CMinecraftApp::GetAdditionalSkinBoxes(unsigned int skinId)
{
    return 0;
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
}
uint32_t CMinecraftApp::IsVoiceChatEnabled()
{
    return 0;
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
