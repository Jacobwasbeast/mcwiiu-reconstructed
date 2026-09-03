#include "game/ui/FJ_HowToPlay.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/fuiRenderNode.hpp"

void LoadHowToPlayPage(FJ_HowToPlay* pNode, int iPage, std::vector<std::wstring>& pages);

FJ_HowToPlay::FJ_HowToPlay(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode), m_page(0), m_initialized(false), m_dynamicHtmlText(NULL)
{
    m_pages.clear();
    InitScene();
}

FJ_HowToPlay::~FJ_HowToPlay()
{
    m_dynamicHtmlText = NULL;
    m_initialized = false;
    m_page = 0;
}

void FJ_HowToPlay::_init(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
}

void FJ_HowToPlay::init(FJ_Event* pEvent)
{
}

void FJ_HowToPlay::InitScene()
{
    m_page = -1;
    m_dynamicHtmlTextName.assign("DynamicHtmlText");
    m_dynamicHtmlText = getChildByName(m_dynamicHtmlTextName);
    if (m_dynamicHtmlText != NULL)
        m_dynamicHtmlText->setHeight(0.0f);
    m_pages.clear();
    m_pages.push_back(L"");
    LoadHowToPlayPage(this, 0, m_pages);
    m_initialized = true;
}

FJ_HowToPlay* FJ_HowToPlay::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_HowToPlay(pRenderNode);
}

unsigned int FJ_HowToPlay::getType()
{
    return 0x58;
}

void FJ_HowToPlay::SetSafeZone(int iLeft, int iTop, int iRight, int iBottom)
{
    setX((float)iRight - 176.0f);
    setY((float)iLeft - 176.0f);
}
