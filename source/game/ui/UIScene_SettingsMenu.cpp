#include "game/ui/UIScene_SettingsMenu.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"

extern CStorage* gStorage;

UIScene_SettingsMenu::UIScene_SettingsMenu(int type, void*, UILayer* layer)
    : UIScene(type, layer), m_language(0)
{
    initialiseMovie();

    m_button1.init(UIString(static_cast<int>(0xf2f331d5)), 0);
    m_button2.init(UIString(static_cast<int>(0xe13a424a)), 1);
    m_button4.init(UIString(static_cast<int>(0xc2b7bf99)), 4);
    m_button5.init(UIString(static_cast<int>(0xfddfe517)), 5);
    m_button6.init(UIString(static_cast<int>(0x337d5132)), 6);

    removeControl(&m_button3, false);

    bool pendingLevelChange = Minecraft::GetInstance()->mPendingLevelChange == 0;
    if (g_pApp->GetPrimaryUserIndex() != m_sceneType)
        removeControl(&m_button2, pendingLevelChange);

    if (!Minecraft::InMiniGame(static_cast<EMiniGameId>(3), true) ||
        Minecraft::GetInstance()->GetMiniGame()->GetHostOption(
            static_cast<eCustomGameModeHostOption>(14)) == 0)
        removeControl(&m_button4, pendingLevelChange);

    m_language = GameSettings::GetMinecraftLanguage(m_sceneType);
    doHorizontalResizeCheck();
    g_pApp->GetLocalPlayerCount();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_SettingsMenu::~UIScene_SettingsMenu()
{
}

unsigned int UIScene_SettingsMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();

    m_button1.setupControl(this, root, std::string("Button1"));
    m_controls.push_back(&m_button1);
    m_button2.setupControl(this, root, std::string("Button2"));
    m_controls.push_back(&m_button2);
    m_button3.setupControl(this, root, std::string("Button3"));
    m_controls.push_back(&m_button3);
    m_button4.setupControl(this, root, std::string("Button4"));
    m_controls.push_back(&m_button4);
    m_button5.setupControl(this, root, std::string("Button5"));
    m_controls.push_back(&m_button5);
    m_button6.setupControl(this, root, std::string("Button6"));
    m_controls.push_back(&m_button6);

    return 1;
}

void UIScene_SettingsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_sceneType, 0x8d6323ed, 0x4f6bcf5e,
                                     -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_SettingsMenu::updateComponents()
{
    bool showComponents = true;
    if (Minecraft::GetInstance()->mPendingLevelChange != 0)
    {
        m_layer->showComponent(m_sceneType, static_cast<EUIScene>(0x42), false);
        showComponents = g_pApp->GetLocalPlayerCount() == 1;
    }
    else
    {
        m_layer->showComponent(m_sceneType, static_cast<EUIScene>(0x42), true);
    }
    m_layer->showComponent(m_sceneType, static_cast<EUIScene>(0x43), showComponents);
}

void UIScene_SettingsMenu::handleReload()
{
    bool pendingLevelChange = Minecraft::GetInstance()->mPendingLevelChange == 0;
    removeControl(&m_button3, pendingLevelChange);

    if (g_pApp->GetPrimaryUserIndex() != m_sceneType)
        removeControl(&m_button2, pendingLevelChange);

    if (!Minecraft::InMiniGame(static_cast<EMiniGameId>(3), true) ||
        Minecraft::GetInstance()->GetMiniGame()->GetHostOption(
            static_cast<eCustomGameModeHostOption>(14)) == 0)
        removeControl(&m_button4, pendingLevelChange);

    m_language = GameSettings::GetMinecraftLanguage(m_sceneType);
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_SettingsMenu::getMoviePath()
{
    if (g_pApp->GetLocalPlayerCount() < 2)
        return std::wstring(L"SettingsMenu");
    return std::wstring(L"SettingsMenuSplit");
}

void UIScene_SettingsMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                       bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_sceneType, action, repeat, released, pressed);
    if (action > 3)
    {
        if (action < 6 || action == 0x14)
            sendInputToMovie(action, repeat, released, pressed);
        else if (action == 0x15 && pressed && !released)
        {
            GameSettings::CheckGameSettingsChanged(true, playerId);
            navigateBack();
        }
    }
}

void UIScene_SettingsMenu::tick()
{
    UIScene::tick();
    if (gConsoleUIController.GetTopScene(m_sceneType, static_cast<EUILayer>(6),
                                         static_cast<EUIGroup>(7)) == this)
    {
        unsigned char language = GameSettings::GetMinecraftLanguage(m_sceneType);
        if (m_language != language)
        {
            m_language = language;
            doHorizontalResizeCheck();
        }
    }
}

int UIScene_SettingsMenu::ResetDefaultsDialogReturned(
    void* context, int button, C4JStorage::EMessageResult result)
{
    UIScene* scene = gConsoleUIController.GetSceneFromCallbackId(
        static_cast<unsigned int>((unsigned long)context));
    if (scene != NULL && result == static_cast<C4JStorage::EMessageResult>(5))
    {
        int playerId = scene->getSceneType();
        GameSettings::SetDefaultGameOptions(gStorage->GetDashboardProfileSettings(playerId),
                                            playerId, true);
        GameSettings::CheckGameSettingsChanged(true, button);
    }
    return 0;
}

void UIScene_SettingsMenu::handlePress(int, int action)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (action > 3)
    {
        if (action < 5)
        {
            int* data = new int[2];
            data[0] = m_sceneType;
            data[1] = 2;
            gConsoleUIController.NavigateToScene(m_sceneType, static_cast<EUIScene>(0x13), data,
                                                 static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
            return;
        }
        if (action != 5)
        {
            if (action != 6)
                return;
            int parameters[2] = { static_cast<int>(0x4fcbc7e7), static_cast<int>(0xb9a1bf4d) };
            gConsoleUIController.RequestAlertMessage(
                0x13efeffb, 0x5694645e, parameters, 2, m_sceneType,
                ResetDefaultsDialogReturned,
                (void*)static_cast<unsigned long>(GetCallbackUniqueId()), NULL,
                true);
            return;
        }
        int* data = new int[2];
        data[0] = m_sceneType;
        data[1] = 3;
        gConsoleUIController.NavigateToScene(m_sceneType, static_cast<EUIScene>(0x13), data,
                                             static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
        return;
    }
    if (action == 0)
    {
        int* data = new int[2];
        data[0] = m_sceneType;
        data[1] = 0;
        gConsoleUIController.NavigateToScene(m_sceneType, static_cast<EUIScene>(0x13), data,
                                             static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
        resetHorizontalResizeCheck();
        return;
    }
    if (action != 1)
        return;
    int* data = new int[2];
    data[0] = m_sceneType;
    data[1] = 1;
    gConsoleUIController.NavigateToScene(m_sceneType, static_cast<EUIScene>(0x13), data,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
}

int UIScene_SettingsMenu::getSceneType()
{
    return 21;
}
