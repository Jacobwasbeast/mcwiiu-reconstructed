#include "game/ui/UIControl_Button.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/ConsoleUIController.hpp"

extern ConsoleUIController gConsoleUIController;

UIControl_Button::UIControl_Button()
    : UIControl_Base()
    , m_focus(false)
    , m_enablePending(false)
    , m_enabled(true)
{
}

void UIControl_Button::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_Button;
    UIControl_Base::setupControl(scene, node, name);
}

void UIControl_Button::init(UIString label, int id)
{
    m_label = label;
    m_controlId = id;
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_Button()) {
        node->asFJ_Button()->Init(label.getString(), id);
    }
    int sceneType = m_scene->getSceneType();
    if (sceneType == 2 || sceneType == 4 || sceneType == 6 || sceneType == 7) {
        gConsoleUIController.TouchBoxAdd(this, m_scene);
    }
}

void UIControl_Button::ReInit()
{
    UIControl::ReInit();
    UIString label = m_label;
    init(label, m_controlId);
}

void UIControl_Button::setEnable(bool enable, bool immediate)
{
    if (enable == m_enabled) {
        return;
    }
    m_enabled = enable;
    if (!immediate) {
        m_enablePending = true;
        return;
    }
    m_fuiRenderNode->getFuiNode()->asFJ_Button()->EnableButton(enable);
}

void UIControl_Button::tick()
{
    UIControl_Base::tick();
    if (m_enablePending) {
        setEnable(m_enabled, true);
    }
}

void UIControl_Button::setFocus(bool focus)
{
    if (m_focus == focus) {
        return;
    }
    m_focus = focus;
    m_fuiRenderNode->getFuiNode()->asFJ_Button()->ChangeState(focus ? 1 : 2);
}
