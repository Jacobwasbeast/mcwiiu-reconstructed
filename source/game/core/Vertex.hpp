#ifndef GAME_CORE_VERTEX_HPP
#define GAME_CORE_VERTEX_HPP

class Vec3;

class Vertex
{
public:
    Vec3* pos;
    float u;
    float v;

    Vertex(float x, float y, float z, float u, float v);
    Vertex(Vec3* pos, float u, float v);
    Vertex(Vertex* vertex, float u, float v);
    ~Vertex();

    Vertex* remap(float u, float v);
};

#endif
