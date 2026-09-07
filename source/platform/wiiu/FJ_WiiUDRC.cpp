#include "platform/wiiu/FJ_WiiUDRC.hpp"

#include <cmath>
#include <string>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "platform/wiiu/CInput.hpp"

extern CConsoleMinecraftApp gApp;

static unsigned char s_drcConnected = 1;

void HandleDRCStatusChanges()
{
    unsigned char drcConnected = g_pInput->IsDRCConnected();
    if (drcConnected == s_drcConnected)
        return;

    if (drcConnected == 0)
    {
        gApp.SetDRCOuputEnabled(false);
    }
    else if (gApp.GetDRCIconEnabled() != 0)
    {
        gApp.SetDRCOuputEnabled(true);
    }

    s_drcConnected = drcConnected;
}

static int FJ_WiiUDRC_Round(float value)
{
    return (int)std::floor(value + 0.5f);
}

FJ_WiiUDRC::FJ_WiiUDRC(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode), m_tileCount(0), m_tilesAcross(0), m_tilesDown(0), m_backgroundVisible(false)
{
    ShowDRCBackground(false);
    ShowDRCIcon(true, true);
    SetSafeZone(25, 25, 25, 25);
}

FJ_WiiUDRC::~FJ_WiiUDRC()
{
}

void FJ_WiiUDRC::UpdateTileBackground(FJ_Event*)
{
    if (m_tileCount != 0)
        return;

    FJ_FuiNode* tile = getChildByName(std::string("DRC_Background"));
    if (tile == NULL)
        return;

    float tileWidth = tile->getWidth();
    float tileHeight = tile->getHeight();
    if (tileWidth <= 0.0f || tileHeight <= 0.0f)
        return;

    m_tilesAcross = (int)std::ceil(1280.0f / tileWidth);
    m_tilesDown = (int)std::ceil(720.0f / tileHeight) + 1;
    m_tileCount = m_tilesAcross * m_tilesDown;
    if (m_tileCount > 24)
        m_tileCount = 24;
}

void FJ_WiiUDRC::_UpdateTileBackground(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
    FJ_WiiUDRC* drc = dynamic_cast<FJ_WiiUDRC*>(pNode);
    if (drc != NULL)
        drc->UpdateTileBackground(pEvent);
}

void FJ_WiiUDRC::ShowDRCBackground(bool bShow)
{
    m_backgroundVisible = bShow;
    FJ_FuiNode* background = getChildByName(std::string("DRC_Background"));
    if (background != NULL)
        background->setVisible(bShow);
}

void FJ_WiiUDRC::ShowDRCIcon(bool bOn, bool bConnected)
{
    FJ_FuiNode* onIcon = getChildByName(std::string("DRC_On_Icon"));
    if (onIcon != NULL)
        onIcon->setVisible(bOn && bConnected);

    FJ_FuiNode* offIcon = getChildByName(std::string("DRC_Off_Icon"));
    if (offIcon != NULL)
        offIcon->setVisible(bOn && !bConnected);
}

void FJ_WiiUDRC::SetSafeZone(int, int iTop, int iRight, int)
{
    float stageWidth = 1280.0f;
    float stageHeight = 720.0f;
    FJ_FuiNode* onIcon = getChildByName(std::string("DRC_On_Icon"));
    FJ_FuiNode* offIcon = getChildByName(std::string("DRC_Off_Icon"));
    float xScale = 1280.0f / stageWidth;
    float yScale = 720.0f / stageHeight;

    if (onIcon != NULL)
    {
        onIcon->setX((float)FJ_WiiUDRC_Round(-(iRight * xScale - (stageWidth - onIcon->getWidth()))));
        onIcon->setY((float)FJ_WiiUDRC_Round(-(iTop * yScale - (stageHeight - onIcon->getHeight()))));
    }
    if (offIcon != NULL)
    {
        offIcon->setX((float)FJ_WiiUDRC_Round(-(iRight * xScale - (stageWidth - offIcon->getWidth()))));
        offIcon->setY((float)FJ_WiiUDRC_Round(-(iTop * yScale - (stageHeight - offIcon->getHeight()))));
    }
}

FJ_WiiUDRC* FJ_WiiUDRC::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_WiiUDRC(pRenderNode);
}

unsigned int FJ_WiiUDRC::getType()
{
    return 0x5a;
}

int FJ_WiiUDRC::GetIconX()
{
    FJ_FuiNode* icon = getChildByName(std::string("DRC_Off_Icon"));
    float x = icon == NULL ? 0.0f : icon->getX();
    return (int)x;
}

int FJ_WiiUDRC::GetIconY()
{
    FJ_FuiNode* icon = getChildByName(std::string("DRC_On_Icon"));
    float y = icon == NULL ? 0.0f : icon->getY();
    return (int)y;
}

int FJ_WiiUDRC::GetIconWidth()
{
    FJ_FuiNode* icon = getChildByName(std::string("DRC_On_Icon"));
    float width = icon == NULL ? 0.0f : icon->getWidth();
    return (int)width;
}

int FJ_WiiUDRC::GetIconHeight()
{
    FJ_FuiNode* icon = getChildByName(std::string("DRC_On_Icon"));
    float height = icon == NULL ? 0.0f : icon->getHeight();
    return (int)height;
}

void FJ_WiiUDRC::SetAlpha(float fAlpha)
{
    FJ_FuiNode::setAlpha(fAlpha);
}
