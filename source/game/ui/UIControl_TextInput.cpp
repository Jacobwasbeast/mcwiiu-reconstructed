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

UIControl_TextInput::~UIControl_TextInput()
{
}

void UIControl_TextInput::SetCharLimit(int limit)
{
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_TextInput())
        node->asFJ_TextInput()->SetCharLimit(limit);
}

void UIControl_TextInput::init(UIString label, int id)
{
    m_label = label;
    m_controlId = id;

    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_TextInput())
        node->asFJ_TextInput()->Init(label.getString(), id);
    else if (node->isFJ_SignInput())
        node->asFJ_SignInput()->Init(label.getString(), id);

    int sceneType = m_scene->getSceneType();
    if (sceneType == 4 || (sceneType > 5 && sceneType < 8))
        gConsoleUIController.TouchBoxAdd(this, m_scene);

    SetCharLimit(101);
}

bool UIControl_TextInput::setupControl(UIScene* scene, fuiRenderNode* node,
                                       const std::string& name)
{
    m_controlType = 17;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_TextInput::ReInit()
{
    UIControl::ReInit();
    UIString label = m_label;
    init(label, m_controlId);
}

void UIControl_TextInput::setFocus(bool focus)
{
    if (m_focus == focus)
        return;

    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    m_focus = focus;
    if (node->isFJ_TextInput())
        node->asFJ_TextInput()->ChangeState(focus ^ 1);
}

bool UIControl_TextInput::hasFocus()
{
    if (!m_focus)
        return m_scene->controlHasFocus(this);
    return true;
}
