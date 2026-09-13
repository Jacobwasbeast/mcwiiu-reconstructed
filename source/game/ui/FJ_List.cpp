#include "game/ui/FJ_List.hpp"

#include <cstdlib>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_ListButtonController.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/FJ_Slider.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

FJ_List::FJ_List(fuiRenderNode* renderNode)
    : FJ_Base(renderNode), m_checkboxTick(false), m_verticalSpacing(0),
      m_action(), m_initialized(false), m_items(),
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
    FJ_FuiNodeStage* stage = getStage();
    if (stage == 0 || stage->getFocus() != this)
        return;

    int index = m_activeItem;
    if (m_initialized)
    {
        if (m_keyDown)
            index = static_cast<int>(m_items.size()) - 1;
        else if (m_keyUp)
            index = 0;
    }

    if (!m_items.empty())
    {
        getStage()->setFocus(m_items[static_cast<unsigned int>(index)]);
        dispatchEvent(new FJ_Event(eFJEventType_FocusChangeInit, true, false));
    }
    if (m_background != 0)
        m_background->gotoAndPlay(std::string("FadeIn"));
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
    if (addArrows && m_items.size() >= static_cast<unsigned int>(m_maxItemsVisibleUp) &&
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
    m_upArrow->setVisible(m_topItem > 0);
    m_downArrow->setVisible(m_topItem + m_maxItemsVisibleUp < static_cast<int>(m_items.size()));
}

uint64_t FJ_List::ScrollList(bool down, bool page, bool fromTouch)
{
    bool skipLabelChecks = false;
    if (!page)
    {
        if (m_highlightFromTouch)
        {
            int overflow = m_activeItem + m_maxItemsVisibleUp - static_cast<int>(m_items.size());
            m_topItem = overflow < 1 ? m_activeItem : m_activeItem - overflow;
            m_highlightFromTouch = false;
            down = false;
            skipLabelChecks = true;
        }
        else if (!down)
        {
            if (m_topItem + m_maxItemsVisibleUp <= m_activeItem)
            {
                GetMaxItemsVisibleUp();
                m_topItem = m_activeItem + 1 - m_maxItemsVisibleUp;
                if (m_downArrow != 0)
                    m_downArrow->ChangeState(4);
            }
        }
        else if (m_activeItem < m_topItem)
        {
            m_topItem = m_activeItem;
            if (m_upArrow != 0)
                m_upArrow->ChangeState(1);
        }
    }
    if (down && !skipLabelChecks && m_activeItem > 0 &&
        m_items[static_cast<unsigned int>(m_activeItem - 1)]->isFJ_Label())
    {
        const int index = m_activeItem - 1;
        if (GetItemLabel(index) != L" " && !m_items[static_cast<unsigned int>(index)]->isVisible())
            --m_topItem;
    }
    if (!down && m_activeItem + 2 < static_cast<int>(m_items.size()) &&
        m_items[static_cast<unsigned int>(m_activeItem + 1)]->isFJ_Label())
    {
        const int index = m_activeItem + 1;
        if (GetItemLabel(index) != L" " && !m_items[static_cast<unsigned int>(index)]->isVisible())
            ++m_topItem;
    }

    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        float beforeTop = 0.0f;
        float beforeItem = 0.0f;
        for (unsigned int j = 0; j < m_items.size(); ++j)
        {
            const float height = static_cast<float>(GetListItemHeight(static_cast<int>(j)));
            if (static_cast<int>(j) < m_topItem)
                beforeTop += height + static_cast<float>(m_verticalSpacing);
            if (static_cast<int>(j) < static_cast<int>(i))
                beforeItem += height + static_cast<float>(m_verticalSpacing);
        }
        float visibleHeight = 0.0f;
        bool reachedLimit = false;
        for (unsigned int j = 0; j < m_items.size(); ++j)
        {
            if (static_cast<int>(j) < m_topItem)
                continue;
            visibleHeight += static_cast<float>(GetListItemHeight(static_cast<int>(j)));
            m_maxItemsVisibleUp = static_cast<int>(j) - m_topItem;
            if (visibleHeight > static_cast<float>(m_touchSize))
            {
                reachedLimit = true;
                break;
            }
            visibleHeight += static_cast<float>(m_verticalSpacing);
        }
        if (!reachedLimit)
            ++m_maxItemsVisibleUp;
        FJ_FuiNode* item = m_items[i];
        fuiRect rect = item->getRect(item);
        item->setY(static_cast<float>(Math::round(getY() + beforeItem - rect.yMin - beforeTop)));
        item->setVisible(static_cast<int>(i) >= m_topItem &&
            static_cast<int>(i) < m_topItem + m_maxItemsVisibleUp);
        FJ_Base* base = item->asFJ_Base();
        if (base != 0)
        {
            for (unsigned int child = 0; child < base->m_objects.size(); ++child)
            {
                base->m_objects[child]->setY(item->getY());
                base->m_objects[child]->setVisible(item->isVisible());
            }
        }
    }
    if (!page)
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
    FJ_FuiNode* background = getChildByName(std::string("ButtonListBackground"))->asMovieClip();
    if (background != 0)
    {
        int height = GetListHeight() - m_verticalSpacing;
        background->setHeight(static_cast<float>(Math::round(static_cast<float>(height) / getScaleY())));
    }
    return 0;
}
int FJ_List::GetListWidth()
{
    int result = static_cast<int>(getWidth());
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        FJ_FuiNode* item = m_items[i];
        int itemWidth;
        if (!item->isFJ_Slider())
        {
            itemWidth = Math::round(item->asFJ_Base()->getWidth());
            int newWidth = item->asFJ_Base()->getNewWidth();
            if (itemWidth < newWidth)
                itemWidth = newWidth;
        }
        else
        {
            fuiRenderNodeEditText* textField = item->asFJ_Base()->GetTextField();
            itemWidth = Math::round(textField->getWidth());
            int oldWidth = item->asFJ_Base()->getOldWidth();
            if (itemWidth < oldWidth)
                itemWidth = static_cast<int>(item->asFJ_Base()->GetTextField()->getWidth() * 1.1f);
        }
        if (result < itemWidth)
            result = itemWidth;
    }
    return result;
}
void FJ_List::SetNewObjectWidth(int width) { if (getWidth() < width) setWidth(static_cast<float>(width)); }

void FJ_List::addNewListItem(FJ_Base* item, const std::wstring& label, int id)
{
    const int index = static_cast<int>(m_items.size());
    item->setId(id);
    item->setListId(index);
    item->setObjListParent(this);
    if (!item->isFJ_CheckBox())
        item->setWidth(getWidth());
    item->setPreScaleWidth(static_cast<int>(item->getWidth()));
    if (!item->isFJ_CheckBox())
    {
        const float x = getX() + static_cast<float>(Math::round(
            (getWidth() - item->getWidth()) * 0.5f));
        item->setX(x);
    }
    else
        item->setX(getX());
    m_items.push_back(item);
    if (index > 0)
    {
        FJ_Base* previous = m_items[static_cast<unsigned int>(index - 1)]->asFJ_Base();
        previous->m_navLinkName[1] = item->getName();
        item->m_navLinkName[0] = previous->getName();
    }
    item->m_navLinkName[1] = m_navLinkName[1];
    item->m_navLinkName[2] = m_navLinkName[2];
    item->m_navLinkName[3] = m_navLinkName[3];
    if (index == 0)
        item->m_navLinkName[0] = m_navLinkName[0];
    (void)label;
    item->addEventListener(eFJEventType_FocusIn, this, _ItemFocusInHandler, false);
    getParent()->addChild(item);
    if (m_initialized != 0)
    {
        if (m_navLinkName[0].empty() && m_navLinkName[1].empty())
        {
            item->m_navLinkName[1] = m_items[0]->getName();
            m_items[0]->asFJ_Base()->m_navLinkName[0] = item->getName();
        }
        else
            getParent()->addEventListener(eFJEventType_KeyUp, this, _ListKeyDownHandler, false);
    }
    if (index == m_activeItem)
        SetListFocus();
    ScrollList(false, true, false);
    if (m_activeItem == 0 && !m_items.empty() && m_items[0]->isFJ_Label())
    {
        for (unsigned int i = 0; i < m_items.size(); ++i)
        {
            if (!m_items[i]->isFJ_Label())
            {
                m_activeItem = static_cast<int>(i);
                SetListFocus();
                break;
            }
        }
    }
    HandleScrollListArrows(true);
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
        else if (item->isFJ_CheckBox()) item->asFJ_CheckBox()->EnableCheckBox(enabled);
        else if (item->isFJ_Slider()) item->asFJ_Slider()->EnableSlider(enabled);
    }
    return 0;
}

uint64_t FJ_List::SetTouchFocus(int x, int y, bool focus)
{
    if (!focus)
    {
        for (unsigned int i = 0; i < m_items.size(); ++i)
        {
            if (m_items[i]->getY() < static_cast<float>(y) &&
                static_cast<float>(y) < m_items[i]->getY() + static_cast<float>(GetListItemHeight(static_cast<int>(i))))
            {
                m_scrollChanged = false;
                m_touchActive = false;
                m_touchItem = static_cast<int>(i);
                HighlightItem(static_cast<int>(i), false);
                m_touchFocusY = y;
                m_touchY = x;
                return 0;
            }
        }
        m_touchFocusY = y;
        m_touchY = x;
        return 0;
    }

    if (!m_items.empty() && static_cast<unsigned int>(m_maxItemsVisibleUp) < m_items.size() && !m_touchActive)
    {
        if (m_touchFocusY < y)
        {
            int rows = static_cast<int>(static_cast<float>(y - m_touchFocusY) /
                (static_cast<float>(GetListItemHeight(0)) + static_cast<float>(m_verticalSpacing)));
            if (rows > 0)
            {
                int newTop = m_topItem - rows;
                m_scrollChanged = true;
                if (newTop < 0) newTop = 0;
                m_topItem = newTop;
                ScrollList(true, false, true);
                m_touchFocusY = y;
                if (m_upArrow != 0) m_upArrow->ChangeState(1);
                int lastVisible = m_topItem + m_maxItemsVisibleUp;
                if (lastVisible - 1 < m_activeItem)
                    HighlightItem(lastVisible - 1, false);
            }
        }
        else if (y < m_touchFocusY)
        {
            int rows = static_cast<int>(static_cast<float>(m_touchFocusY - y) /
                (static_cast<float>(GetListItemHeight(0)) + static_cast<float>(m_verticalSpacing)));
            if (rows > 0)
            {
                int newTop = m_topItem + rows;
                m_scrollChanged = true;
                if (m_items.size() <= static_cast<unsigned int>(newTop + m_maxItemsVisibleUp))
                    newTop = static_cast<int>(m_items.size()) - m_maxItemsVisibleUp;
                m_topItem = newTop;
                ScrollList(false, false, true);
                m_touchFocusY = y;
                if (m_downArrow != 0) m_downArrow->ChangeState(4);
                if (m_activeItem < m_topItem)
                    HighlightItem(m_topItem, false);
            }
        }
    }

    if (!m_scrollChanged && !m_items.empty() && m_items[m_activeItem]->isFJ_Slider())
    {
        FJ_Slider* slider = m_items[m_activeItem]->asFJ_Slider();
        if (slider->getEnabled() && (std::abs(x - m_touchY) > 50 || m_touchActive))
        {
            m_items[m_activeItem]->asFJ_Slider()->SetRelativeSliderPos(static_cast<float>(x) / getWidth());
            m_touchActive = true;
        }
    }
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
    if (action == 10) m_checkboxTick = std::string(value) == "true";
    else if (action == 11) m_verticalSpacing = std::atoi(value);
    else if (action == 12) m_action.assign(value);
    else if (action == 13) m_initialized = std::string(value) == "true";
    return 0;
}
