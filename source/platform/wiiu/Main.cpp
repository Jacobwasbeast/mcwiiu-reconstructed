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
#include "game/client/audio/SoundEngine.hpp"
#include "game/core/LeaderboardManager.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/cCommerceNintendoWiiU.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiUFileSystemHelpers.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"
#include "platform/wiiu/FJ_WiiUDRC.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/CriticalSection.hpp"

#include <coreinit/messagequeue.h>
#include <coreinit/thread.h>
#include <nn/ac/ac_cpp.h>
#include <nn/erreula/erreula_cpp.h>
#include <cwchar>

extern "C" void OSReport(const char*, ...);
extern "C" void OSBlockThreadsOnExit();
extern "C" int OSResumeThread(OSThread* thread);
extern "C" void OSInitMessageQueue(OSMessageQueue*, OSMessage*, int);
extern "C" int ProcUIInitEx(int (*saveCallback)(void*), void* context);
extern "C" int ProcUIRegisterCallback(int, unsigned int (*callback)(void*), void*, int);
extern "C" int ProcUIProcessMessages(int);
extern "C" void DefineActions();
extern "C" void RegisterAwardsWithProfileManager();
extern "C" void ReleaseCallback();
extern "C" int SaveCallback(void*);
extern "C" void ShutdownAndExit();
extern unsigned int CallbackAcquiredForeground(void*);
extern unsigned int CallbackReleaseForeground(void*);
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
OSFastMutex gMainMutex;
static int s_drcIconDelay = 0x78;
static unsigned char s_trialTimerShown = 1;
static OSThread gMainThread;
static unsigned char gMainThreadStack[0x20000];

#define BOOT_TRACE(step) OSReport("[boot] %s\n", step)

int main()
{
    OSBlockThreadsOnExit();
    BOOT_TRACE("main entered");
    ShutdownManager::Initialise();
    CGameNetworkManager::sInstance.WiiU_SetBootFromFriendList();

    BOOT_TRACE("shutdown+network init");
    OSInitMessageQueue(&gMainMessageQueue, &gMainMessageBuffer, 1);
    InitializeCriticalSection(&gMainMutex);
    C4JThreadImpl::OSCreateThread4J(&gMainThread, altMainThread, 0, 0,
                                    gMainThreadStack + sizeof(gMainThreadStack),
                                    sizeof(gMainThreadStack), 0x11, 0);
    OSResumeThread(&gMainThread);
    C4JThreadImpl::SetMainThreadID();
    Compression::CreateNewThreadStorage();
    BOOT_TRACE("alt-main thread created");
    gRenderer->Initialise();
    g_pInput->Initialise(1, 0x3d, 0x17);

    BOOT_TRACE("renderer initialised");
    DefineActions();
    g_pInput->SetJoypadMapArraySetupComplete();
    g_pInput->SetKeyRepeatRate(0.5f, 0.8f);
    g_pInput->SetDeadzoneAndMovementRange(0, 20000, 0x7fff);
    BOOT_TRACE("input configured");
    ProcUIInitEx(SaveCallback, 0);
    ProcUIRegisterCallback(0, CallbackAcquiredForeground, 0, 100);
    ProcUIRegisterCallback(1, CallbackReleaseForeground, 0, 100);
    ProcUIRegisterCallback(5, CallbackHomebuttonDenied, 0, 100);
    BOOT_TRACE("procui callbacks registered");
    nn::ac::Initialize();
    WiiUInitialise();

    BOOT_TRACE("wiiu initialised");
    gApp.loadMediaArchive();
    BOOT_TRACE("media archive loaded");
    gApp.InitialiseDLCInfo();
    if (gApp.ReadProductCodes() == 0)
        gApp.FatalLoadError(L"ReadProductCodes failed");

    BOOT_TRACE("product codes read");
    int productSku = gApp.GetProductSKU();
    if (productSku == 0)
        gProfile->SetPlayerListTitleID(7);
    else if (productSku == 1)
        gProfile->SetPlayerListTitleID(10);
    else if (productSku == 2)
        gProfile->SetPlayerListTitleID(0);
    BOOT_TRACE("sku resolved");
    L10N::loadStringTable();
    g_pInput->SetCircleCrossSwapped(true);

    BOOT_TRACE("string table loaded");
    int uiWidth = 640;
    int uiHeight = 480;
    if (productSku == 1)
    {
        uiWidth = 1280;
        uiHeight = 720;
    }
    BOOT_TRACE("ui size chosen");
    gConsoleUIController.init(uiWidth, uiHeight);
    BOOT_TRACE("CONSOLE UI INIT DONE");
    gApp.CommerceInit();
    BOOT_TRACE("commerce init done");
    gApp.InitTime();
    BOOT_TRACE("time init done");
    static unsigned int s_profileDefaults[5];
    gProfile->Initialise(L"", L"", 0x22, 5, 4, s_profileDefaults, 0x1e60, &gApp.mGameDefinedDataChangedBitmask);
    BOOT_TRACE("profile initialised");
    gProfile->SetSignInChoicesCallback(CConsoleMinecraftApp::RequestSignInUIChoices);
    gProfile->SetNotificationsCallback(
        reinterpret_cast<CProfile::NotificationsHandler>(PopupToast), 0);
    BOOT_TRACE("profile callbacks set");
    RegisterAwardsWithProfileManager();

    bool noSpaceForSaveQuota = false;
    if (gStorage != 0)
    {
        gStorage->Init(0, L10N::GetString(0xc0b26be6), "savegame.dat", 51000000,
                       CConsoleMinecraftApp::DisplaySavingMessageWiiU, &gApp, &noSpaceForSaveQuota);
        gApp.SetNoSpaceForSaveQuota(noSpaceForSaveQuota);
    }
    gProfile->LoadAllMiiData();
    gProfile->SetSignInChangeCallback(
        (CProfile::SignInChangeHandler)CMinecraftApp::SignInChangeCallback, &gApp);
    gProfile->SetProfileReadErrorCallback(CMinecraftApp::ProfileReadErrorCallback, &gApp);

    arrayWithLength<unsigned char> thumbnail = gApp.getArchiveFile(
        L"DefaultSaveThumbnail64x64.png", false);
    if (gStorage != 0)
    {
        gStorage->InitialiseProfileData(0x22, 5, 4, s_profileDefaults, 0x1e60, &gApp.mGameDefinedDataChangedBitmask);
        gStorage->SetDefaultOptionsCallback(
            CMinecraftApp::DefaultOptionsCallback, &gApp);
        gStorage->SetOptionsDataCallback(
            CMinecraftApp::OptionsDataCallback, &gApp);
        gStorage->SetOldProfileVersionCallback(
            GameSettings::OldProfileVersionCallback, &gApp);
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
        gStorage->SetReadDirFn(ReadDir);
        gStorage->SetCloseDirFn(CloseDir);
        gStorage->SetDLCPackageRoot("DLCDrive");
    }

    BOOT_TRACE("storage configured");
    gProfile->SetUpsellCallback(
        reinterpret_cast<CProfile::UpsellHandler>(CMinecraftApp::UpsellReturnedCallback), &gApp);
    BOOT_TRACE("upsell callback set");
    gApp.SetRichPresenceContext(0, 0);

    BOOT_TRACE("rich presence set");
    Tesselator::CreateNewThreadStorage(0x100000);
    BOOT_TRACE("tesselator storage");
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    phys::Vec2::CreateNewThreadStorage();
    BOOT_TRACE("vec storage");
    IntCache::CreateNewThreadStorage();
    OldChunkStorage::CreateNewThreadStorage();
    BOOT_TRACE("chunk storage");
    Level::enableLightingCache();
    BOOT_TRACE("lighting cache enabled");
    Block::CreateNewThreadStorage();
    FireworksRecipe::CreateNewThreadStorage();
    BOOT_TRACE("fireworks storage");
    LevelChunk::CreateNewThreadStorage();
    BOOT_TRACE("levelchunk storage");
    ElementStorage::staticCtor();
    BOOT_TRACE("elementstorage staticCtor");
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_Main);

    BOOT_TRACE("about to call Minecraft::main");
    Minecraft::main();
    Minecraft* minecraft = Minecraft::GetInstance();
    gApp.InitGameSettings();
    gStorage->ReadFromProfile(0, 0);
    gApp.InitialiseTips();
    g_pLeaderboardManager->OpenSession();
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

        gRenderer->StartFrame();
        gApp.UpdateTime();
        PIXBeginNamedEvent(timingColour, "Input manager tick");
        g_pInput->Tick();
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

        if (gApp.GetDRCOutputEnabled() == 0 && gApp.GetDRCIconEnabled() == 0 &&
            CGameNetworkManager::sInstance.IsInGameplay() &&
            gApp.GetDRCCheckPlayerCountEnabled() != 0 && gApp.GetLocalPlayerCount() == 1)
        {
            if (s_drcIconDelay != 0)
                --s_drcIconDelay;
            if (s_drcIconDelay == 0)
                gApp.SetDRCIconEnabled(true);
        }
        else
        {
            s_drcIconDelay = 0x77;
        }
        g_pLeaderboardManager->Tick();

        if (gApp.GetGameStarted() == 0)
        {
            minecraft->mSoundEngine->tick(NULL, 0.0f);
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

        minecraft->mSoundEngine->playMusicTick();
        gApp.GetCommerceInstance()->tick();
        InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_UITick);
        gConsoleUIController.tick();
        InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_UITick);
        gRenderer->ResetRenderState();
        InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_UIRender);
        gConsoleUIController.render();
        InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_UIRender);
        InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_Frame);
        PIXBeginNamedEvent(timingColour, "Frame present");
        gRenderer->Present();
        PIXEndNamedEvent();
        gRenderer->Set_matrixDirty();
        gConsoleUIController.CheckMenuDisplayed();
        PIXBeginNamedEvent(timingColour, "Profile load check");
        if (gApp.mGameDefinedDataChangedBitmask != 0)
        {
            for (int player = 0; player < 4; ++player)
            {
                if ((gApp.mGameDefinedDataChangedBitmask & (1u << player)) != 0)
                {
                    unsigned char* data = gStorage->GetGameDefinedProfileData(player);
                    GameSettings::ClearGameSettingsChangedFlag(player);
                    GameSettings::ApplyGameSettingsChanged(player);
                    minecraft->mStats[player]->clear();
                    minecraft->mStats[player]->parse(data);
                }
            }
            gApp.mGameDefinedDataChangedBitmask = 0;
        }
        PIXEndNamedEvent();
        PIXBeginNamedEvent(timingColour, "Network manager do work #2");
        CGameNetworkManager::sInstance.DoWork();
        PIXEndNamedEvent();
        gApp.HandleXuiActions();
        if (!gProfile->IsFullVersion())
        {
            if (gApp.GetGameStarted() != 0)
            {
                if (gApp.IsAppPaused() != 0)
                    gApp.UpdateTrialPausedTimer();
                gConsoleUIController.UpdateTrialTimer(gProfile->GetPrimaryPad());
            }
        }
        else if (s_trialTimerShown != 0)
        {
            gConsoleUIController.ShowTrialTimer(false);
            s_trialTimerShown = 0;
        }
        Vec3::resetPool();
    }

    ShutdownAndExit();
    return 0;
}
