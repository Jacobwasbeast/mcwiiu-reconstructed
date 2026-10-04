#include "game/ui/UIScene_MessageBox.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <string>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/FJ_MessageBox.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/GameSettings.hpp"
#include "platform/wiiu/CProfile.hpp"

UIScene_MessageBox::UIScene_MessageBox(int sceneType, void* sceneData, UILayer* layer)
    : UIScene(sceneType, layer)
{
    ConsoleUIController::_MessageBoxData* data =
        static_cast<ConsoleUIController::_MessageBoxData*>(sceneData);
    initialiseMovie();
    m_titleId = data->titleId;
    m_buttonCount = data->parameterCount;
    m_alert = data->blocking;

    static_cast<FJ_MessageBox*>(m_movie->getRootNode()->getFuiNode())->Init(data->parameterCount, data->buttonFlags);

    unsigned int button = 0;
    if (data->parameterCount > 3) {
        m_buttons[0].init(UIString(L10N::GetString(data->messageParameters[0])), 0);
        button = 1;
    }
    if (data->parameterCount > 2) {
        m_buttons[1].init(UIString(L10N::GetString(data->messageParameters[button])), button);
        ++button;
    }
    if (data->parameterCount > 1) {
        m_buttons[2].init(UIString(L10N::GetString(data->messageParameters[button])), button);
        ++button;
    }
    m_buttons[3].init(UIString(L10N::GetString(data->messageParameters[button])), button);
    m_title.init(UIString(L10N::GetString(data->titleId)));

    if (data->overrideText == 0) {
        m_content.init(UIString(L10N::GetString(data->messageId)));
    } else {
        wchar_t text[512];
        swprintf(text, 512, data->overrideText, L10N::GetString(data->messageId));
        m_content.init(UIString(text));
    }

    static_cast<FJ_MessageBox*>(m_movie->getRootNode()->getFuiNode())->AutoResize();
    m_callbackData = data->callbackData;
    m_callback = data->callback;
    layer->addComponent(sceneType, EUIScene_MenuBackground, 0);
    gConsoleUIController.HideChestRefillAnimation();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_MessageBox::~UIScene_MessageBox()
{
    if (gProfile->GetPrimaryPad() != m_playerId) {
        gConsoleUIController.SetTooltips(m_playerId, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, -1, false, false);
    }
    m_layer->removeComponent(EUIScene_MenuBackground);
}

unsigned int UIScene_MessageBox::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    {
        std::string name("Button0");
        m_buttons[0].setupControl(this, root, name);
    }
    m_controls.push_back(&m_buttons[0]);
    {
        std::string name("Button1");
        m_buttons[1].setupControl(this, root, name);
    }
    m_controls.push_back(&m_buttons[1]);
    {
        std::string name("Button2");
        m_buttons[2].setupControl(this, root, name);
    }
    m_controls.push_back(&m_buttons[2]);
    {
        std::string name("Button3");
        m_buttons[3].setupControl(this, root, name);
    }
    m_controls.push_back(&m_buttons[3]);
    {
        std::string name("Title");
        m_title.setupControl(this, root, name);
    }
    m_controls.push_back(&m_title);
    {
        std::string name("Content");
        m_content.setupControl(this, root, name);
    }
    m_controls.push_back(&m_content);
    return 1;
}

std::wstring UIScene_MessageBox::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1 && !m_layer->IsFullscreenGroup())
        return std::wstring(L"MessageBoxSplit");
    return std::wstring(L"MessageBox");
}

void UIScene_MessageBox::updateTooltips()
{
    int tooltip = m_alert ? IDS_TOOLTIPS_CANCEL : -1;
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, tooltip, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false,
                                     m_layer->IsFullscreenGroup());
}

void UIScene_MessageBox::handleReload()
{
    int focus = 0;
    if (getControlFocus() >= 0)
        focus = getControlFocus();
    static_cast<FJ_MessageBox*>(m_movie->getRootNode()->getFuiNode())->Init(m_buttonCount, focus);
    static_cast<FJ_MessageBox*>(m_movie->getRootNode()->getFuiNode())->AutoResize();
}

void UIScene_MessageBox::handleInput(int playerId, int action, bool repeat, bool pressed,
                                     bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    case EControllerActions_MenuCancel:
        if (pressed && m_alert)
        {
            navigateBack();
            if (m_callback != 0)
                (*m_callback)(m_callbackData, playerId, C4JStorage::EMessageResult_Cancelled);
        }
        break;
    }
    handled = true;
}

bool UIScene_MessageBox::hasFocus(int playerId)
{
    if (m_playerId != 0xff && gProfile->IsSignedIn(m_playerId))
        return m_hasFocus && playerId == m_playerId;
    return m_hasFocus;
}

void UIScene_MessageBox::handlePress(int controlId, int)
{
    C4JStorage::EMessageResult result = C4JStorage::EMessageResult_Cancelled;
    if (static_cast<unsigned int>(controlId) <= 3) {
        static const unsigned char results[4] = {
            C4JStorage::EMessageResult_Accept, C4JStorage::EMessageResult_Decline,
            C4JStorage::EMessageResult_ThirdOption, C4JStorage::EMessageResult_FourthOption };
        result = static_cast<C4JStorage::EMessageResult>(results[controlId]);
    }
    navigateBack();
    if (m_callback != 0)
        (*m_callback)(m_callbackData, m_playerId, result);
}

int UIScene_MessageBox::getSceneType()
{
    return EUIScene_MessageBox;
}

unsigned int UIScene_MessageBox::getSubSceneType() const
{
    return static_cast<unsigned int>(m_titleId);
}

bool UIScene_MessageBox::hidesLowerScenes()
{
    return false;
}

bool UIScene_MessageBox::blocksInput()
{
    return true;
}
