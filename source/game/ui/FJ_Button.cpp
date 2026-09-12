#include "game/ui/FJ_Button.hpp"
#include <cmath>
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

static std::string s_buttonStates[8] =
{
    std::string("Active_First_Selected", 21),
    std::string("Inactive_Selected", 17),
    std::string("Active_Selected", 15),
    std::string("Inactive_Selected", 17),
    std::string("Active_Unselected", 17),
    std::string("Inactive_Unselected", 19),
    std::string("Active_Pressed", 14),
    std::string("Inactive_Selected", 17)
};

class FJ_ButtonRenderNodeAccess : public fuiRenderNode
{
public:
    static void setTint(fuiRenderNode* node, float value)
    {
        FJ_ButtonRenderNodeAccess* access = static_cast<FJ_ButtonRenderNodeAccess*>(node);
        access->m_greenAdd = value;
        access->m_blueAdd = value;
        access->m_alphaAdd = value;
    }
};

FJ_Button::FJ_Button(fuiRenderNode* renderNode)
    : FJ_Base(renderNode), m_buttonPressed(false), m_showOutline(false), m_enabled(true),
      m_currentState(0), m_keyDownDelegate(0), m_keyDownDelegateNode(0), m_focused(false),
      m_hiddenUntilInit(false), m_hasSecondaryText(false), m_secondaryText(),
      m_secondaryWidth(-1.0f), m_secondaryX(-1.0f), m_updateLabelDelegate(0),
      m_secondaryHtml(false), m_secondaryAutoSize(false)
{
    m_currentState = 2;
    ChangeState(2);
    ToggleOutline(false);
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
    addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    addEventListener(eFJEventType_KeyUp, this, _keyUpHandler, false);
}

FJ_Button::~FJ_Button() {}

FJ_FuiNode* FJ_Button::getObjListParent()
{
    return m_objListParent;
}

void FJ_Button::setButtonPressed(bool pressed) { m_buttonPressed = pressed; }

void FJ_Button::ChangeState(int state)
{
    m_currentState = state;
    std::string& stateName = s_buttonStates[(m_enabled ? 0 : 4) + state];
    if ((state == 0 || state == 3) && m_enabled)
        gotoAndPlay(stateName);
    else
        gotoAndStop(stateName);
}

void FJ_Button::ToggleOutline(bool show)
{
    FJ_FuiNode* outline = getChildByName(std::string("Outline"));
    if (outline != 0)
    {
        FJ_FuiNode* movieClip = outline->asMovieClip();
        if (movieClip != 0)
            movieClip->gotoAndPlay(show ? std::string("Outline_Enabled") : std::string("Outline_Disabled"));
    }
    m_showOutline = show;
}

void FJ_Button::focusInHandler(FJ_Event*)
{
    ChangeState(1);
    m_focused = true;
}

void FJ_Button::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Button*>(node)->focusInHandler(event);
}

void FJ_Button::focusOutHandler(FJ_Event*)
{
    ChangeState(2);
    m_focused = false;
}

void FJ_Button::_focusOutHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Button*>(node)->focusOutHandler(event);
}

void FJ_Button::keyDownHandler(FJ_Event* event)
{
    if (m_enabled && !m_buttonPressed && event->m_pTarget == 0)
    {
        ChangeState(3);
        m_buttonPressed = true;
        dispatchEvent(new FJ_Event(eFJEventType_Press, true, false));
        if (m_keyDownDelegate != 0)
            m_keyDownDelegate(m_keyDownDelegateNode, m_id);
    }
}

void FJ_Button::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Button*>(node)->keyDownHandler(event);
}

void FJ_Button::keyUpHandler(FJ_Event* event)
{
    if (!m_buttonPressed || event->m_pTarget != 0)
        return;
    m_buttonPressed = false;
}

void FJ_Button::_keyUpHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Button*>(node)->keyUpHandler(event);
}

void FJ_Button::HideUntilInit()
{
    setVisible(false);
    m_hiddenUntilInit = true;
}

void FJ_Button::HandleUpdateLabel(FJ_Event*) { UpdateLabel(); }

void FJ_Button::_HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Button*>(node)->HandleUpdateLabel(event);
}

void FJ_Button::Init(const std::wstring& name, int id)
{
    if (m_hiddenUntilInit)
    {
        setVisible(true);
        m_hiddenUntilInit = false;
    }
    InitBase(name, id);
    addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
}

void FJ_Button::SetKeyDownDelegate(void (*delegate)(FJ_FuiNode*, int), FJ_FuiNode* node)
{
    m_keyDownDelegate = delegate;
    m_keyDownDelegateNode = node;
}

fuiRenderNodeEditText* FJ_Button::GetSecondaryTextField()
{
    FJ_FuiNode* container = getChildByName(std::string("FJ_TextContainer_2"));
    if (container == 0)
        return 0;
    FJ_FuiNode* movieClip = container->asMovieClip();
    if (movieClip == 0 || movieClip->getNumChildren() == 0)
        return 0;
    FJ_FuiNode* child = movieClip->getChildAt(0);
    if (child == 0 || child->m_pRenderNode == 0)
        return 0;
    fuiRenderNode* timeline = child->m_pRenderNode->asTimeline();
    if (timeline == 0)
        return 0;
    m_hasSecondaryText = true;
    return static_cast<fuiRenderNodeEditText*>(timeline->asEditText());
}

void FJ_Button::SetSecondaryText(const std::wstring& text, bool html, bool autoSize)
{
    m_secondaryHtml = html;
    m_secondaryAutoSize = autoSize;
    FJ_FuiNode* container = getChildByName(std::string("FJ_TextContainer_2"));
    FJ_FuiNode* movieClip = container == 0 ? 0 : container->asMovieClip();
    fuiRenderNodeEditText* field = GetSecondaryTextField();
    if (m_secondaryWidth < 0.0f && movieClip != 0)
        m_secondaryWidth = movieClip->getWidth();
    if (m_secondaryX < 0.0f && movieClip != 0)
        m_secondaryX = -(movieClip->getX() + movieClip->getWidth()) * getScaleX() + m_objectWidth;
    if (field == 0)
        return;
    m_secondaryText = text.empty() ? std::wstring(L" ") : text;
    if (html)
        field->setHTMLText(m_secondaryText);
    else
        field->setText(m_secondaryText);
    if (movieClip != 0)
    {
        movieClip->setScaleX(1.0f);
        movieClip->setScaleY(1.0f);
    }
    field->setScaleX(1.0f / getScaleX());
    field->setScaleY(1.0f / getScaleY());
    field->setAutoSize(autoSize ? TextFieldAutoSizeRight : TextFieldAutoSizeLeft);
    if (movieClip != 0)
        movieClip->setX((m_objectWidth - m_secondaryWidth) / getScaleX() - m_secondaryX);
}

void FJ_Button::UpdateLabel()
{
    FJ_Base::SetLabel(m_label);
    if (m_hasSecondaryText)
        SetSecondaryText(m_secondaryText, m_secondaryHtml, m_secondaryAutoSize);
    if (m_updateLabelDelegate != 0)
        m_updateLabelDelegate(this);
}

void FJ_Button::SetLabel(const std::wstring& label) { FJ_Base::SetLabel(label); }

void FJ_Button::AdjustDisabledTint()
{
    if (m_pRenderNode != 0)
        FJ_ButtonRenderNodeAccess::setTint(m_pRenderNode, m_enabled ? 100.0f : 0.09803922f);
}

void FJ_Button::EnableButton(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    ChangeState(m_currentState);
    AdjustDisabledTint();
}

void FJ_Button::GetLabelTooWide(fuiRenderNodeEditText* textField)
{
    int width = static_cast<int>(textField->getWidth() * 1.26f);
    if (m_newWidth < width)
        m_newWidth = width;
    if (!m_manualWrap && m_labelWidth < width && !m_manualWrap)
    {
        m_manualWrap = true;
        m_wrapWidth = static_cast<int>(m_objectWidth * 0.8f);
    }
}
