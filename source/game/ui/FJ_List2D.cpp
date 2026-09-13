#include "game/ui/FJ_List2D.hpp"

#include <cstdlib>
#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_ListButtonAchievement.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_List2D::FJ_List2D(fuiRenderNode* renderNode)
    : FJ_Base(renderNode), m_outline(false), m_itemSpacing(0), m_items(), m_activeItem(0),
      m_upArrow(0), m_downArrow(0), m_visibleRows(1), m_itemsPerRow(1), m_activeRow(0),
      m_activeColumn(0), m_firstVisibleRow(0), m_listHeight(0), m_touchY(0), m_touchX(0),
      m_touchRow(0), m_touchFocus(false), m_scrollDelegateNode(0), m_scrollDelegate(0)
{
    m_upArrow = static_cast<FJ_ScrollArrow*>(FJ_ScrollArrow::Create(renderNode));
    m_downArrow = static_cast<FJ_ScrollArrow*>(FJ_ScrollArrow::Create(renderNode));
    if (m_upArrow != 0)
        m_listHeight = static_cast<int>(getHeight() - m_upArrow->getHeight() / 1.3f);
    FJ_FuiNode* background = getChildByName(std::string("ButtonListBackground"));
    if (background != 0)
        background->gotoAndStop(0);
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
}

FJ_List2D::~FJ_List2D()
{
    m_items.clear();
}

void FJ_List2D::HighlightItem(int index)
{
    int row = index / m_itemsPerRow;
    int column = index - row * m_itemsPerRow;
    if (m_items[row].size() - 1 < static_cast<unsigned int>(column))
        column = static_cast<int>(m_items[row].size()) - 1;
    if (static_cast<unsigned int>(row) < m_items.size() && !m_items[row].empty() &&
        static_cast<unsigned int>(column) < m_items[row].size())
    {
        m_activeRow = row;
        m_activeColumn = column;
        getStage()->setFocus(m_items[row][column]);
        dispatchEvent(new FJ_Event(eFJEventType_FocusChangeInit, true, false));
    }
}

void FJ_List2D::HandlePageUpDownNavigation(bool up)
{
    int targetIndex;
    if (up)
    {
        if (m_firstVisibleRow < m_activeRow)
            targetIndex = m_firstVisibleRow * m_itemsPerRow + m_activeColumn;
        else
        {
            int targetRow = m_activeRow - m_visibleRows;
            if (targetRow > 0)
                targetIndex = targetRow * m_itemsPerRow + m_activeColumn;
            else
                targetIndex = m_activeColumn;
        }
    }
    else
    {
        int targetRow = m_firstVisibleRow + m_visibleRows - 1;
        if (m_activeRow < targetRow)
            targetIndex = targetRow * m_itemsPerRow + m_activeColumn;
        else
        {
            targetRow = m_activeRow + m_visibleRows;
            if (static_cast<unsigned int>(targetRow) >= m_items.size())
                targetRow = static_cast<int>(m_items.size()) - 1;
            targetIndex = targetRow * m_itemsPerRow + m_activeColumn;
        }
    }

    int row = targetIndex / m_itemsPerRow;
    int column = targetIndex - row * m_itemsPerRow;
    if (m_items[row].size() - 1 < static_cast<unsigned int>(column))
        column = static_cast<int>(m_items[row].size()) - 1;
    if (static_cast<unsigned int>(row) < m_items.size() && !m_items[row].empty() &&
        static_cast<unsigned int>(column) < m_items[row].size())
    {
        m_activeRow = row;
        m_activeColumn = column;
        getStage()->setFocus(m_items[row][column]);
        dispatchEvent(new FJ_Event(eFJEventType_FocusChangeInit, true, false));
    }
}

FJ_FuiNode* FJ_List2D::getActiveItem()
{
    if (m_activeRow < 0 || m_activeRow >= static_cast<int>(m_items.size()) ||
        m_activeColumn < 0 || m_activeColumn >= static_cast<int>(m_items[m_activeRow].size()))
        return 0;
    return m_items[m_activeRow][m_activeColumn];
}

void FJ_List2D::SetListFocus()
{
    if (getStage()->getFocus() == this)
    {
        int row = m_activeItem / m_itemsPerRow;
        int column = m_activeItem - m_itemsPerRow * row;
        if (!m_items.empty())
        {
            m_activeColumn = column;
            m_activeRow = row;
            getStage()->setFocus(m_items[row][column]);
            dispatchEvent(new FJ_Event(eFJEventType_FocusChangeInit, true, false));
        }
    }
}

void FJ_List2D::focusInHandler(FJ_Event*)
{
    SetListFocus();
}

void FJ_List2D::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_List2D*>(node)->focusInHandler(event);
}

void FJ_List2D::_focusOutHandler(FJ_FuiNode*, FJ_Event*)
{
}

void FJ_List2D::Init(int id)
{
    InitBase(std::wstring(), id);
    if (m_upArrow != 0)
        m_listHeight = static_cast<int>(getHeight() - m_upArrow->getHeight() / 1.3f);
}

void FJ_List2D::UpdateScrollListArrows()
{
    if (m_upArrow == 0 || m_downArrow == 0)
        return;
    m_upArrow->setX(static_cast<float>(Math::round(getX() + getWidth() - m_upArrow->getWidth() * 1.5f)));
    m_upArrow->setY(static_cast<float>(Math::round(getY() + getHeight() - m_upArrow->getHeight() * 0.5f)));
    m_downArrow->setX(static_cast<float>(Math::round(getX() + getWidth() - m_downArrow->getWidth() * 1.5f)));
    m_downArrow->setY(m_upArrow->getY());
}

void FJ_List2D::setX(float x)
{
    FJ_FuiNode::setX(x);
    UpdateScrollListArrows();
    unsigned int row;
    for (row = 0; row < m_items.size(); ++row)
    {
        unsigned int column;
        for (column = 0; column < m_items[row].size(); ++column)
            if (m_items[row][column] != 0)
                m_items[row][column]->setX(getX() + static_cast<float>(column * m_itemSpacing));
    }
}

void FJ_List2D::setY(float y)
{
    FJ_FuiNode::setY(y);
    UpdateScrollListArrows();
    unsigned int row;
    for (row = 0; row < m_items.size(); ++row)
    {
        unsigned int column;
        for (column = 0; column < m_items[row].size(); ++column)
            if (m_items[row][column] != 0)
                m_items[row][column]->setY(getY() + static_cast<float>(row * m_itemSpacing));
    }
}

void FJ_List2D::SetScrollDelegate(void (*delegate)(FJ_FuiNode*), FJ_FuiNode* node)
{
    m_scrollDelegate = delegate;
    m_scrollDelegateNode = node;
}

void FJ_List2D::HandleScrollListArrows(bool addToStage)
{
    if (addToStage && m_items.size() >= static_cast<unsigned int>(m_visibleRows) &&
        m_upArrow->getParent() == 0)
    {
        getParent()->addChild(m_upArrow);
        getParent()->addChild(m_downArrow);
        m_upArrow->setX(-(m_upArrow->getWidth() * 1.5f - (getX() + getWidth())));
        m_upArrow->setY(-(m_upArrow->getHeight() * 0.5f - (getY() + getHeight())));
        m_downArrow->setX(-(m_upArrow->getWidth() * 0.5f - (getX() + getWidth())));
        m_downArrow->setY(m_upArrow->getY());
        m_upArrow->ChangeState(0);
        m_downArrow->ChangeState(3);
    }
    m_upArrow->setVisible(m_firstVisibleRow > 0);
    m_downArrow->setVisible(m_firstVisibleRow + m_visibleRows < static_cast<int>(m_items.size()));
}

void FJ_List2D::ScrollList(bool up, bool initial, bool immediate)
{
    if (!initial)
    {
        if (up && m_activeRow < m_firstVisibleRow)
            m_firstVisibleRow = m_activeRow;
        if (!up && m_activeRow >= m_firstVisibleRow + m_visibleRows)
            m_firstVisibleRow = m_activeRow - m_visibleRows + 1;
    }
    if (m_firstVisibleRow < 0)
        m_firstVisibleRow = 0;
    if (m_firstVisibleRow >= static_cast<int>(m_items.size()) && !m_items.empty())
        m_firstVisibleRow = static_cast<int>(m_items.size()) - 1;
    unsigned int row;
    for (row = 0; row < m_items.size(); ++row)
    {
        bool visible = row >= static_cast<unsigned int>(m_firstVisibleRow) &&
            row < static_cast<unsigned int>(m_firstVisibleRow + m_visibleRows);
        unsigned int column;
        for (column = 0; column < m_items[row].size(); ++column)
            if (m_items[row][column] != 0)
                m_items[row][column]->setVisible(visible);
    }
    HandleScrollListArrows(!immediate);
    if (m_scrollDelegate != 0)
        m_scrollDelegate(m_scrollDelegateNode);
}

void FJ_List2D::ItemFocusInHandler(FJ_Event*)
{
    FJ_FuiNodeStage* stage = getStage();
    if (stage == 0 || stage->getFocus() == this)
        return;
    FJ_FuiNode* focused = stage->getFocus();
    if (focused == 0 || !focused->isFJ_Base())
        return;
    FJ_Base* item = focused->asFJ_Base();
    int previousActiveItem = m_activeItem;
    int activeItem = static_cast<int>(item->getListId());
    m_activeRow = activeItem / m_itemsPerRow;
    m_activeItem = activeItem;
    m_activeColumn = activeItem - m_itemsPerRow * m_activeRow;
    ScrollList(activeItem < previousActiveItem, false, false);
}

void FJ_List2D::_ItemFocusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_List2D*>(node)->ItemFocusInHandler(event);
}

void FJ_List2D::SetNewObjectWidth(int width)
{
    if (getWidth() < static_cast<float>(width))
        setWidth(static_cast<float>(width));
}

void FJ_List2D::addNewListItem(FJ_Base* item, int id)
{
    if (item == 0)
        return;
    int index = 0;
    unsigned int row;
    for (row = 0; row < m_items.size(); ++row)
        index += static_cast<int>(m_items[row].size());
    item->setId(id);
    item->setListId(index);
    item->setObjListParent(this);
    int columns = m_itemsPerRow;
    if (columns < 1)
        columns = 1;
    row = static_cast<unsigned int>(index / columns);
    if (row >= m_items.size())
        m_items.push_back(std::vector<FJ_FuiNode*>());
    m_items[row].push_back(item);
    FJ_FuiNode* parent = getParent();
    if (parent != 0)
        parent->addChild(item);
    item->addEventListener(eFJEventType_FocusIn, this, _ItemFocusInHandler, false);
    if (index == m_activeItem)
        SetListFocus();
    ScrollList(false, true, false);
    HandleScrollListArrows(true);
}

void FJ_List2D::removeAllItems()
{
    bool hadFocus = false;
    unsigned int row;
    for (row = 0; row < m_items.size(); ++row)
    {
        unsigned int column;
        for (column = 0; column < m_items[row].size(); ++column)
        {
            FJ_FuiNode* node = m_items[row][column];
            if (getStage()->getFocus() == node)
                hadFocus = true;
            getParent()->removeChild(node, true);
        }
    }
    m_items.clear();
    m_firstVisibleRow = 0;
    m_activeItem = 0;
    HandleScrollListArrows(false);
    if (hadFocus)
        getStage()->setFocus(this);
}

void FJ_List2D::EnableItem(int index, bool enabled)
{
    int row = index / m_itemsPerRow;
    int column = index - row * m_itemsPerRow;
    if (row >= 0 && row < static_cast<int>(m_items.size()) &&
        column >= 0 && column < static_cast<int>(m_items[row].size()))
    {
        FJ_ListButtonAchievement* achievement = m_items[row][column] == 0 ? 0 :
            m_items[row][column]->asFJ_ListButtonAchievement();
        if (achievement != 0)
            achievement->EnableButton(enabled);
    }
}

void FJ_List2D::SetTouchFocus(int x, int y, bool focus)
{
    if (focus)
    {
        if (!m_items.empty() && static_cast<unsigned int>(m_visibleRows) < m_items.size())
        {
            if (m_touchY < y)
            {
                int rows = (y - m_touchY) /
                    (static_cast<int>(m_items[0][0]->getHeight()) + m_itemSpacing);
                if (rows > 0)
                {
                    m_firstVisibleRow -= rows;
                    m_touchFocus = true;
                    if (m_firstVisibleRow < 0)
                        m_firstVisibleRow = 0;
                    ScrollList(true, false, true);
                    m_touchY = y;
                    if (m_upArrow != 0)
                        m_upArrow->ChangeState(1);
                    int lastVisibleRow = m_firstVisibleRow + m_visibleRows - 1;
                    if (lastVisibleRow < m_activeRow)
                        HighlightItem(lastVisibleRow * m_itemsPerRow + m_activeColumn);
                }
            }
            else if (y < m_touchY)
            {
                int rows = (m_touchY - y) /
                    (static_cast<int>(m_items[0][0]->getHeight()) + m_itemSpacing);
                if (rows > 0)
                {
                    m_firstVisibleRow += rows;
                    m_touchFocus = true;
                    if (static_cast<unsigned int>(m_firstVisibleRow + m_visibleRows) >= m_items.size())
                        m_firstVisibleRow = static_cast<int>(m_items.size()) - m_visibleRows;
                    ScrollList(false, false, true);
                    m_touchY = y;
                    if (m_downArrow != 0)
                        m_downArrow->ChangeState(4);
                    if (m_activeRow < m_firstVisibleRow)
                        HighlightItem(m_firstVisibleRow * m_itemsPerRow + m_activeColumn);
                }
            }
        }
    }
    else
    {
        unsigned int row;
        for (row = 0; row < m_items.size(); ++row)
        {
            unsigned int column;
            for (column = 0; column < m_items[row].size(); ++column)
            {
                FJ_FuiNode* item = m_items[row][column];
                if (item->getY() < static_cast<float>(y) &&
                    static_cast<float>(y) < item->getY() + item->getHeight() &&
                    item->getX() < static_cast<float>(x) &&
                    static_cast<float>(x) < item->getX() + item->getWidth())
                {
                    m_touchRow = static_cast<int>(row);
                    m_touchFocus = false;
                    HighlightItem(static_cast<int>(row) * m_itemsPerRow + static_cast<int>(column));
                    break;
                }
            }
        }
        m_touchY = y;
        m_touchX = x;
    }
}

uint32_t FJ_List2D::CanTouchFocus(int, int)
{
    return 1;
}

uint32_t FJ_List2D::CanTouchTrigger(int x, int y)
{
    unsigned int row;
    for (row = 0; row < m_items.size(); ++row)
    {
        unsigned int column;
        for (column = 0; column < m_items[row].size(); ++column)
        {
            FJ_FuiNode* item = m_items[row][column];
            if (item != 0 && item->getY() < static_cast<float>(y) &&
                static_cast<float>(y) < item->getY() + item->getHeight() &&
                item->getX() < static_cast<float>(x) &&
                static_cast<float>(x) < item->getX() + item->getWidth() &&
                !m_touchFocus && static_cast<int>(row) == m_touchRow)
            {
                m_touchRow = static_cast<int>(row);
                m_touchX = static_cast<int>(column);
                return 1;
            }
        }
    }
    return 0;
}

void FJ_List2D::setAction(unsigned char action, char* value)
{
    if (action == 10)
        m_outline = value != 0 && std::string(value) == "true";
    else if (action == 0x1d && value != 0)
        m_itemSpacing = std::atoi(value);
}
