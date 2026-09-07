#include "game/ui/UIScene_SaveMessage.hpp"

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
#include "platform/wiiu/NQRNetworkManager.hpp"

UIScene_SaveMessage::UIScene_SaveMessage(int playerId, void* sceneData, UILayer* layer)
    : UIScene(playerId, layer)
{
    m_saveComplete = false;
    initialiseMovie();
    layer->addComponent(playerId, (EUIScene)66, 0);
    layer->addComponent(playerId, (EUIScene)67, 0);

    m_confirmButton.init(UIString(L10N::GetString(0xb9a1bf4d)), 0);
    gApp.SetDRCIconEnabled(true);

    if (!gApp.HasNoSpaceForSaveQuota())
    {
        m_descriptionLabel.init(UIString(L10N::GetString(0xcc4c87a5)));
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

    if (gApp.DLCInstallProcessCompleted())
    {
        gApp.StartInstallDLCProcess(0);
    }
    else if (gApp.DLCInstallPending())
    {
        gApp.StartInstallDLCProcess(1);
    }
    else
    {
        gApp.StartInstallDLCProcess(0);
    }
    m_saveComplete = false;
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_SaveMessage::~UIScene_SaveMessage()
{
    if (m_layer != 0)
    {
        m_layer->removeComponent((EUIScene)66);
        m_layer->removeComponent((EUIScene)67);
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
    gConsoleUIController.SetTooltips(0, 0x8d6323ed, -1, -1, -1, -1, -1, -1, -1, -1,
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
        gConsoleUIController.AnimateKeyPress(m_controlFocus, action, repeat, released, pressed);
        if (action == 20)
        {
            NQRNetworkManager::SafeToRespondToGameBootInvite(playerId);
            sendInputToMovie(action, repeat, released, pressed);
        }
    }
}

void UIScene_SaveMessage::handleTimerComplete(int timerId)
{
}

void UIScene_SaveMessage::handlePress(int first, int second)
{
    if (second == 0)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
        m_saveComplete = true;
        bool ignoreInvite = gApp.getIgnoreMainMenuInputDuringInvite() != 0;
        gConsoleUIController.NavigateToHomeMenu(true);
        gApp.setIgnoreMainMenuInputDuringInvite(ignoreInvite);
    }
}

int UIScene_SaveMessage::getSceneType()
{
    return 2;
}

bool UIScene_SaveMessage::hasFocus(int sceneType)
{
    return m_hasFocus;
}

unsigned int UIScene_SaveMessage::getDefaultGtcButtons()
{
    return 0;
}
