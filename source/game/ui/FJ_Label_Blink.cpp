#include "game/ui/FJ_Label_Blink.hpp"

#include <cstdlib>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"

FJ_Label_Blink::FJ_Label_Blink(fuiRenderNode* renderNode)
    : FJ_Base(renderNode), m_blinkEnabled(false)
{
}

FJ_Label_Blink::~FJ_Label_Blink()
{
}

void FJ_Label_Blink::UpdateLabel()
{
    setWidth(m_objectWidth);
}

void FJ_Label_Blink::HandleUpdateLabel(FJ_Event*)
{
    UpdateLabel();
}

void FJ_Label_Blink::_HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Label_Blink*>(node)->HandleUpdateLabel(event);
}

void FJ_Label_Blink::Init(const std::wstring& label)
{
    FJ_Base::SetLabelAlignment(m_blinkAlignment);
    FJ_Base::InitBase(label, -1);
    addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
}

void FJ_Label_Blink::EnableBlink(bool enable)
{
    m_blinkEnabled = enable;
    if (enable)
    {
        gotoAndPlay("");
    }
    else
    {
        gotoAndStop("");
    }
}

void FJ_Label_Blink::SetLabel(const std::wstring& label)
{
    if (!m_widthChanged)
    {
        addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
    }
    FJ_Base::SetLabelAlignment(m_blinkAlignment);
    m_textContainerChanged = false;
    m_textContainerVisible = false;
    m_textOnly = false;
    m_textChanged = false;
    m_isLabel = true;
    FJ_Base::SetLabel(label);
}

void FJ_Label_Blink::GetLabelTooWide(fuiRenderNodeEditText* textField)
{
    float requiredWidth = static_cast<float>(m_oldWidth);
    float textWidth = textField->getWidth();
    if (requiredWidth <= textWidth)
    {
        requiredWidth = textWidth;
    }
    if (getWidth() < requiredWidth)
    {
        int required = static_cast<int>(requiredWidth);
        if (m_newWidth < required)
        {
            m_newWidth = required;
        }
        return;
    }
    int width = static_cast<int>(getWidth());
    if (m_newWidth < width)
    {
        m_newWidth = width;
    }
}

uint64_t FJ_Label_Blink::setAction(unsigned char action, char* value)
{
    if (action == 5)
    {
        m_blinkAlignment = std::atoi(value);
    }
    return (uint64_t)(unsigned long)this;
}
