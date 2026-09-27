#include "game/client/render/Font.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/core/StringUtils.hpp"

Font::Font(Options*, const std::wstring& name, Textures* textures, bool,
           ResourceLocation* location, int characterWidth, int characterHeight,
           int textureWidth, int textureHeight, unsigned short* characterMap)
    : mWidths(0), mCharacters(0), mRandom(new Random()), mCharacterCount(characterWidth * characterHeight),
      mCharacterWidth(characterWidth), mCharacterHeight(characterHeight), mTextureWidth(textureWidth),
      mTextureHeight(textureHeight), mTextures(textures), mX(0.0f), mY(0.0f),
      mEnforceUnicodeSheet(false), mBidirectional(false), mTexture(0), mFontHeight(0),
      mLocation(location), mGlyphCount(characterMap == 0 ? 0 : mCharacterCount)
{
    mWidths = new float[mCharacterCount]();
    if (characterMap != 0)
    {
        for (int i = 0; i < mCharacterCount; ++i)
            mCharacterMap[characterMap[i]] = i;
    }
    mTexture = location->getTexture();
    BufferedImage* image = textures->readImage(static_cast<_TEXTURE_NAME>(mTexture), name);
    const unsigned int imageWidth = image->getWidth();
    const unsigned int imageHeight = image->getHeight();
    std::vector<int> pixels(imageWidth * imageHeight);
    image->getRGB(0, 0, imageWidth, static_cast<int>(imageHeight),
                  arrayWithLength<int>(pixels.data(), static_cast<unsigned int>(pixels.size())),
                  0, imageWidth, 0);
    for (int i = 0; i < mCharacterCount; ++i)
    {
        const int row = i / mCharacterWidth;
        const int col = i % mCharacterWidth;
        int glyphColumn = mTextureWidth - 1;
        if (mTextureWidth > 0)
        {
            bool found = false;
#if defined(RECON_NATIVE)
            const int glyphX = col * mTextureWidth;
            const int glyphY = row * mTextureHeight;
            const int availableWidth = glyphX < static_cast<int>(imageWidth)
                                           ? std::min(mTextureWidth, static_cast<int>(imageWidth) - glyphX)
                                           : 0;
            const int availableHeight = glyphY < static_cast<int>(imageHeight)
                                            ? std::min(mTextureHeight, static_cast<int>(imageHeight) - glyphY)
                                            : 0;
            for (int x = availableWidth - 1; x >= 0 && !found; --x)
#else
            for (int x = mTextureWidth - 1; x >= 0 && !found; --x)
#endif
            {
                for (int y = 0;
#if defined(RECON_NATIVE)
                     y < availableHeight;
#else
                     y < mTextureHeight;
#endif
                     ++y)
                {
                    const int pixelIndex = (row * mTextureHeight + y) * static_cast<int>(imageWidth) +
                                            (col * mTextureWidth + x);
                    if ((static_cast<unsigned int>(pixels[pixelIndex]) >> 24) != 0)
                    {
                        found = true;
                        glyphColumn = x;
                        break;
                    }
                }
                if (!found)
                    glyphColumn = x - 1;
            }
        }
        if (i == 0x20)
            glyphColumn = 2;
        mWidths[i] = static_cast<float>(glyphColumn + 2);
    }
    delete image;
    mCharacters = new Character[mCharacterCount];
    for (int i = 0; i < mCharacterCount; ++i)
        cacheCharacter(i);
}

bool Font::CharacterExists(wchar_t character)
{
    if (mGlyphCount != 0)
        return mCharacterMap.find(static_cast<int>(character)) != mCharacterMap.end();
    return character <= mCharacterWidth * mCharacterHeight;
}

int Font::MapCharacter(wchar_t character)
{
    if (mGlyphCount != 0 && character != L' ')
        return mCharacterMap[static_cast<int>(character)];
    return static_cast<int>(character);
}

std::wstring Font::sanitize(const std::wstring& text, bool trim)
{
    std::wstring result(text);
    std::wstring::size_type index = 0;
    while (index < result.size())
    {
        if (!CharacterExists(result[index]))
        {
            if (trim)
                result.erase(index, 1);
            else
            {
                result[index] = 0;
                ++index;
            }
        }
        else
        {
            result[index] = static_cast<wchar_t>(MapCharacter(result[index]));
            ++index;
        }
    }
    if (trim)
    {
        std::wstring::size_type first = result.find_first_not_of(L' ');
        if (first == std::wstring::npos)
            result = L"";
        else
            result = result.substr(first, result.find_last_not_of(L' ') - first + 1);
    }
    return result;
}

int Font::width(const std::wstring& text)
{
    const std::wstring value = sanitize(text, false);
    float result = 0.0f;
    for (std::wstring::size_type i = 0; i < value.size(); ++i)
    {
        if (value[i] == 0xA7)
        {
            ++i;
            continue;
        }
        result += mWidths[static_cast<unsigned short>(value[i])];
    }
    return static_cast<int>(result + 0.5f);
}

void Font::renderCharacter(wchar_t character, BufferBuilder* builder)
{
    const Character& glyph = mCharacters[static_cast<unsigned short>(character)];
    builder->tex(glyph.u0, glyph.v1);
    builder->vertexNoBounds(mX, mY + glyph.height, 0.0f);
    builder->tex(glyph.u1, glyph.v1);
    builder->vertexNoBounds(mX + glyph.width, mY + glyph.height, 0.0f);
    builder->tex(glyph.u1, glyph.v0);
    builder->vertexNoBounds(mX + glyph.width, mY, 0.0f);
    builder->tex(glyph.u0, glyph.v0);
    builder->vertexNoBounds(mX, mY, 0.0f);
    mX += glyph.advance;
}

unsigned int Font::draw(const std::wstring& text, bool shadow, bool unicode)
{
    bindTexture();
    const std::wstring value = sanitize(text, false);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    if (unicode)
        builder->begin();
    unsigned int color = 0;
    for (std::wstring::size_type i = 0; i < value.size(); ++i)
    {
        wchar_t character = value[i];
        if (character == 0xA7 && i + 1 < value.size())
        {
            wchar_t code = value[++i];
            int index = code >= L'0' && code <= L'9' ? code - L'0' : code >= L'a' && code <= L'f' ? code - L'a' + 10 : -1;
            if (index >= 0)
            {
                color = index + (unicode ? 16 : 0);
                const int rgb = mTexture;
                GlStateManager::color3f(1.0f, 1.0f, 1.0f);
                builder->color(rgb);
            }
            continue;
        }
        renderCharacter(character, builder);
    }
    if (unicode)
        builder->end();
    return 1;
}

unsigned int Font::draw(const std::wstring& text, int x, int y, int color, bool shadow, bool unicode)
{
    if (text.empty()) return 1;
    if (shadow) color = (color & 0xFCFCFC) >> 2 | (color & 0xFF000000);
    GlStateManager::color4f(((color >> 16) & 255) / 255.0f, ((color >> 8) & 255) / 255.0f,
                            (color & 255) / 255.0f, ((color >> 24) & 255) / 255.0f);
    mX = static_cast<float>(x);
    mY = static_cast<float>(y);
    return draw(text, shadow, unicode);
}

unsigned int Font::draw(const std::wstring& text, int x, int y, int color)
{
    return draw(text, x, y, color, false, true);
}

void Font::cacheCharacter(int character)
{
    const int columns = mCharacterWidth;
    const int rows = mCharacterHeight;
    const int column = character % columns;
    const int row = character / columns;
    Character& glyph = mCharacters[character];
    const float textureGlyphWidth = mWidths[character] - 0.01f;
    const float textureGlyphHeight = static_cast<float>(mTextureHeight) - 0.01f;
    if (mTextureWidth > 8)
        mWidths[character] *= 0.5f;
    glyph.width = mWidths[character] - 0.01f;
    glyph.height = std::min(7.99f, textureGlyphHeight);
    glyph.u0 = static_cast<float>(column * mTextureWidth) / (columns * mTextureWidth);
    glyph.u1 = (static_cast<float>(column * mTextureWidth) + textureGlyphWidth) / (columns * mTextureWidth);
    glyph.v0 = static_cast<float>(row * mTextureHeight) / (rows * mTextureHeight);
    glyph.v1 = (static_cast<float>(row * mTextureHeight) + textureGlyphHeight) / (rows * mTextureHeight);
    glyph.advance = mWidths[character];
}

void Font::drawShadow(const std::wstring& text, int x, int y, int color)
{
    draw(text, x + 1, y + 1, color, true, true);
    draw(text, x, y, color, false, true);
}

int Font::wordWrapHeight(const std::wstring& text, int wrapWidth)
{
    std::vector<std::wstring> lines = stringSplit(text, L'\n');
    if (lines.size() > 1)
    {
        int height = 0;
        for (std::vector<std::wstring>::const_iterator it = lines.begin(); it != lines.end(); ++it)
            height += wordWrapHeight(*it, wrapWidth);
        return height;
    }

    std::vector<std::wstring> words = stringSplit(text, L' ');
    int height = 0;
    std::vector<std::wstring>::size_type index = 0;
    while (index < words.size())
    {
        std::wstring line = words[index] + L" ";
        ++index;
        while (index < words.size() && width(line + words[index]) < wrapWidth)
        {
            line += words[index] + L" ";
            ++index;
        }
        if (width(line) > wrapWidth)
        {
            do
            {
                std::wstring::size_type fit = 0;
                while (width(line.substr(0, fit + 1)) <= wrapWidth)
                    ++fit;
                if (!trimString(line.substr(0, fit)).empty())
                    height += 8;
                line = line.substr(fit);
            } while (width(line) > wrapWidth);
        }
        if (!trimString(line).empty())
            height += 8;
    }
    if (height < 8)
        height += 8;
    return height;
}

void Font::drawWordWrapInternal(const std::wstring& text, int x, int y, int wrapWidth, int color, int height)
{
    drawWordWrapInternal(text, x, y, wrapWidth, color, false, height);
}

void Font::drawWordWrapInternal(const std::wstring& text, int x, int y, int wrapWidth, int color, bool unicode, int height)
{
    std::vector<std::wstring> lines = stringSplit(text, L'\n');
    for (std::vector<std::wstring>::const_iterator it = lines.begin(); it != lines.end() && y + 8 <= height; ++it)
    {
        std::wstring line = *it;
        while (width(line) > wrapWidth)
        {
            std::wstring::size_type split = line.find_last_of(L' ', static_cast<std::wstring::size_type>(wrapWidth));
            if (split == std::wstring::npos) split = line.size() > 1 ? 1 : line.size();
            draw(line.substr(0, split), x, y, color, false, unicode);
            line.erase(0, split + (split < line.size() ? 1 : 0));
            y += 8;
        }
        draw(trimString(line), x, y, color, false, unicode);
        y += 8;
    }
}

unsigned int Font::drawNoBeginEndBuilder(const std::wstring& text, int x, int y, int color)
{
    return draw(text, x, y, color, false, false);
}

void Font::bindTexture()
{
    mTextures->bindTexture(mLocation);
}

void Font::drawWordWrap(const std::wstring& text, int x, int y, int widthValue, int color)
{
    drawWordWrapInternal(text, x, y, widthValue, color, wordWrapHeight(text, widthValue));
}
