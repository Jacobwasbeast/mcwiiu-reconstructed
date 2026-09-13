#include "game/ui/UIScene_MessageBox.hpp"

#include <cwchar>
#include <string>

#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/FJ_MessageBox.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIScene_MessageBox::UIScene_MessageBox(int sceneType, void* sceneData, UILayer* layer)
    : UIScene(sceneType, layer), m_callback(0), m_callbackData(0), m_titleId(0),
      m_buttonCount(0), m_alert(false)
{
    ConsoleUIController::_MessageBoxData* data =
        static_cast<ConsoleUIController::_MessageBoxData*>(sceneData);
    initialiseMovie();
    m_titleId = data->titleId;
    m_buttonCount = data->parameterCount;
    m_alert = data->blocking;
    m_callback = data->callback;
    m_callbackData = data->callbackData;

    FJ_MessageBox* document = static_cast<FJ_MessageBox*>(m_movie->getRootNode()->getFuiNode());
    document->Init(m_buttonCount, data->buttonFlags);

    unsigned int button = 0;
    if (m_buttonCount > 3) {
        m_buttons[0].init(UIString(L10N::GetString(data->messageParameters[0])), 0);
        button = 1;
    }
    if (m_buttonCount > 2) {
        m_buttons[1].init(UIString(L10N::GetString(data->messageParameters[button])), button);
        ++button;
    }
    if (m_buttonCount > 1) {
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

    document->AutoResize();
    m_callbackData = data->callbackData;
    m_callback = data->callback;
    m_layer->addComponent(sceneType, static_cast<EUIScene>(0x48), 0);
    gConsoleUIController.HideChestRefillAnimation();
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_MessageBox::~UIScene_MessageBox()
{
    if (gConsoleUIController.GetWinUserIndex() != static_cast<unsigned int>(m_controlFocus)) {
        gConsoleUIController.SetTooltips(m_controlFocus, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, -1, false, false);
    }
    m_layer->removeComponent(static_cast<EUIScene>(0x48));
}

unsigned int UIScene_MessageBox::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_buttons[0].setupControl(this, root, std::string("Button0"));
    m_controls.push_back(&m_buttons[0]);
    m_buttons[1].setupControl(this, root, std::string("Button1"));
    m_controls.push_back(&m_buttons[1]);
    m_buttons[2].setupControl(this, root, std::string("Button2"));
    m_controls.push_back(&m_buttons[2]);
    m_buttons[3].setupControl(this, root, std::string("Button3"));
    m_controls.push_back(&m_buttons[3]);
    m_title.setupControl(this, root, std::string("Title"));
    m_controls.push_back(&m_title);
    m_content.setupControl(this, root, std::string("Content"));
    m_controls.push_back(&m_content);
    return 1;
}

std::wstring UIScene_MessageBox::getMoviePath()
{
    if (g_pApp->GetLocalPlayerCount() < 2 || m_layer->IsFullscreenGroup())
        return std::wstring(L"MessageBox");
    return std::wstring(L"MessageBoxSplit");
}

void UIScene_MessageBox::updateTooltips()
{
    int tooltip = m_alert ? 0xe21d8023 : -1;
    gConsoleUIController.SetTooltips(m_controlFocus, 0x8d6323ed, tooltip, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false,
                                     m_layer->IsFullscreenGroup());
}

void UIScene_MessageBox::handleReload()
{
    int focus = 0;
    if (getControlFocus() >= 0)
        focus = getControlFocus();
    FJ_MessageBox* document = static_cast<FJ_MessageBox*>(m_movie->getRootNode()->getFuiNode());
    document->Init(m_buttonCount, focus);
    document->AutoResize();
}

void UIScene_MessageBox::handleInput(int playerId, int action, bool repeat, bool released,
                                     bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_controlFocus, action, repeat, released, pressed);
    if (action > 3) {
        if (action < 6 || action == 0x14) {
            sendInputToMovie(action, repeat, released, pressed);
        } else if (action == 0x15 && released && m_alert) {
            navigateBack();
            if (m_callback != 0) {
                (*m_callback)(m_callbackData, playerId, static_cast<C4JStorage::EMessageResult>(3));
                handled = true;
                return;
            }
        }
    }
    handled = true;
}

bool UIScene_MessageBox::hasFocus(int sceneType)
{
    if (m_controlFocus != 0xff && gConsoleUIController.GetDRCEnabled())
        return m_hasFocus && sceneType == m_controlFocus;
    return m_hasFocus;
}

void UIScene_MessageBox::handlePress(int controlId, int)
{
    unsigned char result = 3;
    if (static_cast<unsigned int>(controlId) < 4) {
        static const unsigned char results[4] = { 4, 5, 6, 7 };
        result = results[controlId];
    }
    navigateBack();
    if (m_callback != 0)
        (*m_callback)(m_callbackData, m_controlFocus, static_cast<C4JStorage::EMessageResult>(result));
}

int UIScene_MessageBox::getSceneType()
{
    return 75;
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
