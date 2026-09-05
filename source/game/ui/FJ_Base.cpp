#include "game/ui/FJ_Base.hpp"
#include <cmath>
#include "game/ui/fuiRenderNode.hpp"
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
    m_wrapHeight = 0;
    m_preScaleWidth = 0;
    m_maxLabelWidth = -1;
    m_newWidth = static_cast<int>(getWidth());
    m_oldWidth = static_cast<int>(getWidth());
    m_objListParent = 0;
    m_listId = 0;
    CorrectPixelPosition();
}

FJ_Base::~FJ_Base() {}
FJ_FuiNode* FJ_Base::getObjListParent() { return m_objListParent; }
uint32_t FJ_Base::getId() { return static_cast<uint32_t>(m_id); }

uint64_t FJ_Base::LostFocus()
{
    if (m_objListParent != 0 && m_objListParent->isFJ_List())
    {
        return m_objListParent->asFJ_List()->LoseListFocus();
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
    }
    else
    {
        FJ_FuiNode* container = getChildByName(std::string("FJ_TextContainer"));
        if (container == 0 || container->getNumChildren() == 0)
        {
            return 0;
        }
        node = container->getChildAt(0);
    }
    if (node == 0 || node->asMovieClip() == 0 || node->asMovieClip()->m_pRenderNode == 0)
    {
        return 0;
    }
    fuiRenderNode* timeline = node->asMovieClip()->m_pRenderNode->asTimeline();
    return timeline == 0 ? 0 : static_cast<fuiRenderNodeEditText*>(timeline->asEditText());
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
    if (m_wrapWidth == 0) m_wrapWidth = static_cast<int>(getWidth());
    int width = m_wrapWidth;
    if (container != 0) width -= static_cast<int>(container->getX() * getScaleX());
    std::wstring text = field->getText();
    if (field->getWidth() <= static_cast<float>(width))
    {
        m_labelTooWide = true;
        return 0;
    }
    std::wstring::size_type split = text.find_last_of(L' ');
    if (split == std::wstring::npos)
    {
        field->setText(text);
        return 0;
    }
    std::wstring first = text.substr(0, split);
    std::wstring second = text.substr(split + 1);
    field->setText(first);
    m_label = first;
    if (!second.empty()) field->setText(second);
    m_lineCount = 2;
    m_lineWidths.push_back(static_cast<int>(first.size()));
    m_lineWidths.push_back(static_cast<int>(second.size()));
    return 0;
}

uint64_t FJ_Base::DoMultilineWordWrap(FJ_FuiNode* container, fuiRenderNodeEditText* field)
{
    if (m_wrapWidth == 0) m_wrapWidth = static_cast<int>(getWidth());
    if (container != 0) m_wrapWidth -= static_cast<int>(container->getX() * getScaleX());
    m_lineCount = 0;
    m_lineWidths.clear();
    std::wstring source = field->getText();
    std::wstring result;
    while (!source.empty())
    {
        std::wstring::size_type split = source.find(L' ');
        std::wstring line = split == std::wstring::npos ? source : source.substr(0, split);
        source = split == std::wstring::npos ? std::wstring() : source.substr(split + 1);
        if (!result.empty()) result += L"\n";
        result += line;
        m_lineWidths.push_back(static_cast<int>(line.size()));
        ++m_lineCount;
    }
    field->setText(result);
    m_label = field->getText();
    return 0;
}

uint64_t FJ_Base::DoEllipsisCheck(FJ_FuiNode* container, fuiRenderNodeEditText* field)
{
    int width = m_wrapWidth == 0 ? static_cast<int>(getWidth()) : m_wrapWidth;
    if (container != 0 && m_wrapWidth == 0) width = static_cast<int>(container->getX() * getScaleX());
    while (field->getWidth() > static_cast<float>(width) && !field->getText().empty())
    {
        std::wstring text = field->getText();
        text.erase(text.size() - 1);
        field->setText(text);
    }
    field->setText(field->getText() + std::wstring(L"..."));
    m_label = field->getText();
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
