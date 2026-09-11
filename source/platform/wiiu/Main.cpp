#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/C_AOC.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/InfoBarManager.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/core/StatsCounter.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/world/block/Block.hpp"
#include "game/core/Compression.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"
#include "game/item/FireworksRecipe.hpp"
#include "game/client/render/Textures.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiUFileSystemHelpers.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"
#include "platform/wiiu/FJ_WiiUDRC.hpp"
#include "game/client/render/Renderer.hpp"

#include <coreinit/messagequeue.h>
#include <coreinit/thread.h>
#include <nn/ac/ac_cpp.h>
#include <nn/erreula/erreula_cpp.h>
#include <cwchar>

extern "C" void OSBlockThreadsOnExit();
extern "C" int OSResumeThread(OSThread* thread);
extern "C" void OSInitMessageQueue(OSMessageQueue*, OSMessage*, int);
extern void InitializeCriticalSection(OSFastMutex*);
extern void EnterCriticalSection(OSFastMutex*);
extern void LeaveCriticalSection(OSFastMutex*);
extern "C" int ProcUIInitEx(int (*saveCallback)(void*), void* context);
extern "C" int ProcUIRegisterCallback(int, unsigned int (*callback)(void*), void*, int);
extern "C" int ProcUIProcessMessages(int);
void Sleep(unsigned int);
extern "C" void DefineActions();
extern "C" void RegisterAwardsWithProfileManager();
extern "C" void ReleaseCallback();
extern "C" int SaveCallback(void*);
extern "C" void ShutdownAndExit();
extern "C" void CallbackAcquiredForeground(void*);
extern unsigned int CallbackReleaseForeground__FPv(void*);
extern unsigned int CallbackHomebuttonDenied(void*);
extern bool ReadDir(int*, FSDirEntry*);
extern int altMainThread(int, void*);

extern CInput* g_pInput;
extern CStorage* gStorage;
extern CProfile* gProfile;
extern Renderer* gRenderer;
extern CConsoleMinecraftApp gApp;
extern unsigned char s_altMainPending;
extern unsigned char s_altMainOKToRun;
extern unsigned char s_altMainExit;

namespace
{
struct MainStaticInit
{
    MainStaticInit()
    {
        if (gProfile == 0)
            gProfile = new CProfile();
    }
};
MainStaticInit s_mainStaticInit;
}

unsigned int CallbackHomebuttonDenied(void*)
{
    nn::erreula::HomeNixSignArg arg;
    nn::erreula::AppearHomeNixSign(arg);
    return 0;
}

OSMessageQueue gMainMessageQueue;
OSMessage gMainMessageBuffer;
static OSFastMutex gMainMutex;
static OSThread gMainThread;
static unsigned char gMainThreadStack[0x20000];

int main()
{
    OSBlockThreadsOnExit();
    ShutdownManager::Initialise();
    CGameNetworkManager::sInstance.WiiU_SetBootFromFriendList();

    OSInitMessageQueue(&gMainMessageQueue, &gMainMessageBuffer, 1);
    InitializeCriticalSection(&gMainMutex);
    C4JThreadImpl::OSCreateThread4J(&gMainThread, altMainThread, 0, 0,
                                    gMainThreadStack + sizeof(gMainThreadStack),
                                    sizeof(gMainThreadStack), 0x11, 0);
    OSResumeThread(&gMainThread);
    C4JThreadImpl::SetMainThreadID();
    Compression::CreateNewThreadStorage();
    gRenderer->Initialise();
    g_pInput->SetDeadzoneAndMovementRange(1, 0x3d, 0x17);

    DefineActions();
    g_pInput->SetJoypadStickTriggerMap(0, 0, 0);
    g_pInput->SetKeyRepeatRate(0.5f, 0.8f);
    g_pInput->SetDeadzoneAndMovementRange(0, 20000, 0x7fff);
    g_pInput->SetJoypadMapArraySetupComplete();
    ProcUIInitEx(SaveCallback, 0);
    ProcUIRegisterCallback(0, (unsigned int (*)(void*))CallbackAcquiredForeground, 0, 100);
    ProcUIRegisterCallback(1, CallbackReleaseForeground__FPv, 0, 100);
    ProcUIRegisterCallback(5, CallbackHomebuttonDenied, 0, 100);
    nn::ac::Initialize();
    WiiUInitialise();

    gApp.loadMediaArchive();
    gApp.InitialiseDLCInfo();
    if (gApp.ReadProductCodes() == 0)
        gApp.FatalLoadError(L"ReadProductCodes failed");

    int productSku = gApp.GetProductSKU();
    if (productSku == 0)
        gProfile->SetPlayerListTitleID(7);
    else if (productSku == 1)
        gProfile->SetPlayerListTitleID(10);
    else if (productSku == 2)
        gProfile->SetPlayerListTitleID(0);
    L10N::loadStringTable();
    g_pInput->DestroyKeyboard();

    int uiWidth = 640;
    int uiHeight = 480;
    if (productSku == 1)
    {
        uiWidth = 1280;
        uiHeight = 720;
    }
    gConsoleUIController.init(uiWidth, uiHeight);
    gApp.CommerceInit();
    gApp.InitTime();
    static unsigned int s_profileDefaults[5];
    static unsigned int s_profileGameData[0x1e60 / 4];
    gProfile->Initialise(L"", L"", 0x22, 5, 4, s_profileDefaults, 0x1e60, s_profileGameData);
    gProfile->SetSignInChoicesCallback(CConsoleMinecraftApp::RequestSignInUIChoices);
    gProfile->SetNotificationsCallback(
        reinterpret_cast<CProfile::NotificationsHandler>(PopupToast), 0);
    RegisterAwardsWithProfileManager();

    bool noSpaceForSaveQuota = false;
    if (gStorage != 0)
    {
        gStorage->Init(0, L10N::GetString(0xc0b26be6), "savegame.dat", 51000000,
                       CConsoleMinecraftApp::DisplaySavingMessageWiiU, &gApp, &noSpaceForSaveQuota);
        gApp.SetNoSpaceForSaveQuota(noSpaceForSaveQuota);
    }
    gProfile->SetSignInChangeCallback(
        (CProfile::SignInChangeHandler)CMinecraftApp::SignInChangeCallback, &gApp);
    gProfile->SetProfileReadErrorCallback(CMinecraftApp::ProfileReadErrorCallback, &gApp);

    arrayWithLength<unsigned char> thumbnail = gApp.getArchiveFile(
        L"DefaultSaveThumbnail64x64.png", false);
    if (gStorage != 0)
    {
        gStorage->InitialiseProfileData(0x22, 5, 4, s_profileDefaults, 0x1e60, s_profileGameData);
        gStorage->SetDefaultOptionsCallback(
            (CStorage::DefaultOptionsCallback)CMinecraftApp::DefaultOptionsCallback, &gApp);
        gStorage->SetOptionsDataCallback(
            (CStorage::OptionsDataCallback)CMinecraftApp::OptionsDataCallback, &gApp);
        gStorage->SetOldProfileVersionCallback(
            (CStorage::OldProfileVersionCallback)GameSettings::OldProfileVersionCallback, &gApp);
        gStorage->SetCorruptSaveName(L"");
        gStorage->SetMaxSaves(99);
        gStorage->SetDefaultImages(thumbnail.data, thumbnail.length,
                                   thumbnail.data, thumbnail.length,
                                   thumbnail.data, thumbnail.length);
        gStorage->SetIncompleteSaveCallback(
            CConsoleMinecraftApp::Callback_SaveGameIncomplete, &gApp);
        if (thumbnail.data != 0)
            delete[] thumbnail.data;
        gStorage->SetSaveTitle(L"Default Save");
        gStorage->SetOpenDirFn(OpenDir);
        gStorage->SetReadDirFn((CStorage::DirectoryReadCallback)ReadDir);
        gStorage->SetCloseDirFn(CloseDir);
        gStorage->SetDLCPackageRoot("DLCDrive");
    }

    gProfile->SetUpsellCallback(
        reinterpret_cast<CProfile::UpsellHandler>(CMinecraftApp::UpsellReturnedCallback), &gApp);
    gApp.SetRichPresenceContext(0, 0);

    Tesselator::CreateNewThreadStorage(0x100000);
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    phys::Vec2::CreateNewThreadStorage();
    IntCache::CreateNewThreadStorage();
    OldChunkStorage::CreateNewThreadStorage();
    Level::enableLightingCache();
    Block::CreateNewThreadStorage();
    FireworksRecipe::CreateNewThreadStorage();
    LevelChunk::CreateNewThreadStorage();
    ElementStorage::staticCtor();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_Main);

    Minecraft::main();
    Minecraft* minecraft = Minecraft::GetInstance();
    gApp.InitGameSettings();
    gApp.InitialiseTips();
    CGameNetworkManager::sInstance.Initialise();
    CGameNetworkManager::sInstance.SetLocalGame(true);
    C_AOC::getInstance()->Init();

    float timingColour = 0.0f;
    while (true)
    {
        InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_Frame);
        if (s_altMainPending != 0)
        {
            s_altMainOKToRun = 1;
            Sleep(100);
        }
        EnterCriticalSection(&gMainMutex);
        LeaveCriticalSection(&gMainMutex);
        if (s_altMainExit != 0)
            break;
        int processResult = ProcUIProcessMessages(1);
        if (processResult == 3)
            break;
        if (processResult == 2)
            ReleaseCallback();

        g_pInput->Tick();
        gApp.UpdateTime();
        PIXBeginNamedEvent(timingColour, "Input manager tick");
        PIXEndNamedEvent();
        HandleDRCStatusChanges();
        PIXBeginNamedEvent(timingColour, "Profile manager tick");
        gProfile->Tick();
        PIXEndNamedEvent();
        PIXBeginNamedEvent(timingColour, "Storage manager tick");
        if (gStorage != 0)
            gStorage->Tick();
        PIXEndNamedEvent();
        PIXBeginNamedEvent(timingColour, "Render manager tick");
        gRenderer->Tick();
        PIXEndNamedEvent();
        CGameNetworkManager::sInstance.DoWork();

        if (gApp.GetGameStarted() == 0)
        {
            minecraft->tickAllConnections();
            minecraft->mTextures->tick(true, false, false);
            IntCache::Reset();
            if (gApp.GetReallyChangingSessionType() != 0)
                minecraft->tickAllConnections();
        }
        else
        {
            minecraft->run_middle();
            gApp.SetAppPaused(CGameNetworkManager::sInstance.IsLocalGame() &&
                              CGameNetworkManager::sInstance.GetPlayerCount() == 1 &&
                              gConsoleUIController.IsPauseMenuDisplayed(0));
        }

        gConsoleUIController.tick();
        gRenderer->Tick();
        gConsoleUIController.render();
        gRenderer->Present();
        gConsoleUIController.CheckMenuDisplayed();
        InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_Frame);
        CGameNetworkManager::sInstance.DoWork();
        gApp.HandleXuiActions();
        Vec3::resetPool();
    }

    ShutdownAndExit();
    return 0;
}
