#include "game/ui/UIControl_Slider.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_Slider.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_Slider::UIControl_Slider()
    : UIControl_Base()
{
    m_value = 0;
    m_step = 10;
    m_maximum = 100;
    m_unknown = 1;
    m_focus = false;
    m_enabled = false;
    m_controlType = 0;
    m_minimum = 0;
}

UIControl_Slider::~UIControl_Slider()
{
}

void UIControl_Slider::init(UIString label, int controlId, int minimum, int maximum, int value,
                            bool enabled, int step, int unknown)
{
    m_label = label;
    m_controlType = (unsigned short)controlId;
    m_minimum = minimum;
    m_maximum = maximum;
    m_value = (unsigned int)value;
    m_unknown = unknown;
    m_step = step;
    m_enabled = enabled;

    FJ_Slider* slider = m_fuiRenderNode->getFuiNode()->asFJ_Slider();
    slider->Init(std::wstring((const wchar_t*)(long)label.getString()), controlId, minimum, maximum, value,
                 enabled, step, unknown);

    int sceneType = m_scene->getSceneType();
    if (sceneType == 4 || (sceneType > 5 && sceneType < 8))
        gConsoleUIController.TouchBoxAdd(this, m_scene);
}

void UIControl_Slider::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = 15;
    UIControl_Base::setupControl(scene, node, name);
}

void UIControl_Slider::setFocus(bool focus)
{
    if (m_focus == focus)
        return;
    m_focus = focus;
    m_fuiRenderNode->getFuiNode()->asFJ_Slider()->ChangeState(focus ^ 1);
}

void UIControl_Slider::handleSliderMove(int value)
{
    if (m_value != (unsigned int)value)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
        m_value = (unsigned int)value;
        if (!m_possibleLabels.empty() && (unsigned int)value < m_possibleLabels.size())
            setLabel(UIString(m_possibleLabels[value]), false, false);
    }
}

void UIControl_Slider::SetSliderTouchPos(float position)
{
    m_fuiRenderNode->getFuiNode()->asFJ_Slider()->SetRelativeSliderPos(position);
}

void UIControl_Slider::SetSliderValue(int value)
{
    m_fuiRenderNode->getFuiNode()->asFJ_Slider()->SetSliderValue(value);
}

void UIControl_Slider::MoveSliderValue(int amount)
{
    int value = (int)m_value + amount;
    if (value > m_maximum)
        value = m_maximum;
    if (value < m_minimum)
        value = m_minimum;
    SetSliderValue(value);
    handleSliderMove(value);
}

void UIControl_Slider::setAllPossibleLabels(int firstLabel, unsigned int labelCount,
                                            wchar_t (*labels)[256])
{
    m_possibleLabels.clear();

    unsigned int groupCount = labelCount / 4;
    unsigned int remainder = labelCount & 3;
    unsigned int i;
    for (i = 0; i < groupCount; ++i)
    {
        wchar_t* group = labels[i * 4 + 1];
        m_possibleLabels.push_back(std::wstring(group - 256));
        m_possibleLabels.push_back(std::wstring(group));
        m_possibleLabels.push_back(std::wstring(group + 256));
        m_possibleLabels.push_back(std::wstring(group + 512));
    }
    for (i = 0; i < remainder; ++i)
        m_possibleLabels.push_back(std::wstring(labels[groupCount * 4 + i]));

    UIControl_Base::setAllPossibleLabels(firstLabel, labelCount, labels);
}

unsigned int UIControl_Slider::getCurrentValue() const
{
    return m_value;
}

int UIControl_Slider::GetRealWidth()
{
    return (int)m_fuiRenderNode->getFuiNode()->asFJ_Slider()->GetRealWidth();
}

void UIControl_Slider::ReInit()
{
    UIControl::ReInit();
    UIString label = m_label;
    init(label, (int)m_controlType, m_minimum, m_maximum, (int)m_value, m_enabled, m_step, m_unknown);
}
