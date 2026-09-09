#include "game/ui/fuiRenderNodeEditText.hpp"

#include <algorithm>
#include <cwctype>

fuiRenderNodeEditText::RenderStringStateData::RenderStringStateData()
    : currentCharacter(0), lineNumber(0), fontSize(0.0f), colour(0xffffffff), x(0.0f), y(0.0f),
      lineHeight(0.0f), width(0.0f), alignment(0), flags(0), scale(1.0f), texture(0), html(false),
      bold(false), fontIndex(0), clipping(false), shadow(false), firstLine(0), lastLine(0),
      lineStart(0), lineEnd(0), glyphCount(0), imageCount(0)
{
}

void fuiRenderNodeEditText::RenderStringStateData::init(fuiFontSet* font, float fontSizeValue, float spacing)
{
    currentCharacter = 0;
    lineNumber = 0;
    fontSize = fontSizeValue;
    lineHeight = spacing;
    x = 0.0f;
    y = 0.0f;
    width = 0.0f;
    alignment = 0;
    flags = 0;
    scale = font == NULL ? 1.0f : font->getGlyphScale(fontSizeValue);
    texture = 0;
    html = false;
    bold = false;
    clipping = false;
    shadow = false;
    firstLine = 0;
    lastLine = 0;
    lineStart = 0;
    lineEnd = 0;
    glyphCount = 0;
    imageCount = 0;
    fontVerts[0].clear();
    fontVerts[1].clear();
    fontVerts[2].clear();
    fontVerts[3].clear();
    imageData.clear();
}

fuiRenderNodeEditText::fuiRenderNodeEditText(fuiRenderNode* parent, fuiRenderNode* node, fuiObject* object,
    unsigned int symbolIndex, unsigned char type, fuiRGBA* colour, unsigned char renderFlags, bool visible, fuiFile* file)
    : fuiRenderNode(parent, node, object, symbolIndex, static_cast<eFuiObjectType>(type), renderFlags, colour, 0, visible, file),
      m_html(false), m_text(), m_scrollH(0.0f), m_alignment(0), m_wordWrap(false), m_multiline(false),
      m_autoSize(TextFieldAutoSizeCenter), m_htmlAlignment(0), m_textWidth(0.0f), m_textHeight(0.0f),
      m_numLines(0), m_lineWidths(), m_lineHeights(), m_font(NULL), m_glyphScale(1.0f),
      m_colour(0), m_textSet(false), m_fieldWidth(0.0f), m_renderState(), m_htmlText(), m_wrappedText(),
      m_attribute(), m_word(), m_dirty(true)
{
    InitializeCriticalSection(&m_lock);
}

fuiRenderNodeEditText::~fuiRenderNodeEditText()
{
}

const wchar_t* fuiRenderNodeEditText::getHtmlTag(const wchar_t* text, std::wstring* tag)
{
    if (tag != NULL)
        tag->clear();
    if (text == NULL || *text != L'<')
        return text;
    const wchar_t* cursor = text + 1;
    while (*cursor != L'\0' && *cursor != L'>')
    {
        if (tag != NULL)
            tag->push_back(*cursor);
        ++cursor;
    }
    if (*cursor == L'>')
        ++cursor;
    while (std::iswspace(*cursor))
        ++cursor;
    return cursor;
}

const wchar_t* fuiRenderNodeEditText::parseHTMLString(const wchar_t* text, std::wstring* result)
{
    if (result != NULL)
        result->clear();
    if (text == NULL)
        return text;
    const wchar_t* cursor = text;
    while (*cursor == L'<')
        cursor = getHtmlTag(cursor, result);
    if (result != NULL && *text != L'<')
        *result = text;
    return cursor;
}

const wchar_t* fuiRenderNodeEditText::prepStringPtr(const std::wstring& text) const
{
    return text.c_str();
}

const wchar_t* fuiRenderNodeEditText::incStringPtr(const wchar_t* text) const
{
    return text == NULL ? NULL : text + 1;
}

float fuiRenderNodeEditText::getGlyphScale()
{
    return m_boldFont->getGlyphScale(m_glyphScale);
}

float fuiRenderNodeEditText::getRenderedGlyphHeight(wchar_t character)
{
    return m_boldFont->getRenderedGlyphHeight(static_cast<unsigned short>(character), m_glyphScale);
}

bool fuiRenderNodeEditText::useBitmapFont(unsigned short character, fuiFontSet* font)
{
    return font != NULL && font->isBitmapFont() && font->hasGlyph(character);
}

float fuiRenderNodeEditText::getRenderedGlyphWidth(wchar_t character, RenderStringStateData& state)
{
    return m_font == NULL ? state.fontSize : m_font->getRenderedGlyphWidth(static_cast<unsigned short>(character), state.fontSize);
}

float fuiRenderNodeEditText::getWordWidth(const std::wstring& word, RenderStringStateData& state)
{
    float width = 0.0f;
    std::wstring::const_iterator it;
    for (it = word.begin(); it != word.end(); ++it)
        width += getRenderedGlyphWidth(*it, state);
    return width;
}

void fuiRenderNodeEditText::setHtmlAlignPosition(RenderStringStateData& state)
{
    if (state.alignment == 1)
        state.x = (getWidth() - state.width) * 0.5f;
    else if (state.alignment == 2)
        state.x = getWidth() - state.width;
}

void fuiRenderNodeEditText::newLine(RenderStringStateData& state)
{
    ++state.lineNumber;
    state.x = 0.0f;
    state.y += state.lineHeight;
    state.width = 0.0f;
    setHtmlAlignPosition(state);
}

bool fuiRenderNodeEditText::isHtmlTagType(const std::wstring& tag, const std::wstring& type)
{
    return tag == type || tag == L"/" + type || tag.find(type) == 0;
}

bool fuiRenderNodeEditText::htmlFontColour(const std::wstring& tag, RenderStringStateData& state)
{
    std::wstring value = getHtmlTagAttribute(tag, L"color");
    if (value.empty())
        return false;
    state.colour = static_cast<int>(std::wcstol(value.c_str(), NULL, 16));
    return true;
}

std::wstring fuiRenderNodeEditText::getHtmlTagAttribute(const std::wstring& tag, const std::wstring& attribute)
{
    std::wstring key = attribute + L"=";
    std::wstring::size_type position = tag.find(key);
    if (position == std::wstring::npos)
        return std::wstring();
    position += key.size();
    while (position < tag.size() && std::iswspace(tag[position]))
        ++position;
    wchar_t quote = position < tag.size() && (tag[position] == L'"' || tag[position] == L'\'') ? tag[position++] : 0;
    std::wstring::size_type end = position;
    while (end < tag.size() && ((quote != 0 && tag[end] != quote) || (quote == 0 && !std::iswspace(tag[end]))))
        ++end;
    return tag.substr(position, end - position);
}

bool fuiRenderNodeEditText::htmlImage(const std::wstring& tag, RenderStringStateData& state)
{
    std::wstring width = getHtmlTagAttribute(tag, L"width");
    std::wstring height = getHtmlTagAttribute(tag, L"height");
    HtmlImageDrawData image;
    image.x = state.x;
    image.y = state.y;
    image.width = width.empty() ? 0.0f : static_cast<float>(std::wcstod(width.c_str(), NULL));
    image.height = height.empty() ? 0.0f : static_cast<float>(std::wcstod(height.c_str(), NULL));
    state.imageData.push_back(image);
    ++state.imageCount;
    state.x += image.width;
    return true;
}

bool fuiRenderNodeEditText::checkForUnsupportedAttribute(const std::wstring& tag, const std::wstring& attribute)
{
    return tag.find(attribute) != std::wstring::npos;
}

bool fuiRenderNodeEditText::htmlParagraph(const std::wstring& tag, RenderStringStateData& state)
{
    if (isHtmlTagType(tag, L"p"))
        newLine(state);
    return true;
}

bool fuiRenderNodeEditText::renderHtmlTag(const std::wstring& tag, RenderStringStateData& state)
{
    if (isHtmlTagType(tag, L"br"))
        newLine(state);
    else if (isHtmlTagType(tag, L"img"))
        htmlImage(tag, state);
    else if (isHtmlTagType(tag, L"font"))
        htmlFontColour(tag, state);
    else if (isHtmlTagType(tag, L"p"))
        htmlParagraph(tag, state);
    return true;
}

bool fuiRenderNodeEditText::isCJKChar(wchar_t character)
{
    return (character >= 0x2e80 && character <= 0x9fff) || (character >= 0xac00 && character <= 0xd7af);
}

bool fuiRenderNodeEditText::isCJKPunctuationChar(wchar_t character)
{
    return character >= 0x3000 && character <= 0x303f;
}

bool fuiRenderNodeEditText::endOfWord(wchar_t previous, wchar_t current)
{
    return std::iswspace(current) || isCJKChar(previous) != isCJKChar(current) || isCJKPunctuationChar(current);
}

bool fuiRenderNodeEditText::getNextWord(const wchar_t* text, std::wstring* word, RenderStringStateData& state)
{
    if (word != NULL)
        word->clear();
    if (text == NULL || *text == L'\0')
        return false;
    const wchar_t* cursor = text;
    while (*cursor != L'\0')
    {
        if (word != NULL)
            word->push_back(*cursor);
        if (cursor[1] == L'\0' || endOfWord(*cursor, cursor[1]))
            break;
        ++cursor;
    }
    state.lineEnd = static_cast<int>(cursor - text) + 1;
    return true;
}

void fuiRenderNodeEditText::increaseCurrentCharacter(int count, const wchar_t* text, RenderStringStateData& state)
{
    if (text != NULL)
        state.currentCharacter += count < 0 ? 0 : count;
}

void fuiRenderNodeEditText::renderGlyph(unsigned short character, float x, float y, float size, unsigned int colour, RenderStringStateData& state)
{
    if (m_font != NULL)
        m_font->renderGlyph(character, x, y, size, colour, state.fontVerts);
    ++state.glyphCount;
}

bool fuiRenderNodeEditText::renderWord(std::wstring& word, RenderStringStateData& state)
{
    std::wstring::const_iterator it;
    for (it = word.begin(); it != word.end(); ++it)
    {
        renderGlyph(static_cast<unsigned short>(*it), state.x, state.y, state.fontSize, static_cast<unsigned int>(state.colour), state);
        state.x += getRenderedGlyphWidth(*it, state);
    }
    state.width = std::max(state.width, state.x);
    return true;
}

void fuiRenderNodeEditText::renderString(const std::wstring& text, RenderStringStateData& state)
{
    const wchar_t* cursor = text.c_str();
    while (*cursor != L'\0')
    {
        if (*cursor == L'<')
        {
            std::wstring tag;
            cursor = getHtmlTag(cursor, &tag);
            renderHtmlTag(tag, state);
            continue;
        }
        if (*cursor == L'\n')
        {
            newLine(state);
            ++cursor;
            continue;
        }
        renderGlyph(static_cast<unsigned short>(*cursor), state.x, state.y, state.fontSize, static_cast<unsigned int>(state.colour), state);
        state.x += getRenderedGlyphWidth(*cursor, state);
        state.width = std::max(state.width, state.x);
        ++cursor;
    }
    state.lastLine = state.lineNumber;
}

void fuiRenderNodeEditText::getRenderedStringWidthHeight(const std::wstring& text, fuiFontSet* font, float size, float& width, float& height)
{
    fuiFontSet* oldFont = m_font;
    float oldSize = m_glyphScale;
    m_font = font;
    m_glyphScale = size;
    RenderStringStateData state;
    state.init(font, size, getRenderedGlyphHeight(L'M'));
    renderString(text, state);
    width = state.width;
    height = state.lineNumber == 0 ? getRenderedGlyphHeight(L'M') : state.y + state.lineHeight;
    m_font = oldFont;
    m_glyphScale = oldSize;
}

bool fuiRenderNodeEditText::clipBottom(const fuiFontSet::FontVert& vertex)
{
    return vertex.y <= getHeight();
}

bool fuiRenderNodeEditText::clipTop(const fuiFontSet::FontVert& vertex)
{
    return vertex.y >= 0.0f;
}

void fuiRenderNodeEditText::DrawFontVerts(RenderStringStateData&, std::vector<fuiFontSet::FontVert>* vertices)
{
    if (vertices != NULL)
        vertices->clear();
}

void fuiRenderNodeEditText::DrawRenderData(RenderStringStateData& state)
{
    state.imageCount = 0;
}

void fuiRenderNodeEditText::renderString(const std::wstring& text, float x, float y, fuiFontSet* font, float size, unsigned int colour)
{
    m_font = font;
    m_glyphScale = size;
    m_renderState.init(font, size, getRenderedGlyphHeight(L'M'));
    m_renderState.x = x;
    m_renderState.y = y;
    m_renderState.colour = static_cast<int>(colour);
    renderString(text, m_renderState);
    DrawRenderData(m_renderState);
}

void fuiRenderNodeEditText::renderStringShadow(const std::wstring& text, float x, float y, fuiFontSet* font, float size)
{
    renderString(text, x + 1.0f, y + 1.0f, font, size, 0xff000000);
}

void fuiRenderNodeEditText::autoSizeBounds()
{
    float width = 0.0f;
    float height = 0.0f;
    getRenderedStringWidthHeight(m_text, m_font, m_glyphScale, width, height);

    float bounds = -(getGlyphScale() - width);
    if (m_textWidth != bounds || m_textHeight != height)
    {
        m_textWidth = bounds;
        m_textHeight = height;
        m_dirty = true;
    }

    if (m_autoSize == TextFieldAutoSizeLeft)
    {
        m_bounds.xMax = m_bounds.xMin + (getGlyphScale() + width);
        m_bounds.yMax = height * 1.0436591f + m_bounds.yMin;
        progogateBounds();
    }
    else if (m_autoSize == TextFieldAutoSizeRight)
    {
        m_bounds.yMax = height * 1.0436591f + m_bounds.yMin;
        m_alignment = 1;
        progogateBounds();
    }
}

float fuiRenderNodeEditText::getWidth()
{
    autoSizeBounds();
    return fuiRenderNode::getWidth();
}

float fuiRenderNodeEditText::getHeight()
{
    autoSizeBounds();
    return fuiRenderNode::getHeight();
}

void fuiRenderNodeEditText::gather(bool)
{
}

void fuiRenderNodeEditText::preRender()
{
    if (m_font != NULL)
        m_font->preRender();
}

void fuiRenderNodeEditText::render(fuiRect* rect, fuiMatrix* matrix)
{
    if (m_dirty)
    {
        RenderStringStateData state;
        state.init(m_font, m_glyphScale, getGlyphScale());
        renderString(m_html ? m_htmlText : m_text, state);
        m_renderState = state;
        m_dirty = false;
    }
    DrawRenderData(m_renderState);
}

const std::wstring& fuiRenderNodeEditText::getText()
{
    return m_text;
}

void fuiRenderNodeEditText::setText(const std::wstring& text)
{
    m_text = text;
    m_html = false;
    m_dirty = true;
    autoSizeBounds();
}

void fuiRenderNodeEditText::appendText(const std::wstring& text)
{
    m_text += text;
    m_dirty = true;
}

void fuiRenderNodeEditText::setHTMLText(const std::wstring& text)
{
    m_htmlText = text;
    m_html = true;
    m_dirty = true;
}

void fuiRenderNodeEditText::setAutoSize(int autoSize)
{
    if (m_autoSize != static_cast<TextFieldAutoSize>(autoSize))
    {
        if (m_html)
        {
            autoSizeBounds();
            m_dirty = true;
        }
        m_autoSize = static_cast<TextFieldAutoSize>(autoSize);
    }
}

std::pair<float, float> fuiRenderNodeEditText::getCharBoundaries(int index)
{
    if (index < 0 || static_cast<std::wstring::size_type>(index) >= m_text.size())
        return std::pair<float, float>(0.0f, 0.0f);
    RenderStringStateData state;
    state.init(m_font, m_glyphScale, getGlyphScale());
    for (int i = 0; i < index; ++i)
        state.x += getRenderedGlyphWidth(m_text[static_cast<std::wstring::size_type>(i)], state);
    return std::pair<float, float>(state.x, getRenderedGlyphHeight(m_text[static_cast<std::wstring::size_type>(index)]));
}

std::wstring fuiRenderNodeEditText::createWordWrappedString(std::wstring text, fuiFontSet* font, float size, float width)
{
    m_font = font;
    m_glyphScale = size;
    RenderStringStateData state;
    state.init(font, size, getRenderedGlyphHeight(L'M'));
    std::wstring result;
    std::wstring::const_iterator it;
    for (it = text.begin(); it != text.end(); ++it)
    {
        float glyphWidth = getRenderedGlyphWidth(*it, state);
        if (*it != L'\n' && state.x + glyphWidth > width && !result.empty())
        {
            result.push_back(L'\n');
            state.x = 0.0f;
        }
        result.push_back(*it);
        state.x += glyphWidth;
        if (*it == L'\n')
            state.x = 0.0f;
    }
    return result;
}

void fuiRenderNodeEditText::setWidth(float width)
{
    if (fuiRenderNode::getWidth() != width)
        m_dirty = true;
    fuiRenderNode::setWidth(width);
}

void fuiRenderNodeEditText::setHeight(float height)
{
    if (fuiRenderNode::getHeight() != height)
        m_dirty = true;
    fuiRenderNode::setHeight(height);
}

void fuiRenderNodeEditText::setX(float x)
{
    fuiRenderNode::setX(x);
    m_dirty = true;
}

void fuiRenderNodeEditText::setY(float y)
{
    fuiRenderNode::setY(y);
    m_dirty = true;
}

void fuiRenderNodeEditText::getParentSpaceBounds(fuiRect* rect)
{
    fuiRenderNode::getParentSpaceBounds(rect);
}

void fuiRenderNodeEditText::setVisibility(bool visible)
{
    m_dirty = true;
    fuiRenderNode::setVisibility(visible);
}

float fuiRenderNodeEditText::getScrollH()
{
    return m_scrollH;
}

void fuiRenderNodeEditText::setScrollH(float scroll)
{
    m_scrollH = scroll;
    m_dirty = true;
}

unsigned int fuiRenderNodeEditText::getNumLines()
{
    return m_numLines;
}
