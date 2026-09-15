#include "game/core/Cube.hpp"

#include "game/core/BufferBuilder.hpp"
#include "game/core/Mth.hpp"
#include "vendor/_Polygon.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/util/Vec3.hpp"

Cube::Cube(ModelPart* parent, int texU, int texV, float x, float y, float z,
           int width, int height, int depth, float inflate, bool mirror,
           int faceMask, bool flip, float xRot, float yRot, float zRot)
    : mVertices(NULL), mVertexCount(0), mPolygons(NULL), mPolygonCount(0),
      mX0(0.0f), mY0(0.0f), mZ0(0.0f), mX1(0.0f), mY1(0.0f), mZ1(0.0f),
      mTexU(texU), mTexV(texV), mId()
{
    mX0 = x;
    mY0 = y;
    mZ0 = z;
    mX1 = x + (float)width;
    mY1 = y + (float)height;
    mZ1 = z + (float)depth;
    _init(parent, texU, texV, x, y, z, width, height, depth, inflate, mirror, faceMask, flip);
    rotate(xRot, yRot, zRot);
}

arrayWithLength<Vertex*> Cube::VertexArray4(Vertex* a, Vertex* b, Vertex* c, Vertex* d)
{
    arrayWithLength<Vertex*> result(4, false);
    result.data[0] = a;
    result.data[1] = b;
    result.data[2] = c;
    result.data[3] = d;
    return result;
}

void Cube::_init(ModelPart* parent, int texU, int texV, float x, float y, float z,
                 int width, int height, int depth, float inflate, bool mirror,
                 int faceMask, bool flip)
{
    mVertices = new Vertex*[8];
    mVertexCount = 8;
    mPolygons = new _Polygon*[6];
    mPolygonCount = 0;

    float x0 = x - inflate;
    float y0 = y - inflate;
    float z0 = z - inflate;
    float x1 = x + (float)width + inflate;
    float y1 = y + (float)height + inflate;
    float z1 = z + (float)depth + inflate;
    const float epsilon = 0.0009765625f;
    if (x0 == x1) x1 += epsilon;
    if (y0 == y1) y1 += epsilon;
    if (z0 == z1) z1 += epsilon;

    if (mirror)
    {
        float t = x1;
        x1 = x0;
        x0 = t;
    }

    mVertices[0] = new Vertex(x0, y0, z0, 0.0f, 0.0f);
    mVertices[1] = new Vertex(x1, y0, z0, 0.5f, 0.0f);
    mVertices[2] = new Vertex(x1, y1, z0, 0.5f, 0.5f);
    mVertices[3] = new Vertex(x0, y1, z0, 0.0f, 0.5f);
    mVertices[4] = new Vertex(x0, y0, z1, 0.0f, 0.0f);
    mVertices[5] = new Vertex(x1, y0, z1, 0.5f, 0.0f);
    mVertices[6] = new Vertex(x1, y1, z1, 0.5f, 0.5f);
    mVertices[7] = new Vertex(x0, y1, z1, 0.0f, 0.5f);

    float u = (float)texU;
    float v = (float)texV;
    float w = (float)width;
    float h = (float)height;
    float d = (float)depth;
    if (faceMask & 1) mPolygons[mPolygonCount++] = new _Polygon(VertexArray4(mVertices[5], mVertices[1], mVertices[2], mVertices[6]), (int)(u + d + w), (int)(v + d), (int)(u + d + w + w), (int)(v + d + h), w, h);
    if (faceMask & 2) mPolygons[mPolygonCount++] = new _Polygon(VertexArray4(mVertices[0], mVertices[4], mVertices[7], mVertices[3]), (int)u, (int)(v + d), (int)(u + d), (int)(v + d + h), w, h);
    if (faceMask & 4) mPolygons[mPolygonCount++] = new _Polygon(VertexArray4(mVertices[5], mVertices[4], mVertices[0], mVertices[1]), (int)(u + d), (int)v, (int)(u + d + w), (int)(v + d), d, h);
    if (faceMask & 8)
    {
        if (!flip)
            mPolygons[mPolygonCount++] = new _Polygon(VertexArray4(mVertices[2], mVertices[3], mVertices[7], mVertices[6]), (int)(u + d + w), (int)(v + d), (int)(u + d + w + w), (int)v, d, h);
        else
            mPolygons[mPolygonCount++] = new _Polygon(VertexArray4(mVertices[2], mVertices[3], mVertices[7], mVertices[6]), (int)(u + d + w), (int)v, (int)(u + d + w + w), (int)(v + d), d, h);
    }
    if (faceMask & 16) mPolygons[mPolygonCount++] = new _Polygon(VertexArray4(mVertices[1], mVertices[0], mVertices[3], mVertices[2]), (int)(u + d), (int)(v + d), (int)(u + d + w), (int)(v + d + h), w, h);
    if (faceMask & 32) mPolygons[mPolygonCount++] = new _Polygon(VertexArray4(mVertices[4], mVertices[5], mVertices[6], mVertices[7]), (int)(u + d + w), (int)(v + d), (int)(u + d + w + w), (int)(v + d + h), w, h);
    if (mirror)
    {
        for (int i = 0; i < mPolygonCount; ++i) mPolygons[i]->mirror();
    }
}

void Cube::rotate_axis(float angle, double& x, double& y) const
{
    double oldX = x;
    double oldY = y;
    double c = Mth::cos(angle);
    double s = Mth::sin(angle);
    x = oldX * c - oldY * s;
    y = oldX * s + oldY * c;
}

Cube* Cube::rotate(float x, float y, float z)
{
    const float zero = 0.0f;
    if ((x > zero || y > zero) && mVertexCount != 0)
    {
        for (int i = 0; i < mVertexCount; ++i)
        {
            if (x != zero) rotate_axis(x, mVertices[i]->pos->y, mVertices[i]->pos->z);
            if (y != zero) rotate_axis(y, mVertices[i]->pos->x, mVertices[i]->pos->z);
            if (z != zero) rotate_axis(z, mVertices[i]->pos->x, mVertices[i]->pos->y);
        }
    }
    return this;
}

void Cube::render(BufferBuilder* buffer, float scale)
{
    buffer->begin();
    for (int i = 0; i < mPolygonCount; ++i) mPolygons[i]->render(buffer, scale);
    buffer->end();
}

void Cube::renderSkinned(BufferBuilder* buffer, float scale, int boneIndex)
{
    for (int i = 0; i < mPolygonCount; ++i) mPolygons[i]->renderSkinned(buffer, scale, boneIndex);
}

Cube* Cube::setId(const std::wstring& id)
{
    mId.assign(id, 0, 0xffffffffu);
    return this;
}
