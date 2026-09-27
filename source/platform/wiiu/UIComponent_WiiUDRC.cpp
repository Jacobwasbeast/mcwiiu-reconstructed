#include "platform/wiiu/UIComponent_WiiUDRC.hpp"
#include "platform/wiiu/FJ_WiiUDRC.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiFile.hpp"
#include "platform/wiiu/CInput.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"

extern CConsoleMinecraftApp gApp;
extern ConsoleUIController gConsoleUIController;

UIComponent_WiiUDRC::UIComponent_WiiUDRC(int sceneType, void* param, UILayer* layer)
    : UIScene(sceneType, layer)
{
    initialiseMovie();
    m_iconX = -1;
    m_iconY = -1;
    m_iconWidth = -1;
    m_iconHeight = -1;
    m_drcOutputEnabled = true;
    m_drcIconEnabled = false;
    m_touchX = 0;
    m_touchY = 0;
    m_touchingIcon = false;
    m_touchPressed = false;
    SetEnableBackground(true);
    SetDRCIcon(false, false);
    GetIconWidth();
    GetIconHeight();
    GetIconX();
    GetIconY();
}

UIComponent_WiiUDRC::~UIComponent_WiiUDRC()
{
}

int UIComponent_WiiUDRC::GetIconX()
{
    if (m_iconX < 1)
        m_iconX = static_cast<FJ_WiiUDRC*>(m_movie->getRootNode()->getFuiNode())->GetIconX();
    return m_iconX;
}

int UIComponent_WiiUDRC::GetIconWidth()
{
    if (m_iconWidth < 1)
        m_iconWidth = static_cast<FJ_WiiUDRC*>(m_movie->getRootNode()->getFuiNode())->GetIconWidth();
    return m_iconWidth;
}

int UIComponent_WiiUDRC::GetIconY()
{
    if (m_iconY < 1)
        m_iconY = static_cast<FJ_WiiUDRC*>(m_movie->getRootNode()->getFuiNode())->GetIconY();
    return m_iconY;
}

int UIComponent_WiiUDRC::GetIconHeight()
{
    if (m_iconHeight < 1)
        m_iconHeight = static_cast<FJ_WiiUDRC*>(m_movie->getRootNode()->getFuiNode())->GetIconHeight();
    return m_iconHeight;
}

void UIComponent_WiiUDRC::handleDRCTouchInput(unsigned int playerIndex)
{
    unsigned short* touchData = (unsigned short*)g_pInput->GetTouchPadData(
        playerIndex, C_4JInput::ETouchType_0, true);
    bool pressed = false;
    unsigned int x = m_touchX;
    unsigned int y = m_touchY;
    if (touchData != NULL && touchData[2] == 1 && touchData[3] == 0)
    {
        x = touchData[0];
        y = touchData[1];
        m_touchX = x;
        m_touchY = y;
        pressed = true;
    }
    else if (!m_touchingIcon)
    {
        m_touchPressed = false;
        m_touchingIcon = false;
        return;
    }
    if (!gApp.GetDRCIconEnabled())
        return;
    if (GetIconX() < static_cast<int>(x) && static_cast<int>(x) < GetIconX() + GetIconWidth() &&
        GetIconY() < static_cast<int>(y) && static_cast<int>(y) < GetIconY() + GetIconHeight())
    {
        if (pressed)
        {
            if (!m_touchingIcon)
            {
                m_touchingIcon = true;
                m_touchPressed = true;
                return;
            }
        }
        else if (m_touchingIcon && m_touchPressed)
            gApp.SetDRCOuputEnabled(!gApp.GetDRCOutputEnabled());
    }
    m_touchingIcon = pressed;
}

std::wstring UIComponent_WiiUDRC::getMoviePath()
{
    return std::wstring(L"WiiUDRC");
}

void UIComponent_WiiUDRC::SetEnableBackground(bool enable)
{
    static_cast<FJ_WiiUDRC*>(m_movie->getRootNode()->getFuiNode())->ShowDRCBackground(enable);
}

void UIComponent_WiiUDRC::SetDRCIcon(bool enable, bool connected)
{
    static_cast<FJ_WiiUDRC*>(m_movie->getRootNode()->getFuiNode())->ShowDRCIcon(enable, connected);
}

void UIComponent_WiiUDRC::tick()
{
    UIScene::tick();
    if (!gConsoleUIController.IsReloadingSkin())
    {
        bool iconEnabled = gApp.GetDRCIconEnabled();
        bool outputEnabled = gApp.GetDRCOutputEnabled();
        if (iconEnabled != m_drcIconEnabled || outputEnabled != m_drcOutputEnabled)
        {
            SetDRCIcon(iconEnabled, !outputEnabled);
            if (outputEnabled != m_drcOutputEnabled)
                SetEnableBackground(!outputEnabled);
            m_drcOutputEnabled = outputEnabled;
            m_drcIconEnabled = iconEnabled;
        }
    }
}

void UIComponent_WiiUDRC::handleReload()
{
    SetDRCIcon(m_drcIconEnabled, !m_drcOutputEnabled);
    SetEnableBackground(!m_drcOutputEnabled);
}

int UIComponent_WiiUDRC::getSceneType()
{
    return EUIScene_WiiUDRC;
}

bool UIComponent_WiiUDRC::stealsFocus()
{
    return false;
}

bool UIComponent_WiiUDRC::hasFocus(int playerIndex)
{
    return false;
}

bool UIComponent_WiiUDRC::hidesLowerScenes()
{
    return false;
}
