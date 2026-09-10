#include "game/ui/UIScene_ControlsMenu.hpp"

#include <string>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/UIControl_List.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_ControlsMenu::UIScene_ControlsMenu(int player, void* data, UILayer* layer)
    : UIScene(player, layer), m_unknownD4(0), m_unknownD8(0), m_unknownDC(0)
{
    for (int i = 0; i < 10; ++i)
        m_flags[i] = 0;
    initialiseMovie();
    Init(player);
}

UIScene_ControlsMenu::~UIScene_ControlsMenu()
{
    if (m_flags[2])
    {
        GameSettings::CheckGameSettingsChanged(true, m_playerId);
        GameSettings::ActionGameSettings(m_playerId, (eGameSetting)0x29);
    }
}

std::wstring UIScene_ControlsMenu::GetControlsString(int player, EControllerActions action, bool& mapped)
{
    std::wstring result;
    if (GameSettings::GetInputButton(player, false, action) == 0 && action >= 0x20 && action < 0x39)
    {
        result = L" ";
        result += L10N::GetString(0x211cd1ad);
        mapped = false;
    }
    else
    {
        result = gApp.GetActionReplacement(player, (unsigned char)action);
        mapped = true;
    }
    return result;
}

void UIScene_ControlsMenu::SetControlsHelp(int player, int control, std::wstring help)
{
    if (!help.empty())
        gApp.FormatHTMLString(player, help, -1);
    m_flags[3] = 0;
}

void UIScene_ControlsMenu::Init(int player)
{
    m_playerId = player;
    m_flags[7] = 0;
    m_flags[1] = 0;
    m_flags[2] = 0;
    m_flags[3] = 0;
    m_flags[5] = 0;
    mapElementsAndNames();
    m_controls.AddNewCheckbox(L"", 1, GameSettings::GetGameSettings(player, (eGameSetting)8) != 0);
    m_controls.AddNewCheckbox(L"", 2, GameSettings::GetGameSettings(player, (eGameSetting)9) != 0);
    UpdateControls(player);
}

unsigned int UIScene_ControlsMenu::mapElementsAndNames()
{
    m_version.setupControl(this, m_movie->getRootNode(), std::string("Version"));
    m_controls.setupControl(this, m_movie->getRootNode(), std::string("ControlsMultiList"));
    m_controls.AddNewLabel(L"", 0);
    return 1;
}

void UIScene_ControlsMenu::updateTooltips()
{
    if (m_flags[5])
    {
        gConsoleUIController.SetTooltips(m_playerId, 0, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, true, false);
    }
    else if (m_controls.getItemCount() > 0)
    {
        int tooltip = m_controls.IsEnabled(m_controls.getCurrentSelection()) ? 0x8d6323ed : -1;
        gConsoleUIController.SetTooltips(m_playerId, tooltip, 0x4f6bcf5e, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, true, false);
    }
}

void UIScene_ControlsMenu::updateComponents()
{
    if (m_layer)
        m_layer->showComponent(m_playerId, (EUIScene)0x43, false);
}

unsigned char UIScene_ControlsMenu::GetControlsAction(EControl control)
{
    unsigned int id = (unsigned int)control;
    if (id < 3 || id > 0x1d)
        return '=';
    if (id == 3)
        return GameSettings::GetGameSettings(m_playerId, (eGameSetting)9) ? 0x1b : 0x1d;
    if (id == 4)
        return GameSettings::GetGameSettings(m_playerId, (eGameSetting)9) ? 0x1d : 0x1b;
    static const unsigned char actions[] = "ControlsMultiList";
    return actions[id + 0xf];
}

std::wstring UIScene_ControlsMenu::GetControlsString(EControl control, bool& mapped)
{
    unsigned char action = GetControlsAction(control);
    if (action < 0x3d)
        return GetControlsString(m_playerId, (EControllerActions)action, mapped);
    mapped = false;
    return std::wstring();
}

void UIScene_ControlsMenu::UpdateControls(int player)
{
    m_controls.SetCheckboxValue(1, GameSettings::GetGameSettings(player, (eGameSetting)8) != 0, false);
    m_controls.SetCheckboxValue(2, GameSettings::GetGameSettings(player, (eGameSetting)9) != 0, false);
    bool mapped;
    for (int i = 3; i <= 0x1d; ++i)
        m_controls.SetSecondaryLabel(i, UIString(GetControlsString((EControl)i, mapped)), true, mapped, false);
}

unsigned int UIScene_ControlsMenu::ResetControlsWarningDialogReturned(void* context, int button,
                                                                        C4JStorage::EMessageResult result)
{
    UIScene_ControlsMenu* scene = static_cast<UIScene_ControlsMenu*>(
        gConsoleUIController.GetSceneFromCallbackId((unsigned int)context));
    if (scene != NULL)
    {
        if (result == static_cast<C4JStorage::EMessageResult>(4))
        {
            GameSettings::SetDefaultControls(gStorage->GetDashboardProfileSettings(scene->m_playerId),
                                             scene->m_playerId, true);
            GameSettings::CheckGameSettingsChanged(true, button);
            scene->UpdateControls(scene->m_playerId);
        }
        else if (result == static_cast<C4JStorage::EMessageResult>(5) ||
                 result == static_cast<C4JStorage::EMessageResult>(3))
        {
            scene->navigateBack();
        }
    }
    return 0;
}

unsigned int UIScene_ControlsMenu::CheckMapping(int player)
{
    for (int control = 5; control <= 0x1d; control += 2)
    {
        int first = GameSettings::GetActionMapping(player, GetControlsAction((EControl)control) - 0x20);
        int second = GameSettings::GetActionMapping(player, GetControlsAction((EControl)(control + 1)) - 0x20);
        if ((first & 0xf) == 0xd || (first & 0xf) == 0xe || (second & 0xf) == 0xd ||
            (second & 0xf) == 0xe || ((first >> 4) & 0xff) == 0xd ||
            ((first >> 4) & 0xff) == 0xe || ((second >> 4) & 0xff) == 0xd ||
            ((second >> 4) & 0xff) == 0xe)
            return 1;
    }
    return 0;
}

void UIScene_ControlsMenu::tick()
{
    if (m_flags[7])
    {
        CheckMapping(m_playerId);
        m_flags[7] = 0;
    }
    if (m_flags[1])
        UpdateControls(m_playerId);
    UIScene::tick();
}

std::wstring UIScene_ControlsMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() < 2 ? L"Controls" : L"ControlsSplit";
}

void UIScene_ControlsMenu::handleCheckboxElementToggled(int first, int second, bool checked)
{
    gConsoleUIController.PlayUISFX(0);
    if (second == 1)
        GameSettings::SetGameSettings(m_playerId, (eGameSetting)8, checked);
    else if (second == 2)
    {
        if ((GameSettings::GetGameSettings(m_playerId, (eGameSetting)9) != 0) != checked)
            m_flags[1] = 1;
        GameSettings::SetGameSettings(m_playerId, (eGameSetting)9, checked);
    }
}

unsigned int UIScene_ControlsMenu::ResetDefaultsDialogReturned(void* context, int button,
                                                                C4JStorage::EMessageResult result)
{
    UIScene_ControlsMenu* scene = static_cast<UIScene_ControlsMenu*>(
        gConsoleUIController.GetSceneFromCallbackId((unsigned int)context));
    if (scene != NULL && result == static_cast<C4JStorage::EMessageResult>(5))
    {
        GameSettings::SetDefaultControls(gStorage->GetDashboardProfileSettings(scene->m_playerId),
                                         scene->m_playerId, true);
        GameSettings::CheckGameSettingsChanged(true, button);
        scene->UpdateControls(scene->m_playerId);
    }
    return 0;
}

unsigned int UIScene_ControlsMenu::ControlsWarningDialogReturned(void* context, int button,
                                                                  C4JStorage::EMessageResult result)
{
    UIScene_ControlsMenu* scene = static_cast<UIScene_ControlsMenu*>(
        gConsoleUIController.GetSceneFromCallbackId((unsigned int)context));
    if (scene != NULL && result == static_cast<C4JStorage::EMessageResult>(4))
    {
        GameSettings::CheckGameSettingsChanged(true, button);
        GameSettings::ActionGameSettings(button, (eGameSetting)0x29);
        scene->m_flags[2] = 0;
        scene->navigateBack();
    }
    return 0;
}

void UIScene_ControlsMenu::handleReload()
{
    Init(m_playerId);
}

unsigned int UIScene_ControlsMenu::CheckControls(int player, EControl control)
{
    unsigned char action = GetControlsAction(control);
    int mapping = GameSettings::GetActionMapping(player, action - 0x20);
    for (int candidate = 5; candidate <= 0x1d; ++candidate)
    {
        if (candidate != (int)control && GameSettings::GetActionMapping(
                player, GetControlsAction((EControl)candidate) - 0x20) == mapping)
        {
            GameSettings::SetActionMapping(player, action - 0x20, 0);
            return 1;
        }
    }
    return 0;
}

void UIScene_ControlsMenu::HandleButtonEdit(int player, int action, bool repeat, bool released,
                                            bool pressed)
{
    m_flags[5] = 0;
    m_flags[3] = 1;
}

unsigned int UIScene_ControlsMenu::CheckControlsOnExit(int player)
{
    for (int control = 5; control <= 0x1d; ++control)
    {
        if (control == 0xe || control == 0xf || (control >= 0x11 && control <= 0x14))
            continue;
        if (GameSettings::GetActionMapping(player, GetControlsAction((EControl)control) - 0x20) == 0)
            return 1;
    }
    return 0;
}

void UIScene_ControlsMenu::handleInput(int player, int action, bool repeat, bool released,
                                       bool pressed, bool& handled)
{
    if (m_flags[5])
    {
        HandleButtonEdit(player, action, repeat, released, pressed);
        return;
    }
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (action < 4 || (action > 9 && action < 0xe) || (action > 0xf && action != 0x14 && action != 0x15))
        return;
    if (action == 0x14)
    {
        if (!released)
            sendInputToMovie(action, repeat, released, pressed);
        return;
    }
    if (action == 0x15 && pressed)
    {
        if (!CheckControlsOnExit(player))
        {
            GameSettings::CheckGameSettingsChanged(true, player);
            GameSettings::ActionGameSettings(player, (eGameSetting)0x29);
            m_flags[2] = 0;
            navigateBack();
        }
        return;
    }
    m_flags[3] = 1;
    sendInputToMovie(action, repeat, released, pressed);
}

void UIScene_ControlsMenu::handlePress(int first, int second)
{
    gConsoleUIController.PlayUISFX(0);
    if (second == 0)
        return;
    if (second >= 5 && second <= 0x1d)
    {
        m_flags[5] = 1;
        m_flags[3] = 1;
        updateTooltips();
    }
}

void UIScene_ControlsMenu::handleFocusChange(int focus, int childFocus)
{
    m_controls.updateChildFocus(childFocus);
    updateTooltips();
}

int UIScene_ControlsMenu::getSceneType()
{
    return 0x12;
}
