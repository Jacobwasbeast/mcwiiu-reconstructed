#include "game/ui/FJ_CheckBox.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"

bool FJ_CheckBox::s_keyDown = false;

static std::string s_checkBoxStates[2][2] =
{
    {std::string("Active_Selected"), std::string("Inactive_Selected")},
    {std::string("Active_Unselected"), std::string("Inactive_Unselected")}
};

FJ_CheckBox::FJ_CheckBox(fuiRenderNode* renderNode)
    : FJ_Base(renderNode)
{
    m_checked = true;
    m_enabled = true;
    FJ_CheckBox::s_keyDown = false;
    ChangeState(1);
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
    addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    addEventListener(eFJEventType_KeyUp, this, _keyUpHandler, false);
    m_oldWidth = static_cast<int>(getWidth());
    m_labelWidth = static_cast<int>(GetTextField()->getWidth());
}

void FJ_CheckBox::UpdateCheckBox()
{
    getChildByName(std::string("CheckBoxTick"))->asMovieClip()->setVisible(m_checked);
}

void FJ_CheckBox::SetCheckBox(bool checked)
{
    m_checked = checked;
    FJ_FuiNode* parent = getObjListParent();
    if (parent == NULL)
    {
        dispatchEvent(new FJ_Event(eFJEventType_CheckboxToggled, true, false));
    }
    else if (parent->isFJ_List())
    {
        UIScene* scene = getRoot()->m_pCallbackScene;
        scene->handleCheckboxElementToggled(static_cast<FJ_Base*>(parent)->getId(), getId(), m_checked);
    }
    else if (parent->isFJ_List2D())
    {
        UIScene* scene = getRoot()->m_pCallbackScene;
        scene->handleCheckboxElementToggled(static_cast<FJ_Base*>(parent)->getId(), getId(), m_checked);
    }
    UpdateCheckBox();
}

void FJ_CheckBox::ChangeState(int state)
{
    std::string* row = s_checkBoxStates[state];
    gotoAndStop(row[m_enabled ^ 1]);
    m_currentState = state;
}

void FJ_CheckBox::EnableCheckBox(bool enabled)
{
    if (m_enabled != enabled)
    {
        m_enabled = enabled;
        if (!enabled)
            SetCheckBox(false);
        ChangeState(m_currentState);
    }
}

uint8_t FJ_CheckBox::getChecked()
{
    return m_checked ? 1 : 0;
}

uint8_t FJ_CheckBox::getEnabled()
{
    return m_enabled ? 1 : 0;
}

uint32_t FJ_CheckBox::getCurrentState()
{
    return static_cast<uint32_t>(m_currentState);
}

int FJ_CheckBox::getOldWidth()
{
    return m_oldWidth;
}

void FJ_CheckBox::focusInHandler(FJ_Event*)
{
    ChangeState(0);
}

void FJ_CheckBox::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_CheckBox*>(node)->focusInHandler(event);
}

void FJ_CheckBox::focusOutHandler(FJ_Event*)
{
    ChangeState(1);
}

void FJ_CheckBox::_focusOutHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_CheckBox*>(node)->focusOutHandler(event);
}

void FJ_CheckBox::keyDownHandler(FJ_Event* event)
{
    if (!m_enabled || FJ_CheckBox::s_keyDown ||
        static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode != 0)
        return;
    FJ_CheckBox::s_keyDown = true;
    SetCheckBox(!m_checked);
}

void FJ_CheckBox::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_CheckBox*>(node)->keyDownHandler(event);
}

void FJ_CheckBox::keyUpHandler(FJ_Event* event)
{
    if (static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode == 0)
        FJ_CheckBox::s_keyDown = false;
}

void FJ_CheckBox::_keyUpHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_CheckBox*>(node)->keyUpHandler(event);
}

void FJ_CheckBox::UpdateLabel()
{
    SetLabel(m_label);
}

void FJ_CheckBox::HandleUpdateLabel(FJ_Event*)
{
    UpdateLabel();
}

void FJ_CheckBox::_HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_CheckBox*>(node)->HandleUpdateLabel(event);
}

void FJ_CheckBox::Init(const std::wstring& name, int id, bool checked)
{
    SetLabelAlignment(0);
    m_checked = checked;
    if (getScaleX() != 1.0f)
        setScaleX(1.0f);
    if (getScaleY() != 1.0f)
        setScaleY(1.0f);
    InitBase(name, id);
    UpdateCheckBox();
    dispatchEvent(new FJ_Event(eFJEventType_CheckboxInit, true, false));
    addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
}

void FJ_CheckBox::GetLabelTooWide(fuiRenderNodeEditText* textField)
{
    int width = static_cast<int>(textField->getWidth());
    if (width < m_labelWidth)
        width = m_labelWidth;
    int required = m_oldWidth;
    if (width > m_labelWidth)
        required += width - m_labelWidth;
    if (required < m_newWidth)
        required = m_newWidth;
    m_newWidth = required;
}

void FJ_CheckBox::SetNewObjectWidth(int)
{
}
