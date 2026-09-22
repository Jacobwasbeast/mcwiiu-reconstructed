#include "vendor/_Polygon.hpp"

#include "game/core/BufferBuilder.hpp"
#include "game/util/Vec3.hpp"

_Polygon::_Polygon(arrayWithLength<Vertex*> vertices)
{
    mVertices = NULL;
    mVertexCount = 0;
    _init(vertices);
}

_Polygon::_Polygon(arrayWithLength<Vertex*> vertices, int u0, int v0, int u1, int v1, float width, float height)
{
    mVertices = NULL;
    mVertexCount = 0;
    _init(vertices);

    float uInset = -0.01f;
    float vInset = -0.01f;
    if (u0 < u1)
    {
        uInset = 0.01f;
    }
    if (v0 < v1)
    {
        vInset = 0.01f;
    }

    float vMin = (float)v0 / height + vInset / height;
    float uMax = (float)u1 / width - uInset / width;
    mVertices[0] = mVertices[0]->remap(uMax, vMin);
    float uMin = (float)u0 / width + uInset / width;
    mVertices[1] = mVertices[1]->remap(uMin, vMin);
    float vMax = (float)v1 / height - vInset / height;
    mVertices[2] = mVertices[2]->remap(uMin, vMax);
    mVertices[3] = mVertices[3]->remap(uMax, vMax);
}

_Polygon::_Polygon(arrayWithLength<Vertex*> vertices, float u0, float v0, float u1, float v1)
{
    mVertices = NULL;
    mVertexCount = 0;
    _init(vertices);

    mVertices[0] = mVertices[0]->remap(u1, v0);
    mVertices[1] = mVertices[1]->remap(u0, v0);
    mVertices[2] = mVertices[2]->remap(u0, v1);
    mVertices[3] = mVertices[3]->remap(u1, v1);
}

_Polygon::~_Polygon()
{
    delete[] mVertices;
}

void _Polygon::_init(arrayWithLength<Vertex*> vertices)
{
    mFlipNormal = false;
    mVertices = vertices.data;
    mVertexCount = vertices.length;
    mVertexCount2 = vertices.length;
}

void _Polygon::mirror()
{
    unsigned int count = (unsigned int)mVertexCount;
    Vertex** mirrored = new Vertex*[count];
    for (unsigned int i = 0; i < count; ++i)
    {
        mirrored[i] = mVertices[count - i - 1];
    }
    delete[] mVertices;
    mVertices = mirrored;
    mVertexCount = count;
}

void _Polygon::render(BufferBuilder* buffer, float scale)
{
    Vec3* edge0 = mVertices[0]->position->vectorTo(mVertices[1]->position);
    Vec3* edge1 = mVertices[2]->position->vectorTo(mVertices[3]->position);
    Vec3* normalVector = edge1->cross(edge0)->normalize();
    if (mFlipNormal)
    {
        buffer->normal((float)-normalVector->x, (float)-normalVector->y, (float)-normalVector->z);
    }
    else
    {
        buffer->normal((float)normalVector->x, (float)normalVector->y, (float)normalVector->z);
    }

    for (int i = 0; i < 4; ++i)
    {
        Vertex* vertex = mVertices[i];
        buffer->vertexUV((float)(vertex->position->x * scale),
                         (float)(vertex->position->y * scale),
                         (float)(vertex->position->z * scale), vertex->textureU, vertex->textureV);
    }
}

void _Polygon::renderSkinned(BufferBuilder* buffer, float scale, int boneIndex)
{
    Vec3* edge0 = mVertices[0]->position->vectorTo(mVertices[1]->position);
    Vec3* edge1 = mVertices[2]->position->vectorTo(mVertices[3]->position);
    Vec3* normalVector = edge1->cross(edge0)->normalize();
    if (mFlipNormal)
    {
        buffer->normal((float)-normalVector->x, (float)-normalVector->y, (float)-normalVector->z);
    }
    else
    {
        buffer->normal((float)normalVector->x, (float)normalVector->y, (float)normalVector->z);
    }

    for (int i = 0; i < 4; ++i)
    {
        Vertex* vertex = mVertices[i];
        buffer->vertexUVSkinned((float)(vertex->position->x * scale),
                                (float)(vertex->position->y * scale),
                                (float)(vertex->position->z * scale), vertex->textureU, vertex->textureV, boneIndex);
    }
}

void _Polygon::flipNormal()
{
    mFlipNormal = !mFlipNormal;
}
