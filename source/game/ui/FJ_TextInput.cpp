#include "game/ui/FJ_TextInput.hpp"

#include <algorithm>
#include <string>
#include <utility>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_ScrollArrowH.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"

FJ_TextInput::FJ_TextInput(fuiRenderNode* renderNode)
    : FJ_Base(renderNode), m_scrollWidth(100), m_state(0), m_firstFocus(true),
      m_caretIndex(0), m_scrollOffset(0.0f), m_charLimit(-1), m_pCaret(0),
      m_pOutline(0), m_pLeftArrow(0), m_pRightArrow(0), m_pTextField(0),
      m_bKeepCaret(false), m_bUseScale(false)
{
}

FJ_TextInput::~FJ_TextInput()
{
}

FJ_Base* FJ_TextInput::Create(fuiRenderNode* renderNode)
{
    return new FJ_TextInput(renderNode);
}

unsigned int FJ_TextInput::getType()
{
    return 0x803d;
}

void FJ_TextInput::ChangeState(int state)
{
    m_state = state;
    if (m_pOutline != 0 && !m_bKeepCaret)
        m_pOutline->setVisible(state == 0);
    if (m_pCaret != 0)
        m_pCaret->setVisible(state == 0);
}

void FJ_TextInput::PositionScrollArrows()
{
    if (m_pLeftArrow == 0)
        m_pLeftArrow = FJ_Factory::createObject("fourj.FJ_ScrollArrowH", m_pRenderNode, true, 0);
    if (m_pRightArrow == 0)
        m_pRightArrow = FJ_Factory::createObject("fourj.FJ_ScrollArrowH", m_pRenderNode, true, 0);
    if (m_pLeftArrow == 0 || m_pRightArrow == 0)
        return;

    fuiRect rect = getRect(this);
    const float scale = 0.5f;
    m_pLeftArrow->setX(rect.xMin + getX() - m_pLeftArrow->getWidth() * scale);
    m_pLeftArrow->setY(rect.yMin + getY() + getHeight() * scale);
    rect = getRect(this);
    m_pRightArrow->setX(rect.xMin + getX() + getWidth() + m_pRightArrow->getWidth() * scale);
    m_pRightArrow->setY(m_pLeftArrow->getY());
}

void FJ_TextInput::HandleScrollArrows(bool addArrows)
{
    if (addArrows)
    {
        FJ_FuiNode* parent = getParent();
        if (parent != 0)
        {
            parent->addChild(m_pLeftArrow);
            parent->addChild(m_pRightArrow);
        }
        PositionScrollArrows();
        if (m_pLeftArrow != 0)
            static_cast<FJ_ScrollArrowH*>(m_pLeftArrow)->ChangeState(0);
        if (m_pRightArrow != 0)
            static_cast<FJ_ScrollArrowH*>(m_pRightArrow)->ChangeState(2);
    }
    if (m_pTextField == 0 || m_pLeftArrow == 0 || m_pRightArrow == 0)
        return;

    float scroll = m_pTextField->getScrollH();
    if (scroll <= 0.0f)
    {
        m_pLeftArrow->setVisible(false);
        m_pRightArrow->setVisible(isVisible() && m_pTextField->getWidth() < m_pTextField->m_textWidth - scroll);
    }
    else
    {
        m_pLeftArrow->setVisible(isVisible());
        m_pRightArrow->setVisible(isVisible() && m_pTextField->getWidth() < m_pTextField->m_textWidth - scroll);
    }
}

void FJ_TextInput::UpdateScrollPosition(float caretPosition, bool movingRight, bool force)
{
    if (m_pTextField == 0)
        return;
    const float minimum = 0.0f;
    if (!movingRight)
    {
        if (caretPosition < m_pTextField->getScrollH() || (force && !m_pRightArrow->isVisible()))
        {
            float scroll = caretPosition - static_cast<float>(m_scrollWidth);
            if (scroll < minimum)
                scroll = minimum;
            m_pTextField->setScrollH(scroll);
            m_scrollOffset = -m_pTextField->getScrollH();
            if (m_pLeftArrow != 0)
                static_cast<FJ_ScrollArrowH*>(m_pLeftArrow)->ChangeState(1);
        }
    }
    else
    {
        float width = static_cast<float>(static_cast<int>(m_pTextField->getWidth()));
        float scroll = m_pTextField->getScrollH();
        if (!force)
        {
            if (caretPosition - scroll <= width)
            {
                HandleScrollArrows(false);
                return;
            }
            scroll = caretPosition - width + 5.0f + static_cast<float>(m_scrollWidth);
            if (scroll < 0.0f)
                scroll = 0.0f;
        }
        else
        {
            scroll = caretPosition - width + 5.0f;
            if (scroll < 0.0f)
                scroll = 0.0f;
        }
        m_pTextField->setScrollH(scroll);
        m_scrollOffset = -m_pTextField->getScrollH();
        if (m_pRightArrow != 0)
            static_cast<FJ_ScrollArrowH*>(m_pRightArrow)->ChangeState(3);
    }
    HandleScrollArrows(false);
}

void FJ_TextInput::SetCaretIndex(int index, bool movingRight, bool force)
{
    if (index > 0)
    {
        if (index > static_cast<int>(m_label.size()))
            index = static_cast<int>(m_label.size());
        m_caretIndex = index;
        float caretPosition = 0.0f;
        for (int i = 0; i < index; ++i)
        {
            if (m_pTextField != 0)
            {
                std::pair<float, float> bounds = m_pTextField->getCharBoundaries(i);
                caretPosition += bounds.second - bounds.first;
            }
        }
        UpdateScrollPosition(caretPosition, movingRight, force);
        if (m_pCaret != 0 && m_pTextField != 0)
        {
            m_pCaret->setX(m_pTextField->getX() + caretPosition + m_scrollOffset);
            m_pCaret->setY(m_pTextField->getY() + m_pTextField->getHeight() - m_pCaret->getHeight());
        }
        return;
    }
    m_caretIndex = 0;
    UpdateScrollPosition(0.0f, movingRight, force);
    if (m_pCaret != 0 && m_pTextField != 0)
    {
        m_pCaret->setX(m_pTextField->getX() + m_scrollOffset);
        m_pCaret->setY(m_pTextField->getY() + m_pTextField->getHeight() - m_pCaret->getHeight());
    }
}

void FJ_TextInput::focusInHandler(FJ_Event*)
{
    ChangeState(0);
    if (m_firstFocus)
    {
        SetCaretIndex(static_cast<int>(m_label.size()), true, false);
        m_firstFocus = false;
    }
}

void FJ_TextInput::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TextInput*>(node)->focusInHandler(event);
}

void FJ_TextInput::focusOutHandler(FJ_Event*)
{
    m_state = 1;
    if (m_pOutline != 0 && !m_bKeepCaret)
        m_pOutline->setVisible(false);
    if (m_pCaret != 0)
        m_pCaret->setVisible(false);
}

void FJ_TextInput::_focusOutHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TextInput*>(node)->focusOutHandler(event);
}

void FJ_TextInput::MoveCaretLeft()
{
    SetCaretIndex(m_caretIndex - 1, false, false);
}

void FJ_TextInput::MoveCaretRight()
{
    SetCaretIndex(m_caretIndex + 1, true, false);
}

void FJ_TextInput::keyDownHandler(FJ_Event* event)
{
    if (event->m_eType == eFJEventType_FocusChangeInit)
        MoveCaretLeft();
    else if (event->m_eType == eFJEventType_FocusChangeUser)
        MoveCaretRight();
    else if (event->m_eType == eFJEventType_KeyUp)
        getRoot();
}

void FJ_TextInput::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TextInput*>(node)->keyDownHandler(event);
}

void FJ_TextInput::Init(std::wstring name, int id)
{
    m_pCaret = getChildByName(std::string("Caret"))->asMovieClip();
    m_pOutline = getChildByName(std::string("Outline"))->asMovieClip();
    if (m_pCaret != 0)
    {
        m_pCaret->setScaleX(1.0f / getScaleX());
        m_pCaret->setScaleY(1.0f / getScaleY());
    }
    if (m_pLeftArrow == 0)
        m_pLeftArrow = FJ_Factory::createObject("fourj.FJ_ScrollArrowH", m_pRenderNode, true, 0);
    if (m_pRightArrow == 0)
        m_pRightArrow = FJ_Factory::createObject("fourj.FJ_ScrollArrowH", m_pRenderNode, true, 0);
    ChangeState(1);
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
    addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    FJ_FuiNode* container = getChildByName(std::string("FJ_TextContainer"));
    if (container != 0)
    {
        FJ_FuiNode* child = container->getChildAt(0);
        if (child != 0)
        {
            fuiRenderNode* timelineNode = child->asMovieClip()->m_pRenderNode->asTimeline();
            if (timelineNode != 0)
            {
                fuiRenderNodeTimeline* timeline = static_cast<fuiRenderNodeTimeline*>(timelineNode);
                m_pTextField = static_cast<fuiRenderNodeEditText*>(timeline->getChildAt(0)->asEditText());
            }
        }
    }
    fuiRect bounds = m_pTextField->getBounds();
    m_pTextField->setScrollRect(bounds);
    SetLabelAlignment(4);
    m_isLabel = false;
    if (!m_bUseScale)
    {
        if (getScaleX() != 1.0f) setScaleX(1.0f);
        if (getScaleY() != 1.0f) setScaleY(1.0f);
    }
    InitBase(name, id);
    HandleScrollArrows(true);
}

void FJ_TextInput::setX(float x)
{
    FJ_FuiNode::setX(x);
    PositionScrollArrows();
}

void FJ_TextInput::setY(float y)
{
    FJ_FuiNode::setY(y);
    PositionScrollArrows();
}

void FJ_TextInput::setVisible(bool visible)
{
    FJ_FuiNode::setVisible(visible);
    HandleScrollArrows(false);
}

void FJ_TextInput::SetLabel(const std::wstring& label)
{
    FJ_Base::SetLabel(label);
    if (m_pTextField != 0)
    {
        m_pTextField->setScrollH(0.0f);
        SetCaretIndex(static_cast<int>(m_label.size()), true, true);
    }
}

void FJ_TextInput::SetCharLimit(int limit)
{
    m_charLimit = limit;
}

void FJ_TextInput::HandleDirectInput(unsigned int character)
{
    if (m_pTextField == 0)
        return;
    std::wstring text = m_label;
    if (character == 8)
    {
        if (!m_label.empty() && m_caretIndex > 0)
        {
            text.erase(static_cast<std::wstring::size_type>(m_caretIndex - 1), 1);
            FJ_Base::SetLabel(text);
            SetCaretIndex(m_caretIndex - 1, false, true);
        }
        return;
    }
    if (static_cast<int>(m_label.size()) < m_charLimit || m_charLimit == -1)
    {
        text.insert(static_cast<std::wstring::size_type>(m_caretIndex), 1, static_cast<wchar_t>(character & 0xffff));
        FJ_Base::SetLabel(text);
        SetCaretIndex(m_caretIndex + 1, true, false);
    }
}

void FJ_TextInput::HandleCharInput(unsigned int character)
{
    HandleDirectInput(character);
}

void FJ_TextInput::GetLabelTooWide(fuiRenderNodeEditText*)
{
}

void FJ_TextInput::SetNewObjectWidth(int)
{
}
