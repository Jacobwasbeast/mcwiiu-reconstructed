#ifndef GAME_UI_FJ_HOWTOPLAY_HPP
#define GAME_UI_FJ_HOWTOPLAY_HPP

#include <string>
#include <vector>

#include "game/ui/FJ_FuiNode.hpp"

class FJ_Event;
class fuiRenderNode;

bool LoadHowToPlayPage(class FJ_HowToPlay* pNode, int iPage, std::vector<std::wstring>& pages);

class FJ_HowToPlay : public FJ_FuiNode
{
    friend bool LoadHowToPlayPage(FJ_HowToPlay* pNode, int iPage, std::vector<std::wstring>& pages);

    int m_page;
    std::string m_dynamicHtmlTextName;
    std::vector<std::wstring> m_pages;
    bool m_initialized;
    FJ_FuiNode* m_dynamicHtmlText;

public:
    FJ_HowToPlay(fuiRenderNode* pRenderNode);
    virtual ~FJ_HowToPlay();

    static void _init(FJ_FuiNode* pNode, FJ_Event* pEvent);
    void init(FJ_Event* pEvent);
    void InitScene();
    static FJ_HowToPlay* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
