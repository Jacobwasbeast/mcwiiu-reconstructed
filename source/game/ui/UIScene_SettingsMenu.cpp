#include "game/ui/UIScene_SettingsMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UIString.hpp"
#include "platform/wiiu/CProfile.hpp"

extern CStorage* gStorage;

UIScene_SettingsMenu::UIScene_SettingsMenu(int type, void*, UILayer* layer)
    : UIScene(type, layer)
{
    initialiseMovie();

    bool destroyControls = Minecraft::GetInstance()->mPendingLevelChange == 0;

    m_buttons[0].init(UIString((int)IDS_GAME_OPTIONS), 0);
    m_buttons[1].init(UIString((int)IDS_AUDIO), 1);
    m_buttons[4].init(UIString((int)IDS_GRAPHICS), 4);
    m_buttons[5].init(UIString((int)IDS_USER_INTERFACE), 5);
    m_buttons[6].init(UIString((int)IDS_RESET_TO_DEFAULTS), 6);

    removeControl(&m_buttons[2], false);

    if (gProfile->GetPrimaryPad() != m_playerId)
    {
        removeControl(&m_buttons[1], destroyControls);
        if (!Minecraft::InMiniGame(eMiniGameId_Glide, true) ||
            Minecraft::GetInstance()->GetMiniGame()->GetHostOption(
                eCustomGameModeHostOption_TimeTrial) == 0)
            removeControl(&m_buttons[4], destroyControls);
    }

    m_language = GameSettings::GetMinecraftLanguage(m_playerId);
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

    m_buttons[0].setupControl(this, root, std::string("Button1"));
    m_controls.push_back(&m_buttons[0]);
    m_buttons[1].setupControl(this, root, std::string("Button2"));
    m_controls.push_back(&m_buttons[1]);
    m_buttons[2].setupControl(this, root, std::string("Button3"));
    m_controls.push_back(&m_buttons[2]);
    m_buttons[4].setupControl(this, root, std::string("Button4"));
    m_controls.push_back(&m_buttons[4]);
    m_buttons[5].setupControl(this, root, std::string("Button5"));
    m_controls.push_back(&m_buttons[5]);
    m_buttons[6].setupControl(this, root, std::string("Button6"));
    m_controls.push_back(&m_buttons[6]);

    return 1;
}

void UIScene_SettingsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, (int)IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK,
                                     -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_SettingsMenu::updateComponents()
{
    bool showComponents = true;
    if (Minecraft::GetInstance()->mPendingLevelChange != 0)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        showComponents = g_pApp->GetLocalPlayerCount() == 1;
    }
    else
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
    }
    m_layer->showComponent(m_playerId, EUIScene_Logo, showComponents);
}

void UIScene_SettingsMenu::handleReload()
{
    bool destroyControls = Minecraft::GetInstance()->mPendingLevelChange == 0;
    removeControl(&m_buttons[2], destroyControls);

    if (gProfile->GetPrimaryPad() != m_playerId)
    {
        removeControl(&m_buttons[1], destroyControls);
        if (!Minecraft::InMiniGame(eMiniGameId_Glide, true) ||
            Minecraft::GetInstance()->GetMiniGame()->GetHostOption(
                eCustomGameModeHostOption_TimeTrial) == 0)
            removeControl(&m_buttons[4], destroyControls);
    }

    m_language = GameSettings::GetMinecraftLanguage(m_playerId);
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
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, released, pressed);
        break;
    case EControllerActions_MenuCancel:
        if (released)
        {
            GameSettings::CheckGameSettingsChanged(true, playerId);
            navigateBack();
        }
        break;
    }
}

void UIScene_SettingsMenu::tick()
{
    UIScene::tick();
    if (gConsoleUIController.GetTopScene(m_playerId, EUILayer_Scene,
                                         EUIGroup_PlayerDefault) == this)
    {
        unsigned char language = GameSettings::GetMinecraftLanguage(m_playerId);
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
    UIScene_SettingsMenu* scene = static_cast<UIScene_SettingsMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(reinterpret_cast<unsigned int>(context)));
    if (scene != NULL && result == C4JStorage::EMessageResult_Decline)
    {
        GameSettings::SetDefaultGameOptions(
            gStorage->GetDashboardProfileSettings(scene->m_playerId), scene->m_playerId, true);
        GameSettings::CheckGameSettingsChanged(true, button);
    }
    return 0;
}

void UIScene_SettingsMenu::handlePress(int controlId, int)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (controlId > 3)
    {
        if (controlId < 5)
        {
            int* data = new int[2];
            data[0] = m_playerId;
            data[1] = 2;
            gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SettingsListMenu, data,
                                                 EUILayer_Scene, EUIGroup_PlayerDefault);
            return;
        }
        if (controlId != 5)
        {
            if (controlId != 6)
                return;
            int parameters[2] = { static_cast<int>(IDS_CONFIRM_CANCEL), static_cast<int>(IDS_CONFIRM_OK) };
            gConsoleUIController.RequestAlertMessage(
                0x13efeffb, IDS_DEFAULTS_TEXT, parameters, 2, m_playerId,
                ResetDefaultsDialogReturned,
                (void*)static_cast<unsigned long>(GetCallbackUniqueId()), NULL,
                true);
            return;
        }
        int* data = new int[2];
        data[0] = m_playerId;
        data[1] = 3;
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SettingsListMenu, data,
                                             EUILayer_Scene, EUIGroup_PlayerDefault);
        return;
    }
    if (controlId == 0)
    {
        int* data = new int[2];
        data[0] = m_playerId;
        data[1] = 0;
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SettingsListMenu, data,
                                             EUILayer_Scene, EUIGroup_PlayerDefault);
        resetHorizontalResizeCheck();
        return;
    }
    if (controlId != 1)
        return;
    int* data = new int[2];
    data[0] = m_playerId;
    data[1] = 1;
    gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SettingsListMenu, data,
                                         EUILayer_Scene, EUIGroup_PlayerDefault);
}

int UIScene_SettingsMenu::getSceneType()
{
    return EUIScene_SettingsMenu;
}
