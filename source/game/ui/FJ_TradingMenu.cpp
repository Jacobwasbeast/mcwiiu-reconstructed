#include "game/ui/FJ_TradingMenu.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_HtmlTextPanel.hpp"
#include "game/ui/FJ_ScrollArrowH.hpp"
#include "game/ui/FJ_SlotList.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

namespace
{
FJ_FuiNode* child(FJ_FuiNode* node, const char* name)
{
    return node == 0 ? 0 : node->getChildByName(std::string(name));
}

void setArrowVisible(FJ_FuiNode* node, bool visible)
{
    if (node != 0)
        node->setVisible(visible);
}
}

FJ_TradingMenu::FJ_TradingMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode),
      m_scrollRight(0),
      m_scrollLeft(0),
      m_slotWidth(0),
      m_visibleSlots(0),
      m_selectorSlot(0)
{
}

FJ_TradingMenu::~FJ_TradingMenu()
{
}

FJ_Document* FJ_TradingMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_TradingMenu(renderNode);
}

unsigned int FJ_TradingMenu::getType()
{
    return 78;
}

void FJ_TradingMenu::init(FJ_Event* event)
{
    FJ_FuiNode* mainPanel = child(this, "MainPanel");
    FJ_FuiNode* tradingBar = child(mainPanel, "TradingBar");
    FJ_FuiNode* selector = child(mainPanel, "TradingSelector");
    FJ_FuiNode* offer = child(mainPanel, "OfferDescription");

    m_slotWidth = static_cast<int>(tradingBar->asFJ_SlotList()->getWidth());
    m_visibleSlots = Math::round(tradingBar->getWidth() / static_cast<float>(m_slotWidth));
    m_selectorSlot = 0;

    selector->setX(tradingBar->getX() -
                    static_cast<float>(Math::round((selector->getWidth() - static_cast<float>(m_slotWidth)) * 0.5f)) +
                    static_cast<float>(m_selectorSlot * m_slotWidth));
    selector->setY(tradingBar->getY() -
                    static_cast<float>(Math::round((selector->getHeight() - static_cast<float>(m_slotWidth)) * 0.5f)));

    m_scrollRight = child(mainPanel, "ScrollRight");
    m_scrollLeft = child(mainPanel, "ScrollLeft");
    static_cast<FJ_ScrollArrowH*>(m_scrollRight)->ChangeState(2);
    static_cast<FJ_ScrollArrowH*>(m_scrollLeft)->ChangeState(0);

    offer->setX(selector->getX() + selector->getWidth());
    offer->setY(selector->getY());

    fuiRect leftRect = m_scrollLeft->getRect(m_scrollLeft);
    if (tradingBar->getX() < leftRect.xMin * m_scrollLeft->getScaleX() + m_scrollLeft->getX() + m_scrollLeft->getWidth())
    {
        fuiRect rightRect = m_scrollRight->getRect(m_scrollRight);
        const float rightEdgeWorld = rightRect.xMin * m_scrollRight->getScaleX() + m_scrollRight->getX();
        fuiRect leftRect2 = m_scrollLeft->getRect(m_scrollLeft);
        const float tradingBarRight = tradingBar->getX() + tradingBar->getWidth();
        const float innerLeft = leftRect2.xMin * m_scrollLeft->getScaleX() -
                                 (tradingBar->getX() - m_scrollLeft->getWidth());
        m_scrollLeft->setX(-innerLeft - static_cast<float>(static_cast<int>(rightEdgeWorld - tradingBarRight)));
    }
}

void FJ_TradingMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    FJ_FuiNodeStage* stage = getStage();
    const double height = stage->getStageHeight();
    if (height != 360.0 && height != 540.0)
        return;
    setX(-static_cast<float>(right));
    setY(static_cast<float>(bottom));
}

void FJ_TradingMenu::ScrollRight()
{
    if (m_scrollRight != 0)
        static_cast<FJ_ScrollArrowH*>(m_scrollRight)->ChangeState(3);
}

void FJ_TradingMenu::ScrollLeft()
{
    if (m_scrollLeft != 0)
        static_cast<FJ_ScrollArrowH*>(m_scrollLeft)->ChangeState(1);
}

void FJ_TradingMenu::PositionOfferDescription()
{
    FJ_FuiNode* offer = child(this, "MainPanel.OfferDescription");
    FJ_FuiNode* selector = child(this, "MainPanel.TradingSelector");

    const float y = selector->getY() + (selector->getHeight() - offer->getHeight()) * 0.5f;
    float x;
    if (m_selectorSlot < (m_visibleSlots / 2 + (m_visibleSlots < 0 && (m_visibleSlots & 1) != 0)))
        x = selector->getX() + selector->getWidth() * 1.1f;
    else
        x = selector->getX() - offer->getWidth() - selector->getWidth() * 0.1f;
    offer->setX(static_cast<float>(Math::round(x)));
    offer->setY(static_cast<float>(Math::round(y)));
}

void FJ_TradingMenu::MoveSelector(bool right)
{
    FJ_FuiNode* mainPanel = child(this, "MainPanel");
    FJ_FuiNode* tradingBar = child(mainPanel, "TradingBar");
    FJ_FuiNode* selector = child(mainPanel, "TradingSelector");

    if (!right)
    {
        if (m_selectorSlot < 1)
        {
            if (m_scrollLeft->isVisible())
                ScrollLeft();
        }
        else
        {
            --m_selectorSlot;
        }
    }
    else if (m_selectorSlot < m_visibleSlots - 1)
    {
        ++m_selectorSlot;
    }
    else
    {
        if (m_scrollRight->isVisible())
            ScrollRight();
    }

    selector->setX(tradingBar->getX() -
                    static_cast<float>(Math::round((selector->getWidth() - static_cast<float>(m_slotWidth)) * 0.5f)) +
                    static_cast<float>(m_selectorSlot * m_slotWidth));
    selector->setY(tradingBar->getY() -
                    static_cast<float>(Math::round((selector->getHeight() - static_cast<float>(m_slotWidth)) * 0.5f)));
    PositionOfferDescription();
}

void FJ_TradingMenu::SetSelectorSlot(int slot)
{
    FJ_FuiNode* mainPanel = child(this, "MainPanel");
    FJ_FuiNode* tradingBar = child(mainPanel, "TradingBar");
    FJ_FuiNode* selector = child(mainPanel, "TradingSelector");

    m_selectorSlot = slot;
    selector->setX(tradingBar->getX() -
                    static_cast<float>(Math::round((selector->getWidth() - static_cast<float>(m_slotWidth)) * 0.5f)) +
                    static_cast<float>(m_selectorSlot * m_slotWidth));
    selector->setY(tradingBar->getY() -
                    static_cast<float>(Math::round((selector->getHeight() - static_cast<float>(m_slotWidth)) * 0.5f)));
    PositionOfferDescription();
}

void FJ_TradingMenu::SetOfferDescription(const std::wstring& description)
{
    FJ_FuiNode* offer = child(this, "MainPanel.OfferDescription");
    offer->asFJ_HtmlTextPanel()->SetLabel(description);
    PositionOfferDescription();
}

void FJ_TradingMenu::ShowScrollRightArrow(bool show)
{
    setArrowVisible(m_scrollRight, show);
}

void FJ_TradingMenu::ShowScrollLeftArrow(bool show)
{
    setArrowVisible(m_scrollLeft, show);
}

int FJ_TradingMenu::getTotalTradingSlots()
{
    return m_visibleSlots;
}
