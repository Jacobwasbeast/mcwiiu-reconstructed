#include "game/ui/UIScene_MainMenu.hpp"

#include <cstring>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/core/System.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/UIString.hpp"

extern ConsoleUIController gConsoleUIController;
extern CConsoleMinecraftApp gApp;
extern CMinecraftApp* g_pApp;

namespace
{
    static const EUIScene s_componentDRC = static_cast<EUIScene>(0x42);
    static const EUIScene s_componentTV = static_cast<EUIScene>(0x43);
    static unsigned int s_nextScene;
}

UIScene_MainMenu::UIScene_MainMenu(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer), m_button1(), m_button2(), m_button3(), m_button4(), m_button5(),
      m_button6(), m_button7(), m_timer(), m_loading(false), m_showUpsell(false),
      m_signedIn(false), m_unused(false), m_trial(false), m_splashX(0.0f), m_splashY(0.0f),
      m_splashWidth(0.0f), m_splashHeight(0.0f), m_splashes(), m_splash(), m_pendingAction(0)
{
    gApp.SetDRCIconEnabled(true);
    gApp.SetDRCCheckPlayerCountEnabled(true);
    if (m_layer != NULL) {
        m_layer->addComponent(sceneType, s_componentDRC, NULL);
        m_layer->addComponent(sceneType, s_componentTV, NULL);
    }
    m_button1.init(UIString(0xfe3af6d9), 0);
    m_button2.init(UIString(0x3c6fa355), 1);
    m_button3.init(UIString(0x15087ad6), 2);
    m_button4.init(UIString(0x31dacd58), 3);
    m_button5.init(UIString(0x2b1e9f0e), 4);
    m_button6.init(UIString(0x88e0c979), 5);
    m_button7.init(UIString(0x2686c8e5), 6);
    initialiseMovie();
    handleReload();
    doHorizontalResizeCheck();
    m_controls.push_back(&m_button1);
    m_controls.push_back(&m_button2);
    m_controls.push_back(&m_button3);
    m_controls.push_back(&m_button4);
    m_controls.push_back(&m_button5);
    m_controls.push_back(&m_button6);
    m_controls.push_back(&m_button7);
    m_controls.push_back(&m_timer);
    updateComponents();
    gConsoleUIController.TouchBoxRebuild(this);
    gConsoleUIController.ResetSelectedItem();
}

UIScene_MainMenu::~UIScene_MainMenu()
{
    if (m_layer != NULL) {
        m_layer->removeComponent(s_componentDRC);
        m_layer->removeComponent(s_componentTV);
    }
}

unsigned int UIScene_MainMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie == NULL ? NULL : m_movie->getRootNode();
    const char* names[] = { "Button1", "Button2", "Button3", "Button4", "Button5", "Button6", "Button7", "Timer" };
    UIControl* controls[] = { &m_button1, &m_button2, &m_button3, &m_button4, &m_button5,
                              &m_button6, &m_button7, &m_timer };
    for (int i = 0; i < 8; ++i) {
        if (root != NULL)
            controls[i]->setupControl(this, root, names[i]);
    }
    return 1;
}

void UIScene_MainMenu::proceedToScene(int, EUIScene scene)
{
    s_nextScene = static_cast<unsigned int>(scene);
}

void UIScene_MainMenu::handleReload()
{
    removeControl(&m_button7, false);
    removeControl(&m_button3, false);
}

void UIScene_MainMenu::updateTooltips()
{
    if (!gConsoleUIController.IsShowingAlertMessage(0) &&
        !gConsoleUIController.IsShowingErrorMessage(0))
        gConsoleUIController.RefreshTooltips(0);
}

void UIScene_MainMenu::updateComponents()
{
    if (m_layer != NULL) {
        m_layer->showComponent(m_sceneType, s_componentDRC, true);
        m_layer->showComponent(m_sceneType, s_componentTV, true);
    }
}

void UIScene_MainMenu::customDrawSplash(fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    ScreenSizeCalculator calculator(minecraft->mOptions, minecraft->mWidth, minecraft->mHeight, -1);
    m_splashX = static_cast<float>(minecraft->mWidth);
    m_splashY = static_cast<float>(minecraft->mHeight);
    m_splashWidth = static_cast<float>(calculator.mWidthScale);
    m_splashHeight = static_cast<float>(calculator.mHeightScale);
    gRenderer->TextureBindVertex(-1);

    Font* font = minecraft->mFont;
    GlStateManager::disableCull();
    GlStateManager::disableDepthTest();
    GlStateManager::disableLighting();
    GlStateManager::pushMatrix();
    GlStateManager::translatef((rect->xMax - rect->xMin) * 0.5f, rect->yMax - rect->yMin, 0.0f);
    GlStateManager::rotatef(-17.0f, 0.0f, 0.0f, 1.0f);

    float phase = static_cast<float>(System::processTimeInMilliSecs() % 1000) / 1000.0f * 3.1415927f;
    float scale = (1.8f - Mth::abs(Mth::sin(phase + phase) * 0.1f)) *
                  (m_splashX / m_splashWidth) * 100.0f;
    float factor = scale / static_cast<float>(font->width(m_splash) + 32);
    GlStateManager::scalef(factor, factor, factor);
    font->drawShadow(m_splash, -(font->width(m_splash) / 2), -8, 0xffffff00);
    GlStateManager::popMatrix();
    GlStateManager::disableRescaleNormal();
    GlStateManager::enableDepthTest();
}

void UIScene_MainMenu::customDraw(const char* name, fuiRect* rect)
{
    if (name != NULL && std::strcmp(name, "Splash") == 0)
        customDrawSplash(rect);
    else
        UIScene::customDraw(name, rect);
}

std::wstring UIScene_MainMenu::getMoviePath()
{
    return std::wstring(L"MainMenu");
}

void UIScene_MainMenu::tick()
{
    UIScene::tick();
    updateTooltips();
}

void UIScene_MainMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                   bool pressed, bool& handled)
{
    UIScene::handleInput(playerId, action, repeat, released, pressed, handled);
}

void UIScene_MainMenu::handleUnlockFullVersion()
{
    UIScene::handleUnlockFullVersion();
}

void UIScene_MainMenu::PrepareToPlayGame(int playerId)
{
    if (!gApp.DLCInstallProcessCompleted())
        gApp.StartInstallDLCProcess(playerId);
}

void UIScene_MainMenu::RunPlayGameOffline(int playerId)
{
    PrepareToPlayGame(playerId);
    proceedToScene(playerId, static_cast<EUIScene>(0x1b));
}

void UIScene_MainMenu::RunLeaderboards(int playerId)
{
    proceedToScene(playerId, static_cast<EUIScene>(0x16));
}

int UIScene_MainMenu::Leaderboards_SignInReturned(void* context, bool signedIn, int userIndex,
                                                  int)
{
    UIScene_MainMenu* scene = static_cast<UIScene_MainMenu*>(context);
    if (!signedIn)
    {
        scene->m_loading = false;
        gProfile->SetLockedProfile(-1);
        for (int i = 0; i < 4; ++i)
        {
            if (gProfile->IsSignedIn(i))
                gProfile->SetCurrentGameActivity(i, 1, false);
        }
    }
    else
    {
        gProfile->SetCurrentGameActivity(userIndex, 1, false);
        scene->RunLeaderboards(userIndex);
    }
    return 0;
}

void UIScene_MainMenu::RunUnlockOrDLC(int playerId)
{
    m_pendingAction = 5;
    if (gApp.DLCInstallProcessCompleted())
        proceedToScene(playerId, static_cast<EUIScene>(0x2c));
}

int UIScene_MainMenu::UnlockFullGame_SignInReturned(void* context, bool signedIn, int userIndex,
                                                    int playerIndex)
{
    UIScene_MainMenu* scene = static_cast<UIScene_MainMenu*>(context);
    if (!signedIn)
    {
        scene->m_loading = false;
        gProfile->SetLockedProfile(-1);
        for (int i = 0; i < 4; ++i)
        {
            if (gProfile->IsSignedIn(i))
                gProfile->SetCurrentGameActivity(i, 1, false);
        }
    }
    else
    {
        gProfile->SetCurrentGameActivity(userIndex, 1, false);
        gProfile->SetPrimaryPad(playerIndex);
        scene->RunUnlockOrDLC(userIndex);
    }
    return 0;
}

int UIScene_MainMenu::MustConnectReturnedNintendo(void* context, int userIndex,
                                                  C4JStorage::EMessageResult result)
{
    UIScene_MainMenu* scene = static_cast<UIScene_MainMenu*>(context);
    if (result == static_cast<C4JStorage::EMessageResult>(4))
    {
        if (scene->m_pendingAction == 1)
            CGameNetworkManager::sInstance.ShowConnectOnlineNintendoProgress(userIndex,
                                                                  CreateLoad_SignInReturned, scene);
        else if (scene->m_pendingAction == 2)
            CGameNetworkManager::sInstance.ShowConnectOnlineNintendoProgress(userIndex,
                                                                  Leaderboards_SignInReturned,
                                                                  scene);
        else if (scene->m_pendingAction == 5)
            CGameNetworkManager::sInstance.ShowConnectOnlineNintendoProgress(userIndex,
                                                                  UnlockFullGame_SignInReturned,
                                                                  scene);
    }
    else if (scene->m_pendingAction == 1)
    {
        CreateLoad_SignInReturned(scene, result == static_cast<C4JStorage::EMessageResult>(5), 0, 0);
    }
    else
    {
        scene->m_loading = false;
    }
    return 0;
}

int UIScene_MainMenu::MiniGamesUnlock_Returned(void* context, int playerId,
                                               C4JStorage::EMessageResult result)
{
    UIScene_MainMenu* scene = static_cast<UIScene_MainMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(reinterpret_cast<unsigned int>(context)));
    if (scene != NULL)
    {
        if (result == static_cast<C4JStorage::EMessageResult>(4))
            scene->RunUnlockOrDLC(playerId);
        else
            scene->m_loading = false;
    }
    return 0;
}

void UIScene_MainMenu::LoadTrial()
{
    gApp.SetDRCCheckPlayerCountEnabled(true);
}

void UIScene_MainMenu::RunPlayGame(int playerId)
{
    m_pendingAction = 1;
    PrepareToPlayGame(playerId);
    if (m_trial)
        LoadTrial();
    else {
        GameSettings::ApplyGameSettingsChanged(playerId);
        proceedToScene(playerId, static_cast<EUIScene>(0x1b));
    }
}

int UIScene_MainMenu::CreateLoad_SignInReturned(void* context, bool signedIn, int userIndex,
                                                int playerIndex)
{
    UIScene_MainMenu* scene = static_cast<UIScene_MainMenu*>(context);
    gProfile->SetPrimaryPad(playerIndex);
    if (!signedIn)
    {
        scene->m_loading = false;
        gProfile->SetLockedProfile(-1);
        for (int i = 0; i < 4; ++i)
        {
            if (gProfile->IsSignedIn(i))
                gProfile->SetCurrentGameActivity(i, 1, false);
        }
    }
    else
    {
        gProfile->SetCurrentGameActivity(userIndex, 1, false);
        bool online = gProfile->IsSignedInLive(userIndex);
        if (!gProfile->IsGuest(gProfile->GetPrimaryPad()) && gProfile->IsFullVersion() && !online)
            scene->RunPlayGameOffline(userIndex);
        else
            scene->RunPlayGame(userIndex);
    }
    return 0;
}

void UIScene_MainMenu::RunAchievements(int playerId)
{
    proceedToScene(playerId, static_cast<EUIScene>(0x51));
}

int UIScene_MainMenu::Achievements_SignInReturned(void* context, bool signedIn, int userIndex,
                                                  int)
{
    UIScene_MainMenu* scene = static_cast<UIScene_MainMenu*>(context);
    if (!signedIn)
    {
        scene->m_loading = false;
        gProfile->SetLockedProfile(-1);
        for (int i = 0; i < 4; ++i)
        {
            if (gProfile->IsSignedIn(i))
                gProfile->SetCurrentGameActivity(i, 1, false);
        }
    }
    else
    {
        gProfile->SetCurrentGameActivity(userIndex, 1, false);
        scene->RunAchievements(userIndex);
    }
    return 0;
}

void UIScene_MainMenu::RunHelpAndOptions(int playerId)
{
    proceedToScene(playerId, static_cast<EUIScene>(0x0f));
}

int UIScene_MainMenu::HelpAndOptions_SignInReturned(void* context, bool signedIn, int userIndex,
                                                    int)
{
    UIScene_MainMenu* scene = static_cast<UIScene_MainMenu*>(context);
    if (!signedIn)
    {
        scene->m_loading = false;
        gProfile->SetLockedProfile(-1);
        for (int i = 0; i < 4; ++i)
        {
            if (gProfile->IsSignedIn(i))
                gProfile->SetCurrentGameActivity(i, 1, false);
        }
    }
    else
    {
        gProfile->SetCurrentGameActivity(userIndex, 1, false);
        scene->RunHelpAndOptions(userIndex);
    }
    return 0;
}

int UIScene_MainMenu::ExitGameReturned(void*, int, C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(5))
        gApp.ExitGame();
    return 0;
}

void UIScene_MainMenu::RunAction(int playerId)
{
    switch (m_pendingAction) {
    case 1: RunPlayGame(playerId); break;
    case 5: RunUnlockOrDLC(playerId); break;
    default: break;
    }
}

int UIScene_MainMenu::MustSignInReturned(void* context, int userIndex,
                                         C4JStorage::EMessageResult result)
{
    UIScene_MainMenu* scene = static_cast<UIScene_MainMenu*>(context);
    if (result == static_cast<C4JStorage::EMessageResult>(4))
    {
        switch (scene->m_pendingAction)
        {
        case 1:
            gProfile->RequestSignInUI(false, true, true, false, true, CreateLoad_SignInReturned,
                                      scene, userIndex);
            break;
        case 2:
            gProfile->RequestSignInUI(false, false, true, false, true, Leaderboards_SignInReturned,
                                      scene, userIndex);
            break;
        case 3:
            gProfile->RequestSignInUI(false, false, true, false, true, Achievements_SignInReturned,
                                      scene, userIndex);
            break;
        case 4:
            gProfile->RequestSignInUI(false, false, true, false, true, HelpAndOptions_SignInReturned,
                                      scene, userIndex);
            break;
        case 5:
            gProfile->RequestSignInUI(false, false, true, false, true,
                                      UnlockFullGame_SignInReturned, scene, userIndex);
            break;
        default:
            break;
        }
    }
    else
    {
        scene->m_loading = false;
        gProfile->SetLockedProfile(-1);
        for (int i = 0; i < 4; ++i)
        {
            if (gProfile->IsSignedIn(i))
                gProfile->SetCurrentGameActivity(i, 1, false);
        }
    }
    return 0;
}

void UIScene_MainMenu::handlePress(int first, int)
{
    if (first >= 0 && first <= 5) {
        m_pendingAction = first + 1;
        gConsoleUIController.PlayUISFX(NULL);
        RunAction(0);
    } else if (first == 6) {
        gConsoleUIController.RequestErrorMessage(0x5d7629ce, 0xde9a48c3, NULL, 0, 0,
                                                  ExitGameReturned, this, NULL, true);
    }
}

void UIScene_MainMenu::handleGainFocus(bool hadFocus)
{
    UIScene::handleGainFocus(hadFocus);
    updateComponents();
}

bool UIScene_MainMenu::hasFocus(int)
{
    return m_hasFocus;
}

int UIScene_MainMenu::getSceneType()
{
    return 3;
}

unsigned int UIScene_MainMenu::getDefaultGtcButtons()
{
    return 0;
}
