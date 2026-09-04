#include "game/ui/FJ_TradingMenu.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_ScrollArrowH.hpp"
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
    FJ_Document::init(event);

    FJ_FuiNode* mainPanel = child(this, "MainPanel");
    FJ_FuiNode* tradingBar = child(mainPanel, "TradingBar");
    FJ_FuiNode* selector = child(mainPanel, "TradingSelector");
    FJ_FuiNode* offer = child(mainPanel, "OfferDescription");
    m_scrollRight = child(mainPanel, "ScrollRight");
    m_scrollLeft = child(mainPanel, "ScrollLeft");

    if (tradingBar != 0)
        m_slotWidth = static_cast<int>(tradingBar->getWidth());
    if (m_slotWidth != 0 && selector != 0)
        m_visibleSlots = Math::round((tradingBar->getWidth() - static_cast<float>(m_slotWidth)) * 0.5f);
    m_selectorSlot = 0;

    if (selector != 0 && tradingBar != 0)
    {
        selector->setX(tradingBar->getX() + tradingBar->getWidth() - static_cast<float>(m_slotWidth));
        selector->setY(tradingBar->getY() + tradingBar->getHeight() - static_cast<float>(m_slotWidth));
    }
    if (m_scrollRight != 0)
        static_cast<FJ_ScrollArrowH*>(m_scrollRight)->ChangeState(2);
    if (m_scrollLeft != 0)
        static_cast<FJ_ScrollArrowH*>(m_scrollLeft)->ChangeState(0);
    if (offer != 0 && selector != 0)
    {
        offer->setX(selector->getX() + selector->getWidth());
        offer->setY(selector->getY());
    }
    PositionOfferDescription();
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
    if (offer == 0 || selector == 0)
        return;

    const float y = selector->getY() + (selector->getHeight() - offer->getHeight()) * 0.5f;
    float x;
    if (m_selectorSlot < (m_visibleSlots / 2 + (m_visibleSlots < 0 && (m_visibleSlots & 1) != 0)))
        x = selector->getX() + selector->getWidth() * 1.1f;
    else
        x = selector->getX() - offer->getWidth() + selector->getWidth() * 0.1f;
    offer->setX(static_cast<float>(Math::round(x)));
    offer->setY(static_cast<float>(Math::round(y)));
}

void FJ_TradingMenu::MoveSelector(bool right)
{
    if (right)
    {
        if (m_selectorSlot < m_visibleSlots - 1)
            ++m_selectorSlot;
        else if (m_scrollRight != 0 && m_scrollRight->isVisible())
            ScrollRight();
    }
    else
    {
        if (m_selectorSlot > 0)
            --m_selectorSlot;
        else if (m_scrollLeft != 0 && m_scrollLeft->isVisible())
            ScrollLeft();
    }
    SetSelectorSlot(m_selectorSlot);
}

void FJ_TradingMenu::SetSelectorSlot(int slot)
{
    m_selectorSlot = slot;
    FJ_FuiNode* selector = child(this, "MainPanel.TradingSelector");
    FJ_FuiNode* tradingBar = child(this, "MainPanel.TradingBar");
    if (selector != 0 && tradingBar != 0)
    {
        selector->setX(tradingBar->getX() + tradingBar->getWidth() - static_cast<float>(m_slotWidth) +
                       static_cast<float>(m_selectorSlot * m_slotWidth));
        selector->setY(tradingBar->getY() + tradingBar->getHeight() - static_cast<float>(m_slotWidth));
    }
    PositionOfferDescription();
}

void FJ_TradingMenu::SetOfferDescription(const std::wstring& description)
{
    FJ_FuiNode* offer = child(this, "MainPanel.OfferDescription");
    if (offer != 0 && !description.empty())
        offer->setVisible(true);
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
