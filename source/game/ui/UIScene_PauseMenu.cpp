#include "game/ui/UIScene_PauseMenu.hpp"

#include <string>
#include <vector>

#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/L10N.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/DLCPack.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/core/Compression.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"

extern void Sleep(unsigned int milliseconds);
extern CMinecraftApp* g_pApp;
extern ConsoleUIController gConsoleUIController;
extern CConsoleMinecraftApp theApp;
#define gGameNetworkManager CGameNetworkManager::sInstance

void UIScene_PauseMenu::Init()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance() == NULL ? NULL : Minecraft::GetInstance()->GetMiniGame();
    m_buttonList.AddNewMenuButton(L10N::GetString(0xb9b3004c), 0);
    m_buttonList.AddNewMenuButton(L10N::GetString(0x2b1e9f0e), 1);
    if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1f)) == 0)
    {
        if (gGameNetworkManager.IsHost())
        {
            m_buttonList.AddNewMenuButton(L10N::GetString(0xfb98ec1c), 5);
            if (theApp.HasNoSpaceForSaveQuota())
                m_buttonList.EnableItem(5, false, true);
        }
    }
    else if (miniGame != NULL)
    {
        if (miniGame->IsCustomGame())
            m_buttonList.AddNewMenuButton(L10N::GetString(0x66f20471), 8);
        else if (miniGame->GetId() == 1)
            m_buttonList.AddNewMenuButton(L10N::GetString(0xd9c9100d), 7);
        else if (miniGame->GetId() == 2)
            m_buttonList.AddNewMenuButton(L10N::GetString(0x6044b44e), 7);
    }
    m_buttonList.AddNewMenuButton(L10N::GetString(0x2686c8e5), 6);
    if (IsBanEnabled() == 0)
        m_buttonList.EnableItem(6, true, true);
    else
        m_buttonList.EnableItem(6, false, true);
    m_buttonList.SetItemOpacity(6, IsBanEnabled() == 0 ? 1.0f : 0.3f, true);
}

void UIScene_PauseMenu::tick()
{
    UIScene::tick();
    if (gConsoleUIController.GetTopScene(m_resolution, static_cast<EUILayer>(6), static_cast<EUIGroup>(7)) != this)
        return;
    bool enabled = IsBanEnabled() == 0;
    if (m_buttonList.IsEnabled(6) != enabled)
    {
        m_buttonList.EnableItem(6, enabled, true);
        m_buttonList.SetItemOpacity(6, enabled ? 1.0f : 0.3f, true);
    }
}

void UIScene_PauseMenu::updateTooltips()
{
    bool host = gGameNetworkManager.IsHost() && m_resolution == g_pApp->GetPrimaryUserIndex();
    int value = -1;
    if (host && Minecraft::GetInstance() != NULL && Minecraft::GetInstance()->isClientSide())
        value = gConsoleUIController.IsSceneInStack(m_resolution, static_cast<EUIScene>(4)) ? 0x21c0a77d : 0x9eda23db;
    if (gConsoleUIController.IsSceneInStack(m_resolution, static_cast<EUIScene>(4)))
        gConsoleUIController.SetTooltips(m_resolution, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
    else
        gConsoleUIController.SetTooltips(m_resolution, 0x8d6323ed, 0x4f6bcf5e, value, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_PauseMenu::updateComponents()
{
    m_layer->showComponent(m_resolution, static_cast<EUIScene>(0x42), false);
    m_layer->showComponent(m_resolution, static_cast<EUIScene>(0x48), true);
    m_layer->showComponent(m_resolution, static_cast<EUIScene>(0x43), g_pApp->GetLocalPlayerCount() == 1);
}

void UIScene_PauseMenu::handlePreReload() {}

void UIScene_PauseMenu::handleReload()
{
    updateTooltips();
    unsigned int selection = m_buttonList.getCurrentSelection();
    Init();
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
    m_buttonList.setCurrentSelection(selection, false, true);
}

std::wstring UIScene_PauseMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"PauseMenu") : std::wstring(L"PauseMenuSplit");
}

void UIScene_PauseMenu::handleFocusChange(int, int childFocus)
{
    m_buttonList.setCurrentSelection(m_buttonList.getListIndex(childFocus), false, true);
}

void UIScene_PauseMenu::handleInput(int playerId, int action, bool repeat, bool released, bool pressed, bool&)
{
    if (!m_ignoreInput && action > 3)
    {
        gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
        if (action < 6 || action == 0x14)
        {
            if (pressed)
                sendInputToMovie(action, repeat, released, pressed);
        }
        else if (action == 0x15 && pressed)
        {
            gConsoleUIController.PlayUISFX(NULL);
            navigateBack();
        }
    }
}

void UIScene_PauseMenu::handlePress(int, int item)
{
    if (m_ignoreInput)
        return;
    int index = m_buttonList.getListIndex(item);
    if (index == 0) navigateBack();
    else if (index == 1) gConsoleUIController.NavigateToScene(m_resolution, static_cast<EUIScene>(0xf), NULL, static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    else if (index == 2) gConsoleUIController.NavigateToScene(m_resolution, static_cast<EUIScene>(0x16), NULL, static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    else if (index == 4) gConsoleUIController.NavigateToScene(m_resolution, static_cast<EUIScene>(0x51), NULL, static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    else if (index == 5) PerformActionSaveGame();
    else if (index == 6)
        OpenExitGameDialog(m_resolution, reinterpret_cast<void*>(GetCallbackUniqueId()),
                           ExitGameDialogReturned, ExitGameSaveDialogReturned,
                           EOpenExitGameDialog_0);
    else if (index == 7) gConsoleUIController.NavigateToScene(m_resolution, static_cast<EUIScene>(0x14), NULL, static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    else if (index == 8) gConsoleUIController.NavigateToScene(m_resolution, static_cast<EUIScene>(0x3e), (void*)!gGameNetworkManager.IsInPublicJoinableGame(), static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
}

void UIScene_PauseMenu::ShowScene(bool) {}
void UIScene_PauseMenu::SetIgnoreInput(bool ignore) { m_ignoreInput = ignore; }

unsigned int UIScene_PauseMenu::BuyTexturePack_SignInReturned(void*, bool signedIn, int, int)
{
    return signedIn ? 1U : 0U;
}

unsigned int UIScene_PauseMenu::IsBanEnabled()
{
    bool host = gGameNetworkManager.IsHost() && m_resolution == g_pApp->GetPrimaryUserIndex();
    return !gGameNetworkManager.IsLocalGame() && !host &&
           (Minecraft::GetInstance() == NULL || !Minecraft::InMiniGame(static_cast<EMiniGameId>(0xf), true)) ? 1U : 0U;
}

unsigned int UIScene_PauseMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    m_buttonList.setupControl(this, m_movie->getRootNode(), std::string("ButtonList"));
    m_controls.push_back(&m_buttonList);
    return 1;
}

int UIScene_PauseMenu::getSceneType() { return 5; }

UIScene_PauseMenu::UIScene_PauseMenu(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer), m_buttonList(), m_ignoreInput(false)
{
    initialiseMovie();
    if (!Minecraft::GetInstance() || !Minecraft::GetInstance()->isClientSide())
        gConsoleUIController.ShowTrialTimer(false);
    Init();
    doHorizontalResizeCheck();
    gConsoleUIController.HidePressStart();
    if (Minecraft::GetInstance() != NULL && Minecraft::GetInstance()->mGameModes[sceneType] != NULL)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[sceneType]->getTutorial();
        if (tutorial != NULL)
            tutorial->showTutorialPopup(false);
    }
    m_buttonList.setCurrentSelection(0, false, true);
    m_ignoreInput = false;
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_PauseMenu::~UIScene_PauseMenu()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != NULL && minecraft->mGameModes[m_resolution] != NULL)
    {
        Tutorial* tutorial = minecraft->mGameModes[m_resolution]->getTutorial();
        if (tutorial != NULL)
            tutorial->showTutorialPopup(true);
    }
    if (minecraft == NULL || !minecraft->isClientSide())
        gConsoleUIController.ShowTrialTimer(true);
    if (gGameNetworkManager.IsLocalGame())
    {
        if (minecraft != NULL && minecraft->mGameModes[m_resolution] != NULL)
            g_pApp->SetXuiServerAction(m_resolution, static_cast<eXuiServerAction>(5), NULL);
    }
    if (m_layer != NULL)
    {
        m_layer->showComponent(m_resolution, static_cast<EUIScene>(0x42), false);
        m_layer->showComponent(m_resolution, static_cast<EUIScene>(0x48), false);
        m_layer->showComponent(m_resolution, static_cast<EUIScene>(0x43), false);
    }
}

unsigned int UIScene_PauseMenu::SaveWorldThreadProc(void* context)
{
    g_pApp->SetXuiServerAction(g_pApp->GetPrimaryUserIndex(), static_cast<eXuiServerAction>(context != NULL ? 3 : 2), NULL);
    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();
    if (Minecraft::GetInstance() != NULL && Minecraft::GetInstance()->isClientSide())
    {
        g_pApp->SetGameStarted(false);
        while (g_pApp->GetXuiServerAction(g_pApp->GetPrimaryUserIndex()) != 0 && !g_pApp->GetGameStarted())
            Sleep(10);
        if (!g_pApp->GetChangingSessionType())
            g_pApp->SetGameStarted(true);
    }
    return g_pApp->GetChangingSessionType() ? 0x4c7 : 0;
}

int UIScene_PauseMenu::NetworkErrorHandledDuringInvite(void*, int, C4JStorage::EMessageResult)
{
    gGameNetworkManager.WiiU_ClearIsHandlingBootInvite();
    return 0;
}

void UIScene_PauseMenu::_ExitWorld(void*)
{
    if (gGameNetworkManager.IsNetworkThreadRunning())
        gGameNetworkManager.LeaveGame(false);
    g_pApp->SetChangingSessionType(false);
}

unsigned int UIScene_PauseMenu::ExitWorldThreadProc(void* context)
{
    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();
    _ExitWorld(context);
    return 0;
}

void UIScene_PauseMenu::OpenExitGameDialog(
    int player, void* callbackData,
    int (*callback)(void*, int, C4JStorage::EMessageResult),
    int (*saveCallback)(void*, int, C4JStorage::EMessageResult),
    EOpenExitGameDialog action)
{
    if (action != EOpenExitGameDialog_1 && player != g_pApp->GetPrimaryUserIndex())
    {
        g_pApp->SetAction(player, static_cast<eXuiAction>(0xb), NULL);
        return;
    }
    int parameters[3] = { static_cast<int>(0x4fcbc7e7U), static_cast<int>(0xb9a1bf4dU), 0x4940e70d };
    int message = 0xd5e33637;
    unsigned int parameterCount = 2;
    int (*selectedCallback)(void*, int, C4JStorage::EMessageResult) = callback;
    if (!gProfile->IsFullVersion())
    {
        gConsoleUIController.ReduceTrialTimerValue();
        message = 0x59c349d9;
    }
    else if (gGameNetworkManager.IsHost())
    {
        if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1f)) == 0)
        {
            if (gStorage->GetSaveDisabled() || theApp.HasNoSpaceForSaveQuota())
            {
                message = 0x59c349d9;
            }
            else
            {
                message = gGameNetworkManager.GetPlayerCount() > 1 ? 0x54b9ccf4 : 0xc85a78c0;
                parameters[1] = 0x7bf1723b;
                parameterCount = 3;
                selectedCallback = saveCallback;
            }
        }
        else if (gGameNetworkManager.GetPlayerCount() > 1)
        {
            message = 0x094d2642;
        }
    }
    if (action == EOpenExitGameDialog_1)
        message = 0xae8aae0d;
    gConsoleUIController.RequestAlertMessage(0x2686c8e5, message, parameters,
                                              parameterCount, player, selectedCallback,
                                              callbackData, NULL, true);
}

int UIScene_PauseMenu::WarningTrialTexturePackReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(4) && context != NULL)
        gConsoleUIController.RequestErrorMessage(0x8a21894b, 0xc34c0b77, &button, 1,
                                                  gConsoleUIController.GetWinUserIndex(), NULL, NULL, NULL, true);
    return 0;
}

int UIScene_PauseMenu::ExitGameDialogReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    UIScene* scene = gConsoleUIController.GetSceneFromCallbackId((unsigned int)context);
    if (result == static_cast<C4JStorage::EMessageResult>(5) && scene != NULL && scene->getSceneType() == 5)
        g_pApp->SetAction(scene->getSubSceneType(), static_cast<eXuiAction>(1), NULL);
    return 0;
}

int UIScene_PauseMenu::ExitGameAndSaveReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    UIScene* scene = gConsoleUIController.GetSceneFromCallbackId((unsigned int)context);
    if (result == static_cast<C4JStorage::EMessageResult>(5) && scene != NULL)
        g_pApp->SetAction(scene->getSubSceneType(), static_cast<eXuiAction>(1), NULL);
    return 0;
}

int UIScene_PauseMenu::ExitGameDeclineSaveReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(5))
    {
        UIScene* scene = gConsoleUIController.GetSceneFromCallbackId((unsigned int)context);
        if (scene != NULL)
            g_pApp->SetAction(scene->getSubSceneType(), static_cast<eXuiAction>(1), NULL);
    }
    return 0;
}

int UIScene_PauseMenu::ExitGameSaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(5))
    {
        UIScene* scene = gConsoleUIController.GetSceneFromCallbackId((unsigned int)context);
        if (scene != NULL)
            static_cast<UIScene_PauseMenu*>(scene)->PerformActionSaveGame();
    }
    return 0;
}

int UIScene_PauseMenu::SaveGameDialogReturned(void*, int button, C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(5))
        g_pApp->SetAction(button, static_cast<eXuiAction>(1), NULL);
    return 0;
}

int UIScene_PauseMenu::EnableAutosaveDialogReturned(void*, int button, C4JStorage::EMessageResult result)
{
    g_pApp->SetGameHostOption(static_cast<eGameHostOption>(0x16), result != static_cast<C4JStorage::EMessageResult>(5));
    g_pApp->SetAction(button, static_cast<eXuiAction>(1), NULL);
    return 0;
}

int UIScene_PauseMenu::DisableAutosaveDialogReturned(void*, int button, C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(5))
    {
        g_pApp->SetGameHostOption(static_cast<eGameHostOption>(0x16), 1);
        g_pApp->SetAction(button, static_cast<eXuiAction>(1), NULL);
    }
    return 0;
}

void UIScene_PauseMenu::HandleDLCMountingComplete() {}
void UIScene_PauseMenu::HandleDLCInstalled() { g_pApp->StartInstallDLCProcess(m_resolution); }

int UIScene_PauseMenu::UnlockFullSaveReturned(void*, int button, C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(4))
        g_pApp->SetAction(button, static_cast<eXuiAction>(1), NULL);
    return 0;
}

void UIScene_PauseMenu::PerformActionSaveGame()
{
    if (Minecraft::GetInstance() != NULL && !Minecraft::GetInstance()->isClientSide())
    {
        TexturePackRepository* repository = Minecraft::GetInstance()->mTexturePackRepository;
        if (repository != NULL && repository->getSelected() != NULL && repository->getSelected()->getDLCParentPackId() != 0)
            repository->getSelected()->getDLCParentPackId();
    }
    g_pApp->SetAction(m_resolution, static_cast<eXuiAction>(1), NULL);
}

unsigned int UIScene_PauseMenu::SaveGame_SignInReturned(void* context, bool, int, int)
{
    UIScene* scene = gConsoleUIController.GetSceneFromCallbackId((unsigned int)context);
    if (scene != NULL && scene->getSceneType() == 5)
        static_cast<UIScene_PauseMenu*>(scene)->PerformActionSaveGame();
    return 0;
}
