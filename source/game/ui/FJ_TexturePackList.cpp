#include "game/ui/FJ_TexturePackList.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/FJ_ScrollArrowH.hpp"
#include "game/ui/FJ_TexturePackList_Slot.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_TexturePackList::FJ_TexturePackList(fuiRenderNode* renderNode)
    : FJ_Base(renderNode), m_disabledAlpha(0.5f), m_state(0), m_items(),
      m_slotArea(0), m_topSlot(0), m_selectedSlot(0), m_focusSlot(0),
      m_visibleSlots(0), m_slotSpacing(0), m_leftArrow(0), m_rightArrow(0),
      m_selectorEnabled(true), m_touchX(0), m_touchSlot(0), m_touchMoved(false)
{
    m_slotArea = getChildByName(std::string("SlotArea"))->asMovieClip();
    m_slotArea->setVisible(false);
    m_leftArrow = static_cast<FJ_ScrollArrowH*>(FJ_Factory::createObject(
        "fourj.FJ_ScrollArrowH", renderNode, true, 0));
    m_rightArrow = static_cast<FJ_ScrollArrowH*>(FJ_Factory::createObject(
        "fourj.FJ_ScrollArrowH", renderNode, true, 0));
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
    addEventListener(eFJEventType_KeyUp, this, _keyDownHandler, false);
    addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
}

FJ_TexturePackList::~FJ_TexturePackList()
{
    m_items.clear();
}

void FJ_TexturePackList::ChangeState(int state)
{
    std::string current = currentLabel();
    static const char* labels[2] = { "Unselected", "Selected" };
    if (state >= 0 && state < 2 && current != labels[state]) {
        gotoAndPlay(std::string(labels[state]));
        m_state = state;
    }
}

void FJ_TexturePackList::focusInHandler(FJ_Event*) { ChangeState(1); }
void FJ_TexturePackList::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TexturePackList*>(node)->focusInHandler(event);
}

void FJ_TexturePackList::HandleScrollListArrows(bool addArrows)
{
    unsigned int count = static_cast<unsigned int>(m_items.size());
    if (addArrows && count >= static_cast<unsigned int>(m_visibleSlots) &&
        m_leftArrow != 0 && m_leftArrow->getParent() == 0) {
        FJ_FuiNode* parent = getParent();
        if (parent != 0) {
            parent->addChild(m_leftArrow);
            parent->addChild(m_rightArrow);
            m_leftArrow->setX(getX() + static_cast<float>(m_slotSpacing / 2));
            m_leftArrow->setY(getY() + m_slotArea->getHeight() * 0.5f);
            m_rightArrow->setX(getX() + getWidth() - static_cast<float>(m_slotSpacing / 2));
            m_rightArrow->setY(m_leftArrow->getY());
            m_leftArrow->ChangeState(0);
            m_rightArrow->ChangeState(2);
        }
    }
    if (m_leftArrow != 0)
        m_leftArrow->setVisible(m_topSlot > 0);
    if (m_rightArrow != 0)
        m_rightArrow->setVisible(m_topSlot + m_visibleSlots < static_cast<int>(count));
}

void FJ_TexturePackList::ScrollList(bool down, bool page, bool fromTouch)
{
    if (!page && !fromTouch) {
        if (m_selectedSlot < m_topSlot)
        {
            m_topSlot = m_selectedSlot;
            if (m_leftArrow != 0)
                m_leftArrow->ChangeState(1);
        }
        else if (m_selectedSlot >= m_topSlot + m_visibleSlots)
        {
            m_topSlot = m_selectedSlot + 1 - m_visibleSlots;
            if (m_rightArrow != 0)
                m_rightArrow->ChangeState(3);
        }
    }
    for (unsigned int i = 0; i < m_items.size(); ++i) {
        FJ_TexturePackList_Slot* slot = static_cast<FJ_TexturePackList_Slot*>(m_items[i]);
        slot->setX(getX() + static_cast<float>(static_cast<int>(i) - m_topSlot) *
            slot->getWidth() + static_cast<float>(m_slotSpacing));
        slot->setVisible(i >= static_cast<unsigned int>(m_topSlot) &&
            i < static_cast<unsigned int>(m_topSlot + m_visibleSlots));
        slot->SetSelected(static_cast<int>(i) == m_selectedSlot);
    }
    if (!page)
        HandleScrollListArrows(false);
}

void FJ_TexturePackList::GotoSelectedSlot()
{
    if (m_selectedSlot < m_focusSlot)
        m_selectedSlot = m_focusSlot;
    else if (m_selectedSlot > m_focusSlot)
        m_selectedSlot = m_focusSlot;
    ScrollList(false, false, false);
    HandleScrollListArrows(false);
}

void FJ_TexturePackList::focusOutHandler(FJ_Event*)
{
    ChangeState(0);
    if (m_selectedSlot != m_focusSlot) {
        m_selectedSlot = m_focusSlot;
        if (getRoot() != 0) {
            getRoot()->dispatchEvent(new FJ_Event(eFJEventType_FocusChangeUser, true, false));
        }
    }
    GotoSelectedSlot();
}
void FJ_TexturePackList::_focusOutHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TexturePackList*>(node)->focusOutHandler(event);
}

void FJ_TexturePackList::NavigateList(bool down)
{
    int count = static_cast<int>(m_items.size());
    if (count == 0) return;
    if (down) {
        ++m_selectedSlot;
        if (m_selectedSlot >= count) m_selectedSlot = 0;
    } else {
        --m_selectedSlot;
        if (m_selectedSlot < 0) m_selectedSlot = count - 1;
    }
    ScrollList(!down, false, false);
}

void FJ_TexturePackList::keyDownHandler(FJ_Event* event)
{
    if (!m_selectorEnabled) return;
    int keyCode = static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode;
    if (keyCode == 0) {
        m_focusSlot = m_selectedSlot;
        getRoot()->m_pCallbackScene->handlePress(getId(), m_focusSlot);
        keyCode = static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode;
    }
    if (keyCode == 5)
        NavigateList(false);
    else if (keyCode == 6)
        NavigateList(true);
}
void FJ_TexturePackList::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TexturePackList*>(node)->keyDownHandler(event);
}

void FJ_TexturePackList::UpdateLabel()
{
    SetLabel(m_label);
}
void FJ_TexturePackList::HandleUpdateLabel(FJ_Event*) { UpdateLabel(); }
void FJ_TexturePackList::_HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TexturePackList*>(node)->HandleUpdateLabel(event);
}

FJ_TexturePackList* FJ_TexturePackList::Create(fuiRenderNode* renderNode)
{
    return new FJ_TexturePackList(renderNode);
}

unsigned int FJ_TexturePackList::getType() { return 0x8009; }

void FJ_TexturePackList::Init(const std::wstring& name, int id)
{
    InitBase(name, id);
}

void FJ_TexturePackList::SelectSlot(int slot)
{
    if (slot < 0 || slot >= static_cast<int>(m_items.size())) return;
    m_focusSlot = slot;
    m_selectedSlot = slot;
    ScrollList(false, false, false);
    HandleScrollListArrows(false);
}

void FJ_TexturePackList::EnableSelector(bool enabled)
{
    if (!enabled) {
        for (unsigned int i = 0; i < m_items.size(); ++i)
            m_items[i]->setAlpha(static_cast<int>(i) == m_focusSlot ? 1.0f : m_disabledAlpha);
        if (m_leftArrow != 0) m_leftArrow->setVisible(false);
        if (m_rightArrow != 0) m_rightArrow->setVisible(false);
    } else {
        for (unsigned int j = 0; j < m_items.size(); ++j)
            m_items[j]->setAlpha(1.0f);
        HandleScrollListArrows(false);
    }
    m_selectorEnabled = enabled;
}

void FJ_TexturePackList::addPack(int id, const std::wstring& name)
{
    if (getParent() == 0) return;
    FJ_TexturePackList_Slot* slot = static_cast<FJ_TexturePackList_Slot*>(
        FJ_Factory::createObject("fourj.listClasses.FJ_TexturePackList_Slot", m_pRenderNode, true, 0));
    if (slot == 0) return;
    slot->SetSlotID(id);
    slot->SetTextureName(name);
    if (m_visibleSlots == 0) {
        m_visibleSlots = static_cast<int>(getWidth() / slot->getWidth() - 1.0f);
        m_slotSpacing = static_cast<int>(-(static_cast<float>(m_visibleSlots) * slot->getWidth() -
            getWidth()) * 0.5f);
    }
    slot->setY(getY() + Math::round((m_slotArea->getHeight() - slot->getHeight()) * 0.5f));
    m_items.push_back(slot);
    ScrollList(true, true, false);
    getParent()->addChild(slot);
    HandleScrollListArrows(true);
}

void FJ_TexturePackList::removeAllItems()
{
    FJ_FuiNode* parent = getParent();
    for (unsigned int i = 0; i < m_items.size(); ++i)
        if (parent != 0) parent->removeChild(m_items[i], true);
    m_items.clear();
    m_selectedSlot = 0;
    m_topSlot = 0;
    HandleScrollListArrows(false);
}

void FJ_TexturePackList::SetTouchFocus(int x, int y, bool focus)
{
    if (!m_selectorEnabled) return;
    int touchX = x + static_cast<int>(getX());
    if (!focus) {
        for (unsigned int i = 0; i < m_items.size(); ++i) {
            if (touchX >= static_cast<int>(m_items[i]->getX()) &&
                touchX < static_cast<int>(m_items[i]->getX() + m_items[i]->getWidth())) {
                m_touchSlot = static_cast<int>(i);
                m_touchX = touchX;
                m_touchMoved = false;
                return;
            }
        }
    } else if (touchX != m_touchX && !m_items.empty()) {
        int amount = static_cast<int>((touchX > m_touchX ? touchX - m_touchX : m_touchX - touchX) /
            m_items[0]->getHeight());
        if (amount > 0) {
            m_touchMoved = true;
            m_topSlot += touchX > m_touchX ? amount : -amount;
            if (m_topSlot < 0) m_topSlot = 0;
            int maxTop = static_cast<int>(m_items.size()) - m_visibleSlots;
            if (m_topSlot > maxTop) m_topSlot = maxTop;
            ScrollList(touchX > m_touchX, false, true);
        }
        m_touchX = touchX;
    }
}

unsigned int FJ_TexturePackList::CanTouchTrigger(int x, int y)
{
    if (!m_selectorEnabled) return 0;
    int touchX = x + static_cast<int>(getX());
    for (unsigned int i = 0; i < m_items.size(); ++i) {
        if (touchX >= static_cast<int>(m_items[i]->getX()) &&
            touchX < static_cast<int>(m_items[i]->getX() + m_items[i]->getWidth())) {
            if (!m_touchMoved && static_cast<int>(i) == m_touchSlot) {
                if (m_items[i]->isVisible()) {
                    m_selectedSlot = static_cast<int>(i);
                    SelectSlot(m_touchSlot);
                    return 1;
                }
                NavigateList(static_cast<int>(i) > m_selectedSlot);
            }
            break;
        }
    }
    for (unsigned int j = 0; j < m_items.size(); ++j)
        static_cast<FJ_TexturePackList_Slot*>(m_items[j])->SetSelected(static_cast<int>(j) == m_selectedSlot);
    return 0;
}

int FJ_TexturePackList::GetRealHeight()
{
    return m_slotArea == 0 ? 0 : static_cast<int>(m_slotArea->getHeight());
}
