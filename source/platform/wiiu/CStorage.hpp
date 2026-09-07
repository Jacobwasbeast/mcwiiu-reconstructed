#ifndef PLATFORM_WIIU_CSTORAGE_HPP
#define PLATFORM_WIIU_CSTORAGE_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/core/CriticalSection.hpp"
#include "platform/wiiu/C4JStorage.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/FFLiFsClient.hpp"

class C4JThreadImpl;
class CProfile;
struct CONTENT_DATA
{
    unsigned int state;
    bool updatePackage;
    unsigned char reserved[7];
    char name[256];
    char path[640];
    int index;
};
struct SAVE_INFO
{
    char filename[32];
    unsigned char title[256];
    unsigned int timeHigh;
    unsigned int timeLow;
    unsigned char* thumbnail;
    unsigned int thumbnailSize;
    unsigned int nandSize;
    unsigned int reserved;
};

struct SAVE_DETAILS
{
    int count;
    int nandSize;
    SAVE_INFO* saves;
    int reserved;
};
struct FSStat
{
    unsigned int flags;
    unsigned int mode;
    unsigned int owner;
    unsigned int group;
    unsigned int size;
    unsigned int allocatedSize;
    unsigned int quotaSizeHigh;
    unsigned int quotaSizeLow;
    unsigned int entryId;
    unsigned int createdTimeHigh;
    unsigned int createdTimeLow;
    unsigned int modifiedTimeHigh;
    unsigned int modifiedTimeLow;
    unsigned char attributes[48];
};

struct FSDirEntry
{
    FSStat stat;
    char name[256];
};

class CStorage
{
public:
    typedef int (*SavingMessageCallback)(void*, C4JStorage::ESavingMessage, int);
    typedef void (*IncompleteSaveCallback)(void*, C4JStorage::ESaveIncompleteType);
    typedef int (*DefaultOptionsCallback)(void*, C4JStorage::PROFILESETTINGS*, int);
    typedef int (*OptionsDataCallback)(void*, int, unsigned short, C4JStorage::eOptionsCallback);
    typedef int (*OldProfileVersionCallback)(void*, unsigned char*, unsigned short, int);
    typedef int (*GenericDataCallback)(void*, int, C4JStorage::eGeneralData_Callback_Status, int);
    typedef int (*SaveCallback)(void*, bool);
    typedef int (*LoadCallback)(void*, bool, bool);
    typedef int (*SavesInfoCallback)(void*, SAVE_DETAILS*, bool);
    typedef int (*RenameCallback)(void*, bool, C4JStorage::ESaveGameState);
    typedef int (*CopyCallback)(void*, bool, C4JStorage::ESaveGameState);
    typedef bool (*CopyProgressCallback)(void*, int);
    typedef int (*DirectoryOpenCallback)(const char*, int*);
    typedef int (*DirectoryReadCallback)(int*, FSDirEntry*);
    typedef int (*DirectoryCloseCallback)(int*);
    typedef int (*DLCStatusCallback)(void*, int, unsigned int);
    typedef int (*InstalledDLCCallback)(void*, int, int);
    typedef int (*DLCMountCallback)(void*, int, unsigned int, unsigned int);
    typedef void (*DLCUpgradeCallback)(void*, bool);

    struct DriveMapping
    {
        std::string drive;
        std::string path;

        DriveMapping();
        DriveMapping(const std::string&, const std::string&);
    };

    CStorage();
    ~CStorage();

    virtual void Tick();
    virtual void Init(unsigned int, const wchar_t*, const char*, int, SavingMessageCallback, void*, bool*);
    virtual void SetGameSaveFolderTitle(const wchar_t*);
    virtual void SetSaveCacheFolderTitle(const wchar_t*);
    virtual void SetOptionsFolderTitle(const wchar_t*);
    virtual void SetCorruptSaveName(const wchar_t*);
    virtual void SetGameSaveFolderPrefix(const char*);
    virtual void SetMaxSaves(int);
    virtual void SetDefaultImages(unsigned char*, unsigned int, unsigned char*, unsigned int, unsigned char*, unsigned int);
    virtual void SetIncompleteSaveCallback(IncompleteSaveCallback, void*);
    virtual void SetSaveDisabled(bool);
    virtual unsigned char GetSaveDisabled();
    virtual unsigned long long ResetSaveData();
    virtual int DoesSaveExist(bool*);
    virtual int EnoughSpaceForAMinSaveGame();
    virtual int GetSaveState();
    virtual int GetSavesInfo(int, SavesInfoCallback, void*, const char*);
    virtual int ReturnSavesInfo();
    virtual unsigned long long ClearSavesInfo();
    virtual int LoadSaveDataThumbnail(SAVE_INFO*, int (*)(void*, unsigned char*, unsigned int), void*);
    virtual int LoadSaveData(SAVE_INFO*, LoadCallback, void*);
    virtual int GetSaveSize();
    virtual unsigned long long GetSaveData(void*, unsigned int*);
    virtual int GetSaveUniqueNumber(int*);
    virtual int GetSaveUniqueFilename(char*);
    virtual void SetSaveUniqueFilename(char*);
    virtual int GetSaveUniqueFileDir(char*);
    virtual unsigned long long SetSaveTitle(const wchar_t*);
    virtual unsigned long long AllocateSaveData(unsigned int);
    virtual void SetSaveDataSize(unsigned int);
    virtual void GetDefaultSaveImage(unsigned char**, unsigned int*);
    virtual void GetDefaultSaveThumbnail(unsigned char**, unsigned int*);
    virtual unsigned long long SetSaveImages(unsigned char*, unsigned int, unsigned char*, unsigned int, unsigned char*, unsigned int);
    virtual int SaveSaveData(SaveCallback, void*);
    virtual void ContinueIncompleteOperation();
    virtual void CancelIncompleteOperation();
    virtual int DeleteSaveData(SAVE_INFO*, SaveCallback, void*);
    virtual int CopySaveData(SAVE_INFO*, CopyCallback, CopyProgressCallback, void*);
    virtual int RenameSaveData(int, wchar_t*, SaveCallback, void*);
    virtual void GetSaveImage(unsigned char**, int*);
    virtual void GetSaveThumbnail(unsigned char**, int*);
    virtual unsigned long long InitialiseProfileData(unsigned short, unsigned int, unsigned int, unsigned int*, int, unsigned int*);
    virtual int SetDefaultOptionsCallback(DefaultOptionsCallback, void*);
    virtual void SetOptionsDataCallback(OptionsDataCallback, void*);
    virtual int SetOldProfileVersionCallback(OldProfileVersionCallback, void*);
    virtual C4JStorage::PROFILESETTINGS* GetDashboardProfileSettings(int);
    virtual unsigned char* GetGameDefinedProfileData(int);
    virtual void ReadFromProfile(int, int);
    virtual void WriteToProfile(int, bool, bool);
    virtual void DeleteOptionsData(int);
    virtual void ForceQueuedProfileWrites(int);
    virtual int IsOptionsWriteInProgressOrPending(int);
    virtual unsigned long long InitialiseGenericData();
    virtual int ReadFromGenericData(int, const std::wstring&, void**, unsigned int*, GenericDataCallback, void*);
    virtual int WriteToGenericData(int, const std::wstring&, void*, unsigned int, GenericDataCallback, void*);
    virtual void DeleteGenericData(int, const std::wstring&);
    virtual void SetSaveDeviceSelected(unsigned int, bool);
    virtual int GetSaveDeviceSelected(unsigned int);
    virtual void ClearDLCOffers();
    virtual int ReadTMSFile(int, C4JStorage::eGlobalStorage, C4JStorage::eTMS_FileType, wchar_t*, unsigned char**, unsigned int*, int (*)(void*, wchar_t*, int, bool, void*), void*, C4JStorage::eTMS_FileType);
    virtual int WriteTMSFile(int, C4JStorage::eGlobalStorage, wchar_t*, unsigned char*, unsigned int);
    virtual int DeleteTMSFile(int, C4JStorage::eGlobalStorage, wchar_t*);
    virtual int GetDLCOffers(int, DLCStatusCallback, void*, unsigned int);
    virtual void SetOpenDirFn(DirectoryOpenCallback);
    virtual void SetReadDirFn(DirectoryReadCallback);
    virtual void SetCloseDirFn(DirectoryCloseCallback);
    virtual void SetDLCPackageRoot(const char*);
    virtual int GetInstalledDLC(int, InstalledDLCCallback, void*);
    virtual int GetDLC(unsigned int);
    virtual int GetAvailableDLCCount(int);
    virtual int MountInstalledDLC(int, unsigned int, DLCMountCallback, void*, const char*);
    virtual int UnmountInstalledDLC(const char*);
    virtual int GetMountedDLCFileList(const char*, std::vector<std::string>&);
    virtual int GetMountedDLCIndex();
    virtual std::string GetMountedPath(std::string);
    virtual void SetDLCProductCode(const char*);
    virtual void SetProductUpgradeKey(const char*);
    virtual int CheckForTrialUpgradeKey(DLCUpgradeCallback, void*);

    void DisplaySavingMessages();
    void Init(SavingMessageCallback, void*);
    void MakeCRCTable();
    unsigned int UpdateCRC(unsigned int, unsigned char*, int);
    unsigned int CRC(unsigned char*, int);
    void DebugPrintf(const char*, ...);
    void ClearProfileWriteTimer(int);
    void SetOptionsIcon(unsigned char*, unsigned int);
    unsigned long long SetOptionDefaults(int);
    std::string GetOptionsFilename(int);
    unsigned int SaveOptionsThreadProc(int);
    static int _SaveOptionsThreadProc(int, void*);
    unsigned int LoadOptionsThreadProc(int);
    static int _LoadOptionsThreadProc(int, void*);
    void CheckForProfileUpdatesRequired();
    void ProcessOptionsUpdate();
    void TickOptions();
    int IsOptionsWriteInProgress();
    unsigned int DeleteOptionsThreadProc(int);
    int GetFileNANDSize(unsigned int);
    static int saveInfoSortfn(const void*, const void*);
    unsigned int GetSavesInfoThreadProc();
    static int _GetSavesInfoThreadProc(int, void*);
    unsigned int LoadGameThreadProc();
    static int _LoadGameThreadProc(int, void*);
    void CreateSaveUniqueName();
    unsigned long long AddTextFieldToPNG(unsigned char*, unsigned int, unsigned char*, unsigned int, unsigned int);
    unsigned int SaveGameThreadProc();
    static int _SaveGameThreadProc(int, void*);
    unsigned int DeleteGameThreadProc();
    static int _DeleteGameThreadProc(int, void*);
    unsigned int CopyGameThreadProc();
    static int _CopyGameThreadProc(int, void*);
    unsigned int RenameGameThreadProc();
    static int _RenameGameThreadProc(int, void*);
    void InitSaveGame(unsigned int, const wchar_t*, const char*, int, bool*);
    void SetDefaultImages(unsigned char*, unsigned int, unsigned char*, unsigned int);
    void TickSaveGame();
    void AddInstalled(CONTENT_DATA&);
    unsigned int EnumerateLocalDLCPackages();
    void MountInstalledDLC_thread();
    static int MountInstalledDLC_threadEntry(int, void*);
    void DLCOnCreate();
    void TickDLC();
    static char* wstringtochararray(const std::wstring&);
    unsigned int LoadGenericDataThreadProc(int);
    static int _LoadGenericDataThreadProc(int, void*);
    unsigned int SaveGenericDataThreadProc(int);
    static int _SaveGenericDataThreadProc(int, void*);
    void GenericDataTick();
    unsigned int GetOptionsIconSize();
    unsigned char* GetOptionsIconData();
    void InitialiseProfile(int);
    static unsigned int _DeleteOptionsThreadProc(int, void*);
    int ReadEntitlements();
    int DLCInit();
    int GetOffers(int, DLCStatusCallback, void*, unsigned int);
    void ClearOffers();
    const char* FindDriveMapping(const char*);
    void SetBootTypeDisc(bool);
    void CancelGetOffers();
    unsigned int DeleteGenericDataThreadProc(int);
    static unsigned int _DeleteGenericDataThreadProc(int, void*);

private:
    struct StateChangeNotification
    {
        void (*callback)(FSClient*, int, void*);
        void* context;
        unsigned int reserved;
    };

    unsigned int m_crcTable[256];
    bool m_crcReady;
    SavingMessageCallback m_savingMessageCallback;
    void* m_savingMessageContext;
    StateChangeNotification m_saveStateChangeNotification;
    FSClient m_saveClient;
    unsigned char m_saveCommandBlock[0xa80];
    unsigned char m_saveDirectoryCommandBlock[0xa80];
    OSFastMutex m_storageMutex;
    OSThread m_saveThreads[6];
    unsigned int m_saveState;
    unsigned int m_previousSaveState;
    unsigned int m_savesInfoState;
    bool m_savesInfoAvailable;
    SAVE_DETAILS* m_savesInfo;
    SAVE_INFO* m_selectedSave;
    char m_saveUniqueFilename[128];
    wchar_t m_saveTitle[128];
    wchar_t m_gameSaveFolderTitle[128];
    wchar_t m_saveCacheFolderTitle[128];
    wchar_t m_optionsFolderTitle[128];
    wchar_t m_corruptSaveName[128];
    IncompleteSaveCallback m_incompleteSaveCallback;
    void* m_incompleteSaveContext;
    SavesInfoCallback m_savesInfoCallback;
    void* m_savesInfoContext;
    SaveCallback m_saveCallback;
    void* m_saveCallbackContext;
    LoadCallback m_loadCallback;
    void* m_loadCallbackContext;
    SaveCallback m_deleteCallback;
    void* m_deleteCallbackContext;
    SaveCallback m_renameCallback;
    void* m_renameCallbackContext;
    CopyCallback m_copyCallback;
    CopyProgressCallback m_copyProgressCallback;
    void* m_copyCallbackContext;
    wchar_t m_renameTitle[128];
    int m_renameSaveIndex;
    char m_gameSaveFolderPrefix[12];
    int m_maxSaves;
    unsigned char* m_saveData;
    unsigned int m_saveSize;
    unsigned char* m_saveImage;
    unsigned int m_saveImageSize;
    unsigned char* m_saveThumbnail;
    unsigned int m_saveThumbnailSize;
    bool m_saveDisabled;
    unsigned char* m_defaultImage;
    unsigned int m_defaultImageSize;
    unsigned char* m_defaultThumbnail;
    unsigned int m_defaultThumbnailSize;
    std::vector<int> m_profileWrites;
    unsigned int m_incompleteReason;
    unsigned int m_incompleteOperationData;
    void* m_incompleteOperationContext;
    OSThread m_profileSaveThreads[4];
    OSThread m_profileLoadThreads[4];
    OSThread m_profileDeleteThreads[4];
    unsigned char* m_profileReadBuffers[4];
    unsigned char* m_profileBuffers[4];
    unsigned int m_profileReserved0;
    bool m_profileInitialised;
    unsigned short m_profileVersion;
    unsigned int m_profileCount;
    unsigned int m_optionCount;
    bool m_profileUpdateRequired;
    unsigned char* m_optionsIcon;
    unsigned int m_optionsIconSize;
    unsigned int* m_optionDefaults;
    struct ProfileWriteState
    {
        int state;
        unsigned int value;
        unsigned int timer;
    } m_profileWriteState[4];
    struct ProfileReadState
    {
        int state;
        unsigned int value;
        unsigned int reserved;
    } m_profileReadState[4];
    C4JStorage::PROFILESETTINGS m_dashboardProfileSettings[4];
    unsigned int* m_gameProfileSizes;
    unsigned int m_gameProfileStride;
    unsigned char* m_gameProfileData;
    int m_profileResult;
    unsigned int m_profileReserved;
    DefaultOptionsCallback m_defaultOptionsCallback;
    void* m_defaultOptionsContext;
    OptionsDataCallback m_optionsDataCallback;
    void* m_optionsDataContext;
    OldProfileVersionCallback m_oldProfileVersionCallback;
    void* m_oldProfileVersionContext;
    FSClient m_profileClient;
    unsigned char m_profileCommandBlock[0xa80];
    int m_genericState;
    int m_genericSlot;
    unsigned int m_genericDataSize;
    void* m_genericWriteData;
    void* m_genericLoadedData;
    unsigned int m_genericLoadedSize;
    unsigned int* m_genericReadSize;
    void** m_genericReadData;
    std::wstring m_genericFilename;
    unsigned int m_genericThreadReserved[2];
    OSThread m_genericThread;
    FSClient m_genericClient;
    unsigned char m_genericCommandBlock[0xa80];
    GenericDataCallback m_genericCallback;
    void* m_genericContext;
    int m_dlcResult;
    InstalledDLCCallback m_installedCallback;
    void* m_installedContext;
    unsigned int m_dlcReserved0;
    unsigned int m_dlcReserved1;
    int m_dlcState;
    std::vector<CONTENT_DATA> m_installedContent;
    unsigned int m_dlcReserved2;
    unsigned int m_dlcReserved3;
    int m_dlcMountState;
    DLCMountCallback m_dlcMountCallback;
    void* m_dlcMountContext;
    std::string m_dlcMountName;
    unsigned int m_dlcMountNameReserved[2];
    unsigned int m_mountedDLCIndex;
    unsigned int m_dlcMountResult;
    char m_defaultDLCMountName[40];
    unsigned int m_dlcMountNameData;
    std::vector<DriveMapping> m_mountedDrives;
    unsigned int m_mountedDrivesReserved;
    char m_dlcProductCode[16];
    char m_productUpgradeKey[16];
    unsigned char m_dlcThreadContext[52];
    OSThread m_dlcThread;
    DirectoryOpenCallback m_openDir;
    DirectoryReadCallback m_readDir;
    DirectoryCloseCallback m_closeDir;
    int m_dlcDirectory;
    FSDirEntry m_dlcDirectoryEntry;
    unsigned int m_layoutReserved;
};

#endif
