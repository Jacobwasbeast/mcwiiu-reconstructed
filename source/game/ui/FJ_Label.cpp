#include "game/ui/FJ_Label.hpp"

#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"

FJ_Label::FJ_Label(fuiRenderNode* renderNode)
    : FJ_Base(renderNode), m_textAlignment(eTextAlignment_Center), m_multiline(false),
      m_wordWrap(false), m_useEllipsis(false), m_manualWordWrap(false)
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
    m_centered = useHtmlText;
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
    m_isLabel = false;
    m_newWidth = static_cast<int>(getWidth());
}

void FJ_Label::SetLabel(const std::wstring& label)
{
    FJ_Base::SetLabelAlignment(static_cast<int>(m_textAlignment));
    m_textChanged = m_manualWordWrap;
    m_textContainerChanged = m_wordWrap;
    m_textOnly = m_useEllipsis;
    m_textContainerVisible = m_multiline;
    m_isLabel = true;
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
    const float measured = textField->getWidth();
    float required = static_cast<float>(m_maxLabelWidth) <= measured
                         ? static_cast<float>(static_cast<int>(measured))
                         : static_cast<float>(m_maxLabelWidth);
    if (required <= getWidth())
    {
        required = getWidth();
    }
    if (m_newWidth < static_cast<int>(required))
    {
        m_newWidth = static_cast<int>(required);
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
        m_multiline = std::strcmp(value, "true") == 0;
    }
    else if (action == 7)
    {
        m_useEllipsis = std::strcmp(value, "true") == 0;
    }
    else if (action == 8)
    {
        m_wordWrap = std::strcmp(value, "true") == 0;
    }
    else if (action == 9)
    {
        m_manualWordWrap = std::strcmp(value, "true") == 0;
    }
    return (uint64_t)(unsigned long)this;
}

typedef std::vector<FJ_Label*> FJ_LabelVector;
typedef FJ_LabelVector::iterator FJ_LabelIterator;

void __CPR217___Insert_n__Q2_3std62vector__tm__48_P8FJJ_LabelQ2_3std28allocator__tm__11_PJ36JFQ2_3std68_Vector_iterator__tm__44_Z1ZQ3_3std25_Vector_val__tm__7_Z1ZZ2Z5_AltyQ4_3stdJ127JJ154J9size_typeRCZ1Z_v(
    FJ_LabelVector& vector, FJ_LabelIterator position, FJ_LabelVector::size_type count, FJ_Label* const& value)
{
    vector.insert(position, count, value);
}

FJ_LabelIterator __CPR240__insert__Q2_3std62vector__tm__48_P8FJJ_LabelQ2_3std28allocator__tm__11_PJ33JFQ2_3std68_Vector_iterator__tm__44_Z1ZQ3_3std25_Vector_val__tm__7_Z1ZZ2Z5_AltyRCZ1Z_Q2_3stdJ87J(
    FJ_LabelVector& vector, FJ_LabelIterator position, FJ_Label* const& value)
{
    return vector.insert(position, value);
}

FJ_LabelIterator __CPR234__erase__Q2_3std62vector__tm__48_P8FJJ_LabelQ2_3std28allocator__tm__11_PJ32JFQ2_3std68_Vector_iterator__tm__44_Z1ZQ3_3std25_Vector_val__tm__7_Z1ZZ2Z5_Alty_Q2_3stdJ86J(
    FJ_LabelVector& vector, FJ_LabelIterator position)
{
    return vector.erase(position);
}
