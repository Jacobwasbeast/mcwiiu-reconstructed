#ifndef PLATFORM_WIIU_FJ_WIIUDRC_HPP
#define PLATFORM_WIIU_FJ_WIIUDRC_HPP

#include "game/ui/FJ_FuiNode.hpp"

class FJ_Event;
class fuiRenderNode;

class FJ_WiiUDRC : public FJ_FuiNode
{
private:
    int m_tileCount;
    int m_tilesAcross;
    int m_tilesDown;
    bool m_backgroundVisible;

public:
    FJ_WiiUDRC(fuiRenderNode* pRenderNode);
    virtual ~FJ_WiiUDRC();

    void UpdateTileBackground(FJ_Event* pEvent);
    static void _UpdateTileBackground(FJ_FuiNode* pNode, FJ_Event* pEvent);
    void ShowDRCBackground(bool bShow);
    void ShowDRCIcon(bool bOn, bool bConnected);
    void SetSafeZone(int iLeft, int iTop, int iRight, int iBottom);
    static FJ_WiiUDRC* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    int GetIconX();
    int GetIconY();
    int GetIconWidth();
    int GetIconHeight();
    void SetAlpha(float fAlpha);
};

#endif
