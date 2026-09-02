#include "game/client/render/TextureUVCoordinateSet.hpp"

TextureUVCoordinateSet::TextureUVCoordinateSet(float u0,
                                               float v0,
                                               float u1,
                                               float v1,
                                               unsigned short textureWidth,
                                               unsigned short textureHeight,
                                               float scale)
    : scale(scale)
    , u0(u0)
    , v0(v0)
    , u1(u1)
    , v1(v1)
    , textureWidth(textureWidth)
    , textureHeight(textureHeight)
{
}

TextureUVCoordinateSet::TextureUVCoordinateSet()
    : scale(1.0f)
    , u0(0.0f)
    , v0(0.0f)
    , u1(0.0f)
    , v1(0.0f)
    , textureWidth(1)
    , textureHeight(1)
{
}

TextureUVCoordinateSet TextureUVCoordinateSet::fromOldSystem(int index)
{
    float column = (float)(index % 16);
    float row = (float)(index / 16);

    return TextureUVCoordinateSet(column * 0.0625f,
                                  row * 0.0625f,
                                  (column + 1.0f) * 0.0625f,
                                  (row + 1.0f) * 0.0625f,
                                  16,
                                  16);
}
