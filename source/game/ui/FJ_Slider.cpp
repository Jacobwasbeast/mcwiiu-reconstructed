#include "game/ui/FJ_Slider.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

FJ_Slider::FJ_Slider(fuiRenderNode* renderNode)
    : FJ_Base(renderNode)
{
}

FJ_Slider::~FJ_Slider()
{
}

uint8_t FJ_Slider::getEnabled()
{
    return m_enabled;
}

uint32_t FJ_Slider::getSliderCurrent()
{
    return m_sliderCurrent;
}

uint32_t FJ_Slider::getOldWidth()
{
    return m_oldWidth;
}

void FJ_Slider::UpdateSliderScaleMask()
{
    FJ_FuiNode* slider = getChildByName(std::string("Slider"));
    FJ_FuiNode* background = getChildByName(std::string("SliderBackground"));
    FJ_FuiNode* outline = getChildByName(std::string("Outline"));
    FJ_FuiNode* terminator = getChildByName(std::string("SliderTrack_TerminatorContainer"));
    m_sliderMask = getChildByName(std::string("SliderBackgroundHighlight"));
    m_tintActiveMask = getChildByName(std::string("TintActive"));
    m_tintInactiveMask = getChildByName(std::string("TintInactive"));

    if (slider != NULL)
    {
        slider->setScaleX(1.0f / getScaleX());
    }
    m_leftPosition = background->getX();
    m_rightPosition = background->getX() + background->getWidth() - slider->getWidth();

    if (getScaleX() != 1.0f)
    {
        const float scale = getScaleX();
        background->setScaleX(1.0f / scale);
        outline->setScaleX(1.0f / scale);
        m_sliderMask->setScaleX(1.0f / scale);
        m_tintActiveMask->setScaleX(1.0f / scale);
        m_tintInactiveMask->setScaleX(1.0f / scale);
        outline->setX(-(outline->getWidth() - background->getWidth()) * 0.5f);
        m_sliderMask->setX(-(m_sliderMask->getWidth() - background->getWidth()) * 0.5f);
        m_tintActiveMask->setX(background->getX());
        m_tintInactiveMask->setX(background->getX());
    }

    fuiRect rect;
    rect.xMin = getScaleX();
    rect.xMax = 0.0f;
    rect.yMin = getHeight() * 0.0f;
    rect.yMax = background->getHeight();
    addMask(new FJ_FuiNode(new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, "bgMask")));
    rect.xMin = getScaleX();
    addMask(new FJ_FuiNode(new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, "bgTintActiveMask")));
    rect.xMin = getScaleX();
    addMask(new FJ_FuiNode(new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, "bgTintInactiveMask")));
    rect.xMin = getScaleX() + m_sliderMask->getWidth() - background->getWidth();
    rect.yMax = m_sliderMask->getHeight();
    m_sliderMask->addMask(new FJ_FuiNode(new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, "bghMask")));
    rect.xMin = getScaleX() + outline->getWidth() - background->getWidth();
    rect.yMax = outline->getHeight();
    outline->addMask(new FJ_FuiNode(new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, "outlineMask")));
    m_objectWidth = -(background->getWidth() - m_leftPosition) * 0.5f;
    terminator->setScaleX(1.0f / getScaleX());
    terminator->setX(m_leftPosition - terminator->getWidth());
    terminator->setY(background->getY());
}

void FJ_Slider::SetSlider(int value)
{
    const float range = (float)(m_maxValue - m_minValue);
    const float position = (float)(value - m_minValue) / range;
    FJ_FuiNode* slider = getChildByName(std::string("Slider"));
    slider->setX((m_rightPosition - m_leftPosition) * position + m_leftPosition);
}

void FJ_Slider::ChangeState(int state)
{
    gotoAndStop(std::string("Slider"));
    m_currentState = state;
    FJ_FuiNode* outline = getChildByName(std::string("Outline"));
    if (outline != NULL)
        outline->setVisible(state == 0);
    if (m_sliderMask != NULL)
        m_sliderMask->setVisible(state == 0);
    if (m_tintActiveMask != NULL)
        m_tintActiveMask->setVisible(state == 0);
    if (m_tintInactiveMask != NULL)
        m_tintInactiveMask->setVisible(state != 0);
}

void FJ_Slider::InitSliderMask()
{
    UpdateSliderScaleMask();
    SetSlider(m_sliderCurrent);
    ChangeState(1);
    m_maskInitialized = true;
}

void FJ_Slider::focusInHandler(FJ_Event*)
{
    ChangeState(0);
    m_keyPressTicks = 0;
}

void FJ_Slider::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Slider*>(node)->focusInHandler(event);
}

void FJ_Slider::focusOutHandler(FJ_Event*)
{
    ChangeState(1);
    m_keyPressDelta = 0;
    m_keyPressTicks = 0;
}

void FJ_Slider::_focusOutHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Slider*>(node)->focusOutHandler(event);
}

void FJ_Slider::keyUpHandler(FJ_Event* event)
{
    if (!m_enabled || (event->m_eType != eFJEventType_KeyUp && event->m_eType != eFJEventType_KeyDown))
        return;
    m_keyPressTicks = 0;
    m_keyPressDelta = 0;
}

void FJ_Slider::_keyUpHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Slider*>(node)->keyUpHandler(event);
}

uint64_t FJ_Slider::MoveSlider(int amount)
{
    const int oldValue = m_sliderCurrent;
    m_sliderCurrent += amount;
    if (m_sliderCurrent > m_maxValue)
        m_sliderCurrent = m_maxValue;
    if (m_sliderCurrent < m_minValue)
        m_sliderCurrent = m_minValue;
    if (oldValue != m_sliderCurrent)
        SetSlider(m_sliderCurrent);
    return 0;
}

void FJ_Slider::keyDownHandler(FJ_Event* event)
{
    if (!m_enabled)
        return;
    ++m_keyPressTicks;
    if (!m_repeat || m_keyPressTicks < m_repeatDelay)
        m_keyPressDelta = 0;
    else
    {
        m_keyPressTicks = 0;
        m_keyPressDelta += m_repeatStep;
    }
    if (event->m_eType == eFJEventType_KeyUp)
        return;
    if (event->m_eType == eFJEventType_KeyDown)
        MoveSlider(event->m_eType == eFJEventType_KeyDown ? (m_keyPressDelta == 0 ? 1 : m_keyPressDelta) : 0);
}

void FJ_Slider::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Slider*>(node)->keyDownHandler(event);
}

void FJ_Slider::UpdateLabel()
{
    SetLabel(m_label);
}

void FJ_Slider::HandleUpdateLabel(FJ_Event*)
{
    UpdateLabel();
}

void FJ_Slider::_HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Slider*>(node)->HandleUpdateLabel(event);
}

void FJ_Slider::ASConstructor()
{
    m_enabled = true;
    m_minValue = 0;
    m_maxValue = 100;
    m_sliderCurrent = 50;
    m_currentState = 0;
    m_leftPosition = 0.0f;
    m_rightPosition = 0.0f;
    m_maskInitialized = false;
    m_oldWidth = (int)getWidth();
    m_sliderMask = NULL;
    m_tintActiveMask = NULL;
    m_tintInactiveMask = NULL;
    m_keyPressTicks = 0;
    m_keyPressDelta = 0;
    m_repeat = false;
    m_repeatDelay = 10;
    m_repeatStep = 1;
    if (getParent() != NULL)
        InitSliderMask();
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
    addEventListener(eFJEventType_KeyUp, this, _keyUpHandler, false);
    addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
}

void FJ_Slider::Init(const std::wstring& name, int id, int minValue, int maxValue,
                     int value, bool repeat, int repeatDelay, int repeatStep)
{
    if (!m_maskInitialized)
        InitSliderMask();
    SetLabelAlignment(2);
    InitBase(name, id);
    m_maxValue = maxValue;
    m_sliderCurrent = value;
    m_minValue = minValue;
    SetSlider(value);
    m_repeatStep = repeatStep;
    m_repeat = repeat;
    m_repeatDelay = repeatDelay;
}

void FJ_Slider::SetRelativeSliderPos(float position)
{
    if (position < 0.0f)
        position = 0.0f;
    if (position > 1.0f)
        position = 1.0f;
    const int oldValue = m_sliderCurrent;
    const int range = m_maxValue - m_minValue;
    m_sliderCurrent = Math::round((float)range * position + (float)m_minValue);
    if (m_sliderCurrent > m_maxValue)
        m_sliderCurrent = m_maxValue;
    if (m_sliderCurrent < m_minValue)
        m_sliderCurrent = m_minValue;
    if (oldValue != m_sliderCurrent)
        SetSlider(m_sliderCurrent);
}

double FJ_Slider::GetRealWidth()
{
    return (double)(float)m_oldWidth;
}

void FJ_Slider::SetSliderValue(int value)
{
    if (value > m_maxValue)
        value = m_maxValue;
    if (value < m_minValue)
        value = m_minValue;
    m_sliderCurrent = value;
    SetSlider(value);
}

void FJ_Slider::EnableSlider(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    ChangeState(m_currentState);
}

void FJ_Slider::GetLabelTooWide(fuiRenderNodeEditText* textField)
{
    const int measuredWidth = (int)(textField->getWidth() * 1.1f);
    if (m_labelWidth < measuredWidth)
        m_labelWidth = measuredWidth;
    if (!m_labelTooWide && m_oldWidth < (int)textField->getWidth())
    {
        m_labelTooWide = true;
        m_labelTooWide = true;
        m_newWidth = (int)((float)m_oldWidth * 0.8f);
    }
}

void FJ_Slider::SetNewObjectWidth(int width)
{
    if (m_oldWidth < width)
    {
        SetTextOnly(std::wstring(L"Slider"));
        setWidth(getWidth() + (float)(width - m_oldWidth) / getScaleX());
        m_oldWidth = width;
        fuiRect rect = getRect(this);
        m_labelTooWide = false;
        m_labelWidth = 0;
        m_objectHeight = getWidth() + rect.xMin + rect.xMin;
        FJ_FuiNode* slider = getChildByName(std::string("Slider"));
        FJ_FuiNode* background = getChildByName(std::string("SliderBackground"));
        if (slider != NULL && background != NULL)
        {
            const float oldSliderWidth = slider->getWidth();
            slider->setScaleX(1.0f / getScaleX());
            m_rightPosition += oldSliderWidth - slider->getWidth();
            SetSlider(m_sliderCurrent);
        }
        UpdateLabel();
    }
}
