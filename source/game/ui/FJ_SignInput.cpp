#include "game/ui/FJ_SignInput.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"

FJ_SignInput::FJ_SignInput(fuiRenderNode* renderNode) : FJ_Base(renderNode), m_maxLength(15), m_leftArrow(0), m_rightArrow(0)
{
}

FJ_Base* FJ_SignInput::Create(fuiRenderNode* renderNode)
{
    return new FJ_SignInput(renderNode);
}

FJ_SignInput::~FJ_SignInput()
{
}

unsigned int FJ_SignInput::getType()
{
    return 0x8010;
}

void FJ_SignInput::ChangeState(int state)
{
    if (m_leftArrow != 0 && m_rightArrow != 0)
    {
        const char* leftFrame = state == 0 ? "Selected_Left" : "UnSelected";
        const char* rightFrame = state == 0 ? "Selected_Right" : "UnSelected";
        m_leftArrow->gotoAndPlay(std::string(leftFrame));
        m_rightArrow->gotoAndPlay(std::string(rightFrame));
    }
}

void FJ_SignInput::focusInHandler(FJ_Event*)
{
    ChangeState(0);
}

void FJ_SignInput::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_SignInput*>(node)->focusInHandler(event);
}

void FJ_SignInput::focusOutHandler(FJ_Event*)
{
    ChangeState(1);
}

void FJ_SignInput::_focusOutHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_SignInput*>(node)->focusOutHandler(event);
}

void FJ_SignInput::keyDownHandler(FJ_Event* event)
{
    if (event != 0 && !event->m_bCancelable)
    {
        FJ_FuiNode* root = getRoot();
        if (root != 0)
            root->getRoot();
    }
}

void FJ_SignInput::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_SignInput*>(node)->keyDownHandler(event);
}

void FJ_SignInput::Init(std::wstring name, int id)
{
    SetLabelAlignment(4);
    InitBase(name, id);
    m_leftArrow = getChildByName(std::string("ArrowLeft"));
    m_rightArrow = getChildByName(std::string("ArrowRight"));
    addEventListener(eFJEventType_FocusIn, this, &_focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, &_focusOutHandler, false);
    addEventListener(eFJEventType_KeyDown, this, &_keyDownHandler, false);
    ChangeState(1);
}

void FJ_SignInput::SetLabel(const std::wstring& label)
{
    FJ_Base::SetLabel(label);
}

void FJ_SignInput::HandleDirectInput(unsigned int character)
{
    if (m_label.size() < static_cast<std::wstring::size_type>(m_maxLength) || m_maxLength == -1)
    {
        m_label.push_back(static_cast<wchar_t>(character));
        SetLabel(m_label);
    }
}

void FJ_SignInput::HandleCharInput(unsigned int character)
{
    HandleDirectInput(character);
}

void FJ_SignInput::GetLabelTooWide(fuiRenderNodeEditText*)
{
}

void FJ_SignInput::SetNewObjectWidth(int)
{
}
