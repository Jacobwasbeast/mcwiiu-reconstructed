#ifndef GAME_CORE_VECTOR3F_HPP
#define GAME_CORE_VECTOR3F_HPP

class Vector3f
{
public:
    float x;
    float y;
    float z;

    Vector3f()
        : x(0.0f)
        , y(0.0f)
        , z(0.0f)
    {
    }

    Vector3f(float x, float y, float z)
        : x(x)
        , y(y)
        , z(z)
    {
    }
};

#endif
