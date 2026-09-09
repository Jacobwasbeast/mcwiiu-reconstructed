#include "game/ui/FJ_Base.hpp"
#include <cmath>
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

FJ_Base::FJ_Base(fuiRenderNode* renderNode) : FJ_FuiNode(renderNode)
{
    m_isLabel = true;
    m_textContainerVisible = false;
    m_textContainerChanged = false;
    m_textChanged = false;
    m_centered = false;
    m_textOnly = false;
    m_lineCount = 0;
    m_manualWrap = false;
    m_labelTooWide = false;
    m_resizing = false;
    m_widthChanged = false;
    m_heightChanged = false;
    m_labelWidth = 0;
    m_textWidth = 0;
    m_textHeight = 0;
    m_id = -1;
    m_labelAlignment = 0;
    m_objectWidth = 0.0f;
    m_objectHeight = -1.0f;
    m_textScale = -1.0f;
    m_wrapWidth = 0;
    m_wrapHeight = false;
    m_preScaleWidth = 0;
    m_maxLabelWidth = -1;
    m_newWidth = static_cast<int>(getWidth());
    m_oldWidth = static_cast<int>(getWidth());
    m_objListParent = 0;
    m_listId = 0;
    CorrectPixelPosition();
}

FJ_Base::~FJ_Base() {}

void FJ_Base::CheckLabelWidths(std::vector<std::wstring>& labels)
{
    for (std::vector<std::wstring>::size_type i = 0; i < labels.size(); ++i)
    {
        CheckLabelWidth(labels[i]);
    }
}

FJ_FuiNode* FJ_Base::getObjListParent() { return m_objListParent; }
uint32_t FJ_Base::getId() { return static_cast<uint32_t>(m_id); }

uint64_t FJ_Base::LostFocus()
{
    if (m_objListParent != 0 && m_objListParent->isFJ_List())
    {
        m_objListParent->asFJ_List()->LoseListFocus();
        return 0;
    }
    if (m_objListParent != 0)
    {
        m_objListParent->isFJ_List2D();
    }
    return 0;
}

uint32_t FJ_Base::getNewWidth() { return static_cast<uint32_t>(m_newWidth); }
void FJ_Base::ResizingComplete() { m_resizing = true; }
void FJ_Base::ResetResizing() { m_resizing = false; }
void FJ_Base::setObjListParent(FJ_FuiNode* parent) { m_objListParent = parent; }
uint32_t FJ_Base::getListId() { return static_cast<uint32_t>(m_listId); }
void FJ_Base::setListId(int id) { m_listId = id; }
void FJ_Base::setPreScaleWidth(int width) { m_preScaleWidth = width; }
void FJ_Base::setId(int id) { m_id = id; }
uint8_t FJ_Base::getLabelTooWide() { return m_labelTooWide ? 1 : 0; }
int FJ_Base::getOldWidth() { return static_cast<int>(getWidth()); }

uint64_t FJ_Base::CorrectPixelPosition()
{
    float x = getX();
    float y = getY();
    if (std::fmod(x, 1.0f) != 0.0f)
    {
        setX(x < 0.0f ? std::ceil(x - 0.5f) : std::floor(x + 0.5f));
    }
    if (std::fmod(y, 1.0f) != 0.0f)
    {
        setY(y < 0.0f ? std::ceil(y - 0.5f) : std::floor(y + 0.5f));
    }
    return 0;
}

void FJ_Base::InitBase(const std::wstring&, int id)
{
    m_isLabel = true;
    m_id = id;
    setVisible(true);
}

fuiRenderNodeEditText* FJ_Base::GetTextField()
{
    FJ_FuiNode* node = 0;
    if (isFJ_Label())
    {
        node = getChildAt(0);
        node = node->asMovieClip();
    }
    else
    {
        node = getChildByName(std::string("FJ_TextContainer"));
        if (node == 0 || node->getNumChildren() == 0)
            return 0;
        node = node->getChildAt(0);
    }
    if (node == 0)
        return 0;
    fuiRenderNode* timelineNode = node->m_pRenderNode->asTimeline();
    if (timelineNode == 0)
        return 0;
    fuiRenderNodeTimeline* timeline = static_cast<fuiRenderNodeTimeline*>(timelineNode);
    return static_cast<fuiRenderNodeEditText*>(timeline->getChildAt(0)->asEditText());
}

void FJ_Base::CheckLabelWidth(const std::wstring& label)
{
    fuiRenderNodeEditText* field = GetTextField();
    if (field != 0)
    {
        field->setText(label);
        m_maxLabelWidth = static_cast<int>(field->getWidth());
        field->setText(std::wstring(L" "));
    }
}

void FJ_Base::SetLabelAlignment(int alignment) { m_labelAlignment = alignment; }

uint64_t FJ_Base::DoManualWordWrapCheck(FJ_FuiNode* container, fuiRenderNodeEditText* field)
{
    if (m_wrapWidth == 0)
        m_wrapWidth = static_cast<int>(getWidth());
    float offset = 0.0f;
    unsigned int limit = static_cast<unsigned int>(m_wrapWidth);
    if (container == 0)
        limit = static_cast<unsigned int>(static_cast<float>(limit) * getScaleX());
    else
        offset = static_cast<float>(static_cast<int>(container->getX() * getScaleX()) * 2);
    float measured = field->getWidth();
    float total = offset + measured;
    if (total <= static_cast<float>(limit))
    {
        m_labelTooWide = true;
        return 0;
    }
    const std::wstring source = field->getText();
    int split = 0;
    int index = static_cast<int>(source.size()) - 1;
    const unsigned int pairCount = static_cast<unsigned int>(source.size()) >> 1;
    const float halfTotal = static_cast<float>(Math::round(total * 0.5f));
    m_labelTooWide = false;
    unsigned int pairs = pairCount;
    while (pairs != 0)
    {
        if (source[static_cast<std::wstring::size_type>(index)] == L' ')
        {
            field->setText(source.substr(0, static_cast<std::wstring::size_type>(index)));
            if (offset + field->getWidth() <= halfTotal)
            {
                split = index;
                break;
            }
        }
        const int second = index - 1;
        if (source[static_cast<std::wstring::size_type>(second)] == L' ')
        {
            field->setText(source.substr(0, static_cast<std::wstring::size_type>(second)));
            if (offset + field->getWidth() <= halfTotal)
            {
                split = second;
                break;
            }
        }
        --pairs;
        index -= 2;
    }
    if (pairs == 0 && (source.size() & 1u) != 0u && source[static_cast<std::wstring::size_type>(index)] == L' ')
    {
        field->setText(source.substr(0, static_cast<std::wstring::size_type>(index)));
        if (offset + field->getWidth() <= static_cast<float>(limit))
            split = index;
    }
    field->setText(source.substr(0, static_cast<std::wstring::size_type>(split)));
    const int firstWidth = static_cast<int>(field->getWidth());
    field->setText(source.substr(static_cast<std::wstring::size_type>(split + 1)));
    const int secondWidth = static_cast<int>(field->getWidth());
    field->setText(std::wstring(L" "));
    const float spaceWidth = field->getWidth();
    const int widthDelta = firstWidth < secondWidth ? secondWidth - firstWidth : firstWidth - secondWidth;
    const unsigned int padding = spaceWidth == 0.0f ? 0u : static_cast<unsigned int>(Math::round(static_cast<float>(widthDelta) / spaceWidth));
    std::wstring spaces;
    for (unsigned int i = 0; i < padding; ++i)
        spaces.append(L" ");
    const std::wstring first = source.substr(0, static_cast<std::wstring::size_type>(split));
    const std::wstring secondText = source.substr(static_cast<std::wstring::size_type>(split + 1));
    if (firstWidth < secondWidth)
        field->setText(first + spaces + L"\n" + secondText);
    else if (secondWidth < firstWidth)
        field->setText(first + L"\n" + spaces + secondText);
    else
        field->setText(first + L"\n" + secondText);
    return 0;
}

uint64_t FJ_Base::DoMultilineWordWrap(FJ_FuiNode* container, fuiRenderNodeEditText* field)
{
    if (m_wrapWidth == 0)
        m_wrapWidth = static_cast<int>(getWidth());
    float offset;
    float limit;
    if (container == 0)
    {
        offset = 0.0f;
        limit = static_cast<float>(static_cast<int>(static_cast<float>(m_wrapWidth) * getScaleX()));
    }
    else
    {
        offset = static_cast<float>(static_cast<int>(container->getX() * getScaleX()));
        limit = static_cast<float>(m_wrapWidth);
    }
    m_lineCount = 0;
    float measured = field->getWidth();
    if (offset + measured <= limit)
        return 0;
    std::wstring source = field->getText();
    std::wstring accumulated = std::wstring(L"");
    while (limit < offset + measured)
    {
        field->setText(std::wstring(L""));
        std::size_t split = static_cast<std::size_t>(-1);
        std::size_t latestSpace = 0;
        std::size_t index = 0;
        while (index < source.size())
        {
            std::wstring character = source.substr(index, 1);
            field->appendText(character);
            if (limit <= offset + field->getWidth())
                break;
            if (source[index] == L' ')
                latestSpace = index;
            ++index;
        }
        if (latestSpace > 0)
            split = latestSpace;
        ++m_lineCount;
        m_lineWidths.push_back(static_cast<int>(split));
        accumulated += source.substr(0, split + 1);
        accumulated += L"\n";
        source = source.substr(split + 1);
        field->setText(source);
        measured = field->getWidth();
    }
    field->setText(accumulated + source);
    m_label = field->getText();
    return 0;
}

uint64_t FJ_Base::DoEllipsisCheck(FJ_FuiNode* container, fuiRenderNodeEditText* field)
{
    if (m_wrapWidth == 0)
        m_wrapWidth = static_cast<int>(getWidth());
    int offset = 0;
    int suffixAllowance = 5;
    int limitWidth;
    if (container == 0)
        limitWidth = static_cast<int>(static_cast<float>(m_wrapWidth) * getScaleX());
    else
    {
        offset = static_cast<int>(container->getX() * getScaleX());
        limitWidth = m_wrapWidth;
    }
    if (m_wrapWidth > 0)
    {
        suffixAllowance = 0;
        limitWidth = m_wrapWidth;
    }
    if (static_cast<float>(limitWidth) < static_cast<float>(offset) + field->getWidth() + static_cast<float>(suffixAllowance))
    {
        const float targetWidth = static_cast<float>(limitWidth - offset - suffixAllowance);
        do
        {
            const std::wstring lengthSource = field->getText();
            const std::wstring textSource = field->getText();
            field->setText(textSource.substr(0, lengthSource.size() - 1));
        }
        while (targetWidth < field->getWidth());
        const std::wstring lengthSource = field->getText();
        const std::wstring textSource = field->getText();
        field->setText(textSource.substr(0, lengthSource.size() - 3));
        m_label = field->getText() + L"...";
        field->setText(m_label);
    }
    return 0;
}

void FJ_Base::SetLabel(const std::wstring& label)
{
    m_label = label;
    CheckLabelWidth(label);
    fuiRenderNodeEditText* field = GetTextField();
    if (field != 0) DoManualWordWrapCheck(0, field);
}

int FJ_Base::GetLabel() { return (int)(intptr_t)&m_label; }

void FJ_Base::SetTextOnly(const std::wstring& text)
{
    fuiRenderNodeEditText* field = GetTextField();
    if (field != 0) field->setText(text.empty() ? std::wstring(L" ") : text);
}

void FJ_Base::GetLabelTooWide(fuiRenderNodeEditText* field)
{
    float measured = field->getWidth();
    int required = m_maxLabelWidth > static_cast<int>(measured) ? m_maxLabelWidth : static_cast<int>(measured);
    if (m_newWidth < required) m_newWidth = required;
    if (!m_labelTooWide && getWidth() < static_cast<float>(m_maxLabelWidth))
    {
        m_labelTooWide = true;
        m_manualWrap = true;
        m_wrapWidth = static_cast<int>(m_objectWidth * 0.8f);
    }
}

void FJ_Base::SetNewObjectWidth(int width)
{
    if (static_cast<int>(getWidth()) < width)
    {
        SetTextOnly(std::wstring(L" "));
        setWidth(static_cast<float>(width));
        m_preScaleWidth = width;
        fuiRect rect = getRect(this);
        m_labelTooWide = false;
        m_manualWrap = false;
        m_objectWidth = getWidth() + rect.xMin + rect.xMin;
    }
}

void FJ_Base::SetNewObjectHeight(int height)
{
    if (static_cast<int>(getHeight()) < height && !m_textChanged)
    {
        m_textChanged = true;
        SetTextOnly(std::wstring(L" "));
        setHeight(static_cast<float>(height));
        fuiRect rect = getRect(this);
        m_objectHeight = getHeight() + rect.yMin + rect.yMin;
    }
}

void FJ_Base::HandleCharInput(unsigned int) {}
uint32_t FJ_Base::getType() { return 0x8000; }
