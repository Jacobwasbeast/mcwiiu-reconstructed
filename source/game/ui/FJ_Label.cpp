#include "game/ui/FJ_Label.hpp"

#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>

#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"

FJ_Label::FJ_Label(fuiRenderNode* renderNode)
    : FJ_Base(renderNode), m_textAlignment(eTextAlignment_Center), m_actionUseHtmlText(false),
      m_actionUseSmallText(false), m_actionUseLargeText(false), m_actionUseBoldText(false)
{
}

FJ_Label::~FJ_Label()
{
}

FJ_Base* FJ_Label::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label(renderNode);
}

void FJ_Label::SetTextAlignment(ETextAlignment alignment)
{
    m_textAlignment = alignment;
}

void FJ_Label::setUseHtmlText(bool useHtmlText)
{
    m_textChanged = useHtmlText;
}

unsigned int FJ_Label::getType()
{
    return 0x8059;
}

void FJ_Label::Init(const std::wstring& label)
{
    FJ_Base::SetLabelAlignment(static_cast<int>(m_textAlignment));
    FJ_Base::InitBase(label, -1);
}

void FJ_Label::ResetLabel()
{
    m_objectWidth = -1.0f;
    m_objectHeight = -1.0f;
    m_textChanged = false;
    m_newWidth = static_cast<int>(getWidth());
}

void FJ_Label::SetLabel(const std::wstring& label)
{
    FJ_Base::SetLabelAlignment(static_cast<int>(m_textAlignment));
    m_textContainerChanged = m_actionUseBoldText;
    m_textContainerVisible = m_actionUseSmallText;
    m_textOnly = m_actionUseLargeText;
    m_isLabel = m_actionUseHtmlText;
    m_textChanged = true;
    FJ_Base::SetLabel(label);
}

int FJ_Label::GetAverageCharHeight()
{
    fuiRenderNodeEditText* textField = GetTextField();
    const std::wstring& text = textField->getText();
    int count = 0;
    int total = 0;
    for (std::wstring::size_type index = 0; index < text.size() && count < 5; ++index)
    {
        std::pair<float, float> boundaries = textField->getCharBoundaries(static_cast<int>(index));
        ++count;
        total += static_cast<int>(boundaries.second);
    }
    return count == 0 ? 0 : total / count;
}

void FJ_Label::GetLabelTooWide(fuiRenderNodeEditText* textField)
{
    float textWidth = textField->getWidth();
    float requiredWidth = m_objectWidth <= textWidth ? textWidth : static_cast<float>(static_cast<int>(m_objectWidth));
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

uint64_t FJ_Label::setAction(unsigned char action, char* value)
{
    if (action == 5)
    {
        m_textAlignment = static_cast<ETextAlignment>(std::atoi(value));
    }
    else if (action == 6)
    {
        m_actionUseHtmlText = std::strcmp(value, "true") == 0;
    }
    else if (action == 7)
    {
        m_actionUseSmallText = std::strcmp(value, "true") == 0;
    }
    else if (action == 8)
    {
        m_actionUseLargeText = std::strcmp(value, "true") == 0;
    }
    else if (action == 9)
    {
        m_actionUseBoldText = std::strcmp(value, "true") == 0;
    }
    return (uint64_t)(unsigned long)this;
}
