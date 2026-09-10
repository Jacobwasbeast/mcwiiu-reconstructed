#ifndef GAME_CORE_CMINECRAFTAPP_HPP
#define GAME_CORE_CMINECRAFTAPP_HPP
#include <string>
#include <vector>
#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "vendor/_SkinAdjustments.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/PlayerUID.hpp"
unsigned int hash_value(const PlayerUID& uid);
#include "game/minigame/EMiniGameId.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/GameRuleManager.hpp"
#include "game/core/ColourTable.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/C4JStorage.hpp"
#include "game/core/CriticalSection.hpp"
struct NOTIFICATION
{
    unsigned int type;
    unsigned int value;
};

class LevelGenerationOptions;
class LevelChunk;
class Level;
class CConsoleMinecraftApp;
class CStorage;
class LevelGenerators;
class LevelRules;
class LevelRuleset;
class Block;
class Player;
class LocalPlayer;
class Inventory;
class Container;
class Merchant;
class SignBlockEntity;
class MinecartHopper;
class AbstractHorse;
class ItemInstance;
class CommandBlockEntity;
class Stat;
class DLCPack;
class ModelPart;
struct SKIN_BOX;
struct MEMDATA;
struct MOJJANG_DATA;
struct FEATURE_DATA
{
    int type;
    int x;
    int z;
};
enum eGameHostOption
{
    eGameHostOption_0 = 0,
    eGameHostOption_12 = 0x0c,
    eGameHostOption_13 = 0x0d,
    eGameHostOption_14 = 0x0e,
    eGameHostOption_15 = 0x0f,
    eGameHostOption_21 = 0x15,
    eGameHostOption_24 = 0x18,
    eGameHostOption_25 = 0x19,
    eGameHostOption_26 = 0x1a,
    eGameHostOption_27 = 0x1b,
    eGameHostOption_28 = 0x1c,
    eGameHostOption_29 = 0x1d,
    eGameHostOption_30 = 0x1e,
    eGameHostOption_32 = 0x20
};
enum eXuiServerAction { eXuiServerAction_0 = 0 };
enum eXuiAction { eXuiAction_0 = 0 };
enum eGameExtraData { eGameExtraData_0 = 0 };
enum eGameMode { eGameMode_0 = 0 };
enum eTPDFileType { eTPDFileType_0 = 0 };
enum eTMSAction { eTMSAction_Default = 0 };
enum eFileExtensionType { eFileExtensionType_Default = 0 };
enum eTMSFileType { eTMSFileType_Default = 0 };
enum _eTerrainFeatureType { eTerrainFeatureType_0 = 0 };
enum EHTMLFontSize { EHTMLFontSize_0 = 0 };
struct TIPSTRUCT
{
    unsigned int sortValue;
    unsigned int tipId;
};
class CMinecraftApp {
public:
    class PngTagData
    {
    public:
        unsigned int flags;
        unsigned int imageType;
        unsigned int width;
        unsigned int height;
        unsigned int values[13];
        unsigned int dataSize;

        PngTagData();
    };
    class _TipData {
        unsigned int m_index;
        unsigned int m_count;
        TIPSTRUCT* m_tips;
    public:
        _TipData();
        _TipData(TIPSTRUCT* tips, unsigned int count);
        unsigned int getNextTip();
    };
    CMinecraftApp();
    virtual ~CMinecraftApp();
    virtual void CaptureScreenshot(int player);
    virtual void SetRichPresenceContext(int context, int value) = 0;
    virtual void FatalLoadError(const wchar_t* message, ...);
    virtual void StoreLaunchData();
    virtual void ExitGame();
    virtual void RunFrame();
    virtual void CaptureSaveThumbnail() = 0;
    virtual void GetSaveThumbnail(unsigned char** data, unsigned int* size) = 0;
    virtual void ReleaseSaveThumbnail() = 0;
    virtual void GetScreenshot(int player, unsigned char** data, unsigned int* size) = 0;
    virtual void ReadBannedList(int player, eTMSAction action, bool force) = 0;
    virtual uint32_t LoadLocalTMSFile(wchar_t* path) = 0;
    virtual uint32_t LoadLocalTMSFile(wchar_t* path, eFileExtensionType extension) = 0;
    virtual void FreeLocalTMSFiles(eTMSFileType type) = 0;
    virtual uint32_t GetLocalTMSFileIndex(wchar_t* path, bool create, eFileExtensionType extension) = 0;
    virtual uint32_t GetTMSGlobalFileListRead();
    virtual uint32_t GetTMSDLCInfoRead();
    virtual uint32_t GetTMSXUIDsFileRead();
    virtual void GetFileFromTPD(eTPDFileType type, unsigned char* source, unsigned int sourceSize, unsigned char** data, unsigned int* size);
    virtual void TemporaryCreateGameStart() = 0;
    int GetPrimaryUserIndex() { return gProfile->GetPrimaryPad(); }
    void SetGameHostOption(unsigned int& options, eGameHostOption option, unsigned int value);
    void SetGameHostOption(eGameHostOption option, unsigned int value);
    void SetXuiServerAction(int player, eXuiServerAction action, void* param);
    uint32_t GetGameHostOption(unsigned int options, eGameHostOption option);
    uint32_t GetGameHostOption(eGameHostOption option);
    uint32_t GetOptionsCallbackStatus(int32_t firstArgument);
    void UpdateTime();
    uint8_t GetLocalPlayerCount();
    void loadMediaArchive();
    arrayWithLength<unsigned char> getArchiveFile(const std::wstring& firstArgument, bool secondArgument);
    bool hasArchiveFile(const std::wstring& firstArgument, bool secondArgument);
    void InitTime();
    uint32_t IsActionPending(int player, eXuiAction action);
    static bool isValidStateTransition(eXuiAction current, eXuiAction next);
    void SetActionInternal(int player, eXuiAction action, void* param);
    void SetAction(int player, eXuiAction action, void* param);
    void SetBanListCheck(int32_t firstArgument, bool secondArgument);
    void InvalidateBannedList(int32_t firstArgument);
    void ClearAndResetDLCDownloadQueue();
    void ClearDLCInstalled();
    uint8_t GetLiveLinkRequired();
    static uint64_t SignInChangeCallback(void* context, bool signedIn, unsigned int userIndex);
    static void ProfileReadErrorCallback(void* context);
    static uint32_t DefaultOptionsCallback(void* context, C4JStorage::PROFILESETTINGS* settings, int profile);
    static uint32_t OptionsDataCallback(void* context, int profile, unsigned short size, C4JStorage::eOptionsCallback status);
    void EnterSaveNotificationSection();
    void LeaveSaveNotificationSection();
    uint8_t GetGameStarted();
    void SetDefaultHostOptions();
    void InitGameSettings();
    static uint64_t UpsellReturnedCallback(void* context, eUpsellType type, eUpsellResponse response, int userIndex);
    static int TipsSortFunction(const void* firstTip, const void* secondTip);
    void InitialiseTips();
    void SetAppPaused(bool firstArgument);
    uint8_t GetReallyChangingSessionType();
    eXuiAction GetGlobalXuiAction();
    uint32_t GetTMSAction(int32_t firstArgument);
    static uint32_t EthernetDisconnectReturned(void* context, int button, C4JStorage::EMessageResult result);
    static uint32_t BannedLevelDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    void SetGlobalXuiAction(eXuiAction action);
    uint8_t GetChangingSessionType();
    static uint32_t UnlockFullSaveReturned(void* context, int button, C4JStorage::EMessageResult result);
    void SetAutosaveTimerTime();
    void SetGameStarted(bool firstArgument);
    uint32_t GetXuiServerAction(int32_t firstArgument);
    static uint32_t UnlockFullExitReturned(void* context, int button, C4JStorage::EMessageResult result);
    void SetChangingSessionType(bool firstArgument);
    uint32_t GetDisconnectReason();
    void SetReallyChangingSessionType(bool firstArgument);
    static uint32_t SignoutExitWorldThreadProc(void* context);
    static uint32_t TrialOverReturned(void* context, int button, C4JStorage::EMessageResult result);
    static uint32_t UnlockFullInviteReturned(void* context, int button, C4JStorage::EMessageResult result);
    static uint32_t ExitAndJoinFromInvite(void* context, int button, C4JStorage::EMessageResult result);
    static uint32_t ExitAndJoinFromInviteAndSaveReturned(void* context, int button, C4JStorage::EMessageResult result);
    static uint32_t ExitAndJoinFromInviteDeclineSaveReturned(void* context, int button, C4JStorage::EMessageResult result);
    static uint32_t ExitAndJoinFromInviteSaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    void SetExtraGameData(unsigned int& options, eGameExtraData type, unsigned int value) const;
    void SetExtraGameData(eGameExtraData type, unsigned int value);
    void SetTutorialMode(bool tutorial, const boost::shared_ptr<Player>& player);
    uint32_t GetXuiAction(int32_t firstArgument);
    static uint32_t RemoteSaveThreadProc(void* context);
    static int ExitGameFromRemoteSaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    static uint64_t ExitGameFromRemoteSave(void* context);
    static uint32_t TexturePackDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    static void SetupMiniGameMusic();
    static uint32_t GameModeTransitionThreadProc(void* context);
    static uint64_t GameModeTransitionComplete(void* context);
    void SetTMSAction(int32_t firstArgument, uint32_t secondArgument);
    uint8_t IsAppPaused();
    void UpdateTrialPausedTimer();
    void setLevelGenerationOptions(LevelGenerationOptions* firstArgument);
    uint8_t DLCInstallProcessCompleted();
    std::wstring getDLCFolderName(unsigned int dlcId);
    std::wstring getRootPath(unsigned int contentType, bool create, bool mounted, std::wstring suffix);
    std::wstring getFilePath(unsigned int firstArgument, std::wstring secondArgument, bool thirdArgument, std::wstring fourthArgument);
    uint64_t HandleDLC(DLCPack* pack);
    static int DLCMountedCallback(void* context, int result, unsigned int contentType, unsigned int index);
    void MountNextDLC(int32_t firstArgument);
    static int DLCInstalledCallback(void* context, int result, int index);
    uint32_t StartInstallDLCProcess(int32_t firstArgument);
    LevelGenerators* getLevelGenerators();
    void ClearTerrainFeaturePosition();
    void LocaleAndLanguageInit();
    void HandleButtonPresses();
    uint64_t LoadInventoryMenu(int player, boost::shared_ptr<LocalPlayer> localPlayer, bool creative);
    uint64_t LoadCreativeMenu(int player, boost::shared_ptr<LocalPlayer> localPlayer, bool creative);
    uint64_t LoadEnchantingMenu(int player, boost::shared_ptr<Inventory> inventory, int x, int y, int z, Level* level, const std::wstring& name);
    uint64_t LoadFurnaceMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container);
    uint64_t LoadBrewingStandMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container);
    uint64_t LoadContainerMenu(int player, boost::shared_ptr<Container> inventory, boost::shared_ptr<Container> container);
    uint64_t LoadTrapMenu(int player, boost::shared_ptr<Container> inventory, boost::shared_ptr<Container> container);
    uint32_t LoadCrafting2x2Menu(int player, boost::shared_ptr<LocalPlayer> localPlayer);
    uint64_t LoadCrafting3x3Menu(int player, boost::shared_ptr<LocalPlayer> localPlayer, int x, int y, int z);
    uint64_t LoadFireworksMenu(int player, boost::shared_ptr<LocalPlayer> localPlayer, int x, int y, int z);
    uint32_t LoadSignEntryMenu(int player, boost::shared_ptr<SignBlockEntity> sign);
    uint64_t LoadRepairingMenu(int player, boost::shared_ptr<Inventory> inventory, Level* level, int x, int y, int z);
    uint64_t LoadTradingMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Merchant> merchant, Level* level, const std::wstring& name);
    uint32_t LoadShulkerBoxMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container);
    uint64_t LoadHopperMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container);
    uint32_t LoadHopperMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<MinecartHopper> hopper);
    uint64_t LoadHorseMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container, boost::shared_ptr<AbstractHorse> horse);
    uint64_t LoadBeaconMenu(int player, boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container);
    uint32_t LoadWritingBookMenu(int player, not_null_ptr<ItemInstance> item, bool signing);
    bool IsConsoleChatCensored();
    uint32_t IsUUIDLocal(const std::wstring& uuid);
    uint8_t GetTutorialMode(PlayerUID uid);
    uint8_t GetTutorialMode(const boost::shared_ptr<Player>& player);
    void SetGameMode(eGameMode mode);
    void StartReloadingTexturePack();
    static void StaticCtor();
    void* GetXuiServerActionParam(int player);
    void SetDisconnectReason(DisconnectPacket::eDisconnectReason reason);
    uint8_t GetLoadSavesFromFolderEnabled();
    uint32_t GetOpacityTimer(int32_t firstArgument);
    void SetOpacityTimer(int32_t firstArgument);
    void TickOpacityTimer(int32_t firstArgument);
    uint32_t IsLocalMultiplayerAvailable();
    static uint64_t NotificationsCallback(void* context, unsigned int notification, unsigned int value);
    void SetLiveLinkRequired(bool firstArgument);
    std::wstring GetDebugRootPath();
    uint32_t DebugSettingsOn();
    uint32_t DebugArtToolsOn();
    uint32_t DebugDevToolsOn();
    uint8_t DebugUIFocussed();
    void SetDebugUIFocussed(bool focussed);
    static uint32_t DebugInputCallback(void* context);
    void SetDebugSequence(const char* sequence);
    uint8_t DLCInstallPending();
    void SetNewDLCAvailable(bool firstArgument);
    uint64_t UpsellDLC(uint32_t firstArgument, uint32_t secondArgument, uint32_t thirdArgument, uint32_t fourthArgument);
    bool isXuidNotch(PlayerUID uid);
    bool isXuidDeadmau5(PlayerUID uid);
    void AddMemoryTextureFile(const std::wstring& path, unsigned char* data, unsigned int size);
    void GetMemFileDetails(const std::wstring& firstArgument, unsigned char** secondArgument, unsigned int* thirdArgument);
    bool IsFileInMemoryTextures(const std::wstring& path);
    void GetTPD(int type, unsigned char** data, unsigned int* size);
    bool DefaultCapeExists();
    uint64_t RemoveLevelFromBannedLevelList(int player, PlayerUID uid, char* levelId);
    void SetUniqueMapName(char* name);
    uint64_t getArchiveFileSize(const std::wstring& path);
    void SetTrialTimerStart();
    double getTrialTimer();
    static uint32_t WarningTrialTexturePackReturned(void* context, int button, C4JStorage::EMessageResult result);
    static uint32_t tipFiltered(int32_t firstArgument);
    uint32_t GetNextTip(bool firstArgument, EMiniGameId secondArgument);
    unsigned int GetHTMLColour(eMinecraftColour colour);
    unsigned int GetHTMLColor(eMinecraftColour colour);
    std::wstring GetSingleActionReplacement(int32_t firstArgument, uint32_t secondArgument);
    std::wstring GetActionReplacement(int32_t firstArgument, uint8_t secondArgument);
    std::wstring GetVKReplacement(int32_t firstArgument, uint32_t secondArgument, bool thirdArgument);
    std::wstring GetIconReplacement(uint32_t firstArgument);
    std::wstring FormatHTMLString(int player, const std::wstring& value, int flags);
    double getAppTime();
    void AddTerrainFeaturePosition(_eTerrainFeatureType type, int x, int z);
    uint32_t GetTerrainFeaturePosition(_eTerrainFeatureType type, int* x, int* z);
    MOJJANG_DATA* GetMojangDataForXuid(PlayerUID uid);
    uint8_t GetBanListCheck(int32_t firstArgument);
    bool AutosaveDue();
    uint32_t SecondsToAutosave();
    uint32_t GetExtraGameData(unsigned int& options, eGameExtraData type) const;
    uint32_t GetExtraGameData(eGameExtraData type);
    void SetResetNether(bool firstArgument);
    uint8_t GetResetNether();
    void SetResetEnd(bool firstArgument);
    uint8_t GetResetEnd();
    uint32_t CanRecordStatsAndAchievements(Stat* stat, arrayWithLength<unsigned char> players);
    void SetBaseSaveName(const std::wstring& name);
    void SetCurrentImageTextData(const PngTagData* data);
    void CurrentImageTextData(PngTagData* data);
    void ParseImageTextData(const unsigned char* image, unsigned int size, PngTagData* data);
    void GetImageTextData(const unsigned char* image, unsigned int size, PngTagData* data);
    unsigned int CreateImageTextData(unsigned char* image, const PngTagData* data);
    void processSchematics(LevelChunk* firstArgument);
    void processSchematicsLighting(LevelChunk* firstArgument);
    void loadDefaultGameRules();
    LevelRules* getLevelRules();
    LevelRuleset* getGameRuleDefinitions();
    LevelGenerationOptions* getLevelGenerationOptions();
    const wchar_t* GetGameRulesString(const std::wstring& firstArgument);
    bool ShouldCollideWith(Block* firstArgument);
    void UpdatePlayerInfo(uint8_t firstArgument, int16_t secondArgument, uint32_t thirdArgument, uint8_t fourthArgument);
    uint32_t GetPlayerColour(uint8_t firstArgument);
    uint32_t GetPlayerPrivileges(uint8_t firstArgument);
    uint8_t GetPlayerGameType(uint8_t firstArgument);
    std::wstring getEntityName(uint64_t firstArgument, int32_t secondArgument);
    static uint32_t DLCOffersReturned(uint32_t firstArgument, uint32_t secondArgument, uint32_t thirdArgument, int32_t fourthArgument);
    uint32_t RetrieveNextDLCContent();
    void SetCorruptSaveDeleted(bool firstArgument);
    uint8_t GetCorruptSaveDeleted();
    void SetAdditionalSkinBoxes(unsigned int skinId, SKIN_BOX* boxes, unsigned int count, _SkinAdjustments adjustments);
    std::vector<ModelPart*>* SetAdditionalSkinBoxes(unsigned int skinId, std::vector<SKIN_BOX*>* boxes, _SkinAdjustments adjustments);
    std::vector<ModelPart*>* GetAdditionalModelParts(unsigned int firstArgument);
    std::vector<SKIN_BOX*>* GetAdditionalSkinBoxes(unsigned int skinId);
    static unsigned int getSkinIdFromPath(const std::wstring& path);
    static std::wstring getSkinPathFromId(unsigned int id);
    void SetRequiredTexturePackID(uint32_t firstArgument);
    void getLocale(std::vector<std::wstring>& locale);
    void SetFirstMiniGameType(EMiniGameId type);
    EMiniGameId GetFirstMiniGameType();
    void ProcessFakePadValueA();
    uint32_t IsVoiceChatEnabled();
    void HandleXuiActions();
    void SetSkinAdjustments(uint32_t id, _SkinAdjustments value);
    _SkinAdjustments GetSkinAdjustments(uint32_t id);

    static void ClearSignInChangeUsersMask();
    void HandleButtonPresses(int value);
    bool IntroRunning();
    void SetIntroRunning(bool value);
    static void DisplaySavingMessage(void* context, C4JStorage::ESavingMessage message, int value);
    uint32_t LoadCommandBlockMenu(int player, boost::shared_ptr<CommandBlockEntity> commandBlock);
    uint32_t GetGameMode();
    uint32_t GetGlobalXuiServerAction();
    void SetGlobalXuiServerAction(eXuiServerAction value);
    static void SetActionConfirmed(void* context);
    void SetLoadSavesFromFolderEnabled(bool value);
    bool GetWriteSavesToFolderEnabled();
    void SetWriteSavesToFolderEnabled(bool value);
    bool GetMobsDontAttackEnabled();
    void SetMobsDontAttackEnabled(bool value);
    bool GetUseDPadForDebug();
    void SetUseDPadForDebug(bool value);
    bool GetMobsDontTickEnabled();
    void SetMobsDontTickEnabled(bool value);
    bool GetFreezePlayers();
    void SetFreezePlayers(bool value);
    std::wstring toStringOptionsStatus(const C4JStorage::eOptionsCallback& status);
    void SetOptionsCallbackStatus(int profile, C4JStorage::eOptionsCallback status);
    uint32_t GetAdditionalModelParts(int value);
    static void PrimaryPlayerSignedOutReturned(void* context, int button, C4JStorage::EMessageResult result);
    static void MarketplaceCountsCallback(void* context, C4JStorage::DLC_TMS_DETAILS* details, int count);
    void ClearNewDLCAvailable();
    bool GetNewDLCAvailable();
    void DisplayNewDLCTipAgain();
    void DisplayNewDLCTip();
    void RemoveMemoryTextureFile(const std::wstring& value);
    void AddMemoryTPDFile(int p1, unsigned char* p2, unsigned int p3);
    void RemoveMemoryTPDFile(int value);
    bool IsFileInTPD(int value);
    uint32_t GetTPDSize();
    uint32_t GetTPConfigVal(wchar_t* value);
    void AddLevelToBannedLevelList(int p1, PlayerUID p2, char* p3, bool p4);
    bool IsInBannedLevelList(int p1, PlayerUID p2, char* p3);
    std::wstring GetUniqueMapName();
    bool GetResourcesLoaded();
    void SetResourcesLoaded(bool value);
    uint32_t GetNotifications();
    uint32_t GetHTMLFontSize(EHTMLFontSize value);
    bool IsTerrainFeature(int p1, int p2);
    static void RegisterMojangData(wchar_t* p1, PlayerUID p2, wchar_t* p3, wchar_t* p4);
    static void RegisterConfigValues(wchar_t* p1, int p2);
    void RegisterDLCData(char* p1, unsigned int p2);
    uint32_t getDebugGameRuleDefinitions();
    void setDebugGameRuleDefinitions(LevelRuleset* value);
    void AddDLCRequest(int p1, bool p2);
    bool CheckTMSDLCCanStop();
    int GetDLCContentType(int value);
    int Find_eDLCContentType(unsigned int value);
    uint32_t GetDLCOffersCount();
    void DLCContentRetrieved(int value);
    void TickDLCOffersRetrieved();
    void RetrieveNextTMSPPContent();
    void TickTMSPPFilesRetrieved();
    void ClearTMSPPFilesRetrieved();
    void AddTMSPPFileTypeRequest(int p1, bool p2);
    static void TMSPPFileReturned(void* context, int result, int index, C4JStorage::TMSPP_FILEDATA* data, const char* name);
    bool GetBanListRead(int value);
    void SetBanListRead(int p1, bool p2);
    void ClearBanList(int value);
    uint32_t GetRequiredTexturePackID();
    bool IsLocaleKorea();
    uint32_t get_eMCLang(wchar_t* value);
    uint32_t get_xcLang(wchar_t* value);
    void SetTickTMSDLCFiles(bool value);
    void SetFakePadValue(int p1, unsigned char p2);

    static int GetPlayerUIDIdentityField(const PlayerUID& uid);
    static void SetPlayerUIDIdentityField(PlayerUID& uid, int value);
    static void SyncCachedPlayerUID(CMinecraftApp* app, int player);
    static bool HasActiveLocalPlayer(int player);
    static void HandleSignInPlayerSlot(CMinecraftApp* app, unsigned int player, unsigned int primaryPad,
                                       unsigned int usersMask, unsigned int changedMask, bool identityChanged);

private:
    struct _DLCRequest
    {
        unsigned int mContentId;
        unsigned int mStatus;
    };

    struct XuiActionData
    {
        unsigned int action;
        void* param;
    };

    std::vector<std::wstring> m_dlcSkinNames;
public:
    DLCManager mDLCManager;
private:
    PlayerUID mPrimaryPlayerUID;
    bool mDebugUIFocussed;
    bool mGameStarted;
    bool mAppPaused;
    bool mChangingSessionType;
    bool mReallyChangingSessionType;
    unsigned int mGameMode;
    unsigned int mHostOptions;
    bool mLoadSavesFromFolderEnabled;
    unsigned int mDisconnectReason;
    XuiActionData mXuiActions[4];
    std::vector<XuiActionData> mPendingActions[4];
    OSFastMutex mPendingActionsMutex;
    OSFastMutex mActionMutex;
    unsigned int mTMSActions[4];
    unsigned int mGlobalXuiAction;
    unsigned int mXuiServerActions[4];
    void* mXuiServerActionParams[4];
    bool mLiveLinkRequired;
    bool mDebugInputEnabled;
    float mTrialTimerStart;
    float mTrialPausedTime;
    long long mLastPerformanceCounter;
    long long mAccumulatedPerformanceCounter;
    float mAppTime;
    float mFrameTime;
    float mPerformanceCounterScale;
    unsigned int mTipRotationIndex;
    _TipData mTipData[5];
    bool mDLCInstallProcessCompleted;
    bool mDLCInstallPending;
    unsigned int mDLCInstallCount;
    unsigned int mDLCInstallIndex;
    bool mBanListCheck[4];
    unsigned int mAutosaveTime;
    int mOpacityTimer[4];
    bool mNewDLCAvailable;
    PngTagData mCurrentImageTextData;
    GameRuleManager mGameRuleManager;
    std::vector<_DLCRequest*> mDLCDownloadQueue;
    OSFastMutex mDLCRetrieveMutex;
    unsigned int mDLCOffersRetrievedValue;
    OSFastMutex mDLCDownloadQueueMutex;
    bool mCorruptSaveDeleted;
    bool mResetNether;
    bool mResetEnd;
    unsigned int mRequiredTexturePackID;
    unsigned int mOptionsCallbackStatus[4];
    EMiniGameId mFirstMiniGameType;
    unsigned int mFakePadPending[10];
    unsigned char mPlayerIds[8];
    unsigned int mPlayerPrivileges[8];
    unsigned char mPlayerGameTypes[8];
    std::vector<FEATURE_DATA*> mTerrainFeaturePositions;
    OSFastMutex mTerrainFeaturePositionsMutex;
    boost::unordered_map<std::wstring, MEMDATA*> mMemoryTextureFiles;
    boost::unordered_map<int, MEMDATA*> mMemoryTPDFiles;
    OSFastMutex mMemoryTextureMutex;
    OSFastMutex mMemoryTPDMutex;
    boost::unordered_map<unsigned int, std::vector<ModelPart*>*> mAdditionalModelParts;
    boost::unordered_map<unsigned int, std::vector<SKIN_BOX*>*> mAdditionalSkinBoxes;
    OSFastMutex mAdditionalModelPartsMutex;
    OSFastMutex mAdditionalSkinBoxesMutex;
    unsigned int mExtraGameData;
    boost::unordered_map<int, std::wstring> mLocaleToString;
    boost::unordered_map<std::wstring, int> mStringToLocale;
    PlayerUID mCachedSignInUIDs[4];
    std::vector<NOTIFICATION*> mNotifications;
    static unsigned int sPreviousSignInUsersMask;
};
extern CConsoleMinecraftApp gApp;
extern CMinecraftApp* g_pApp;
extern CStorage* gStorage;

uint32_t DeviceFullErrorReturn__FPv(void* context);
#endif
