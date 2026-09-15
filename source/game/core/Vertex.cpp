#include "game/core/Vertex.hpp"

#include "game/util/Vec3.hpp"

Vertex::Vertex(float x, float y, float z, float u, float v)
    : position(Vec3::newPermanent(x, y, z)), textureU(u), textureV(v)
{
}

Vertex::Vertex(Vec3* source, float u, float v)
    : position(Vec3::newPermanent(source)), textureU(u), textureV(v)
{
}

void Vertex::remap(float u, float v)
{
    position = Vec3::newPermanent(position);
    textureU = u;
    textureV = v;
}
