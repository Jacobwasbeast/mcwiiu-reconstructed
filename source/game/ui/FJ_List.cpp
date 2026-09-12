#include "game/ui/FJ_List.hpp"

#include <cstdlib>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_ListButtonController.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/FJ_Slider.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_List::FJ_List(fuiRenderNode* renderNode)
    : FJ_Base(renderNode), m_checkboxTick(false), m_verticalSpacing(0),
      m_action(), m_initialized(false), m_items(), m_vectorPadding(0),
      m_activeItem(0), m_topItem(0), m_upArrow(0), m_downArrow(0),
      m_background(0), m_maxItemsVisibleUp(1), m_touchSize(0),
      m_keyDown(false), m_keyUp(false), m_touchFocusY(0), m_touchY(0),
      m_touchItem(0), m_scrollChanged(false), m_touchActive(false),
      m_blockTouchTrigger(false), m_highlightFromTouch(false)
{
    m_upArrow = static_cast<FJ_ScrollArrow*>(FJ_Factory::createObject(
        "fourj.FJ_ScrollArrow", renderNode, true, 0));
    m_downArrow = static_cast<FJ_ScrollArrow*>(FJ_Factory::createObject(
        "fourj.FJ_ScrollArrow", renderNode, true, 0));
    FJ_FuiNode* background = getChildByName(std::string("ButtonListBackground"));
    if (background != 0)
        m_background = background->asMovieClip();
    if (m_background != 0)
        m_background->gotoAndPlay(std::string("ButtonListBackground"));
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
    m_touchSize = m_upArrow == 0 ? static_cast<int>(getHeight()) :
        static_cast<int>(getHeight() - m_upArrow->getHeight() / 1.5f);
}

FJ_List::~FJ_List()
{
    if (m_upArrow != 0 && m_upArrow->getParent() != 0)
        m_upArrow->getParent()->removeChild(m_upArrow, true);
    if (m_downArrow != 0 && m_downArrow->getParent() != 0)
        m_downArrow->getParent()->removeChild(m_downArrow, true);
    m_items.clear();
    m_action.clear();
}

uint32_t FJ_List::FindHighlightItem(int index, bool forward, bool wrap)
{
    unsigned int count = static_cast<unsigned int>(m_items.size());
    if (!forward)
    {
        for (unsigned int i = static_cast<unsigned int>(index); i < count; ++i)
            if (!m_items[i]->isFJ_Label()) return i;
    }
    else if (index >= -1)
    {
        int current = index;
        while (current >= 0)
        {
            int first = current - 7;
            int i;
            for (i = current; i > first && i >= 0; --i)
                if (!m_items[static_cast<unsigned int>(i)]->isFJ_Label()) return static_cast<uint32_t>(i);
            current = first;
        }
    }
    if (wrap)
        return FindHighlightItem(index, !forward, false);
    return static_cast<uint32_t>(index);
}

void FJ_List::HighlightItem(int index, bool fromTouch)
{
    if (index < 0 || static_cast<unsigned int>(index) >= m_items.size()) return;
    if (m_items[static_cast<unsigned int>(index)]->isFJ_Label())
        index = static_cast<int>(FindHighlightItem(index, index < m_activeItem, true));
    if (index < 0 || static_cast<unsigned int>(index) >= m_items.size()) return;
    m_highlightFromTouch = fromTouch;
    getStage()->setFocus(m_items[static_cast<unsigned int>(index)]);
    dispatchEvent(new FJ_Event(eFJEventType_FocusChangeUser, true, false));
}

void FJ_List::HandlePageUpDownNavigation(bool up)
{
    int item = m_topItem;
    if (up)
    {
        if (m_activeItem <= item) item = m_activeItem - m_maxItemsVisibleUp;
        if (item < 0) item = 0;
    }
    else
    {
        item += m_maxItemsVisibleUp - 1;
        if (item <= m_activeItem)
        {
            item = m_activeItem + m_maxItemsVisibleUp;
            if (item >= static_cast<int>(m_items.size())) item = static_cast<int>(m_items.size()) - 1;
        }
    }
    if (item >= 0 && static_cast<unsigned int>(item) < m_items.size()) HighlightItem(item, false);
}

uint64_t FJ_List::LoseListFocus()
{
    FJ_FuiNodeStage* stage = getStage();
    FJ_FuiNode* focus = stage == 0 ? 0 : stage->getFocus();
    FJ_Base* focusedBase = focus == 0 ? 0 : focus->asFJ_Base();
    bool belongsToList = focus != 0 &&
        ((focusedBase == 0 && focus->isFJ_List() && focus != this) ||
         (focusedBase != 0 && focusedBase->getObjListParent() != this));
    if (belongsToList && m_background != 0)
        m_background->gotoAndPlay(std::string("FadeOut"));
    return 0;
}

int FJ_List::getINumButtons() { return static_cast<int>(m_items.size()); }

void FJ_List::SetListFocus()
{
    if (m_activeItem < 0 || static_cast<unsigned int>(m_activeItem) >= m_items.size()) m_activeItem = 0;
    if (m_items.empty()) return;
    HighlightItem(m_activeItem, false);
}

void FJ_List::focusInHandler(FJ_Event*) { SetListFocus(); }
void FJ_List::_focusInHandler(FJ_FuiNode* node, FJ_Event* event) { static_cast<FJ_List*>(node)->focusInHandler(event); }

uint64_t FJ_List::focusOutHandler(FJ_Event*)
{
    if (m_items.empty()) return 0;
    if (m_background != 0) m_background->gotoAndPlay(std::string("FadeOut"));
    return 0;
}

void FJ_List::_focusOutHandler(FJ_FuiNode* node, FJ_Event* event) { static_cast<FJ_List*>(node)->focusOutHandler(event); }

void FJ_List::Init(int id)
{
    InitBase(std::wstring(L"ButtonList"), id);
    FJ_FuiNode* parent = getParent();
    if (parent != 0) m_background = parent->getChildByName(m_action)->asMovieClip();
    if (m_upArrow != 0)
        m_touchSize = static_cast<int>(getHeight() - m_upArrow->getHeight() / 1.5f);
    else
        m_touchSize = static_cast<int>(getHeight());
}

void FJ_List::UpdateScrollListArrows()
{
    if (m_upArrow == 0 || m_downArrow == 0) return;
    float x = -(m_upArrow->getWidth() * 1.5f - (getX() + getWidth()));
    float y = -(m_upArrow->getHeight() * 0.5f - (getY() + getHeight()));
    m_upArrow->setX(x); m_upArrow->setY(y);
    m_downArrow->setX(x); m_downArrow->setY(m_upArrow->getY());
}

void FJ_List::setX(float x)
{
    float old = getX();
    FJ_Base::setX(x);
    UpdateScrollListArrows();
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        m_items[i]->setX(m_items[i]->getX() + x - old);
        for (int j = 0; j < m_items[i]->getNumChildren(); ++j)
            m_items[i]->getChildAt(j)->setX(m_items[i]->getChildAt(j)->getX() + x - old);
    }
}

void FJ_List::setY(float y)
{
    float old = getY();
    FJ_Base::setY(y);
    UpdateScrollListArrows();
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        m_items[i]->incY(y - old);
        for (int j = 0; j < m_items[i]->getNumChildren(); ++j)
            m_items[i]->getChildAt(j)->incY(y - old);
    }
}

void FJ_List::ListKeyDownHandler(FJ_Event* event)
{
    m_keyDown = false; m_keyUp = false;
    if (event->m_eType == eFJEventType_FocusIn) m_keyDown = true;
    else if (event->m_eType == eFJEventType_FocusOut) m_keyUp = true;
}
void FJ_List::_ListKeyDownHandler(FJ_FuiNode* node, FJ_Event* event) { static_cast<FJ_List*>(node)->ListKeyDownHandler(event); }

double FJ_List::GetListItemHeight(int index)
{
    if (index < 0 || static_cast<unsigned int>(index) >= m_items.size()) return 100.0;
    FJ_FuiNode* item = m_items[static_cast<unsigned int>(index)];
    FJ_ListButtonController* controller = item->isFJ_ListButtonController() ? item->asFJ_ListButtonController() : 0;
    return controller != 0 ? controller->getOriginalHeight() : item->getHeight();
}

void FJ_List::GetMaxItemsVisibleUp()
{
    int height = 0; int count = 0;
    for (int i = m_activeItem; i >= 0; --i)
    {
        height += Math::round(static_cast<float>(GetListItemHeight(i))) + (i > 0 ? m_verticalSpacing : 0);
        if (height >= m_touchSize) break;
        ++count;
    }
    m_maxItemsVisibleUp = count;
}

std::wstring FJ_List::GetItemLabel(int index)
{
    if (index < 0 || static_cast<unsigned int>(index) >= m_items.size()) return std::wstring(L" ");
    FJ_Base* base = m_items[static_cast<unsigned int>(index)]->asFJ_Base();
    if (base == 0) return std::wstring(L" ");
    const std::wstring* label = reinterpret_cast<const std::wstring*>(
        static_cast<uintptr_t>(base->GetLabel()));
    return label == 0 ? std::wstring(L" ") : *label;
}

void FJ_List::HandleScrollListArrows(bool addArrows)
{
    if (m_upArrow == 0 || m_downArrow == 0) return;
    if (addArrows && m_items.size() >= static_cast<unsigned int>(m_maxItemsVisibleUp) && m_upArrow->getParent() == 0)
    {
        FJ_FuiNode* parent = getParent();
        if (parent != 0)
        {
            parent->addChild(m_upArrow);
            parent->addChild(m_downArrow);
            UpdateScrollListArrows();
            m_upArrow->ChangeState(0);
            m_downArrow->ChangeState(3);
        }
    }
    if (m_upArrow != 0) m_upArrow->setVisible(m_topItem > 0);
    if (m_downArrow != 0) m_downArrow->setVisible(m_topItem + m_maxItemsVisibleUp < static_cast<int>(m_items.size()));
}

uint64_t FJ_List::ScrollList(bool down, bool page, bool fromTouch)
{
    if (!fromTouch && m_highlightFromTouch)
    {
        m_topItem = m_activeItem + m_maxItemsVisibleUp - static_cast<int>(m_items.size());
        if (m_topItem < 0) m_topItem = m_activeItem;
        m_highlightFromTouch = false;
    }
    int amount = page ? m_maxItemsVisibleUp : 1;
    if (amount < 1) amount = 1;
    if (down) m_topItem += amount; else m_topItem -= amount;
    if (m_topItem < 0) m_topItem = 0;
    int last = static_cast<int>(m_items.size()) - m_maxItemsVisibleUp;
    if (last >= 0 && m_topItem > last) m_topItem = last;

    int y = static_cast<int>(getY());
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        FJ_FuiNode* item = m_items[i];
        item->setY(static_cast<float>(y));
        const int itemHeight = Math::round(static_cast<float>(GetListItemHeight(static_cast<int>(i))));
        const bool isVisible = static_cast<int>(i) >= m_topItem &&
            static_cast<int>(i) < m_topItem + m_maxItemsVisibleUp;
        item->setVisible(isVisible);
        y += itemHeight + m_verticalSpacing;
    }
    m_scrollChanged = true;
    HandleScrollListArrows(false);
    return 0;
}

uint64_t FJ_List::ItemFocusInHandler(FJ_Event* event)
{
    FJ_Base* item = event->m_pTarget->asFJ_Base();
    if (item != 0) { int old = m_activeItem; m_activeItem = item->getListId(); ScrollList(old > m_activeItem, false, false); }
    return 0;
}
void FJ_List::_ItemFocusInHandler(FJ_FuiNode* node, FJ_Event* event) { static_cast<FJ_List*>(node)->ItemFocusInHandler(event); }

int FJ_List::GetListHeight()
{
    if ((m_upArrow != 0 && m_upArrow->isVisible()) || (m_downArrow != 0 && m_downArrow->isVisible())) return static_cast<int>(getHeight());
    int result = 0;
    for (unsigned int i = 0; i < m_items.size(); ++i) result += Math::round(static_cast<float>(GetListItemHeight(static_cast<int>(i)) + m_verticalSpacing));
    return result;
}

uint64_t FJ_List::UpdateTouchSize()
{
    m_touchSize = GetListHeight();
    return 0;
}
int FJ_List::GetListWidth() { return static_cast<int>(getWidth()); }
void FJ_List::SetNewObjectWidth(int width) { if (getWidth() < width) setWidth(static_cast<float>(width)); }

void FJ_List::addNewListItem(FJ_Base* item, const std::wstring& label, int id)
{
    if (item == 0) return;
    item->setListId(static_cast<int>(m_items.size())); item->setObjListParent(this); item->setId(id); item->SetLabel(label);
    m_items.push_back(item); addChild(item); HandleScrollListArrows(true); UpdateTouchSize();
}

void FJ_List::removeAllItems()
{
    FJ_FuiNodeStage* stage = getStage();
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        if (stage != 0 && stage->getFocus() == m_items[i]) stage->removeFocus(m_items[i]);
        if (m_items[i]->getParent() != 0) m_items[i]->getParent()->removeChild(m_items[i], true);
    }
    m_items.clear(); m_activeItem = 0; m_topItem = 0; HandleScrollListArrows(false);
}

uint64_t FJ_List::SetButtonLabel(int index, const std::wstring& label)
{
    SetItemLabel(index, label);
    return 0;
}

uint64_t FJ_List::SetItemAlpha(int index, float alpha)
{
    if (index >= 0 && static_cast<unsigned int>(index) < m_items.size())
        m_items[index]->setAlpha(alpha);
    return 0;
}

void FJ_List::SetItemLabel(int index, const std::wstring& label)
{
    if (index >= 0 && static_cast<unsigned int>(index) < m_items.size())
    {
        FJ_Base* item = m_items[index]->asFJ_Base();
        if (item != 0)
            item->SetLabel(label);
    }
}

uint64_t FJ_List::EnableItem(int index, bool enabled)
{
    if (index >= 0 && static_cast<unsigned int>(index) < m_items.size())
    {
        FJ_FuiNode* item = m_items[index];
        if (item->isFJ_Button()) item->asFJ_Button()->EnableButton(enabled);
        else if (item->isFJ_CheckBox()) item->setVisible(enabled);
        else if (item->isFJ_Slider()) item->asFJ_Slider()->EnableSlider(enabled);
    }
    return 0;
}

uint64_t FJ_List::SetTouchFocus(int x, int y, bool focus)
{
    if (!focus)
    {
        for (unsigned int i = 0; i < m_items.size(); ++i) if (y >= m_items[i]->getY() && y < m_items[i]->getY() + GetListItemHeight(static_cast<int>(i))) { m_touchItem=i; HighlightItem(static_cast<int>(i), false); break; }
    }
    else if (y != m_touchFocusY) ScrollList(y > m_touchFocusY, true, true);
    m_touchFocusY = y; m_touchY = x;
    return 0;
}

uint32_t FJ_List::CanTouchFocus(int, int y)
{
    for (unsigned int i = 0; i < m_items.size(); ++i) if (y >= m_items[i]->getY() && y < m_items[i]->getY() + GetListItemHeight(static_cast<int>(i)) && m_items[i]->isFJ_Label()) return 0;
    return 1;
}
void FJ_List::BlockTouchTrigger(bool block) { m_blockTouchTrigger = block; }
uint32_t FJ_List::CanTouchTrigger(int, int y)
{
    if (m_blockTouchTrigger) return 0;
    for (unsigned int i = 0; i < m_items.size(); ++i) if (y >= m_items[i]->getY() && y < m_items[i]->getY() + GetListItemHeight(static_cast<int>(i))) return m_touchActive || static_cast<int>(i) == m_touchItem;
    return 0;
}
void FJ_List::FixNavigation(FJ_FuiNode* node) { if (node != 0 && node->getParent() == this) removeChild(node, false); }
void FJ_List::RemoveItem(int index)
{
    if (index < 0 || static_cast<unsigned int>(index) >= m_items.size())
        return;
    FixNavigation(m_items[index]);
    if (m_items[index]->getParent() != 0)
        m_items[index]->getParent()->removeChild(m_items[index], true);
    m_items.erase(m_items.begin() + index);
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        FJ_Base* item = m_items[i]->asFJ_Base();
        if (item != 0)
            item->setListId(static_cast<int>(i));
    }
    if (m_activeItem >= static_cast<int>(m_items.size()))
        m_activeItem = static_cast<int>(m_items.size()) - 1;
    if (m_activeItem < 0) m_activeItem = 0;
    if (index <= m_topItem && m_topItem > 0) --m_topItem;
    ScrollList(true, true, false);
    HandleScrollListArrows(true);
}
FJ_FuiNode* FJ_List::ReturnItem(int index) { return index >= 0 && static_cast<unsigned int>(index) < m_items.size() ? m_items[index] : 0; }
int FJ_List::GetActiveItem()
{
    if (m_activeItem < 0 || static_cast<unsigned int>(m_activeItem) >= m_items.size()) return -1;
    FJ_FuiNodeStage* stage = getStage();
    return stage != 0 && stage->getFocus() == m_items[static_cast<unsigned int>(m_activeItem)] ?
        m_activeItem : -1;
}
uint32_t FJ_List::GetTopItem() { return static_cast<uint32_t>(m_topItem); }

uint64_t FJ_List::setAction(unsigned char action, char* value)
{
    if (action == 10) m_checkboxTick = std::string(value) == "CheckBoxTick";
    else if (action == 11) m_verticalSpacing = std::atoi(value);
    else if (action == 12) m_action.assign(value);
    else if (action == 13) m_initialized = std::string(value) == "CheckBoxTick";
    return 0;
}
