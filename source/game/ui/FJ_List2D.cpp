#include "game/ui/FJ_List2D.hpp"

#include <cstdlib>
#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
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
    m_upArrow = static_cast<FJ_ScrollArrow*>(
        FJ_Factory::createObject("fourj.FJ_ScrollArrow", renderNode, true, NULL));
    m_downArrow = static_cast<FJ_ScrollArrow*>(
        FJ_Factory::createObject("fourj.FJ_ScrollArrow", renderNode, true, NULL));
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
        if (!up)
        {
            if (m_firstVisibleRow + m_visibleRows <= m_activeRow)
            {
                m_firstVisibleRow = m_activeRow + 1 - m_visibleRows;
                if (m_downArrow != 0)
                    m_downArrow->ChangeState(4);
            }
        }
        else if (m_activeRow < m_firstVisibleRow)
        {
            m_firstVisibleRow = m_activeRow;
            if (m_upArrow != 0)
                m_upArrow->ChangeState(1);
        }
    }

    unsigned int row;
    for (row = 0; row < m_items.size(); ++row)
    {
        bool reachedLimit = false;
        float heightBeforeFirstVisible = 0.0f;
        float heightBeforeRow = 0.0f;
        float visibleHeight = 0.0f;
        unsigned int r;
        for (r = 0; r < m_items.size(); ++r)
        {
            float itemHeight = m_items[r][0]->getHeight();
            if (static_cast<int>(r) < m_firstVisibleRow)
                heightBeforeFirstVisible += itemHeight + static_cast<float>(m_itemSpacing);
            if (r < row)
                heightBeforeRow += itemHeight + static_cast<float>(m_itemSpacing);
            if (static_cast<int>(r) >= m_firstVisibleRow && !reachedLimit)
            {
                visibleHeight += itemHeight;
                m_visibleRows = static_cast<int>(r) - m_firstVisibleRow;
                if (visibleHeight <= static_cast<float>(m_listHeight))
                    visibleHeight += static_cast<float>(m_itemSpacing);
                else
                    reachedLimit = true;
            }
        }
        if (!reachedLimit)
            m_visibleRows += 1;

        unsigned int column;
        for (column = 0; column < m_items[row].size(); ++column)
        {
            FJ_FuiNode* item = m_items[row][column];
            fuiRect rect = item->getRect(item);
            float itemX = static_cast<float>(Math::round(getX() +
                static_cast<float>(column) * m_items[0][0]->getWidth() +
                static_cast<float>(static_cast<int>(column) * m_itemSpacing) - rect.xMin));
            item->setX(itemX);
            rect = item->getRect(item);
            float itemY = static_cast<float>(Math::round(getY() + heightBeforeRow - rect.yMin -
                heightBeforeFirstVisible));
            item->setY(itemY);
            if (item->isFJ_Base())
            {
                FJ_Base* base = item->asFJ_Base();
                std::vector<FJ_FuiNode*>::iterator objIt;
                for (objIt = base->m_objects.begin(); objIt != base->m_objects.end(); ++objIt)
                    (*objIt)->setY(item->getY());
            }
        }

        if (row < static_cast<unsigned int>(m_firstVisibleRow) ||
            static_cast<unsigned int>(m_firstVisibleRow + m_visibleRows) <= row)
        {
            if (!immediate && row == static_cast<unsigned int>(m_activeRow))
            {
                ScrollList(up, initial, true);
                if (!initial)
                    HandleScrollListArrows(false);
                if (m_scrollDelegate != 0)
                    m_scrollDelegate(m_scrollDelegateNode);
                return;
            }
            for (column = 0; column < m_items[row].size(); ++column)
                m_items[row][column]->setVisible(false);
        }
        else
        {
            for (column = 0; column < m_items[row].size(); ++column)
                m_items[row][column]->setVisible(true);
        }
    }

    if (!initial)
        HandleScrollListArrows(false);
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
    int index = 0;
    unsigned int r;
    for (r = 0; r < m_items.size(); ++r)
        index += static_cast<int>(m_items[r].size());
    item->setId(id);
    item->setListId(index);
    item->setObjListParent(this);

    int newCount = index + 1;
    int itemWidth = static_cast<int>(item->getWidth());
    int cellWidth = itemWidth + m_itemSpacing;
    int availableWidth = Math::round(getWidth());
    int row;
    int rowStart;
    if (availableWidth < cellWidth * newCount - m_itemSpacing)
    {
        row = index / m_itemsPerRow;
        rowStart = m_itemsPerRow * row;
    }
    else
    {
        row = index / newCount;
        rowStart = newCount * row;
        m_itemsPerRow = newCount;
    }
    int column = index - rowStart;

    if (column == 0)
        m_items.push_back(std::vector<FJ_FuiNode*>());
    m_items[row].push_back(item);

    item->m_navLinkName[2] = m_navLinkName[2];
    item->m_navLinkName[3] = m_navLinkName[3];
    item->m_navLinkName[0] = m_navLinkName[0];
    item->m_navLinkName[1] = m_navLinkName[1];

    if (column == 0)
    {
        if (row >= 1)
        {
            FJ_FuiNode* prevRowLast = m_items[row - 1][m_itemsPerRow - 1];
            item->m_navLinkName[2] = prevRowLast->getName();
            prevRowLast->asFJ_Base()->m_navLinkName[3] = item->getName();
        }
    }
    else
    {
        FJ_FuiNode* prevItem = m_items[row][column - 1];
        item->m_navLinkName[2] = prevItem->getName();
        prevItem->asFJ_Base()->m_navLinkName[3] = item->getName();
    }

    if (row >= 1)
    {
        unsigned int col;
        for (col = 0; col < m_items[row - 1].size(); ++col)
        {
            if (static_cast<int>(col) > column)
                m_items[row - 1][col]->asFJ_Base()->m_navLinkName[1] = item->getName();
        }
        FJ_FuiNode* above = m_items[row - 1][column];
        item->m_navLinkName[0] = above->getName();
        above->asFJ_Base()->m_navLinkName[1] = item->getName();
    }

    item->addEventListener(eFJEventType_FocusIn, this, _ItemFocusInHandler, false);
    FJ_FuiNode* parent = getParent();
    parent->addChild(item);
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
