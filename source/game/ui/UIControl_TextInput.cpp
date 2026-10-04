#include "game/ui/UIControl_TextInput.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_SignInput.hpp"
#include "game/ui/FJ_TextInput.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_TextInput::UIControl_TextInput()
    : UIControl_Base()
    , m_focus(false)
{
}

void UIControl_TextInput::SetCharLimit(int limit)
{
    if (m_fuiRenderNode->getFuiNode()->isFJ_TextInput())
        static_cast<FJ_TextInput*>(m_fuiRenderNode->getFuiNode())->SetCharLimit(limit);
}

void UIControl_TextInput::init(UIString label, int id)
{
    m_label = label;
    m_controlId = id;

    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (!node->isFJ_TextInput())
    {
        if (m_fuiRenderNode->getFuiNode()->isFJ_SignInput())
            m_fuiRenderNode->getFuiNode()->asFJ_SignInput()->Init(label.getString(), id);
    }
    else
    {
        m_fuiRenderNode->getFuiNode()->asFJ_TextInput()->Init(label.getString(), id);
    }

    unsigned int layerId = m_scene->m_layer->m_layerId;
    if (layerId == EUILayer_Fullscreen || (layerId > EUILayer_Tutorial && layerId < EUILayer_Count))
        gConsoleUIController.TouchBoxAdd(this, m_scene);

    SetCharLimit(101);
}

void UIControl_TextInput::setupControl(UIScene* scene, fuiRenderNode* node,
                                       const std::string& name)
{
    m_controlType = UIControl::eUIControlType_TextInput;
    UIControl_Base::setupControl(scene, node, name);
}

void UIControl_TextInput::ReInit()
{
    UIControl::ReInit();
    init(m_label, m_controlId);
}

void UIControl_TextInput::setFocus(bool focus)
{
    if (m_focus == focus)
        return;

    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    m_focus = focus;
    if (node->isFJ_TextInput())
        static_cast<FJ_TextInput*>(m_fuiRenderNode->getFuiNode())->ChangeState(focus ^ 1);
}

bool UIControl_TextInput::hasFocus()
{
    if (!m_focus)
        return m_scene->controlHasFocus(this);
    return true;
}
