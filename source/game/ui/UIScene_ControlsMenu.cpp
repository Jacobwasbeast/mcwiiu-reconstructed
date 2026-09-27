#include "game/ui/UIScene_ControlsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <string>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/ui/FJ_ControlsMenu.hpp"
#include "game/ui/UIControl_List.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/GameSettings.hpp"

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
        GameSettings::ActionGameSettings(m_playerId, eGameSetting_ControlMappings);
    }
}

std::wstring UIScene_ControlsMenu::GetControlsString(int player, EControllerActions action, bool& mapped)
{
    std::wstring result;
    if (GameSettings::GetInputButton(player, false, action) == 0 && action >= EControllerActions_Jump && action <= EControllerActions_LookBehind)
    {
        result = L" ";
        result += L10N::GetString(IDS_CONTROLS_NONE);
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
        HtmlString html(help, eMinecraftColour_HTMLColor_White, false, false, 0);
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
                  MiniGameDef::GetCustomGameModeById(eMiniGameId_Battle, true)
                      ->getVersionString().c_str(),
                  L"");
    m_version.init(UIString(versionText));

    m_flags[0] = isFlying;

    bool mapped;
    std::wstring controlString;
    m_controlsList.AddNewButton(L10N::GetString(IDS_RESET_TO_DEFAULTS), EControl_ResetToDefaults, true);

    m_controlsList.AddNewLabel(L" ", EControl_Label);
    m_controlsList.AddNewLabel(L10N::GetString(0x8eb977d2), EControl_Label);

    m_controlsList.AddNewCheckbox(L10N::GetString(IDS_INVERT_LOOK), EControl_InvertLook,
                              GameSettings::GetGameSettings(player, eGameSetting_InvertLook) != 0);
    m_controlsList.AddNewCheckbox(L10N::GetString(IDS_SOUTHPAW), EControl_SouthPaw,
                              GameSettings::GetGameSettings(player, eGameSetting_SouthPaw) != 0);

    controlString = GetControlsString(EControl_Look, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_LOOK), controlString, mapped, EControl_Look);
    controlString = GetControlsString(EControl_Move, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_MOVE), controlString, mapped, EControl_Move);

    m_controlsList.AddNewLabel(L" ", EControl_Label);
    m_controlsList.AddNewLabel(L10N::GetString(0x19988bba), EControl_Label);

    controlString = GetControlsString(EControl_Use, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_USE), controlString, mapped, EControl_Use);
    controlString = GetControlsString(EControl_Action, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_ACTION), controlString, mapped, EControl_Action);
    controlString = GetControlsString(EControl_Jump, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(isFlying ? 0x774c60ee : IDS_CONTROLS_JUMP), controlString, mapped, EControl_Jump);
    controlString = GetControlsString(EControl_Drop, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_DROP), controlString, mapped, EControl_Drop);
    controlString = GetControlsString(EControl_Inventory, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_INVENTORY), controlString, mapped, EControl_Inventory);
    controlString = GetControlsString(EControl_Crafting, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_CRAFTING), controlString, mapped, EControl_Crafting);
    controlString = GetControlsString(EControl_CycleHeldItemLeft, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(0xc513139b), controlString, mapped, EControl_CycleHeldItemLeft);
    controlString = GetControlsString(EControl_CycleHeldItemRight, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(0x72ab6b50), controlString, mapped, EControl_CycleHeldItemRight);
    controlString = GetControlsString(EControl_Sneak, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(isFlying ? 0x8efa4866 : IDS_CONTROLS_SNEAK), controlString, mapped, EControl_Sneak);
    controlString = GetControlsString(EControl_Sprint, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_SPRINT), controlString, mapped, EControl_Sprint);
    controlString = GetControlsString(EControl_PickBlock, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_PICKBLOCK), controlString, mapped, EControl_PickBlock);
    controlString = GetControlsString(EControl_ChangeCameraMode, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(0x88874831), controlString, mapped, EControl_ChangeCameraMode);

    m_controlsList.AddNewLabel(L" ", EControl_Label);
    m_controlsList.AddNewLabel(L10N::GetString(0x4aebca1a), EControl_Label);

    controlString = GetControlsString(EControl_FlyLeft, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(0x880870a4), controlString, mapped, EControl_FlyLeft);
    controlString = GetControlsString(EControl_FlyRight, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(0xdb0d0c07), controlString, mapped, EControl_FlyRight);
    controlString = GetControlsString(EControl_FlyUp, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(0xbfc2db62), controlString, mapped, EControl_FlyUp);
    controlString = GetControlsString(EControl_FlyDown, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(0x48a6649d), controlString, mapped, EControl_FlyDown);

    m_controlsList.AddNewLabel(L" ", EControl_Label);
    m_controlsList.AddNewLabel(L10N::GetString(0xd62470c6), EControl_Label);

    controlString = GetControlsString(EControl_ConfirmReady, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(0x2edff749), controlString, mapped, EControl_ConfirmReady);
    controlString = GetControlsString(EControl_Vote, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(0xc9f98a25), controlString, mapped, EControl_Vote);

    m_controlsList.AddNewLabel(L" ", EControl_Label);
    m_controlsList.AddNewLabel(L10N::GetString(0x1fd34dbd), EControl_Label);

    controlString = GetControlsString(EControl_SpectatePrevious, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(0xbc932ff8), controlString, mapped, EControl_SpectatePrevious);
    controlString = GetControlsString(EControl_SpectateNext, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_SPECTATE_NEXT), controlString, mapped, EControl_SpectateNext);
    controlString = GetControlsString(EControl_SpectateNoise, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_SPECTATE_NOISE), controlString, mapped, EControl_SpectateNoise);
    controlString = GetControlsString(EControl_SpectateCancel, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_CONTROLS_SPECTATE_CANCEL), controlString, mapped, EControl_SpectateCancel);

    m_controlsList.AddNewLabel(L" ", EControl_Label);
    m_controlsList.AddNewLabel(L10N::GetString(0x48a5d3ba), EControl_Label);

    controlString = GetControlsString(EControl_RestartSection, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_TOOLTIP_RESTART_SECTION), controlString, mapped, EControl_RestartSection);
    controlString = GetControlsString(EControl_RestartRace, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_TOOLTIP_RESTART_RACE), controlString, mapped, EControl_RestartRace);
    controlString = GetControlsString(EControl_LookBehind, mapped);
    m_controlsList.AddNewControllerButton(L10N::GetString(IDS_TOOLTIP_LOOK_BEHIND), controlString, mapped, EControl_LookBehind);

    m_controlsList.EnableItem(EControl_Look, false, true);
    m_controlsList.EnableItem(EControl_Move, false, true);

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
    fuiRenderNode* root = m_movie->getRootNode();
    m_version.setupControl(this, root, std::string("Version"));
    m_controls.push_back(&m_version);
    m_controlsList.setupControl(this, root, std::string("ControlsMultiList"));
    m_controls.push_back(&m_controlsList);
    return 1;
}

void UIScene_ControlsMenu::updateTooltips()
{
    if (m_flags[5])
    {
        gConsoleUIController.SetTooltips(m_playerId, 0, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, true, false);
    }
    else if (m_controlsList.getItemCount() > 0)
    {
        int tooltip = m_controlsList.IsEnabled(m_controlsList.getCurrentSelection()) ? IDS_TOOLTIPS_SELECT : -1;
        gConsoleUIController.SetTooltips(m_playerId, tooltip, IDS_TOOLTIPS_BACK, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, true, false);
    }
}

void UIScene_ControlsMenu::updateComponents()
{
    if (m_layer)
        m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

unsigned char UIScene_ControlsMenu::GetControlsAction(EControl control)
{
    if (control < EControl_Look || control > EControl_LookBehind)
        return '=';
    if (control == EControl_Look)
        return GameSettings::GetGameSettings(m_playerId, eGameSetting_SouthPaw) ? 0x1b : 0x1d;
    if (control == EControl_Move)
        return GameSettings::GetGameSettings(m_playerId, eGameSetting_SouthPaw) ? 0x1d : 0x1b;
    static const unsigned char actions[] = {
        EControllerActions_Use, EControllerActions_Action, EControllerActions_Jump, EControllerActions_Drop,
        EControllerActions_Inventory, EControllerActions_Crafting, EControllerActions_CycleHeldItemLeft,
        EControllerActions_CycleHeldItemRight, EControllerActions_Sneak, EControllerActions_Sprint,
        EControllerActions_PickBlock, EControllerActions_ChangeCameraMode, EControllerActions_FlyLeft,
        EControllerActions_FlyRight, EControllerActions_FlyUp, EControllerActions_FlyDown,
        EControllerActions_SpectatePrevious, EControllerActions_SpectateNext, EControllerActions_SpectateNoise,
        EControllerActions_SpectateCancel, EControllerActions_ConfirmReady, EControllerActions_Vote,
        EControllerActions_RestartSection, EControllerActions_RestartRace, EControllerActions_LookBehind,
    };
    return actions[control - EControl_Use];
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
    m_controlsList.SetCheckboxValue(EControl_InvertLook, GameSettings::GetGameSettings(player, eGameSetting_InvertLook) != 0, false);
    m_controlsList.SetCheckboxValue(EControl_SouthPaw, GameSettings::GetGameSettings(player, eGameSetting_SouthPaw) != 0, false);
    bool mapped;
    std::wstring controlString;
    for (int i = EControl_Look; i <= EControl_LookBehind; ++i)
    {
        controlString = GetControlsString(static_cast<EControl>(i), mapped);
        m_controlsList.SetSecondaryLabel(i, UIString(controlString), true, mapped, false);
    }
    m_flags[1] = 0;
}

int UIScene_ControlsMenu::ResetControlsWarningDialogReturned(void* context, int pad,
                                                                        C4JStorage::EMessageResult result)
{
    UIScene_ControlsMenu* scene = static_cast<UIScene_ControlsMenu*>(
        gConsoleUIController.GetSceneFromCallbackId((unsigned int)context));
    if (scene != NULL)
    {
        if (result == C4JStorage::EMessageResult_Accept)
        {
            GameSettings::SetDefaultControls(gStorage->GetDashboardProfileSettings(scene->m_playerId),
                                             scene->m_playerId, true);
            GameSettings::CheckGameSettingsChanged(true, pad);
            scene->UpdateControls(scene->m_playerId);
        }
        else if (result == C4JStorage::EMessageResult_Decline ||
                 result == C4JStorage::EMessageResult_Cancelled)
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
    for (int control = EControl_Use; control <= EControl_LookBehind; control += 2)
    {
        int first = GameSettings::GetActionMapping(player, GetControlsAction(static_cast<EControl>(control)) - EControllerActions_Jump);
        int second = GameSettings::GetActionMapping(player, GetControlsAction(static_cast<EControl>(control + 1)) - EControllerActions_Jump);
        if ((first & 0xf) == 0xd || (first & 0xf) == 0xe || (second & 0xf) == 0xd ||
            (second & 0xf) == 0xe || ((first >> 4) & 0xff) == 0xd ||
            ((first >> 4) & 0xff) == 0xe || ((second >> 4) & 0xff) == 0xd ||
            ((second >> 4) & 0xff) == 0xe)
        {
            int parameters[2] = { static_cast<int>(IDS_CONFIRM_CANCEL), static_cast<int>(IDS_CONFIRM_OK) };
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
    if (m_flags[3])
    {
        SetControlsHelp(m_playerId, m_controlsList.getCurrentSelection(), L10N::GetString(0x4add6081));
        m_flags[3] = 0;
    }
    if (m_flags[4])
    {
        SetControlsHelp(m_playerId, 0, L"");
        m_flags[4] = 0;
    }
    UIScene::tick();
}

std::wstring UIScene_ControlsMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() < 2 ? L"Controls" : L"ControlsSplit";
}

void UIScene_ControlsMenu::handleCheckboxElementToggled(int first, int second, bool checked)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (second == 1)
        GameSettings::SetGameSettings(m_playerId, eGameSetting_InvertLook, checked);
    else if (second == 2)
    {
        if ((GameSettings::GetGameSettings(m_playerId, eGameSetting_SouthPaw) != 0) != checked)
            m_flags[1] = 1;
        GameSettings::SetGameSettings(m_playerId, eGameSetting_SouthPaw, checked);
    }
}

int UIScene_ControlsMenu::ResetDefaultsDialogReturned(void* context, int pad,
                                                                C4JStorage::EMessageResult result)
{
    UIScene_ControlsMenu* scene = static_cast<UIScene_ControlsMenu*>(
        gConsoleUIController.GetSceneFromCallbackId((unsigned int)context));
    if (scene != NULL && result == C4JStorage::EMessageResult_Decline)
    {
        GameSettings::SetDefaultControls(gStorage->GetDashboardProfileSettings(scene->m_playerId),
                                         scene->m_playerId, true);
        GameSettings::CheckGameSettingsChanged(true, pad);
        scene->UpdateControls(scene->m_playerId);
    }
    return 0;
}

int UIScene_ControlsMenu::ControlsWarningDialogReturned(void* context, int pad,
                                                                  C4JStorage::EMessageResult result)
{
    UIScene_ControlsMenu* scene = static_cast<UIScene_ControlsMenu*>(
        gConsoleUIController.GetSceneFromCallbackId((unsigned int)context));
    if (scene != NULL && result == C4JStorage::EMessageResult_Accept)
    {
        GameSettings::CheckGameSettingsChanged(true, pad);
        GameSettings::ActionGameSettings(pad, eGameSetting_ControlMappings);
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
    int mapping = GameSettings::GetActionMapping(player, action - EControllerActions_Jump);

    int groupFirst;
    int groupLast;
    if (control >= EControl_Use && control <= EControl_FlyDown)
    {
        groupFirst = EControl_Use;
        groupLast = EControl_FlyDown;
    }
    else if (control >= EControl_ConfirmReady && control <= EControl_Vote)
    {
        groupFirst = EControl_ConfirmReady;
        groupLast = EControl_Vote;
    }
    else if (control >= EControl_SpectatePrevious && control <= EControl_SpectateCancel)
    {
        groupFirst = EControl_SpectatePrevious;
        groupLast = EControl_SpectateCancel;
    }
    else if (control >= EControl_RestartSection && control <= EControl_LookBehind)
    {
        groupFirst = EControl_RestartSection;
        groupLast = EControl_LookBehind;
    }
    else
    {
        return 0;
    }

    int conflicting = -1;
    for (int candidate = groupFirst; candidate <= groupLast; ++candidate)
    {
        if (candidate == control)
            continue;
        unsigned char candidateAction = GetControlsAction(static_cast<EControl>(candidate));
        if (GameSettings::GetActionMapping(player, candidateAction - EControllerActions_Jump) == mapping)
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
    text += L10N::GetString(IDS_CONTROLS_NONE);
    m_controlsList.SetSecondaryLabel(conflicting, UIString(text), true, false, false);
    m_controlsList.HighlightItem(conflicting, false);
    SetControlsHelp(player, conflicting, L10N::GetString(IDS_CONTROLS_BUTTON_ASSIGN_CONFLICT));
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
            int selection = m_controlsList.getCurrentSelection();
            std::wstring controlString = GetControlsString((EControl)selection, mapped);
            m_controlsList.SetSecondaryLabel(selection, UIString(controlString), true, mapped, false);
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
        int selection = m_controlsList.getCurrentSelection();
        GameSettings::SetActionMapping(player, GetControlsAction((EControl)selection) - 0x20, mapping);
        GameSettings::SetSettingsChanged(player);
        m_flags[2] = 1;

        bool mapped = true;
        std::wstring controlString = GetControlsString((EControl)selection, mapped);
        m_controlsList.SetSecondaryLabel(selection, UIString(controlString), true, mapped, false);
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
    int selection = m_controlsList.getCurrentSelection();
    m_controlsList.SetSecondaryLabel(selection, UIString(text), true, true, false);
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
            m_controlsList.SetSecondaryLabel(control, UIString(std::wstring(L10N::GetString(IDS_CONTROLS_NONE))),
                                         true, false, false);
            m_controlsList.HighlightItem(control, false);
            SetControlsHelp(player, control, L10N::GetString(IDS_CONTROLS_BUTTON_ASSIGN_CONFLICT));
            int parameters[2] = { static_cast<int>(IDS_CONFIRM_CANCEL), static_cast<int>(IDS_CONFIRM_OK) };
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
    switch (action)
    {
    case EControllerActions_MenuOk:
        if (released)
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        if (!repeat)
            sendInputToMovie(action, false, released, pressed);
        break;
    case EControllerActions_MenuCancel:
        if (released)
        {
            if (!CheckControlsOnExit(player))
            {
                GameSettings::CheckGameSettingsChanged(true, player);
                GameSettings::ActionGameSettings(player, eGameSetting_ControlMappings);
                m_flags[2] = 0;
                navigateBack();
            }
            break;
        }
        m_flags[3] = 1;
        sendInputToMovie(action, repeat, released, pressed);
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
        m_flags[3] = 1;
        sendInputToMovie(action, repeat, released, pressed);
        break;
    }
}

void UIScene_ControlsMenu::handlePress(int controlId, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (childId == 0)
    {
        int parameters[2] = { static_cast<int>(IDS_CONFIRM_CANCEL), static_cast<int>(IDS_CONFIRM_OK) };
        gConsoleUIController.RequestAlertMessage(
            IDS_DEFAULTS_CONTROLS_TITLE, IDS_DEFAULTS_CONTROLS_TEXT, parameters, 2, m_playerId,
            ResetDefaultsDialogReturned,
            (void*)static_cast<unsigned long>(GetCallbackUniqueId()), NULL, true);
        return;
    }
    if (static_cast<unsigned int>(childId - 5) < 0x19)
    {
        m_controlsList.SetSecondaryLabel(childId, UIString(std::wstring(L"...")), true, false, false);
        m_flags[5] = 1;
        m_flags[3] = 1;
        updateTooltips();
    }
}

void UIScene_ControlsMenu::handleFocusChange(int focus, int childFocus)
{
    m_controlsList.updateChildFocus(childFocus);
    updateTooltips();
}

int UIScene_ControlsMenu::getSceneType()
{
    return EUIScene_ControlsMenu;
}
