#include "game/ui/FJ_HtmlText.hpp"

#include <string>
#include <utility>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

FJ_HtmlText::FJ_HtmlText(fuiRenderNode* renderNode)
    : FJ_Base(renderNode)
{
    m_pText = NULL;
    m_bScrollingEnabled = 1;
    m_bScrollingDown = false;
    m_bTouchScrolling = false;
    m_scrollDirection = 0;
    m_iFocusDirection = 0;
    m_bNoBackground = false;
    m_bScrollingUp = false;
    m_pScrollingCompleteDelegate = NULL;
    m_bInitialized = 1;
    m_iTouchStart = 0;
    m_iTouchPosition = 0;
    m_iTouchScroll = 0;
    m_iTouchTarget = 0;
    m_bUseScrollRect = true;
    m_iTargetScroll = 0;
    m_pUpArrow = FJ_Factory::createObject("fourj.FJ_ScrollArrow", renderNode, true, NULL);
    m_pDownArrow = static_cast<FJ_ScrollArrow*>(FJ_Factory::createObject("fourj.FJ_ScrollArrow", renderNode, true, NULL));
    FJ_FuiNodeStage* stage = getStage();
    if (stage != NULL) {
        stage->addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
        stage->addEventListener(eFJEventType_KeyUp, this, _keyUpHandler, false);
    }
    if (stage != NULL && (stage->getStageHeight() == 1080.0f || stage->getStageHeight() == 540.0f))
        m_fScrollSpeed = 1.5f;
}

FJ_HtmlText::~FJ_HtmlText()
{
    if (m_pUpArrow != NULL && m_pUpArrow->getParent() != NULL)
        m_pUpArrow->getParent()->removeChild(m_pUpArrow, true);
    if (m_pDownArrow != NULL && m_pDownArrow->getParent() != NULL)
        m_pDownArrow->getParent()->removeChild(m_pDownArrow, true);
}

unsigned int FJ_HtmlText::CanScroll(bool up)
{
    if (m_pText != NULL && m_bScrollingEnabled) {
        float position = m_fScrollTop;
        float height = static_cast<fuiRenderNodeEditText*>(m_pText->asEditText())->getHeight();
        float distance = m_fScrollBottom - position;
        if (distance <= height) {
            if (!up) {
                if (position < height - distance)
                    return 1;
            } else if (0.0f < position)
                return 1;
        }
    }
    return 0;
}

void FJ_HtmlText::HandleScrollListArrows(bool positionArrows)
{
    if (positionArrows) {
        if (getParent() != NULL) {
            getParent()->addChild(m_pUpArrow);
            getParent()->addChild(m_pDownArrow);
        }
        m_pUpArrow->setVisible(true);
        m_pDownArrow->setVisible(true);
        if (isVisible()) {
            static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(0);
            m_pDownArrow->ChangeState(3);
        }
    }
    if (isVisible() && !m_bNoBackground) {
        UpdateScrollListArrows();
        m_pUpArrow->setVisible(CanScroll(true) != 0);
        m_pDownArrow->setVisible(CanScroll(false) != 0);
        return;
    }
    m_pUpArrow->setVisible(false);
    m_pDownArrow->setVisible(false);
}

void FJ_HtmlText::UpdateScrolling(FJ_Event*)
{
    float step;
    if (m_iScrollDelay < 1) {
        if (m_iFlickDelay > -1) {
            if (m_iFlickDelay > 0)
                --m_iFlickDelay;
            step = 1.0f + (30.0f - static_cast<float>(m_iFlickDelay)) / 176.0f;
        }
        step = static_cast<float>(Math::round(120.0f * m_fScrollSpeed * step));
    } else {
        step = 120.0f * m_fScrollSpeed / 30.0f;
        --m_iScrollDelay;
        step = static_cast<float>(Math::round(step));
    }
    if ((!m_bScrollingUp || !CanScroll(true)) && (!m_scrollDirection || !CanScroll(false))) {
        HandleScrollListArrows(false);
        FJ_FuiNodeStage* stage = getStage();
        if (stage != NULL) stage->removeEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling);
        if (!isVisible()) {
            fuiRect scrollRect;
            scrollRect.xMin = static_cast<float>(m_iWidth);
            scrollRect.xMax = m_fWidth;
            scrollRect.yMin = m_fScrollTop;
            scrollRect.yMax = m_fScrollBottom;
            m_pText->setScrollRect(scrollRect);
            return;
        }
        static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(0);
        m_pDownArrow->ChangeState(3);
        m_fScrollTop = m_fScrollTop;
        m_fScrollBottom = m_fScrollBottom;
        return;
    }
    float oldPosition = m_fScrollTop;
    float newPosition;
    if (m_bScrollingUp) {
        newPosition = oldPosition - step;
        m_fScrollBottom += newPosition - oldPosition;
    } else {
        newPosition = oldPosition + step;
        m_fScrollBottom += newPosition - oldPosition;
    }
    m_fScrollTop = newPosition;
    if (m_bTouchScrolling) {
        float target = static_cast<float>(m_iTouchTarget);
        if ((m_bScrollingUp && target <= newPosition) || (!m_bScrollingUp && target >= newPosition)) {
            m_fScrollTop = target;
            m_fScrollBottom += target - newPosition;
            m_scrollDirection = 0;
            fuiRect scrollRect;
            scrollRect.xMin = static_cast<float>(m_iWidth);
            scrollRect.xMax = m_fWidth;
            scrollRect.yMin = m_fScrollTop;
            scrollRect.yMax = m_fScrollBottom;
            m_pText->setScrollRect(scrollRect);
            return;
        }
    }
    fuiRect scrollRect;
    scrollRect.xMin = static_cast<float>(m_iWidth);
    scrollRect.xMax = m_fWidth;
    scrollRect.yMin = m_fScrollTop;
    scrollRect.yMax = m_fScrollBottom;
    m_pText->setScrollRect(scrollRect);
}

void FJ_HtmlText::_UpdateScrolling(FJ_FuiNode* node, FJ_Event* event)
{
    FJ_HtmlText* text = node->asFJ_HtmlText();
    if (text != NULL) text->UpdateScrolling(event);
}

uint64_t FJ_HtmlText::keyDownHandler(FJ_Event* event)
{
    if (!m_bScrollingEnabled) return 0;
    FJ_FuiNodeStage* stage = getStage();
    bool focusChange = stage != NULL && stage->getFocus() != this;
    m_iFocusDirection = focusChange ? 1 : 0;
    int type = event->m_eType;
    bool directional = focusChange ? (type == 0x11 || type == 0x12) : (type == 3 || type == 4);
    bool pageDown = type == 2;
    if (type == eFJEventType_KeyDown) {
        bool wasScrollingUp = m_bScrollingUp;
        m_bScrollingDown = false;
        m_scrollDirection = 0;
        if (!wasScrollingUp && CanScroll(true)) {
            m_bScrollingUp = true;
            m_iScrollDelay = pageDown ? 0 : 30;
            m_iFlickDelay = pageDown ? 0 : 30;
            if (stage != NULL) stage->addEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling, false);
            if (isVisible()) {
                static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(2);
                m_pDownArrow->ChangeState(3);
            }
        }
    } else {
        if (pageDown) directional = true;
        if (!directional) return 0;
        m_bScrollingUp = false;
        m_scrollDirection = 0;
        if (!m_scrollDirection && CanScroll(false)) {
            m_bScrollingDown = true;
            m_iScrollDelay = pageDown ? 0 : 30;
            m_iFlickDelay = pageDown ? 0 : 30;
            m_scrollDirection = 1;
            if (stage != NULL) stage->addEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling, false);
            if (isVisible()) {
                m_pDownArrow->ChangeState(5);
                static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(0);
            }
        }
    }
    HandleScrollListArrows(false);
    return 0;
}

void FJ_HtmlText::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    FJ_HtmlText* text = node->asFJ_HtmlText();
    if (text != NULL) text->keyDownHandler(event);
}

uint64_t FJ_HtmlText::GetScrollTarget(bool up)
{
    int textHeight = getTextHeight();
    int current = Math::round(m_fScrollTop);
    int target = 0;
    if (!m_bScrollingUp) {
        if (!m_scrollDirection) return 0;
        if (up) current += Math::round(m_fScrollBottom - m_fScrollTop - m_iCharacterHeight * 3.0f);
        do { target += m_iCharacterHeight; } while (target < current);
        if (textHeight < target + m_iCharacterHeight) target = textHeight;
    } else {
        if (up) current -= Math::round(m_fScrollBottom - m_fScrollTop - m_iCharacterHeight * 3.0f);
        do { textHeight -= m_iCharacterHeight; } while (current < textHeight);
        target = textHeight;
        if (textHeight - m_iCharacterHeight < 0) { m_iTargetScroll = 0; return 0; }
    }
    m_iTargetScroll = target;
    return 0;
}

void FJ_HtmlText::keyUpHandler(FJ_Event* event)
{
    int type = event->m_eType;
    bool directional = type == eFJEventType_KeyUp || type == eFJEventType_KeyDown;
    if (!directional && type != 3 && type != 0x11) return;
    m_bScrollingDown = true;
    GetScrollTarget(type == eFJEventType_KeyUp || type == 3);
}

void FJ_HtmlText::_keyUpHandler(FJ_FuiNode* node, FJ_Event* event)
{
    FJ_HtmlText* text = node->asFJ_HtmlText();
    if (text != NULL) text->keyUpHandler(event);
}

int FJ_HtmlText::getTextHeight()
{
    return static_cast<int>(static_cast<fuiRenderNodeEditText*>(m_pText->asEditText())->getHeight());
}

unsigned int FJ_HtmlText::getType()
{
    return 0x80a1;
}

void FJ_HtmlText::setVisible(bool visible)
{
    FJ_FuiNode::setVisible(visible);
    if (m_pText != NULL) m_pText->setVisibility(visible);
    if (m_pUpArrow != NULL) { m_pUpArrow->setVisible(visible); static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(0); }
    if (m_pDownArrow != NULL) { m_pDownArrow->setVisible(visible); m_pDownArrow->ChangeState(3); }
    FJ_FuiNodeStage* stage = getStage();
    if (stage != NULL) {
        if (visible) {
            HandleScrollListArrows(false);
            stage->addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
            stage->addEventListener(eFJEventType_KeyUp, this, _keyUpHandler, false);
        } else {
            stage->removeEventListener(eFJEventType_KeyDown, this, _keyDownHandler);
            stage->removeEventListener(eFJEventType_KeyUp, this, _keyUpHandler);
        }
    }
}

void FJ_HtmlText::UpdateScrollListArrows()
{
    if (m_pUpArrow == NULL || m_pDownArrow == NULL || m_pText == NULL) return;
    m_pUpArrow->setX(-(m_pUpArrow->getWidth() * 1.5f - (m_pText->getX() + static_cast<float>(m_iRealWidth) - 176.0f)));
    m_pUpArrow->setY(m_pText->getY() + m_fScrollBottom - m_fScrollTop + m_pUpArrow->getHeight() * 0.5f);
    m_pDownArrow->setX(-(m_pDownArrow->getWidth() * 0.5f - (m_pText->getX() + static_cast<float>(m_iRealWidth) - 176.0f)));
    m_pDownArrow->setY(m_pUpArrow->getY());
}

void FJ_HtmlText::setX(float x)
{
    FJ_FuiNode::setX(x);
    if (m_pText != NULL) { m_pText->setX(x); UpdateScrollListArrows(); }
}

void FJ_HtmlText::setY(float y)
{
    float oldY = FJ_FuiNode::getY();
    FJ_FuiNode::setY(y);
    if (m_pText != NULL) { m_pText->setY(m_pText->getY() + (y - oldY)); UpdateScrollListArrows(); }
}

void FJ_HtmlText::setHeight(float height)
{
    FJ_FuiNode::setHeight(height);
    if (m_pText != NULL) {
        int arrowHeight = m_bNoBackground ? 0 : static_cast<int>(m_pUpArrow->getHeight());
        m_fScrollBottom = m_fScrollTop + height - static_cast<float>(arrowHeight);
        m_pText->setHeight(height);
        UpdateScrollListArrows();
    }
}

void FJ_HtmlText::Init(const std::wstring& label)
{
    float width = getWidth();
    float height = getHeight();
    FJ_FuiNode* background = getChildByName(std::string("HtmlBackground"));
    if (background != NULL) {
        m_iRealWidth = static_cast<int>(background->getWidth() * getScaleX());
        m_iRealHeight = static_cast<int>(background->getHeight() * getScaleY());
        removeChild(background, true);
    }

    removeChild(m_pText, false);
    getParent()->addChild(m_pText);
    m_pText->setX(getX());
    m_pText->setY(getY());
    m_pText->setWidth(width);
    const float availableHeight = height - static_cast<float>(m_bNoBackground ? 0 : static_cast<int>(m_pUpArrow->getHeight()));
    m_pText->setHeight(availableHeight);
    fuiRenderNodeEditText* text = static_cast<fuiRenderNodeEditText*>(m_pText->asEditText());
    text->setAutoSize(TextFieldAutoSizeLeft);
    text->m_wordWrap = true;
    text->m_multiline = true;
    m_scrollDirection = m_bUseScrollRect ? 1 : 0;
    m_bScrollingEnabled = m_bUseScrollRect;
    m_iWidth = 0;
    m_fWidth = width;
    m_fScrollTop = 0.0f;
    m_fScrollBottom = availableHeight;
    if (m_bUseScrollRect)
    {
        fuiRect scrollRect;
        scrollRect.xMin = 0.0f;
        scrollRect.xMax = width;
        scrollRect.yMin = 0.0f;
        scrollRect.yMax = availableHeight;
        m_pText->setScrollRect(scrollRect);
    }
    SetLabel(label);
    HandleScrollListArrows(true);
}

uint64_t FJ_HtmlText::GetScrollDistance()
{
    m_iCharacterHeight = 0;
    fuiRenderNodeEditText* text = static_cast<fuiRenderNodeEditText*>(m_pText->asEditText());
    unsigned int count = static_cast<unsigned int>(text->getText().size());
    int total = 0;
    int samples = 0;
    for (unsigned int i = 0; i < count && samples < 5; ++i) {
        std::pair<float, float> bounds = text->getCharBoundaries(static_cast<int>(i));
        total += static_cast<int>(bounds.second - bounds.first);
        ++samples;
        m_iCharacterHeight = total;
    }
    m_iCharacterHeight = total / samples;
    return 0;
}

void FJ_HtmlText::SetLabel(const std::wstring& label)
{
    std::wstring value = label;
    if (label.empty()) value = L" ";
    static_cast<fuiRenderNodeEditText*>(m_pText->asEditText())->setHTMLText(value);
    if (m_bScrollingEnabled) {
        m_fScrollBottom = m_fScrollBottom - m_fScrollTop;
        m_fScrollTop = 0.0f;
        fuiRect scrollRect;
        scrollRect.xMin = static_cast<float>(m_iWidth);
        scrollRect.xMax = m_fWidth;
        scrollRect.yMin = m_fScrollTop;
        scrollRect.yMax = m_fScrollBottom;
        m_pText->setScrollRect(scrollRect);
    }
    HandleScrollListArrows(false);
    GetScrollDistance();
    if (m_bScrollingEnabled) {
        m_bScrollingUp = false;
        m_bScrollingDown = false;
        m_scrollDirection = 0;
        m_iTargetScroll = 0;
    }
}

uint64_t FJ_HtmlText::GetNumberOfLines()
{
    return static_cast<uint64_t>(Math::round((m_fScrollBottom - m_fScrollTop) / static_cast<float>(m_iCharacterHeight)));
}

unsigned int FJ_HtmlText::GetOptimalHeight(int lines)
{
    int arrowHeight = m_bNoBackground ? 0 : static_cast<int>(m_pUpArrow->getHeight());
    if (lines == -1) {
        lines = GetNumberOfLines();
        if (lines < 3) return (m_iCharacterHeight + 2) * 3 + arrowHeight;
        if (lines > 8) {
            int height = (m_iCharacterHeight + 2) * lines + arrowHeight;
            while (lines > 8 && static_cast<float>(height) > m_fScrollBottom - m_fScrollTop)
                height = (m_iCharacterHeight + 2) * --lines + arrowHeight;
            return height;
        }
    }
    return (m_iCharacterHeight + 2) * lines + arrowHeight;
}

double FJ_HtmlText::GetRealWidth()
{
    return static_cast<double>(m_iRealWidth);
}

double FJ_HtmlText::GetRealHeight()
{
    return static_cast<double>(m_iRealHeight);
}

void FJ_HtmlText::TouchScroll(int position, bool active)
{
    if (!active) {
        m_scrollDirection = 0;
        m_iTouchStart = 0;
        m_iTouchPosition = 0;
        m_bTouchScrolling = false;
        m_bScrollingUp = false;
        m_bScrollingDown = true;
        m_iTouchScroll = 0;
        m_iTouchTarget = static_cast<int>(m_fScrollTop);
        return;
    }
    m_iTouchPosition = position;
    if (!m_bTouchScrolling) {
        m_iTouchStart = position;
        m_bTouchScrolling = true;
        m_iTouchTarget = static_cast<int>(m_fScrollTop);
        return;
    }
    int movement = position - m_iTouchStart;
    if (movement < 0) {
        m_bScrollingDown = false;
        m_bScrollingUp = false;
        if (!m_scrollDirection && CanScroll(false)) {
            m_iScrollDelay = 30;
            m_iFlickDelay = 30;
            m_scrollDirection = 1;
            m_iTouchTarget = static_cast<int>(m_fScrollTop) - movement;
            FJ_FuiNodeStage* stage = getStage();
            if (stage != NULL) stage->addEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling, false);
            m_pDownArrow->ChangeState(5);
            static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(0);
            HandleScrollListArrows(false);
        } else {
            m_iTouchTarget = m_iTouchScroll - movement;
            HandleScrollListArrows(false);
        }
        if (m_pScrollingCompleteDelegate != NULL && !CanScroll(false)) {
            m_pScrollingCompleteDelegate();
            m_pScrollingCompleteDelegate = NULL;
        }
    } else if (movement > 0) {
        m_bScrollingDown = false;
        m_bScrollingUp = false;
        if (!m_scrollDirection && CanScroll(true)) {
            m_iScrollDelay = 30;
            m_iFlickDelay = 30;
            m_scrollDirection = 1;
            m_iTouchTarget = static_cast<int>(m_fScrollTop) - movement;
            FJ_FuiNodeStage* stage = getStage();
            if (stage != NULL) stage->addEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling, false);
            static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(2);
            m_pDownArrow->ChangeState(3);
            HandleScrollListArrows(false);
        } else {
            m_iTouchTarget = m_iTouchScroll - movement;
            HandleScrollListArrows(false);
        }
    }
}
