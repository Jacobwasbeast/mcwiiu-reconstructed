#include "game/ui/fuiBitmapFont.hpp"

#include <string>

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/CFontData.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/client/render/SFontData.hpp"
#include "game/ui/fui.hpp"

fuiBitmapFont::fuiBitmapFont(SFontData& fontData, unsigned short missingGlyphChar)
    : fuiFontSet(fontData.fontName.c_str()), UIBitmapFont(fontData),
      m_missingGlyphChar(missingGlyphChar)
{
    m_sFontData = &fontData;
    BufferedImage image(fontData.textureFileName, true, true, L"");

    m_textureIndex = static_cast<unsigned int>(gRenderer->TextureCreate());
#ifdef RECON_NATIVE
    for (unsigned int i = 0; i < image.getWidth() * image.getHeight(); ++i)
    {
        const unsigned int argb = static_cast<unsigned int>(image.getData()[i]);
        unsigned char* rgba = reinterpret_cast<unsigned char*>(&image.getData()[i]);
        rgba[0] = static_cast<unsigned char>(argb >> 16);
        rgba[1] = static_cast<unsigned char>(argb >> 8);
        rgba[2] = static_cast<unsigned char>(argb);
        rgba[3] = static_cast<unsigned char>(argb >> 24);
    }
#else
    fui::EndianSwapImageData(image.getData(), image.getWidth() * image.getHeight());
#endif
    gRenderer->TextureBind(static_cast<int>(m_textureIndex));
    gRenderer->TextureSetTextureLevels(1);
    gRenderer->TextureData(static_cast<int>(image.getWidth()), static_cast<int>(image.getHeight()), image.getData(), 0,
                           C4JRender::eTextureFormat_RGBA8);
    m_registered = false;
}

int fuiBitmapFont::getTextureIndex(float fontSize, int ttfIndex)
{
    return static_cast<int>(m_textureIndex);
}

float fuiBitmapFont::getGlyphScale(float fontSize)
{
    for (std::vector<GlyphScaleData>::iterator it = m_glyphScaleData.begin(); it != m_glyphScaleData.end(); ++it)
    {
        if (it->fontSize == fontSize)
        {
            return it->scale;
        }
    }

    float scale = 1.0f;
    while ((scale + 0.5f) * (1.0f / getFontData()->getFontData().glyphWidthScale) < fontSize)
    {
        scale += 1.0f;
    }

    GlyphScaleData data;
    data.fontSize = fontSize;
    data.scale = scale;
    m_glyphScaleData.push_back(data);
    return scale;
}

void fuiBitmapFont::getGlyphTexPos(unsigned short codepoint, float& texX, float& texY)
{
    CFontData* fontData = getFontData();
    const unsigned short glyphId = fontData->getGlyphId(codepoint);
    int row = 0;
    int column = 0;
    fontData->getPos(glyphId, row, column);

    texX = static_cast<float>(column * static_cast<int>(m_sFontData->glyphWidth)) / static_cast<float>(m_sFontData->imageWidth);
    texY = static_cast<float>(row * static_cast<int>(m_sFontData->glyphHeight)) / static_cast<float>(m_sFontData->imageHeight);
}

void fuiBitmapFont::getGlyphWidthHeight(float& texWidth, float& texHeight)
{
    texWidth = static_cast<float>(m_sFontData->glyphWidth) / static_cast<float>(m_sFontData->imageWidth);
    texHeight = static_cast<float>(m_sFontData->glyphHeight) / static_cast<float>(m_sFontData->imageHeight);
}

float fuiBitmapFont::renderGlyph(unsigned short codepoint, float x, float y, float fontSize, unsigned int colour,
                                 std::vector<fuiFontSet::FontVert> (&verts)[4])
{
    CFontData* fontData = getFontData();
    const unsigned short glyphId = fontData->getGlyphId(codepoint);
    const float glyphScale = getGlyphScale(fontSize);
    const float right = x + static_cast<float>(getFontData()->getFontData().glyphWidth) * glyphScale;
    const float bottom = y + static_cast<float>(getFontData()->getFontData().glyphHeight) * glyphScale;

    float texX;
    float texY;
    getGlyphTexPos(codepoint, texX, texY);

    float texWidth;
    float texHeight;
    getGlyphWidthHeight(texWidth, texHeight);

    fuiFontSet::FontVert vert;
    vert.x = x;
    vert.y = y;
    vert.u = texX;
    vert.v = texY;
    vert.colour = colour;
    verts[0].push_back(vert);

    vert.x = right;
    vert.u = texX + texWidth;
    verts[0].push_back(vert);

    vert.x = right;
    vert.y = bottom;
    vert.v = texY + texHeight;
    verts[0].push_back(vert);

    vert.x = x;
    vert.u = texX;
    verts[0].push_back(vert);

    return static_cast<float>(fontData->getWidth(glyphId)) * glyphScale;
}

float fuiBitmapFont::getRenderedGlyphWidth(unsigned short codepoint, float fontSize)
{
    const unsigned short glyphId = getFontData()->getGlyphId(codepoint);
    const float glyphScale = getGlyphScale(fontSize);
    return static_cast<float>(static_cast<int>(getFontData()->getWidth(glyphId))) * glyphScale;
}

float fuiBitmapFont::getRenderedGlyphHeight(unsigned short codepoint, float fontSize)
{
    const float glyphScale = getGlyphScale(fontSize);
    return static_cast<float>(getFontData()->getFontData().glyphHeight) * glyphScale - 2.0f;
}

bool fuiBitmapFont::isBitmapFont()
{
    return true;
}

void fuiBitmapFont::preRender()
{
}

void fuiBitmapFont::registerFont()
{
    if (!m_registered)
    {
        gFui->installBitmapFont(this);
        m_registered = true;
    }

    gFui->setFontIndirection(m_name.c_str(), m_name.c_str());
}

int fuiBitmapFont::getFallbackFontTexture(float fontSize)
{
    return -1;
}

void fuiBitmapFont::loadGlyph(int codepoint, float fontSize)
{
    getFontData()->getGlyphId(static_cast<unsigned int>(codepoint));
}

bool fuiBitmapFont::isGlyphLoaded(int codepoint, float fontSize)
{
    getFontData()->getGlyphId(static_cast<unsigned int>(codepoint));
    return true;
}

bool fuiBitmapFont::hasGlyph(int codepoint)
{
    return getFontData()->getGlyphId(static_cast<unsigned int>(codepoint)) != 0;
}
