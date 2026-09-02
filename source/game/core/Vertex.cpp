#include "game/core/Vertex.hpp"

#include "game/util/Vec3.hpp"

Vertex::Vertex(float x, float y, float z, float u, float v)
{
    this->pos = Vec3::newPermanent(x, y, z);
    this->u = u;
    this->v = v;
}

Vertex::Vertex(Vertex* vertex, float u, float v)
{
    this->pos = vertex->pos;
    this->u = u;
    this->v = v;
}

Vertex* Vertex::remap(float u, float v)
{
    return new Vertex(this, u, v);
}
