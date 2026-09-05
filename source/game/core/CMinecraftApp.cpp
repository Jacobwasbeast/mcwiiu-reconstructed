#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/util/Random.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include <algorithm>
#include <cstdlib>
#include <map>

struct _LARGE_INTEGER
{
    long long QuadPart;
};

void QueryPerformanceCounter(_LARGE_INTEGER* value);
void QueryPerformanceFrequency(_LARGE_INTEGER* value);

static std::map<unsigned int, _SkinAdjustments> g_skinAdjustments;
CConsoleMinecraftApp gApp;
CMinecraftApp* g_pApp = &gApp;
static Random sTipRandom;
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
    : mLastPerformanceCounter(0), mAccumulatedPerformanceCounter(0), mAppTime(0.0f),
      mFrameTime(0.0f), mPerformanceCounterScale(0.0f)
{
}
CMinecraftApp::~CMinecraftApp() {}
void CMinecraftApp::SetGameHostOption(int* firstArgument, uint32_t secondArgument, uint32_t thirdArgument)
{
}
void CMinecraftApp::SetGameHostOption(uint32_t firstArgument, uint32_t secondArgument)
{
}
void CMinecraftApp::SetXuiServerAction(int32_t firstArgument, uint32_t secondArgument, uint32_t thirdArgument)
{
}
uint32_t CMinecraftApp::GetGameHostOption(uint32_t firstArgument, uint32_t secondArgument)
{
    return 0;
}
uint32_t CMinecraftApp::GetGameHostOption(uint32_t firstArgument)
{
    return 0;
}
uint32_t CMinecraftApp::GetOptionsCallbackStatus(int32_t firstArgument)
{
    return 0;
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
uint32_t CMinecraftApp::IsActionPending(int32_t firstArgument, int32_t secondArgument)
{
    return 0;
}
bool CMinecraftApp::isValidStateTransition(uint32_t firstArgument, uint32_t secondArgument)
{
    return false;
}
void CMinecraftApp::SetActionInternal(int32_t firstArgument, int32_t secondArgument, int32_t thirdArgument)
{
}
void CMinecraftApp::SetAction(int32_t firstArgument, uint32_t secondArgument, uint32_t thirdArgument)
{
}
void CMinecraftApp::SetBanListCheck(int32_t firstArgument, bool secondArgument)
{
}
void CMinecraftApp::InvalidateBannedList(int32_t firstArgument)
{
}
void CMinecraftApp::ClearAndResetDLCDownloadQueue()
{
}
void CMinecraftApp::ClearDLCInstalled()
{
}
uint8_t CMinecraftApp::GetLiveLinkRequired()
{
    return 0;
}
uint64_t CMinecraftApp::SignInChangeCallback(int32_t firstArgument, bool secondArgument, uint32_t thirdArgument)
{
    return 0;
}
void CMinecraftApp::ProfileReadErrorCallback(uint32_t firstArgument)
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
    return 0;
}
void CMinecraftApp::SetDefaultHostOptions()
{
}
void CMinecraftApp::InitGameSettings()
{
}
uint64_t CMinecraftApp::UpsellReturnedCallback(uint32_t firstArgument, uint32_t secondArgument, uint32_t thirdArgument, int32_t fourthArgument)
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
}
uint8_t CMinecraftApp::GetReallyChangingSessionType()
{
    return 0;
}
uint32_t CMinecraftApp::GetGlobalXuiAction()
{
    return 0;
}
uint32_t CMinecraftApp::GetTMSAction(int32_t firstArgument)
{
    return 0;
}
uint32_t CMinecraftApp::EthernetDisconnectReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
uint32_t CMinecraftApp::BannedLevelDialogReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
void CMinecraftApp::SetGlobalXuiAction(uint32_t firstArgument)
{
}
uint8_t CMinecraftApp::GetChangingSessionType()
{
    return 0;
}
uint32_t CMinecraftApp::UnlockFullSaveReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
void CMinecraftApp::SetAutosaveTimerTime()
{
}
void CMinecraftApp::SetGameStarted(bool firstArgument)
{
}
uint32_t CMinecraftApp::GetXuiServerAction(int32_t firstArgument)
{
    return 0;
}
uint32_t CMinecraftApp::UnlockFullExitReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
void CMinecraftApp::SetChangingSessionType(bool firstArgument)
{
}
uint32_t CMinecraftApp::GetDisconnectReason()
{
    return 0;
}
void CMinecraftApp::SetReallyChangingSessionType(bool firstArgument)
{
}
uint32_t CMinecraftApp::SignoutExitWorldThreadProc(int32_t firstArgument)
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
uint64_t CMinecraftApp::SetExtraGameData(int* firstArgument, uint32_t secondArgument, uint32_t thirdArgument)
{
    return 0;
}
void CMinecraftApp::SetExtraGameData(uint32_t firstArgument, uint32_t secondArgument)
{
}
void CMinecraftApp::SetTutorialMode(bool firstArgument, int* secondArgument)
{
}
uint32_t CMinecraftApp::GetXuiAction(int32_t firstArgument)
{
    return 0;
}
uint32_t CMinecraftApp::RemoteSaveThreadProc(uint32_t firstArgument)
{
    return 0;
}
uint32_t CMinecraftApp::ExitGameFromRemoteSaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    return 0;
}
uint64_t CMinecraftApp::ExitGameFromRemoteSave(uint32_t firstArgument)
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
uint32_t CMinecraftApp::GameModeTransitionThreadProc(uint32_t firstArgument)
{
    return 0;
}
uint64_t CMinecraftApp::GameModeTransitionComplete(int32_t firstArgument)
{
    return 0;
}
void CMinecraftApp::SetTMSAction(int32_t firstArgument, uint32_t secondArgument)
{
}
uint8_t CMinecraftApp::IsAppPaused()
{
    return 0;
}
void CMinecraftApp::UpdateTrialPausedTimer()
{
}
void CMinecraftApp::setLevelGenerationOptions(LevelGenerationOptions* firstArgument)
{
    mGameRuleManager.setLevelGenerationOptions(firstArgument);
}
uint8_t CMinecraftApp::DLCInstallProcessCompleted()
{
    return 0;
}
std::wstring CMinecraftApp::getDLCFolderName(uint32_t firstArgument)
{
    return 0;
}
void CMinecraftApp::getRootPath(uint32_t firstArgument, uint32_t secondArgument, int32_t thirdArgument, int32_t fourthArgument, uint32_t fifthArgument)
{
}
std::wstring CMinecraftApp::getFilePath(unsigned int firstArgument, std::wstring secondArgument, bool thirdArgument, std::wstring fourthArgument)
{
    return 0;
}
uint64_t CMinecraftApp::HandleDLC(int* firstArgument)
{
    return 0;
}
uint32_t CMinecraftApp::DLCMountedCallback(uint32_t firstArgument, int32_t secondArgument, uint32_t thirdArgument, uint32_t fourthArgument)
{
    return 0;
}
void CMinecraftApp::MountNextDLC(int32_t firstArgument)
{
}
uint32_t CMinecraftApp::DLCInstalledCallback(uint32_t firstArgument, int32_t secondArgument, int32_t thirdArgument)
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
uint64_t CMinecraftApp::LoadInventoryMenu(int32_t firstArgument, uint32_t secondArgument, bool thirdArgument)
{
    return 0;
}
uint64_t CMinecraftApp::LoadCreativeMenu(int32_t firstArgument, uint32_t secondArgument, bool thirdArgument)
{
    return 0;
}
uint64_t CMinecraftApp::LoadEnchantingMenu(int32_t firstArgument, int* secondArgument, int32_t thirdArgument, int32_t fourthArgument, int32_t fifthArgument, int* sixthArgument, int* seventhArgument)
{
    return 0;
}
uint64_t CMinecraftApp::LoadFurnaceMenu(int32_t firstArgument, int* secondArgument, int* thirdArgument)
{
    return 0;
}
uint64_t CMinecraftApp::LoadBrewingStandMenu(int32_t firstArgument, int* secondArgument, int* thirdArgument)
{
    return 0;
}
uint64_t CMinecraftApp::LoadContainerMenu(int32_t firstArgument, int* secondArgument, int* thirdArgument)
{
    return 0;
}
uint64_t CMinecraftApp::LoadTrapMenu(int32_t firstArgument, int* secondArgument, int* thirdArgument)
{
    return 0;
}
uint32_t CMinecraftApp::LoadCrafting2x2Menu(int32_t firstArgument, uint32_t secondArgument)
{
    return 0;
}
uint64_t CMinecraftApp::LoadCrafting3x3Menu(int32_t firstArgument, uint32_t secondArgument, int32_t thirdArgument, int32_t fourthArgument, int32_t fifthArgument)
{
    return 0;
}
uint64_t CMinecraftApp::LoadFireworksMenu(int32_t firstArgument, uint32_t secondArgument, int32_t thirdArgument, int32_t fourthArgument, int32_t fifthArgument)
{
    return 0;
}
uint32_t CMinecraftApp::LoadSignEntryMenu(int32_t firstArgument, uint32_t secondArgument)
{
    return 0;
}
uint64_t CMinecraftApp::LoadRepairingMenu(int32_t firstArgument, int* secondArgument, int* thirdArgument, int32_t fourthArgument, int32_t fifthArgument, int32_t sixthArgument)
{
    return 0;
}
uint64_t CMinecraftApp::LoadTradingMenu(int32_t firstArgument, int* secondArgument, uint32_t thirdArgument, int* fourthArgument, int* fifthArgument)
{
    return 0;
}
uint32_t CMinecraftApp::LoadShulkerBoxMenu(uint32_t firstArgument, uint32_t secondArgument, uint32_t thirdArgument, uint32_t fourthArgument, uint32_t fifthArgument, uint32_t sixthArgument, uint32_t seventhArgument)
{
    return 0;
}
uint64_t CMinecraftApp::LoadHopperMenu(int32_t firstArgument, int* secondArgument, int* thirdArgument)
{
    return 0;
}
uint32_t CMinecraftApp::LoadHopperMenu(int32_t firstArgument, int* secondArgument, uint32_t thirdArgument)
{
    return 0;
}
uint64_t CMinecraftApp::LoadHorseMenu(int32_t firstArgument, int* secondArgument, int* thirdArgument, uint32_t fourthArgument)
{
    return 0;
}
uint64_t CMinecraftApp::LoadBeaconMenu(int32_t firstArgument, int* secondArgument, int* thirdArgument)
{
    return 0;
}
uint32_t CMinecraftApp::LoadWritingBookMenu(int32_t firstArgument, int* secondArgument, bool thirdArgument)
{
    return 0;
}
bool CMinecraftApp::IsConsoleChatCensored()
{
    return false;
}
uint32_t CMinecraftApp::IsUUIDLocal(int* firstArgument)
{
    return 0;
}
uint8_t CMinecraftApp::GetTutorialMode(uint32_t firstArgument, uint32_t secondArgument)
{
    return 0;
}
uint8_t CMinecraftApp::GetTutorialMode(int* firstArgument)
{
    return 0;
}
void CMinecraftApp::SetGameMode(uint32_t firstArgument)
{
}
void CMinecraftApp::StartReloadingTexturePack()
{
}
void CMinecraftApp::StaticCtor()
{
}
uint32_t CMinecraftApp::GetXuiServerActionParam(int32_t firstArgument)
{
    return 0;
}
void CMinecraftApp::SetDisconnectReason(uint32_t firstArgument)
{
}
uint8_t CMinecraftApp::GetLoadSavesFromFolderEnabled()
{
    return 0;
}
uint32_t CMinecraftApp::GetOpacityTimer(int32_t firstArgument)
{
    return 0;
}
void CMinecraftApp::SetOpacityTimer(int32_t firstArgument)
{
}
void CMinecraftApp::TickOpacityTimer(int32_t firstArgument)
{
}
uint32_t CMinecraftApp::IsLocalMultiplayerAvailable()
{
    return 0;
}
uint64_t CMinecraftApp::NotificationsCallback(int32_t firstArgument, uint32_t secondArgument, uint32_t thirdArgument)
{
    return 0;
}
void CMinecraftApp::SetLiveLinkRequired(bool firstArgument)
{
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
    return 0;
}
uint64_t CMinecraftApp::SetDebugUIFocussed(bool firstArgument)
{
    return 0;
}
uint32_t CMinecraftApp::DebugInputCallback(int32_t firstArgument)
{
    return 0;
}
void CMinecraftApp::SetDebugSequence(int* firstArgument)
{
}
uint8_t CMinecraftApp::DLCInstallPending()
{
    return 0;
}
void CMinecraftApp::SetNewDLCAvailable(bool firstArgument)
{
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
bool CMinecraftApp::isXuidNotch(int* firstArgument)
{
    return false;
}
uint32_t CMinecraftApp::isXuidDeadmau5(PlayerUID firstArgument)
{
    return 0;
}
void CMinecraftApp::AddMemoryTextureFile(int* firstArgument, int* secondArgument, uint32_t thirdArgument)
{
}
void CMinecraftApp::GetMemFileDetails(const std::wstring& firstArgument, unsigned char** secondArgument, unsigned int* thirdArgument)
{
}
bool CMinecraftApp::IsFileInMemoryTextures(int* firstArgument)
{
    return false;
}
void CMinecraftApp::GetTPD(int32_t firstArgument, int** secondArgument, int* thirdArgument)
{
}
bool CMinecraftApp::DefaultCapeExists()
{
    return false;
}
uint64_t CMinecraftApp::RemoveLevelFromBannedLevelList(int32_t firstArgument, int* secondArgument, int* thirdArgument)
{
    return 0;
}
void CMinecraftApp::SetUniqueMapName(int* firstArgument)
{
}
uint64_t CMinecraftApp::getArchiveFileSize(int* firstArgument)
{
    return 0;
}
void CMinecraftApp::SetTrialTimerStart()
{
}
double CMinecraftApp::getTrialTimer()
{
    return 0;
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
uint64_t CMinecraftApp::GetHTMLColour(uint32_t firstArgument)
{
    return 0;
}
void CMinecraftApp::GetHTMLColor(uint32_t firstArgument)
{
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
void CMinecraftApp::FormatHTMLString(int32_t firstArgument, int* secondArgument, int32_t thirdArgument)
{
}
double CMinecraftApp::getAppTime()
{
    return mAppTime;
}
void CMinecraftApp::AddTerrainFeaturePosition(int32_t firstArgument, int32_t secondArgument, int32_t thirdArgument)
{
}
uint32_t CMinecraftApp::GetTerrainFeaturePosition(int32_t firstArgument, int* secondArgument, int* thirdArgument)
{
    return 0;
}
uint32_t CMinecraftApp::GetMojangDataForXuid(PlayerUID firstArgument)
{
    return 0;
}
uint8_t CMinecraftApp::GetBanListCheck(int32_t firstArgument)
{
    return 0;
}
bool CMinecraftApp::AutosaveDue()
{
    return false;
}
uint32_t CMinecraftApp::SecondsToAutosave()
{
    return 0;
}
uint32_t CMinecraftApp::GetExtraGameData(int* firstArgument, uint32_t secondArgument)
{
    return 0;
}
uint32_t CMinecraftApp::GetExtraGameData(uint32_t firstArgument)
{
    return 0;
}
void CMinecraftApp::SetResetNether(bool firstArgument)
{
}
uint8_t CMinecraftApp::GetResetNether()
{
    return 0;
}
void CMinecraftApp::SetResetEnd(bool firstArgument)
{
}
uint8_t CMinecraftApp::GetResetEnd()
{
    return 0;
}
uint32_t CMinecraftApp::CanRecordStatsAndAchievements(int* firstArgument, int* secondArgument)
{
    return 0;
}
void CMinecraftApp::SetBaseSaveName(int* firstArgument)
{
}
void CMinecraftApp::SetCurrentImageTextData(int* firstArgument)
{
}
uint64_t CMinecraftApp::CurrentImageTextData(int* firstArgument)
{
    return 0;
}
void CMinecraftApp::ParseImageTextData(int* firstArgument, uint32_t secondArgument, int* thirdArgument)
{
}
uint64_t CMinecraftApp::GetImageTextData(int* firstArgument, uint32_t secondArgument, int* thirdArgument)
{
    return 0;
}
uint32_t CMinecraftApp::CreateImageTextData(int* firstArgument, int* secondArgument)
{
    return 0;
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
}
uint8_t CMinecraftApp::GetCorruptSaveDeleted()
{
    return 0;
}
void CMinecraftApp::SetAdditionalSkinBoxes(uint32_t firstArgument, int32_t secondArgument, uint32_t thirdArgument, int32_t fourthArgument)
{
}
int32_t CMinecraftApp::SetAdditionalSkinBoxes(uint32_t firstArgument, int32_t secondArgument, int* thirdArgument)
{
    return 0;
}
uint32_t CMinecraftApp::GetAdditionalModelParts(unsigned int firstArgument)
{
    return 0;
}
uint32_t CMinecraftApp::GetAdditionalSkinBoxes(uint32_t firstArgument)
{
    return 0;
}
uint32_t CMinecraftApp::getSkinIdFromPath(int* firstArgument)
{
    return 0;
}
uint64_t CMinecraftApp::getSkinPathFromId(int32_t firstArgument, uint32_t secondArgument)
{
    return 0;
}
uint32_t CMinecraftApp::GetTMSGlobalFileListRead()
{
    return 0;
}
uint32_t CMinecraftApp::GetTMSDLCInfoRead()
{
    return 0;
}
uint32_t CMinecraftApp::GetTMSXUIDsFileRead()
{
    return 0;
}
void CMinecraftApp::SetRequiredTexturePackID(uint32_t firstArgument)
{
}
void CMinecraftApp::GetFileFromTPD(uint32_t firstArgument, int* secondArgument, uint32_t thirdArgument, int** fourthArgument, int* fifthArgument)
{
}
void CMinecraftApp::getLocale(int* firstArgument)
{
}
void CMinecraftApp::SetFirstMiniGameType(uint32_t firstArgument)
{
}
uint32_t CMinecraftApp::GetFirstMiniGameType()
{
    return 0;
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
