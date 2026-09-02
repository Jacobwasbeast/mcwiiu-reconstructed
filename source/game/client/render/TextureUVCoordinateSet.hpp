#ifndef GAME_CLIENT_RENDER_TEXTUREUVCOORDINATESET_HPP
#define GAME_CLIENT_RENDER_TEXTUREUVCOORDINATESET_HPP

class TextureUVCoordinateSet
{
public:
    float scale;
    float u0;
    float v0;
    float u1;
    float v1;
    unsigned short textureWidth;
    unsigned short textureHeight;

    TextureUVCoordinateSet(float u0,
                           float v0,
                           float u1,
                           float v1,
                           unsigned short textureWidth,
                           unsigned short textureHeight,
                           float scale = 1.0f);
    TextureUVCoordinateSet();

    float width() const;
    float height() const;
    float offsetWidth(float offset, float width) const;
    float offsetHeight(float offset, float height) const;
    float asPixelU1() const;
    float asPixelU0() const;
    float pixelWidth() const;
    float offsetWidthPixel(float offset, float width) const;
    float asPixelV1() const;
    float asPixelV0() const;
    float pixelHeight() const;
    float offsetHeightPixel(float offset, float height) const;
    float inverseOffsetWidth(float offset, float width) const;
    float inverseOffsetHeight(float offset, float height) const;
    TextureUVCoordinateSet getFlippedHorizontal() const;
    TextureUVCoordinateSet getFlippedVertically() const;
    TextureUVCoordinateSet offset(float amount) const;
    float pixelSizeU() const;
    float pixelSizeV() const;
    TextureUVCoordinateSet subTexture(float x, float y, int width, int height) const;
    TextureUVCoordinateSet topLeftPixel() const;
    int hashCode() const;
    bool operator==(const TextureUVCoordinateSet& other) const;

    static TextureUVCoordinateSet fromOldSystem(int index);
};

#endif
