#include "game/ui/UIControl_CheckBox.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/ConsoleUIController.hpp"

extern ConsoleUIController gConsoleUIController;

UIControl_CheckBox::UIControl_CheckBox()
    : UIControl_Base()
{
}

unsigned char UIControl_CheckBox::IsEnabled()
{
    return m_enabled;
}

void UIControl_CheckBox::setChecked(bool checked)
{
    if (m_fuiRenderNode != 0)
        static_cast<FJ_CheckBox*>(m_fuiRenderNode->getFuiNode())->SetCheckBox(checked);
    m_checked = checked;
}

unsigned int UIControl_CheckBox::IsChecked()
{
    if (m_fuiRenderNode == 0)
        return 0;
    return static_cast<FJ_CheckBox*>(m_fuiRenderNode->getFuiNode())->getChecked();
}

void UIControl_CheckBox::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_CheckBox;
    UIControl_Base::setupControl(scene, node, name);
    m_enabled = true;
}

void UIControl_CheckBox::init(UIString label, int id, bool checked)
{
    m_label = label;
    m_controlId = id;
    m_checked = checked;
    static_cast<FJ_CheckBox*>(m_fuiRenderNode->getFuiNode())->Init(label.getString(), id, checked);

    int layerId = m_scene->m_layer->m_layerId;
    if (layerId == 4 || (layerId >= 6 && layerId <= 7))
        gConsoleUIController.TouchBoxAdd(this, m_scene);
}

unsigned char UIControl_CheckBox::TouchIsChecked()
{
    return m_checked;
}

void UIControl_CheckBox::SetEnable(bool enable)
{
    m_enabled = enable;
    if (m_fuiRenderNode == 0)
        return;
    static_cast<FJ_CheckBox*>(m_fuiRenderNode->getFuiNode())->EnableCheckBox(enable);
}

void UIControl_CheckBox::updateCheckedStatus(bool checked)
{
    m_checked = checked;
}

void UIControl_CheckBox::ReInit()
{
    UIControl::ReInit();
    init(m_label, m_controlId, m_checked);
}
