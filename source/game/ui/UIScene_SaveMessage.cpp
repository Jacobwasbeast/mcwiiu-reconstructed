#include "game/ui/UIScene_SaveMessage.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_SaveMessage.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/NQRNetworkManager.hpp"
#include "game/core/GameSettings.hpp"

UIScene_SaveMessage::UIScene_SaveMessage(int playerId, void* sceneData, UILayer* layer)
    : UIScene(playerId, layer)
{
    m_saveComplete = false;
    initialiseMovie();
    layer->addComponent(playerId, EUIScene_Panorama, 0);
    layer->addComponent(playerId, EUIScene_Logo, 0);

    m_confirmButton.init(UIString(L10N::GetString(IDS_CONFIRM_OK)), 0);
    gApp.SetDRCIconEnabled(true);

    if (!gApp.HasNoSpaceForSaveQuota())
    {
        m_descriptionLabel.init(UIString(L10N::GetString(IDS_SAVE_ICON_MESSAGE)));
    }
    else
    {
        m_descriptionLabel.init(UIString(L10N::GetString(0x89199462)));
    }

    fuiRenderNode* root = m_movie->getRootNode();
    if (root != 0)
    {
        FJ_Document* document = root->getFuiNode()->asFJ_Document();
        if (document != 0)
        {
            static_cast<FJ_SaveMessage*>(document)->AutoResize();
        }
    }

    for (int pad = 0; pad < 4; ++pad)
    {
        bool padInUse = g_pInput->IsPadConnected(pad) != 0 || gProfile->IsSignedIn(pad);
        if (padInUse && !gApp.DLCInstallProcessCompleted() && !gApp.DLCInstallPending())
        {
            gApp.StartInstallDLCProcess(pad);
            break;
        }
    }
    m_saveComplete = false;
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_SaveMessage::~UIScene_SaveMessage()
{
    if (m_layer != 0)
    {
        m_layer->removeComponent(EUIScene_Panorama);
        m_layer->removeComponent(EUIScene_Logo);
    }
}

unsigned int UIScene_SaveMessage::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_confirmButton.setupControl(this, root, std::string("Confirm"));
    m_controls.push_back(&m_confirmButton);
    m_descriptionLabel.setupControl(this, root, std::string("Description"));
    m_controls.push_back(&m_descriptionLabel);
    return 1;
}

void UIScene_SaveMessage::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, false, false);
}

std::wstring UIScene_SaveMessage::getMoviePath()
{
    return std::wstring(L"SaveMessage");
}

void UIScene_SaveMessage::handleInput(int playerId, int action, bool repeat, bool released,
                                      bool pressed, bool& handled)
{
    if (!m_saveComplete)
    {
        gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
        if (action == EControllerActions_MenuOk)
        {
            NQRNetworkManager::SafeToRespondToGameBootInvite(playerId);
            sendInputToMovie(action, repeat, released, pressed);
        }
    }
}

void UIScene_SaveMessage::handleTimerComplete(int timerId)
{
}

void UIScene_SaveMessage::handlePress(int controlId, int)
{
    if (controlId == 0)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        m_saveComplete = true;
        bool ignoreInvite = gApp.getIgnoreMainMenuInputDuringInvite() != 0;
        gConsoleUIController.NavigateToHomeMenu(true);
        gApp.setIgnoreMainMenuInputDuringInvite(ignoreInvite);
    }
}

int UIScene_SaveMessage::getSceneType()
{
    return EUIScene_SaveMessage;
}

bool UIScene_SaveMessage::hasFocus(int sceneType)
{
    return m_hasFocus;
}

unsigned int UIScene_SaveMessage::getDefaultGtcButtons()
{
    return 0;
}
