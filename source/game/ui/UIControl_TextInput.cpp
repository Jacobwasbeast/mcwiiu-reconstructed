#include "game/ui/UIControl_TextInput.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_TextInput.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

void ConsoleUIController_TouchBoxAdd(UIControl* control, UIScene* scene);

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
    m_controlType = (unsigned short)id;

    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    std::wstring text((const wchar_t*)(long)label.getString());
    if (node->isFJ_TextInput())
        node->asFJ_TextInput()->Init(text, id);
    else if (node->isFJ_SignInput())
        node->asFJ_SignInput()->Init(text, id);

    int sceneType = m_scene->getSceneType();
    if (sceneType == 4 || (sceneType > 5 && sceneType < 8))
        ConsoleUIController_TouchBoxAdd(this, m_scene);

    SetCharLimit(101);
}

void UIControl_TextInput::setupControl(UIScene* scene, fuiRenderNode* node,
                                       const std::string& name)
{
    m_controlType = 17;
    UIControl_Base::setupControl(scene, node, name);
}

void UIControl_TextInput::ReInit()
{
    UIControl::ReInit();
    UIString label = m_label;
    init(label, m_controlType);
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
        return m_scene->controlHasFocus(m_controlType);
    return true;
}
