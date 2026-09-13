#include "game/ui/UIScene_MainMenu.hpp"

#include <cstring>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/BufferedReader.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/InputStreamReader.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/core/System.hpp"
#include "game/core/StringUtils.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/UIString.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/generation/LevelGenerators.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "vendor/_NetworkGameInitData.hpp"

extern ConsoleUIController gConsoleUIController;
extern CConsoleMinecraftApp gApp;
extern CMinecraftApp* g_pApp;
extern CTelemetryManager gTelemetryManager;

namespace
{
    static const EUIScene s_componentDRC = static_cast<EUIScene>(0x42);
    static const EUIScene s_componentTV = static_cast<EUIScene>(0x43);
    static unsigned int s_nextScene = 0x54;
    static Random s_splashRandom;

    struct FullscreenProgressState
    {
        bool showConfirmation;
        bool unknownState1;
        bool unknownState2;
        bool showTip;
        int closeMode;
        int playerId;
        EUIScene backScene;
    };
}

UIScene_MainMenu::UIScene_MainMenu(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer), m_button1(), m_button2(), m_button3(), m_button4(), m_button5(),
      m_button6(), m_button7(), m_buttonUpsell(), m_timer(), m_loading(false), m_showUpsell(false),
      m_signedIn(false), m_unused(false), m_trial(false), m_splashX(0.0f), m_splashY(0.0f),
      m_splashWidth(0.0f), m_splashHeight(0.0f), m_splashes(), m_splash(), m_pendingAction(0)
{
    gApp.SetDRCIconEnabled(true);
    gApp.SetDRCCheckPlayerCountEnabled(true);
    initialiseMovie();
    if (m_layer != NULL) {
        m_layer->addComponent(sceneType, s_componentDRC, NULL);
        m_layer->addComponent(sceneType, s_componentTV, NULL);
    }
    m_button1.init(UIString(0xfe3af6d9), 0);
    m_button2.init(UIString(0x3c6fa355), 1);
    m_button3.init(UIString(0x15087ad6), 2);
    m_button4.init(UIString(0x31dacd58), 3);
    m_button5.init(UIString(0x2b1e9f0e), 4);
    if (gProfile->IsFullVersion())
    {
        m_button6.init(UIString(0xe6987a73), 5);
        m_showUpsell = false;
    }
    else
    {
        m_button6.init(UIString(0x88e0c979), 5);
        m_showUpsell = true;
    }
    m_button7.init(UIString(0x2686c8e5), 6);
    handleReload();
    doHorizontalResizeCheck();
    m_splash = L"Default Text";
    if (gApp.hasArchiveFile(L"splashes.txt", false)) {
        arrayWithLength<unsigned char> data = gApp.getArchiveFile(L"splashes.txt", false);
        ByteArrayInputStream input(data);
        InputStreamReader reader(&input);
        BufferedReader buffered(&reader);
        std::wstring line = buffered.readLine();
        while (!line.empty()) {
            line = trimString(line);
            if (!line.empty())
                m_splashes.push_back(line);
            line = buffered.readLine();
        }
        buffered.close();
    }
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
    fuiRenderNode* root = m_movie->getRootNode();
    const char* names[] = { "Button1", "Button2", "Button3", "Button4", "Button5", "Button6",
                            "Button7", "ButtonUpsell", "Timer" };
    UIControl* controls[] = { &m_button1, &m_button2, &m_button3, &m_button4, &m_button5,
                              &m_button6, &m_button7, &m_buttonUpsell, &m_timer };
    for (int i = 0; i < 9; ++i) {
        controls[i]->setupControl(this, root, names[i]);
        m_controls.push_back(controls[i]);
    }
    return 1;
}

void UIScene_MainMenu::proceedToScene(int, EUIScene scene)
{
    s_nextScene = static_cast<unsigned int>(scene);
}

void UIScene_MainMenu::handleReload()
{
    removeControl(&m_buttonUpsell, false);
    removeControl(&m_button3, false);
}

void UIScene_MainMenu::updateTooltips()
{
    UIScene* topScene = gConsoleUIController.GetTopScene(0, static_cast<EUILayer>(4),
                                                          static_cast<EUIGroup>(7));
    if (!gConsoleUIController.IsShowingAlertMessage(0) &&
        !gConsoleUIController.IsShowingErrorMessage(0) && topScene != NULL &&
        topScene->getSceneType() == 3) {
        int tooltip = m_loading ? -1 : 0x8d6323ed;
        gConsoleUIController.SetTooltips(0, tooltip, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, false, false);
    }
}

void UIScene_MainMenu::updateComponents()
{
    if (m_layer != NULL) {
        m_layer->showComponent(m_playerId, s_componentDRC, true);
        m_layer->showComponent(m_playerId, s_componentTV, true);
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
    if (s_nextScene != 0x54 && g_pApp->GetTMSAction(m_playerId) == 0) {
        int playerId = gProfile->GetPrimaryPad();
        if (!m_unused) {
            gConsoleUIController.NavigateToScene(playerId, static_cast<EUIScene>(s_nextScene),
                                                 0, static_cast<EUILayer>(6),
                                                 static_cast<EUIGroup>(7));
        } else {
            gConsoleUIController.NavigateToScene(
                playerId, static_cast<EUIScene>(s_nextScene),
                reinterpret_cast<void*>(static_cast<int>(s_nextScene == 0x1b)),
                static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
        }
        s_nextScene = 0x54;
    }

    if (m_showUpsell && gProfile->IsFullVersion()) {
        m_showUpsell = false;
        UIString label(L10N::GetString(0xe6987a73));
        m_button6.init(label, 5);
    }
}

void UIScene_MainMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                   bool pressed, bool&)
{
    if (!gApp.getIgnoreMainMenuInputDuringInvite() && !m_loading && s_nextScene == 0x54) {
        gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
        if (action > 3) {
            if (action > 5) {
                if (action != 0x14)
                    return;
                if (!released)
                    return;
                gProfile->SetPrimaryPad(playerId);
                gProfile->SetLockedProfile(-1);
            }
            sendInputToMovie(action, repeat, released, pressed);
        }
    }
}

void UIScene_MainMenu::handleUnlockFullVersion()
{
    m_button6.setLabel(UIString(0xe6987a73), true, false);
}

void UIScene_MainMenu::PrepareToPlayGame(int playerId)
{
    if (!gApp.DLCInstallProcessCompleted())
        gApp.StartInstallDLCProcess(playerId);
}

void UIScene_MainMenu::RunPlayGameOffline(int playerId)
{
    PrepareToPlayGame(playerId);
    proceedToScene(gProfile->GetPrimaryPad(), static_cast<EUIScene>(0x1b));
}

void UIScene_MainMenu::RunLeaderboards(int playerId)
{
    int messageParameter = 0xb9a1bf4d;
    if (gProfile->IsGuest(playerId))
    {
        gConsoleUIController.RequestErrorMessage(0xe6bf122, 0x8ba9896f, &messageParameter, 1, 0xff,
                                                 NULL, NULL, NULL, true);
        return;
    }
    if (!gProfile->IsSignedInLive(playerId))
    {
        gConsoleUIController.RequestErrorMessage(0x8a21894b, 0xc34c0b77, &messageParameter, 1, 0xff,
                                                 NULL, NULL, NULL, true);
        return;
    }
    gProfile->SetLockedProfile(playerId);
    gProfile->QuerySigninStatus();
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
    int messageParameter = 0xb9a1bf4d;
    if (!gProfile->IsFullVersion())
    {
        if (gProfile->IsGuest(playerId))
        {
            m_loading = false;
            gConsoleUIController.RequestErrorMessage(0xc6058d6b, 0xfa275479, &messageParameter, 1,
                                                     playerId, NULL, NULL, NULL, true);
            return;
        }
        if (!gProfile->IsSignedInLive(playerId))
        {
            gConsoleUIController.RequestErrorMessage(0x8a21894b, 0xc34c0b77, &messageParameter, 1,
                                                     0xff, NULL, NULL, NULL, true);
            return;
        }
        gProfile->QuerySigninStatus();
        gTelemetryManager.RecordUpsellPresented(playerId, static_cast<ESen_UpsellID>(1), 1);
        gProfile->DisplayFullVersionPurchase(false, playerId, 1);
        return;
    }

    if (!gProfile->IsSignedInLive(playerId))
    {
        int messageParameters[2] = { 0x51d141b, 0x3f0766c7 };
        m_pendingAction = 5;
        gConsoleUIController.RequestErrorMessage(0x8a21894b, 0xb837024e, messageParameters, 2,
                                                 playerId, MustConnectReturnedNintendo, this, NULL,
                                                 true);
        return;
    }

    if (gProfile->IsGuest(playerId))
    {
        m_loading = false;
        gConsoleUIController.RequestErrorMessage(0xe6bf122, 0x8ba9896f, &messageParameter, 1, 0xff,
                                                 NULL, NULL, NULL, true);
        return;
    }

    gProfile->QuerySigninStatus();
    gProfile->SetLockedProfile(playerId);
    proceedToScene(gProfile->GetPrimaryPad(), static_cast<EUIScene>(0x2c));
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
    boost::shared_ptr<Player> nullPlayer;
    gApp.SetTutorialMode(true, nullPlayer);
    gApp.ClearTerrainFeaturePosition();
    gStorage->ResetSaveData();
    gProfile->StartTrialGame();
    gStorage->SetSaveDisabled(true);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0x17), 0);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0x16), 1);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(6), GameType::SURVIVAL->getId());
    gStorage->SetSaveTitle(L"Tutorial");
    gApp.SetAutosaveTimerTime();
    CGameNetworkManager::sInstance.HostGame(0, false, true, 0, static_cast<EMiniGameId>(0xf), 0, 0);
    CGameNetworkManager::sInstance.FakeLocalPlayerJoined();

    _NetworkGameInitData* initData = new _NetworkGameInitData();
    initData->m_seed = 0;
    initData->m_value2 = 0;
    initData->m_value3 = gApp.GetGameHostOption(static_cast<eGameHostOption>(5)) |
                         gApp.GetGameHostOption(static_cast<eGameHostOption>(0x16));
    initData->m_value9 = gApp.GetGameHostOption(static_cast<eGameHostOption>(6));
    initData->m_value5 = 0;

    std::vector<LevelGenerationOptions*>* generators = gApp.getLevelGenerators()->getLevelGenerators();
    for (std::vector<LevelGenerationOptions*>::iterator it = generators->begin();
         it != generators->end(); ++it)
    {
        if ((*it)->isTutorial())
        {
            initData->m_value5 = *it;
            break;
        }
    }

    _LoadingInputParams* loadingParams = new _LoadingInputParams();
    loadingParams->m_firstParameter = &CGameNetworkManager::RunNetworkGameThreadProc;
    loadingParams->m_secondParameter = initData;

    FullscreenProgressState* state = new FullscreenProgressState();
    state->showConfirmation = false;
    state->backScene = static_cast<EUIScene>(0x53);
    state->showTip = true;
    state->unknownState1 = true;
    state->unknownState2 = true;
    state->closeMode = 3;
    loadingParams->m_thirdParameter = state;
    state->playerId = gProfile->GetPrimaryPad();

    gConsoleUIController.ShowTrialTimer(true);
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), static_cast<EUIScene>(4),
                                         loadingParams, static_cast<EUILayer>(6),
                                         static_cast<EUIGroup>(7));
}

void UIScene_MainMenu::RunPlayGame(int playerId)
{
    Minecraft::GetInstance();
    gApp.ReleaseSaveThumbnail();
    if (gProfile->IsGuest(playerId))
    {
        int messageParameter = 0xb9a1bf4d;
        m_loading = false;
        gConsoleUIController.RequestErrorMessage(0xe6bf122, 0x8ba9896f, &messageParameter, 1, 0xff,
                                                 NULL, NULL, NULL, true);
        return;
    }

    PrepareToPlayGame(playerId);
    if (gProfile->IsFullVersion())
    {
        if (!gProfile->IsSignedInLive(playerId))
        {
            int messageParameters[2] = { 0x51d141b, 0x3f0766c7 };
            m_pendingAction = 1;
            gConsoleUIController.RequestErrorMessage(0x8a21894b, 0xb837024e, messageParameters, 2,
                                                     playerId, MustConnectReturnedNintendo, this,
                                                     NULL, true);
            return;
        }
        GameSettings::ApplyGameSettingsChanged(playerId);
        proceedToScene(gProfile->GetPrimaryPad(), static_cast<EUIScene>(0x1b));
        return;
    }

    if (m_unused)
    {
        int messageParameters[2];
        messageParameters[0] = 0x88e0c979;
        messageParameters[1] = 0x4fcbc7e7;
        gConsoleUIController.RequestErrorMessage(
            0xc6058d6b, 0x1e1ffdaf, messageParameters, 2, gProfile->GetPrimaryPad(),
            MiniGamesUnlock_Returned, reinterpret_cast<void*>(GetCallbackUniqueId()), NULL, true);
        return;
    }

    LoadTrial();
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
    if (gProfile->IsGuest(playerId))
    {
        int messageParameter = 0xb9a1bf4d;
        gConsoleUIController.RequestErrorMessage(0xe6bf122, 0x8ba9896f, &messageParameter, 1, 0xff,
                                                 NULL, NULL, NULL, true);
        return;
    }
    gProfile->SetLockedProfile(playerId);
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
    if (gProfile->IsGuest(playerId))
    {
        int messageParameter = 0xb9a1bf4d;
        gConsoleUIController.RequestErrorMessage(0xe6bf122, 0x8ba9896f, &messageParameter, 1, 0xff,
                                                 NULL, NULL, NULL, true);
        return;
    }
    gProfile->QuerySigninStatus();
    gProfile->SetLockedProfile(playerId);
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
    case 2: RunLeaderboards(playerId); break;
    case 3: RunAchievements(playerId); break;
    case 4: RunHelpAndOptions(playerId); break;
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
    int pad = gProfile->GetPrimaryPad();
    switch (first) {
    case 0:
        m_pendingAction = 1;
        m_unused = false;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        break;
    case 1:
        m_unused = true;
        m_pendingAction = 1;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        break;
    case 2:
        m_unused = false;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        m_pendingAction = 2;
        break;
    case 3:
        m_unused = false;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        m_pendingAction = 3;
        break;
    case 4:
        m_unused = false;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        m_pendingAction = 4;
        break;
    case 5:
        m_unused = false;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        m_pendingAction = 5;
        break;
    case 6:
        {
            int messageParameters[2];
            messageParameters[0] = 0x4fcbc7e7;
            messageParameters[1] = 0xb9a1bf4d;
            m_unused = false;
            gConsoleUIController.RequestErrorMessage(0x5d7629ce, 0xde9a48c3, messageParameters, 2,
                                                     0xff, ExitGameReturned, this, NULL, true);
        }
        return;
    default:
        m_unused = false;
        return;
    }

    if (gProfile->IsSignedIn(pad)) {
        RunAction(pad);
        return;
    }

    int messageParameters[2];
    messageParameters[0] = 0x4fcbc7e7;
    messageParameters[1] = 0xb9a1bf4d;
    gConsoleUIController.RequestErrorMessage(0x14b60640, 0x221d8d9d, messageParameters, 2, pad,
                                             MustSignInReturned, this, NULL, true);
}

void UIScene_MainMenu::handleGainFocus(bool hadFocus)
{
    UIScene::handleGainFocus(hadFocus);
    gConsoleUIController.ShowPlayerDisplayname(false);
    const bool noSceneChangePending = (s_nextScene == 0x54);
    m_loading = false;
    if (!noSceneChangePending)
        return;

    if (!hadFocus) {
        for (int i = 0; i < 4; ++i) {
            if (gProfile->IsSignedIn(i))
                gProfile->SetCurrentGameActivity(i, 1, false);
        }
    }

    if (!CGameNetworkManager::sInstance.IsHandlingBootInvite()) {
        gProfile->SetLockedProfile(-1);
        gProfile->ClearGameUsers();
        m_loading = false;
        updateTooltips();
    } else {
        int pad = gProfile->GetPrimaryPad();
        if (gProfile->IsSignedIn(pad)) {
            gProfile->GetPrimaryPad();
            GameSettings::ApplyGameSettingsChanged(pad);
        }
        m_loading = false;
        updateTooltips();
    }

    if (hadFocus && gProfile->IsFullVersion())
        m_button6.setLabel(UIString(0xe6987a73), false, false);

    m_timer.setVisible(false);
    m_button1.setVisible(true);
    m_button2.setVisible(true);
    m_button3.setVisible(true);
    m_button4.setVisible(true);
    m_button5.setVisible(true);
    m_button6.setVisible(true);
    m_button7.setVisible(true);
    m_buttonUpsell.setVisible(true);

    int splashIndex = 0;
    if (!m_splashes.empty())
        splashIndex = static_cast<int>(m_splashes.size());
    splashIndex = s_splashRandom.nextInt(splashIndex - 4) + 4;

    SYSTEMTIME localTime;
    GetLocalTime(&localTime);
    if (localTime.month == 0xc && localTime.day == 0x18) {
        m_splash = m_splashes.at(0);
        return;
    }
    if (localTime.month == 1 && localTime.day == 1) {
        m_splash = m_splashes.at(1);
        return;
    }
    if (localTime.month == 10 && localTime.day == 0x1f)
        splashIndex = 2;
    m_splash = m_splashes.at(splashIndex);
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
