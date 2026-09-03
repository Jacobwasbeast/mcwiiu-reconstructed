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
    if (m_fuiRenderNode == 0)
        return;
    FJ_FuiNode* node = m_fuiRenderNode->getRoot();
    if (node != 0 && node->isFJ_TextInput())
        node->asFJ_TextInput()->SetCharLimit(limit);
}

void UIControl_TextInput::init(UIString label, int id)
{
    m_label = label;
    m_unknown0 = (unsigned int)id;
    if (m_fuiRenderNode != 0)
    {
        FJ_FuiNode* node = m_fuiRenderNode->getRoot();
        if (node != 0 && node->isFJ_TextInput())
            node->asFJ_TextInput()->Init((const wchar_t*)(long)label.getString(), id);
    }
    if (m_scene != 0)
    {
        int sceneType = m_scene->getSceneType();
        if (sceneType == 4 || (sceneType > 5 && sceneType < 8))
            ConsoleUIController_TouchBoxAdd(this, m_scene);
    }
    SetCharLimit(101);
}

bool UIControl_TextInput::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_unknown0 = 17;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_TextInput::ReInit()
{
    UIControl::ReInit();
    UIString label = m_label;
    init(label, (int)m_unknown0);
}

void UIControl_TextInput::setFocus(bool focus)
{
    if (m_focus == focus)
        return;
    m_focus = focus;
    if (m_fuiRenderNode != 0)
    {
        FJ_FuiNode* node = m_fuiRenderNode->getRoot();
        if (node != 0 && node->isFJ_TextInput())
            node->asFJ_TextInput()->ChangeState(focus ? 0 : 1);
    }
}

bool UIControl_TextInput::hasFocus()
{
    if (!m_focus && m_scene != 0)
        return m_scene->controlHasFocus((int)m_unknown0);
    return true;
}

void UIControl_TextInput::MoveCaretRight()
{
    if (m_fuiRenderNode != 0)
    {
        FJ_FuiNode* node = m_fuiRenderNode->getRoot();
        if (node != 0 && node->isFJ_TextInput())
            node->asFJ_TextInput()->MoveCaretRight();
    }
}

void UIControl_TextInput::MoveCaretLeft()
{
    if (m_fuiRenderNode != 0)
    {
        FJ_FuiNode* node = m_fuiRenderNode->getRoot();
        if (node != 0 && node->isFJ_TextInput())
            node->asFJ_TextInput()->MoveCaretLeft();
    }
}
