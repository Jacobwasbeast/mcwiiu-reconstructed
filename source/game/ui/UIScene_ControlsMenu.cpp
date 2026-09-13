#include "game/ui/UIScene_ControlsMenu.hpp"

#include <cwchar>
#include <string>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/FJ_ControlsMenu.hpp"
#include "game/ui/UIControl_List.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

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

void UIScene_ControlsMenu::SetControlsHelp(int, int control, std::wstring help)
{
    UIString text(L"");
    if (!help.empty())
    {
        HtmlString html(help, static_cast<eMinecraftColour>(0x1b5), false, false, 0);
        std::wstring formatted = gApp.FormatHTMLString(m_playerId, html.ToString(), -1);
        text = UIString(formatted);
    }
    FJ_ControlsMenu* menu = static_cast<FJ_ControlsMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->SetControlsHelp(control, std::wstring(text.c_str()));
}

void UIScene_ControlsMenu::Init(int player)
{
    m_playerId = player;

    bool isFlying = false;

    wchar_t versionText[128];
    std::swprintf(versionText, 128, L"%ls %ls %ls", L"1.12.2177.0",
                  MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(1), true)
                      ->getVersionString().c_str(),
                  L"");
    m_version.init(UIString(versionText));

    m_flags[0] = isFlying;

    bool mapped;
    m_controls.AddNewButton(L10N::GetString(0x337d5132), 0, true);

    m_controls.AddNewLabel(L" ", 0x1e);
    m_controls.AddNewLabel(L10N::GetString(0x8eb977d2), 0x1e);

    m_controls.AddNewCheckbox(L10N::GetString(0xa1531e06), 1,
                              GameSettings::GetGameSettings(player, (eGameSetting)8) != 0);
    m_controls.AddNewCheckbox(L10N::GetString(0xfd098a4f), 2,
                              GameSettings::GetGameSettings(player, (eGameSetting)9) != 0);

    m_controls.AddNewControllerButton(L10N::GetString(0x1c7a7950),
                                      GetControlsString((EControl)3, mapped), mapped, 3);
    m_controls.AddNewControllerButton(L10N::GetString(0x1eda7c36),
                                      GetControlsString((EControl)4, mapped), mapped, 4);

    m_controls.AddNewLabel(L" ", 0x1e);
    m_controls.AddNewLabel(L10N::GetString(0x19988bba), 0x1e);

    m_controls.AddNewControllerButton(L10N::GetString(0x5617db3c),
                                      GetControlsString((EControl)5, mapped), mapped, 5);
    m_controls.AddNewControllerButton(L10N::GetString(0xe90d9e49),
                                      GetControlsString((EControl)6, mapped), mapped, 6);
    m_controls.AddNewControllerButton(L10N::GetString(isFlying ? 0x774c60ee : 0x3fef334d),
                                      GetControlsString((EControl)7, mapped), mapped, 7);
    m_controls.AddNewControllerButton(L10N::GetString(0x7a6ead8e),
                                      GetControlsString((EControl)8, mapped), mapped, 8);
    m_controls.AddNewControllerButton(L10N::GetString(0xc2695d1d),
                                      GetControlsString((EControl)9, mapped), mapped, 9);
    m_controls.AddNewControllerButton(L10N::GetString(0x4ef5d075),
                                      GetControlsString((EControl)0xa, mapped), mapped, 0xa);
    m_controls.AddNewControllerButton(L10N::GetString(0xc513139b),
                                      GetControlsString((EControl)0xb, mapped), mapped, 0xb);
    m_controls.AddNewControllerButton(L10N::GetString(0x72ab6b50),
                                      GetControlsString((EControl)0xc, mapped), mapped, 0xc);
    m_controls.AddNewControllerButton(L10N::GetString(isFlying ? 0x8efa4866 : 0x464be825),
                                      GetControlsString((EControl)0xd, mapped), mapped, 0xd);
    m_controls.AddNewControllerButton(L10N::GetString(0x75549fcd),
                                      GetControlsString((EControl)0xe, mapped), mapped, 0xe);
    m_controls.AddNewControllerButton(L10N::GetString(0x50c103af),
                                      GetControlsString((EControl)0xf, mapped), mapped, 0xf);
    m_controls.AddNewControllerButton(L10N::GetString(0x88874831),
                                      GetControlsString((EControl)0x10, mapped), mapped, 0x10);

    m_controls.AddNewLabel(L" ", 0x1e);
    m_controls.AddNewLabel(L10N::GetString(0x4aebca1a), 0x1e);

    m_controls.AddNewControllerButton(L10N::GetString(0x880870a4),
                                      GetControlsString((EControl)0x11, mapped), mapped, 0x11);
    m_controls.AddNewControllerButton(L10N::GetString(0xdb0d0c07),
                                      GetControlsString((EControl)0x12, mapped), mapped, 0x12);
    m_controls.AddNewControllerButton(L10N::GetString(0xbfc2db62),
                                      GetControlsString((EControl)0x13, mapped), mapped, 0x13);
    m_controls.AddNewControllerButton(L10N::GetString(0x48a6649d),
                                      GetControlsString((EControl)0x14, mapped), mapped, 0x14);

    m_controls.AddNewLabel(L" ", 0x1e);
    m_controls.AddNewLabel(L10N::GetString(0xd62470c6), 0x1e);

    m_controls.AddNewControllerButton(L10N::GetString(0x2edff749),
                                      GetControlsString((EControl)0x19, mapped), mapped, 0x19);
    m_controls.AddNewControllerButton(L10N::GetString(0xc9f98a25),
                                      GetControlsString((EControl)0x1a, mapped), mapped, 0x1a);

    m_controls.AddNewLabel(L" ", 0x1e);
    m_controls.AddNewLabel(L10N::GetString(0x1fd34dbd), 0x1e);

    m_controls.AddNewControllerButton(L10N::GetString(0xbc932ff8),
                                      GetControlsString((EControl)0x15, mapped), mapped, 0x15);
    m_controls.AddNewControllerButton(L10N::GetString(0xea6bb036),
                                      GetControlsString((EControl)0x16, mapped), mapped, 0x16);
    m_controls.AddNewControllerButton(L10N::GetString(0xf4a1c60f),
                                      GetControlsString((EControl)0x17, mapped), mapped, 0x17);
    m_controls.AddNewControllerButton(L10N::GetString(0x6a023a0f),
                                      GetControlsString((EControl)0x18, mapped), mapped, 0x18);

    m_controls.AddNewLabel(L" ", 0x1e);
    m_controls.AddNewLabel(L10N::GetString(0x48a5d3ba), 0x1e);

    m_controls.AddNewControllerButton(L10N::GetString(0x7a8e2667),
                                      GetControlsString((EControl)0x1b, mapped), mapped, 0x1b);
    m_controls.AddNewControllerButton(L10N::GetString(0xec79c513),
                                      GetControlsString((EControl)0x1c, mapped), mapped, 0x1c);
    m_controls.AddNewControllerButton(L10N::GetString(0x78d551aa),
                                      GetControlsString((EControl)0x1d, mapped), mapped, 0x1d);

    m_controls.EnableItem(3, false, true);
    m_controls.EnableItem(4, false, true);

    m_unknownD8 = 0;
    m_flags[8] = 0;
    m_flags[5] = 0;
    m_flags[6] = 0;
    m_flags[1] = 0;
    m_flags[3] = 0;
    m_flags[9] = 0;
    m_unknownD4 = 0;
    m_flags[4] = 0;
    m_flags[2] = 0;
    m_unknownDC = -1;

    SetControlsHelp(player, 0, L"");
    m_flags[7] = 1;
    gConsoleUIController.TouchBoxRebuild(this);
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
    m_flags[1] = 0;
}

int UIScene_ControlsMenu::ResetControlsWarningDialogReturned(void* context, int button,
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
    if (g_pInput->IsPadClassicController(player) == 0)
        return 0;
    for (int control = 5; control <= 0x1d; control += 2)
    {
        int first = GameSettings::GetActionMapping(player, GetControlsAction((EControl)control) - 0x20);
        int second = GameSettings::GetActionMapping(player, GetControlsAction((EControl)(control + 1)) - 0x20);
        if ((first & 0xf) == 0xd || (first & 0xf) == 0xe || (second & 0xf) == 0xd ||
            (second & 0xf) == 0xe || ((first >> 4) & 0xff) == 0xd ||
            ((first >> 4) & 0xff) == 0xe || ((second >> 4) & 0xff) == 0xd ||
            ((second >> 4) & 0xff) == 0xe)
        {
            int parameters[2] = { static_cast<int>(0x4fcbc7e7), static_cast<int>(0xb9a1bf4d) };
            gConsoleUIController.RequestAlertMessage(
                0x8038cb11, 0x185bdd14, parameters, 2, m_playerId,
                ResetControlsWarningDialogReturned,
                (void*)static_cast<unsigned long>(GetCallbackUniqueId()), NULL, true);
            return 1;
        }
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

int UIScene_ControlsMenu::ResetDefaultsDialogReturned(void* context, int button,
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

int UIScene_ControlsMenu::ControlsWarningDialogReturned(void* context, int button,
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

    int groupFirst;
    int groupLast;
    if (static_cast<unsigned int>((int)control - 5) < 0x10)
    {
        groupFirst = 5;
        groupLast = 0x14;
    }
    else if (static_cast<unsigned int>((int)control - 0x19) < 2)
    {
        groupFirst = 0x19;
        groupLast = 0x1a;
    }
    else if (static_cast<unsigned int>((int)control - 0x15) < 4)
    {
        groupFirst = 0x15;
        groupLast = 0x18;
    }
    else if (static_cast<unsigned int>((int)control - 0x1b) < 3)
    {
        groupFirst = 0x1b;
        groupLast = 0x1d;
    }
    else
    {
        return 0;
    }

    int conflicting = -1;
    for (int candidate = groupFirst; candidate <= groupLast; ++candidate)
    {
        if (candidate == (int)control)
            continue;
        unsigned char candidateAction = GetControlsAction((EControl)candidate);
        if (GameSettings::GetActionMapping(player, candidateAction - 0x20) == mapping)
        {
            conflicting = candidate;
            break;
        }
    }
    if (conflicting < 0)
        return 0;

    unsigned char conflictingAction = GetControlsAction((EControl)conflicting);
    GameSettings::SetActionMapping(player, conflictingAction - 0x20, 0);

    std::wstring text = L" ";
    text += L10N::GetString(0x211cd1ad);
    m_controls.SetSecondaryLabel(conflicting, UIString(text), true, false, false);
    m_controls.HighlightItem(conflicting, false);
    SetControlsHelp(player, conflicting, L10N::GetString(0x23b1b1ae));
    return 1;
}

void UIScene_ControlsMenu::HandleButtonEdit(int player, int action, bool repeat, bool released,
                                            bool pressed)
{
    if (action == 0x13)
    {
        if (!repeat && released && !pressed)
        {
            bool mapped = true;
            int selection = m_controls.getCurrentSelection();
            m_controls.SetSecondaryLabel(selection, UIString(GetControlsString((EControl)selection, mapped)),
                                         true, mapped, false);
            m_flags[5] = 0;
            updateTooltips();
            m_unknownD4 = 0;
            m_unknownD8 = 0;
            m_flags[9] = 0;
            m_flags[8] = 0;
            m_unknownDC = -1;
            SetControlsHelp(player, 0, L"");
        }
        return;
    }

    int captured = m_unknownD8;
    if (captured != 0 && !repeat && !released && pressed)
    {
        for (int button = 0; button <= 0xd; ++button)
        {
            if (g_pInput->GetValue(player, static_cast<unsigned char>(button), true) ||
                g_pInput->ButtonDown(player, static_cast<unsigned char>(button)))
                return;
        }

        unsigned char mapping = (m_unknownD8 == 1)
            ? m_flags[8]
            : static_cast<unsigned char>(m_flags[9] | (m_flags[8] << 4));
        int selection = m_controls.getCurrentSelection();
        GameSettings::SetActionMapping(player, GetControlsAction((EControl)selection) - 0x20, mapping);
        GameSettings::SetSettingsChanged(player);
        m_flags[2] = 1;

        bool mapped = true;
        m_controls.SetSecondaryLabel(selection, UIString(GetControlsString((EControl)selection, mapped)),
                                     true, mapped, false);
        updateTooltips();
        m_flags[5] = 0;
        m_flags[6] = 0;
        m_flags[8] = 0;
        m_unknownD4 = 0;
        m_unknownDC = -1;
        m_flags[9] = 0;
        m_unknownD8 = 0;

        if (CheckControls(player, (EControl)selection) == 0)
            SetControlsHelp(player, 0, L"");
        return;
    }

    if (action > 0xd || !released || m_unknownDC == action)
        return;

    unsigned int bit;
    unsigned char id;
    switch (action)
    {
    case 0:
        if (g_pInput->IsCircleCrossSwapped()) { bit = 2; id = 2; } else { bit = 1; id = 1; }
        break;
    case 1:
        if (g_pInput->IsCircleCrossSwapped()) { bit = 1; id = 1; } else { bit = 2; id = 2; }
        break;
    case 2:
        bit = 4; id = 3;
        break;
    case 3:
        bit = 8; id = 4;
        break;
    case 4:
        if (g_pInput->GetJoypadStick_Menu_LY(player) != 50.0)
            return;
        bit = 0x400; id = 7;
        break;
    case 5:
        if (g_pInput->GetJoypadStick_Menu_LY(player) != 50.0)
            return;
        bit = 0x800; id = 8;
        break;
    case 6:
        if (g_pInput->GetJoypadStick_Menu_LX(player) != 50.0)
            return;
        bit = 0x2000; id = 6;
        break;
    case 7:
        if (g_pInput->GetJoypadStick_Menu_LX(player) != 50.0)
            return;
        bit = 0x1000; id = 5;
        break;
    case 8:
        bit = 0x800000; id = 0xc;
        break;
    case 9:
        bit = 0x400000; id = 0xb;
        break;
    case 10:
        bit = 0x40; id = 9;
        break;
    case 0xb:
        bit = 0x80; id = 0xa;
        break;
    case 0xc:
        bit = 0x200; id = 0xe;
        break;
    case 0xd:
        bit = 0x100; id = 0xd;
        break;
    default:
        return;
    }

    if (captured != 0)
        return;

    m_unknownD4 = bit;
    m_flags[8] = id;

    std::wstring text = L"...";
    text += gApp.GetSingleActionReplacement(player, action);
    int selection = m_controls.getCurrentSelection();
    m_controls.SetSecondaryLabel(selection, UIString(text), true, true, false);
    m_unknownD8 = m_unknownD8 + 1;
}

unsigned int UIScene_ControlsMenu::CheckControlsOnExit(int player)
{
    for (int control = 5; control <= 0x1d; ++control)
    {
        if (control == 0xe || control == 0xf || (control >= 0x11 && control <= 0x14))
            continue;
        if (GameSettings::GetActionMapping(player, GetControlsAction((EControl)control) - 0x20) == 0)
        {
            m_controls.SetSecondaryLabel(control, UIString(std::wstring(L10N::GetString(0x211cd1ad))),
                                         true, false, false);
            m_controls.HighlightItem(control, false);
            SetControlsHelp(player, control, L10N::GetString(0x23b1b1ae));
            int parameters[2] = { static_cast<int>(0x4fcbc7e7), static_cast<int>(0xb9a1bf4d) };
            gConsoleUIController.RequestAlertMessage(
                0x9eaa5b0a, 0x9954ebe7, parameters, 2, m_playerId,
                ControlsWarningDialogReturned,
                (void*)static_cast<unsigned long>(GetCallbackUniqueId()), NULL, true);
            return 1;
        }
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
    {
        int parameters[2] = { static_cast<int>(0x4fcbc7e7), static_cast<int>(0xb9a1bf4d) };
        gConsoleUIController.RequestAlertMessage(
            0x6ec67fa0, 0x432195fd, parameters, 2, m_playerId,
            ResetDefaultsDialogReturned,
            (void*)static_cast<unsigned long>(GetCallbackUniqueId()), NULL, true);
        return;
    }
    if (static_cast<unsigned int>(second - 5) < 0x19)
    {
        m_controls.SetSecondaryLabel(second, UIString(std::wstring(L"...")), true, false, false);
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
