#ifndef GAME_CLIENT_RENDER_FONT_HPP
#define GAME_CLIENT_RENDER_FONT_HPP

#include <map>
#include <string>

#include "game/util/Random.hpp"

class BufferBuilder;
class Options;
class ResourceLocation;
class Textures;

class Font : public Random
{
private:
    struct Character
    {
        float width;
        float height;
        float u0;
        float u1;
        float v0;
        float v1;
        float advance;
    };

    float* mWidths;
    Character* mCharacters;
    Random* mRandom;
    std::map<int, int> mCharacterMap;
    int mCharacterCount;
    int mCharacterWidth;
    int mCharacterHeight;
    int mTextureWidth;
    int mTextureHeight;
    Textures* mTextures;
    float mX;
    float mY;
    bool mEnforceUnicodeSheet;
    bool mBidirectional;
    int mTexture;
    int mFontHeight;
    ResourceLocation* mLocation;
    int mGlyphCount;

public:
    Font(Options* options, const std::wstring& name, Textures* textures, bool anaglyph,
         ResourceLocation* location, int characterWidth, int characterHeight,
         int textureWidth, int textureHeight, unsigned short* characterMap);

    bool CharacterExists(wchar_t character);
    int MapCharacter(wchar_t character);
    std::wstring sanitize(const std::wstring& text, bool trim);
    int width(const std::wstring& text);
    void renderCharacter(wchar_t character, BufferBuilder* builder);
    unsigned int draw(const std::wstring& text, bool shadow, bool unicode);
    unsigned int draw(const std::wstring& text, int x, int y, int color, bool shadow, bool unicode);
    unsigned int draw(const std::wstring& text, int x, int y, int color);
    void cacheCharacter(int character);
    void drawShadow(const std::wstring& text, int x, int y, int color);
    int wordWrapHeight(const std::wstring& text, int width);
    void drawWordWrapInternal(const std::wstring& text, int x, int y, int width, int color, int height);
    void drawWordWrapInternal(const std::wstring& text, int x, int y, int width, int color, bool unicode, int height);
    unsigned int drawNoBeginEndBuilder(const std::wstring& text, int x, int y, int color);
    void bindTexture();
    void drawWordWrap(const std::wstring& text, int x, int y, int width, int color);

    ~Font();
    void drawShadowWordWrap(const std::wstring& text, int x, int y, int width, int color);
    std::wstring reorderBidi(const std::wstring& text);
    void drawWordWrap(const std::wstring& text, int x, int y, int width, bool unicode, int color);
    void setEnforceUnicodeSheet(bool enforce);
    void setBidirectional(bool bidirectional);
    static bool AllCharactersValid(const std::wstring& text);
};

#endif
