#include "game/ui/UIScene_ConnectingProgress.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/System.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FullScreenProgress.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/GameSettings.hpp"
#include "vendor/_ConnectionProgressParams.hpp"

UIScene_ConnectingProgress::UIScene_ConnectingProgress(int type, void* data, UILayer* layer)
    : UIScene(type, layer)
{
    _ConnectionProgressParams* params = static_cast<_ConnectionProgressParams*>(data);

    initialiseMovie();
    m_tipLabel.init(UIString(L""));
    m_tipLabel.setVisible(false);
    m_miniGameId = params->m_miniGameId;

    if (!g_pApp->GetGameHostOption(eGameHostOption_MiniGame))
    {
        m_miniGameLogoSet = false;
        SetLoadingScreenType(0);
    }
    else
    {
        SetLoadingScreenType(1);
        setMiniGameLogoTexture();
        m_miniGameLogoSet = true;
    }
    layer->addComponent(type, EUIScene_Panorama, NULL);
    layer->addComponent(type, EUIScene_Logo, NULL);
    m_progressControl.showBar(false);
    m_progressControl.setVisible(false);

    if (params->m_stringId == -1)
        m_titleLabel.init(UIString(L""));
    else
        m_titleLabel.init(UIString(L10N::GetString(params->m_stringId)));

    m_progressControl.init(UIString(L""), 0, 0, 100, 0);
    m_confirmationButton.init(UIString(L10N::GetString(IDS_CONFIRM_OK)), 0);
    ShowConfirmationButton(false);

    m_showTooltips = params->m_showTooltips;
    m_cancelled = false;
    m_setFailTimer = params->m_setFailTimer;
    m_removeLocalPlayer = false;
    m_cancelFunc = params->m_cancelFunc;
    m_showingButton = false;
    m_cancelFuncParam = params->m_cancelFuncParam;
    m_movieLoaded = false;
    m_timerTime = params->m_timerTime;
    delete params;
}

UIScene_ConnectingProgress::~UIScene_ConnectingProgress()
{
    m_layer->removeComponent(EUIScene_Panorama);
    m_layer->removeComponent(EUIScene_Logo);
    if (m_showingButton)
    {
        tryRemovePlayer();
        if (m_removeLocalPlayer)
            removeLocalPlayer();
    }
}

void UIScene_ConnectingProgress::SetLoadingScreenType(int type)
{
    FJ_FuiNode* root = m_movie->getRootNode()->getFuiNode();
    static_cast<FJ_FullScreenProgress*>(root->asFJ_Document())->SetLoadingScreenType(type);
}

void UIScene_ConnectingProgress::setMiniGameLogoTexture()
{
    const MiniGameDef* game = Minecraft::GetInstance()->GetMiniGame();
    std::wstring icon;
    bool found;
    if (game->GetId() == eMiniGameId_Any || game->GetId() == eMiniGameId_Lobby)
    {
        icon = Minecraft::GetInstance()->GetFirstMiniGame()->getIcon();
        found = g_pApp->hasArchiveFile(icon, false);
    }
    else
    {
        icon = game->getIcon();
        found = g_pApp->hasArchiveFile(icon, false);
    }

    if (!found)
    {
        m_logoControl.setTextureName(L"", false);
    }
    else
    {
        arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(icon, false);
        if (file.data != NULL && file.length != 0)
            registerSubstitutionTexture(icon, file.data, file.length, true);
        m_logoControl.setTextureName(icon, false);
    }
}

void UIScene_ConnectingProgress::ShowConfirmationButton(bool show)
{
    FJ_FuiNode* root = m_movie->getRootNode()->getFuiNode();
    static_cast<FJ_FullScreenProgress*>(root->asFJ_Document())->ShowConfirmationButton(show);
}

unsigned int UIScene_ConnectingProgress::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();

    m_progressControl.setupControl(this, root, "ProgressBar");
    m_controls.push_back(&m_progressControl);
    m_titleLabel.setupControl(this, root, "Title");
    m_controls.push_back(&m_titleLabel);
    m_tipLabel.setupControl(this, root, "Tip");
    m_controls.push_back(&m_tipLabel);
    m_gameModeIconControl.setupControl(this, root, "GMIcon");
    m_controls.push_back(&m_gameModeIconControl);
    m_logoControl.setupControl(this, m_gameModeIconControl.getFuiRenderNode(), "MG_Logo");
    m_controls.push_back(&m_logoControl);
    m_confirmationButton.setupControl(this, root, "Confirm");
    m_controls.push_back(&m_confirmationButton);
    m_timerControl.setupControl(this, root, "Timer");
    m_controls.push_back(&m_timerControl);
    return 1;
}

void UIScene_ConnectingProgress::tryRemovePlayer()
{
    m_showingButton = false;
    if (m_playerId != gProfile->GetPrimaryPad() && CGameNetworkManager::sInstance.IsInSession())
    {
        m_removeLocalPlayer = true;
        return;
    }
    gConsoleUIController.NavigateToHomeMenu(false);
}

void UIScene_ConnectingProgress::removeLocalPlayer()
{
    m_removeLocalPlayer = false;
    Minecraft::GetInstance()->removeLocalPlayerIdx(m_playerId);
    gProfile->SignOutUser(m_playerId);
}

void UIScene_ConnectingProgress::tick()
{
    UIScene::tick();
    if (m_removeLocalPlayer)
        removeLocalPlayer();

    if (!m_miniGameLogoSet && m_movie != NULL &&
        Minecraft::GetInstance()->GetMiniGame()->GetId() != eMiniGameId_Any)
    {
        SetLoadingScreenType(1);
        setMiniGameLogoTexture();
        m_miniGameLogoSet = true;
        m_miniGameId = Minecraft::GetInstance()->GetMiniGame()->GetId();
        g_pApp->SetFirstMiniGameType(m_miniGameId);
    }
}

void UIScene_ConnectingProgress::handleReload()
{
    if (g_pApp->GetGameHostOption(eGameHostOption_MiniGame))
    {
        SetLoadingScreenType(1);
        setMiniGameLogoTexture();
        m_miniGameLogoSet = true;
        return;
    }
    m_miniGameLogoSet = false;
    SetLoadingScreenType(0);
}

void UIScene_ConnectingProgress::updateTooltips()
{
    int cancelTooltip = m_showTooltips ? static_cast<int>(IDS_TOOLTIPS_CANCEL_JOIN) : -1;
    gConsoleUIController.SetTooltips(m_playerId, -1, cancelTooltip, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_ConnectingProgress::handleGainFocus(bool navigatedBack)
{
    if (!navigatedBack && m_setFailTimer)
        addTimer(0, m_timerTime);
}

void UIScene_ConnectingProgress::handleLoseFocus()
{
    const int timerId = 0;
    m_timers[timerId];
    System::processTimeInMilliSecs();
    m_timers[timerId];
    killTimer(timerId);
}

void UIScene_ConnectingProgress::handleTimerComplete(int)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (!minecraft->mConnectionFailed[m_playerId] && CGameNetworkManager::sInstance.IsInSession())
        return;

    int message;
    switch (minecraft->mDisconnectReasons[m_playerId])
    {
    case DisconnectPacket::eDisconnectReason_Quit:                 message = 0x5b106d79; break;
    case DisconnectPacket::eDisconnectReason_LoginTimeout:         message = 0x434f7b7f; break;
    case DisconnectPacket::eDisconnectReason_KeepAliveTimeout:     message = 0xad3e1729; break;
    case DisconnectPacket::eDisconnectReason_Kicked:               message = 0xf3a95051; break;
    case DisconnectPacket::eDisconnectReason_GameFull:             message = 0xb86be603; break;
    case DisconnectPacket::eDisconnectReason_HostOutOfDate:        message = 0xb59cada7; break;
    case DisconnectPacket::eDisconnectReason_ClientOutOfDate:      message = 0xce44990b; break;
    case DisconnectPacket::eDisconnectReason_NotFriendsWithHost:   message = 0xfb7f0496; break;
    case DisconnectPacket::eDisconnectReason_NotFriendsWithPlayer: message = 0x4d669d55; break;
    case DisconnectPacket::eDisconnectReason_AgeRestricted:        message = 0x3103be06; break;
    case DisconnectPacket::eDisconnectReason_StorageError:         message = 0x98521f17; break;
    case DisconnectPacket::eDisconnectReason_ContentNotOwned:      message = 0xec3b3da6; break;
    default:                                                       message = IDS_CONNECTION_LOST_SERVER; break;
    }

    if (m_playerId != gProfile->GetPrimaryPad() && CGameNetworkManager::sInstance.IsInSession())
    {
        ShowConfirmationButton(true);
        m_showingButton = true;
        m_titleLabel.setLabel(UIString(L10N::GetString(IDS_CONNECTION_FAILED)), false, false);
        m_progressControl.setLabel(UIString(L10N::GetString(message)), false, false);
        m_progressControl.setVisible(true);
        m_timerControl.setVisible(false);
        return;
    }

    if (!m_cancelled)
    {
        int parameter = IDS_CONFIRM_OK;
        gConsoleUIController.RequestErrorMessage(IDS_CONNECTION_FAILED, message, &parameter, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
    }
    else
    {
        gApp.SetDisconnectReason(static_cast<DisconnectPacket::eDisconnectReason>(0x21));
    }
    gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_ExitWorld, reinterpret_cast<void*>(1));
}

std::wstring UIScene_ConnectingProgress::getMoviePath()
{
    if (g_pApp->GetLocalPlayerCount() < 2 || m_layer->IsFullscreenGroup())
        return L"FullscreenProgress";
    return L"FullscreenProgressSplit";
}

void UIScene_ConnectingProgress::handleInput(int playerId, int action, bool repeat,
                                             bool pressed, bool released, bool& handled)
{
    if (!m_showTooltips)
    {
        if (!m_showingButton || action != EControllerActions_MenuOk)
            return;
    }
    else
    {
        gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
        switch (action)
        {
        case EControllerActions_MenuUp:
        case EControllerActions_MenuDown:
        case EControllerActions_MenuOk:
            break;
        case EControllerActions_MenuCancel:
            if (m_cancelFunc != NULL)
                m_cancelFunc(m_cancelFuncParam);
            handled = true;
            m_cancelled = true;
            return;
        default:
            return;
        }
    }

    if (pressed)
        sendInputToMovie(action, repeat, pressed, released);
}

void UIScene_ConnectingProgress::handlePress(int controlId, int)
{
    if (controlId == 0 && m_showingButton)
        tryRemovePlayer();
}

int UIScene_ConnectingProgress::getSceneType()
{
    return EUIScene_ConnectingProgress;
}
