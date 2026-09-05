#include "game/ui/FJ_DynamicHtmlText.hpp"

#include <cstdio>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/fuiRenderNode.hpp"

class FJ_Factory
{
public:
    static FJ_FuiNode* createObject(const char*, fuiRenderNode*, bool, const char*);
};

FJ_DynamicHtmlText::FJ_DynamicHtmlText(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode), m_text(), m_labels(), m_labelTextIndices()
{
    m_pHtmlTextPanel = NULL;
}

FJ_FuiNode* FJ_DynamicHtmlText::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_DynamicHtmlText(pRenderNode);
}

FJ_DynamicHtmlText::~FJ_DynamicHtmlText()
{
    if (m_pUpArrow != NULL && m_pUpArrow->getParent() != NULL)
        m_pUpArrow->getParent()->removeChild(m_pUpArrow, true);
    if (m_pDownArrow != NULL && m_pDownArrow->getParent() != NULL)
        m_pDownArrow->getParent()->removeChild(m_pDownArrow, true);
    removeEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling);
    m_labelTextIndices.clear();
    m_labels.clear();
    m_text.clear();
}

int FJ_DynamicHtmlText::getActiveLabelHeight() const
{
    int height = m_iScrollPosition * 2 + m_iTopMargin;
    if (!m_labels.empty())
        height += (int)m_labels.back()->getY() + (int)m_labels.back()->getHeight();
    return height;
}

void FJ_DynamicHtmlText::UpdateTouchFlickScrolling()
{
    if (m_bTouchScrolling || !m_bTouchFlick)
        return;
    if (m_iFlickOffset < 1) {
        if (m_iFlickOffset >= -1) {
            m_iScrollDirection = 0;
            m_iTouchStart = 0;
            m_iCurrentScroll = 0;
            m_bTouchFlick = false;
            if (m_pUpArrow != NULL) m_pUpArrow->ChangeState(0);
            if (m_pDownArrow != NULL) m_pDownArrow->ChangeState(3);
            return;
        }
        m_iFlickOffset += 5;
        if (m_iFlickOffset > 0) m_iFlickOffset = 0;
        if (m_iScrollDirection != -1) {
            if (m_pUpArrow != NULL) m_pUpArrow->ChangeState(2);
            if (m_pDownArrow != NULL) m_pDownArrow->ChangeState(3);
        }
        if (m_pUpArrow != NULL && m_pUpArrow->isVisible()) {
            m_iScrollDirection = -1;
            m_iTouchScroll = -1;
            m_iTargetDirection = -1;
            return;
        }
    } else {
        m_iFlickOffset -= 5;
        if (m_iFlickOffset < 0) m_iFlickOffset = 0;
        if (m_iScrollDirection != 1) {
            if (m_pDownArrow != NULL) m_pDownArrow->ChangeState(5);
            if (m_pUpArrow != NULL) m_pUpArrow->ChangeState(0);
        }
        if (m_pDownArrow != NULL && m_pDownArrow->isVisible()) {
            m_iScrollDirection = 1;
            m_iTouchScroll = 1;
            m_iTargetDirection = 1;
            return;
        }
    }
    m_iScrollDirection = 0;
    m_iCurrentScroll = m_iTouchStart;
}

unsigned int FJ_DynamicHtmlText::SetNextLabel(int index, bool append)
{
    FJ_FuiNode* labelNode = m_pLabels[0];
    int slot = 0;
    if (isVisible()) {
        while (slot < 20 && m_pLabels[slot] != NULL && m_pLabels[slot]->isVisible())
            ++slot;
        if (slot >= 20) return 0;
        labelNode = m_pLabels[slot];
    }
    if (labelNode == NULL) return 0;
    labelNode->setVisible(true);
    labelNode->gotoAndStop(index);
    if (append) {
        if (!m_labels.empty()) {
            FJ_FuiNode* previous = m_labels.back();
            labelNode->setY(previous->getY() + previous->getHeight());
        } else {
            labelNode->setY(176.0f);
        }
        m_labels.push_back(static_cast<FJ_Label*>(labelNode));
        m_labelTextIndices.push_back(index);
        return 1;
    }
    if (!m_labels.empty()) {
        FJ_FuiNode* previous = m_labels.front();
        labelNode->setY(previous->getY() - labelNode->getHeight());
    }
    m_labels.insert(m_labels.begin(), static_cast<FJ_Label*>(labelNode));
    m_labelTextIndices.insert(m_labelTextIndices.begin(), index);
    return 1;
}

void FJ_DynamicHtmlText::HandleScrollListArrows(bool positionArrows)
{
    if (positionArrows) {
        if (getParent() != NULL) {
            getParent()->addChild(m_pUpArrow);
            getParent()->addChild(m_pDownArrow);
        }
        if (m_pUpArrow != NULL) {
            m_pUpArrow->setX(getX() + m_fRealWidth - m_iScrollPosition);
            m_pUpArrow->setY(getY() + m_iScrollPosition * -1.0f);
        }
        if (m_pDownArrow != NULL) {
            m_pDownArrow->setX(getX() + m_fRealWidth - m_iScrollPosition);
            m_pDownArrow->setY(getY() + m_fRealHeight - m_iScrollPosition);
        }
    }
    if (m_labels.empty()) return;
    bool end = m_labels.back()->getY() + m_labels.back()->getHeight() + m_iScrollPosition * 2 < m_fRealHeight;
    m_bListReady = !end;
    m_bAtEnd = end;
    if (m_pUpArrow != NULL) m_pUpArrow->setVisible(!m_bListReady);
    if (m_pDownArrow != NULL) m_pDownArrow->setVisible(!end && !m_bAtEnd);
    if (!m_pUpArrow->isVisible() && !m_pDownArrow->isVisible())
        removeEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling);
}

void FJ_DynamicHtmlText::UpdateScrolling(FJ_Event*)
{
    UpdateTouchFlickScrolling();
    if (m_labels.empty()) return;
    int delta = 0;
    if (m_iScrollDirection < 0) delta = -m_iCharacterHeight;
    if (m_iScrollDirection > 0) delta = m_iCharacterHeight;
    if (delta == 0) {
        HandleScrollListArrows(false);
        return;
    }
    for (std::vector<FJ_Label*>::iterator it = m_labels.begin(); it != m_labels.end(); ++it)
        (*it)->setY((*it)->getY() - (float)delta);
    m_iCurrentScroll -= delta;
    HandleScrollListArrows(false);
}

void FJ_DynamicHtmlText::_UpdateScrolling(FJ_FuiNode* node, FJ_Event* event)
{
    FJ_DynamicHtmlText* text = node->asFJ_DynamicHtmlText();
    if (text != NULL) text->UpdateScrolling(event);
}

void FJ_DynamicHtmlText::Init()
{
    m_fRealWidth = getWidth();
    m_fRealHeight = getHeight() - (m_pBackground != NULL ? m_pBackground->getHeight() : 0.0f);
    FJ_FuiNode* panel = getChildByName(std::string("HtmlTextPanel"));
    if (panel != NULL) {
        m_pHtmlTextPanel = panel->getChildAt(0);
        panel->removeChild(m_pHtmlTextPanel, false);
        removeChild(panel, true);
        if (getParent() != NULL) getParent()->addChild(m_pHtmlTextPanel);
        m_pHtmlTextPanel->setX(getX());
        m_pHtmlTextPanel->setY(getY());
        m_pHtmlTextPanel->setWidth(m_fRealWidth);
        m_pHtmlTextPanel->setHeight(m_fRealHeight);
    }
    if (getChildByName(std::string("HtmlBackground")) != NULL) {
        removeChild(getChildByName(std::string("HtmlBackground")), true);
        m_pBackground = NULL;
    }
    m_pHtmlTextPanel = FJ_Factory::createObject("FJ_HtmlLabel", m_pRenderNode, true, NULL);
    if (getParent() != NULL) getParent()->addChild(m_pHtmlTextPanel);
    m_pHtmlTextPanel->setX(getX() + (float)m_iScrollPosition);
    m_pHtmlTextPanel->setY(getY() + (float)m_iScrollPosition);
    m_scrollRect.xMin = 0.0f;
    m_scrollRect.xMax = m_fRealWidth;
    m_scrollRect.yMin = 0.0f;
    m_scrollRect.yMax = m_fRealHeight;
    m_pHtmlTextPanel->setScrollRect(m_scrollRect);
    for (int i = 0; i < 20; ++i) {
        char name[64];
        std::sprintf(name, "FJ_HtmlLabel_%d", i);
        FJ_FuiNode* label = getChildByName(std::string(name));
        m_pLabels[i] = label;
        if (label != NULL) {
            m_pHtmlTextPanel->addChild(label);
            label->setWidth(m_fRealWidth - (float)(m_iScrollPosition * 2));
            label->setX(0.0f);
            label->setY(0.0f);
            label->setVisible(false);
            static_cast<FJ_Label*>(label)->ResetLabel();
        }
    }
}

void FJ_DynamicHtmlText::ASConstructor()
{
    m_bAtEnd = false;
    m_iCharacterHeight = 10;
    m_iScrollPosition = 20;
    m_bTouchFlick = false;
    m_iScrollDelay = 0;
    m_iFlickDelay = 0;
    m_iTargetDirection = 0;
    m_iCurrentScroll = 0;
    m_iTouchScroll = 0;
    m_iTouchStart = 0;
    m_iFlickOffset = 0;
    m_iScrollDirection = 0;
    m_bListReady = false;
    m_bKeyboardScroll = false;
    m_pUpArrow = static_cast<FJ_ScrollArrow*>(FJ_Factory::createObject("fourj.FJ_ScrollArrow", m_pRenderNode, true, NULL));
    m_pDownArrow = static_cast<FJ_ScrollArrow*>(FJ_Factory::createObject("fourj.FJ_ScrollArrow", m_pRenderNode, true, NULL));
    Init();
    FJ_FuiNodeStage* stage = getStage();
    if (stage != NULL) {
        stage->addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
        stage->addEventListener(eFJEventType_KeyUp, this, _keyUpHandler, false);
    }
    m_fScrollSpeed = 1.0f;
    if (stage != NULL && stage->getStageHeight() == 540.0)
        m_fScrollSpeed = 1.5f;
}

void FJ_DynamicHtmlText::keyDownHandler(FJ_Event* event)
{
    if ((m_pUpArrow != NULL && m_pUpArrow->isVisible()) ||
        (m_pDownArrow != NULL && m_pDownArrow->isVisible())) {
        if (event->m_eType == eFJEventType_FocusOut ||
            event->m_eType == 0x11 || event->m_eType == eFJEventType_KeyDown) {
            if (m_iScrollDirection != -1) {
                m_pUpArrow->ChangeState(2);
                m_pDownArrow->ChangeState(3);
            }
            if (event->m_eType == eFJEventType_KeyDown) {
                m_iFlickDelay = 0;
                m_iScrollDelay = 0;
                m_bKeyboardScroll = true;
            }
            if (m_pUpArrow->isVisible()) {
                m_iTargetDirection = -1;
                m_iScrollDirection = -1;
            }
        }
        if (event->m_eType == 0x04 || event->m_eType == 0x12 ||
            event->m_eType == eFJEventType_FocusIn) {
            if (m_iScrollDirection != 1) {
                m_pDownArrow->ChangeState(5);
                m_pUpArrow->ChangeState(0);
            }
            if (event->m_eType == eFJEventType_FocusIn) {
                m_iFlickDelay = 0;
                m_iScrollDelay = 0;
                m_bKeyboardScroll = true;
            }
            if (m_pDownArrow->isVisible()) {
                m_iTargetDirection = 1;
                m_iScrollDirection = 1;
            }
        }
    }
}

void FJ_DynamicHtmlText::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    FJ_DynamicHtmlText* text = node->asFJ_DynamicHtmlText();
    if (text != NULL) text->keyDownHandler(event);
}

void FJ_DynamicHtmlText::keyUpHandler(FJ_Event*)
{
    m_iScrollDirection = 0;
    m_bKeyboardScroll = false;
    if (m_pUpArrow != NULL) m_pUpArrow->ChangeState(0);
    if (m_pDownArrow != NULL) m_pDownArrow->ChangeState(3);
}

void FJ_DynamicHtmlText::_keyUpHandler(FJ_FuiNode* node, FJ_Event* event)
{
    FJ_DynamicHtmlText* text = node->asFJ_DynamicHtmlText();
    if (text != NULL) text->keyUpHandler(event);
}

unsigned int FJ_DynamicHtmlText::getType()
{
    return 0x80aa;
}

void FJ_DynamicHtmlText::setVisible(bool visible)
{
    FJ_FuiNode::setVisible(visible);
    if (!visible) {
        removeEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling);
        for (int i = 0; i < 20; ++i)
            if (m_pLabels[i] != NULL) m_pLabels[i]->setVisible(false);
    }
    if (m_pHtmlTextPanel != NULL) m_pHtmlTextPanel->setVisible(visible);
    if (m_pUpArrow != NULL) m_pUpArrow->setVisible(visible);
    if (m_pDownArrow != NULL) m_pDownArrow->setVisible(visible);
    if (visible) HandleScrollListArrows(false);
}

void FJ_DynamicHtmlText::Reload(float width, float height)
{
    m_fRealWidth = width;
    m_fRealHeight = height - (m_pUpArrow != NULL ? m_pUpArrow->getHeight() : 0.0f);
    if (m_pHtmlTextPanel != NULL) {
        m_pHtmlTextPanel->setX(getX());
        m_pHtmlTextPanel->setY(getY());
        m_pHtmlTextPanel->setWidth(width);
        m_pHtmlTextPanel->setHeight(height);
    }
    m_scrollRect.xMin = getX() + (float)m_iScrollPosition;
    m_scrollRect.xMax = m_scrollRect.xMin + width;
    m_scrollRect.yMin = getY() + (float)m_iScrollPosition;
    m_scrollRect.yMax = m_scrollRect.yMin + m_fRealHeight;
    if (m_pHtmlTextPanel != NULL) m_pHtmlTextPanel->setScrollRect(m_scrollRect);
    for (int i = 0; i < 20; ++i) {
        if (m_pLabels[i] != NULL) {
            static_cast<FJ_Label*>(m_pLabels[i])->ResetLabel();
            m_pLabels[i]->setWidth(width - (float)(m_iScrollPosition * 2));
            m_pLabels[i]->setVisible(false);
        }
    }
    m_iCharacterHeight = 0;
    m_iScrollDirection = 0;
    m_iTargetDirection = 0;
    m_iCurrentScroll = 0;
    m_iTouchStart = 0;
    m_iFlickOffset = 0;
    m_bKeyboardScroll = false;
    m_bTouchScrolling = false;
    m_bTouchFlick = false;
    m_iContentHeight = 0;
    m_labels.clear();
    m_labelTextIndices.clear();
    removeEventListener(eFJEventType_AnimationStep, this, _UpdateScrolling);
}

void FJ_DynamicHtmlText::SetupList()
{
    unsigned int line = 0;
    bool finished = false;
    while (line < m_text.size() && !finished) {
        SetNextLabel((int)line, true);
        ++line;
        if (!m_labels.empty()) {
            FJ_Label* last = m_labels.back();
            finished = getActiveLabelHeight() + m_iScrollPosition >= (int)m_fRealHeight;
            m_bListReady = true;
            m_bAtEnd = finished;
            (void)last;
        }
    }
}

void FJ_DynamicHtmlText::AddText(const std::wstring& text, bool setup)
{
    m_text.push_back(text);
    if (setup) {
        addEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling, false);
        SetupList();
        HandleScrollListArrows(true);
        if (!m_labels.empty()) m_iCharacterHeight = m_labels.front()->GetAverageCharHeight();
    }
}

double FJ_DynamicHtmlText::GetRealWidth()
{
    return (double)m_fRealWidth;
}

double FJ_DynamicHtmlText::GetRealHeight()
{
    return (double)m_fRealHeight;
}

void FJ_DynamicHtmlText::TouchScroll(int offset, bool active)
{
    if (!active) {
        if (m_iFlickOffset != 0) m_bTouchFlick = true;
        m_iScrollDirection = 0;
        m_iTouchStart = 0;
        m_iCurrentScroll = 0;
        m_bTouchScrolling = false;
        if (m_pUpArrow != NULL) m_pUpArrow->ChangeState(0);
        if (m_pDownArrow != NULL) m_pDownArrow->ChangeState(3);
        return;
    }
    m_iTouchStart = offset;
    if (!m_bTouchScrolling) {
        m_iCurrentScroll = offset;
        m_bTouchFlick = false;
        m_bTouchScrolling = true;
        return;
    }
    int movement = offset - m_iCurrentScroll;
    if (movement < 0) m_iScrollDirection = 1;
    else if (movement > 0) m_iScrollDirection = -1;
    else m_iScrollDirection = 0;
    m_iFlickOffset = movement;
}
