#include "game/ui/UIControl_CheckBox.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

class ConsoleUIController
{
public:
    void TouchBoxAdd(UIControl* control, UIScene* scene);
};

extern ConsoleUIController g_ConsoleUIController;

UIControl_CheckBox::UIControl_CheckBox()
    : UIControl_Base()
{
}

UIControl_CheckBox::~UIControl_CheckBox()
{
}

unsigned char UIControl_CheckBox::IsEnabled()
{
    return m_enabled;
}

void UIControl_CheckBox::setChecked(bool checked)
{
    if (m_fuiRenderNode != 0)
        m_fuiRenderNode->getFuiNode()->asFJ_CheckBox()->SetCheckBox(checked);
    m_checked = checked;
}

unsigned int UIControl_CheckBox::IsChecked()
{
    if (m_fuiRenderNode == 0)
        return 0;
    return m_fuiRenderNode->getFuiNode()->asFJ_CheckBox()->getChecked();
}

void UIControl_CheckBox::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = 2;
    UIControl_Base::setupControl(scene, node, name);
    m_enabled = true;
}

void UIControl_CheckBox::init(UIString label, int id, bool checked)
{
    m_label = label;
    m_controlType = (unsigned short)id;
    m_checked = checked;
    m_fuiRenderNode->getFuiNode()->asFJ_CheckBox()->Init((const wchar_t*)(long)label.getString(), id, checked);

    int sceneType = m_scene->getSceneType();
    if (sceneType == 4 || (sceneType > 5 && sceneType < 8))
        g_ConsoleUIController.TouchBoxAdd(this, m_scene);
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
    m_fuiRenderNode->getFuiNode()->asFJ_CheckBox()->EnableCheckBox(enable);
}

void UIControl_CheckBox::updateCheckedStatus(bool checked)
{
    m_checked = checked;
}

void UIControl_CheckBox::ReInit()
{
    UIControl::ReInit();
    UIString label = m_label;
    init(label, (int)m_controlType, m_checked);
}
