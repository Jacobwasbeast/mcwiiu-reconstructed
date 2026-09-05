#ifndef GAME_UI_FUIRENDERNODEEDITTEXT_HPP
#define GAME_UI_FUIRENDERNODEEDITTEXT_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiFontSet.hpp"
#include "game/ui/fuiRenderNode.hpp"

enum TextFieldAutoSize
{
    TextFieldAutoSizeNone = 0,
    TextFieldAutoSizeLeft = 1,
    TextFieldAutoSizeCenter = 2,
    TextFieldAutoSizeRight = 3
};

class fuiRenderNodeEditText : public fuiRenderNode
{
public:
    struct HtmlImageDrawData
    {
        float x;
        float y;
        float width;
        float height;
    };

    class RenderStringStateData
    {
    public:
        RenderStringStateData();
        void init(fuiFontSet* font, float fontSize, float lineSpacing);

        int currentCharacter;
        int lineNumber;
        float fontSize;
        int colour;
        float x;
        float y;
        float lineHeight;
        float width;
        int alignment;
        int flags;
        float scale;
        int texture;
        bool html;
        bool bold;
        int fontIndex;
        bool clipping;
        bool shadow;
        int firstLine;
        int lastLine;
        int lineStart;
        int lineEnd;
        int glyphCount;
        int imageCount;
        std::vector<fuiFontSet::FontVert> fontVerts[4];
        std::vector<HtmlImageDrawData> imageData;
        unsigned char reserved[84];
    };

    fuiRenderNodeEditText(fuiRenderNode*, fuiRenderNode*, fuiObject*, unsigned int, unsigned char, fuiRGBA*, unsigned char, bool, fuiFile*);
    virtual ~fuiRenderNodeEditText();
    static const wchar_t* getHtmlTag(const wchar_t*, std::wstring*);
    static const wchar_t* parseHTMLString(const wchar_t*, std::wstring*);
    const wchar_t* prepStringPtr(const std::wstring&) const;
    const wchar_t* incStringPtr(const wchar_t*) const;
    float getGlyphScale();
    float getRenderedGlyphHeight(wchar_t);
    bool useBitmapFont(unsigned short, fuiFontSet*);
    float getRenderedGlyphWidth(wchar_t, RenderStringStateData&);
    float getWordWidth(const std::wstring&, RenderStringStateData&);
    void setHtmlAlignPosition(RenderStringStateData&);
    void newLine(RenderStringStateData&);
    bool isHtmlTagType(const std::wstring&, const std::wstring&);
    bool htmlFontColour(const std::wstring&, RenderStringStateData&);
    std::wstring getHtmlTagAttribute(const std::wstring&, const std::wstring&);
    bool htmlImage(const std::wstring&, RenderStringStateData&);
    bool checkForUnsupportedAttribute(const std::wstring&, const std::wstring&);
    bool htmlParagraph(const std::wstring&, RenderStringStateData&);
    bool renderHtmlTag(const std::wstring&, RenderStringStateData&);
    bool isCJKChar(wchar_t);
    bool isCJKPunctuationChar(wchar_t);
    bool endOfWord(wchar_t, wchar_t);
    bool getNextWord(const wchar_t*, std::wstring*, RenderStringStateData&);
    void increaseCurrentCharacter(int, const wchar_t*, RenderStringStateData&);
    void renderGlyph(unsigned short, float, float, float, unsigned int, RenderStringStateData&);
    bool renderWord(std::wstring&, RenderStringStateData&);
    void renderString(const std::wstring&, RenderStringStateData&);
    void getRenderedStringWidthHeight(const std::wstring&, fuiFontSet*, float, float&, float&);
    bool clipBottom(const fuiFontSet::FontVert&);
    bool clipTop(const fuiFontSet::FontVert&);
    void DrawFontVerts(RenderStringStateData&, std::vector<fuiFontSet::FontVert>*);
    void DrawRenderData(RenderStringStateData&);
    void renderString(const std::wstring&, float, float, fuiFontSet*, float, unsigned int);
    void renderStringShadow(const std::wstring&, float, float, fuiFontSet*, float);
    void autoSizeBounds();
    virtual float getWidth();
    virtual float getHeight();
    virtual void gather(bool);
    virtual void preRender();
    virtual void render(fuiRect*, fuiMatrix*);
    const std::wstring& getText();
    void setText(const std::wstring&);
    void appendText(const std::wstring&);
    void setHTMLText(const std::wstring&);
    void setAutoSize(TextFieldAutoSize);
    std::pair<float, float> getCharBoundaries(int);
    std::wstring createWordWrappedString(std::wstring, fuiFontSet*, float, float);
    virtual void setWidth(float);
    virtual void setHeight(float);
    virtual void setX(float);
    virtual void setY(float);
    virtual void getParentSpaceBounds(fuiRect*);
    virtual void setVisibility(bool);
    float getScrollH();
    void setScrollH(float);
    unsigned int getNumLines();

private:
    std::wstring m_text;
    std::wstring m_htmlText;
    std::wstring m_word;
    std::wstring m_wrappedText;
    std::wstring m_attribute;
    std::vector<float> m_lineWidths;
    std::vector<float> m_lineHeights;
    RenderStringStateData m_renderState;
    fuiFontSet* m_font;
    float m_scrollH;
    float m_glyphScale;
    float m_lineSpacing;
    float m_textWidth;
    float m_textHeight;
    unsigned int m_numLines;
    TextFieldAutoSize m_autoSize;
    bool m_html;
    bool m_dirty;
};

#endif
