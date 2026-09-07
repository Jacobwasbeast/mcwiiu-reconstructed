#include "game/ui/FJ_CreativeMenu.hpp"

#include <string>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_CreativeMenu::FJ_CreativeMenu(fuiRenderNode* renderNode)
    : FJ_AbstractContainerDocument(renderNode)
{
    FJ_FuiNode* scrollBar = getChildByName(std::string("MainPanel.Scrollbar"));
    if (scrollBar != NULL)
    {
        FJ_Base* pageNumber = getChildByName(std::string("MainPanel.PageNumber"))->asFJ_Base();
        pageNumber->SetTextOnly(std::wstring(L""));

        m_scrollUpArrow = scrollBar->getChildByName(std::string("ScrollUpArrow"))->asFJ_ScrollArrow();
        m_scrollDownArrow = scrollBar->getChildByName(std::string("ScrollDownArrow"))->asFJ_ScrollArrow();
        m_scrollUpArrow->ChangeState(0);
        m_scrollDownArrow->ChangeState(3);
        m_currentPage = 0;
        m_scrollBarButtonPosition = static_cast<int>(scrollBar->getChildByName(std::string("ScrollBarButton"))->getY());

        FJ_FuiNode* track = scrollBar->getChildByName(std::string("ScrollBarTrack"));
        FJ_FuiNode* button = scrollBar->getChildByName(std::string("ScrollBarButton"));
        m_scrollBarEndPosition = static_cast<int>(
            button->getY() - track->getY() + track->getY() + track->getHeight() - button->getHeight());
    }
    addEventListener(eFJEventType_SlideComplete, this, _handleSlideComplete, false);
}

FJ_CreativeMenu::~FJ_CreativeMenu()
{
}

FJ_CreativeMenu* FJ_CreativeMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_CreativeMenu(renderNode);
}

void FJ_CreativeMenu::handleSlideComplete(FJ_Event*)
{
    m_pCallbackScene->handleTouchBoxRebuild();
}

void FJ_CreativeMenu::_handleSlideComplete(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_CreativeMenu*>(node)->handleSlideComplete(event);
}

unsigned int FJ_CreativeMenu::getType()
{
    return 10;
}

void FJ_CreativeMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    double stageHeight = getStage()->getStageHeight();
    if (stageHeight != 360.0 && stageHeight != 540.0)
        return;
    setX(-static_cast<float>(right) - 176.0f);
}

void FJ_CreativeMenu::SetActiveTab(int tab)
{
    int frame = tab + 1;
    m_activeTab = tab;
    if (frame < 1)
        frame = 1;
    else if (frame > 8)
        frame = 8;

    FJ_FuiNode* tabs = getChildByName(std::string("MainPanel.CreativeTabs"));
    tabs->getChildAt(0)->gotoAndStop(frame);
    m_currentPage = 0;
    m_scrollUpArrow->setVisible(true);
    m_scrollDownArrow->setVisible(true);
}

void FJ_CreativeMenu::SetScrollBar(int pageCount, int currentPage)
{
    FJ_FuiNode* scrollBar = getChildByName(std::string("MainPanel.Scrollbar"));
    if (scrollBar == NULL)
        return;

    int lastPage = pageCount - 1;
    int page = lastPage;
    if (currentPage < pageCount)
    {
        page = currentPage;
        if (currentPage < 0)
            page = 0;
    }

    if (pageCount == 1)
    {
        scrollBar->setAlpha(0.5f);
        m_scrollUpArrow->setVisible(false);
        m_scrollDownArrow->setVisible(false);
    }
    else
    {
        scrollBar->setAlpha(1.0f);
        scrollBar->getChildByName(std::string("ScrollBarButton"))->setVisible(true);
        if (pageCount > 1)
        {
            if (m_currentPage < page)
            {
                if (page > 0)
                {
                    m_scrollDownArrow->setVisible(true);
                    m_scrollDownArrow->ChangeState(4);
                }
                m_scrollUpArrow->setVisible(true);
            }
            else if (page < m_currentPage)
            {
                if (page < lastPage)
                {
                    m_scrollUpArrow->setVisible(true);
                    m_scrollUpArrow->ChangeState(1);
                }
                m_scrollDownArrow->setVisible(true);
            }
        }
    }

    if (page == lastPage)
        m_scrollDownArrow->setVisible(false);
    if (page == 0)
        m_scrollUpArrow->setVisible(false);

    FJ_FuiNode* button = scrollBar->getChildByName(std::string("ScrollBarButton"));
    if (pageCount < 2)
    {
        button->setX(static_cast<float>(m_scrollBarButtonPosition));
    }
    else
    {
        button->setX(static_cast<float>(m_scrollBarButtonPosition) +
                     static_cast<float>(m_scrollBarEndPosition - m_scrollBarButtonPosition) *
                     (static_cast<float>(page) / static_cast<float>(lastPage)));
    }
    m_currentPage = page;
}
