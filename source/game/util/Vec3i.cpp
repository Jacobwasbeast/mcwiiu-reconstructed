#include "game/util/Vec3i.hpp"

#include <math.h>

Vec3i::Vec3i(double x, double y, double z)
{
    int flooredX = (int)x;
    this->x = x < (double)flooredX ? flooredX - 1 : flooredX;

    int flooredY = (int)y;
    this->y = y < (double)flooredY ? flooredY - 1 : flooredY;

    int flooredZ = (int)z;
    this->z = z < (double)flooredZ ? flooredZ - 1 : flooredZ;
}

Vec3i Vec3i::offset(int dx, int dy, int dz) const
{
    return Vec3i(x + dx, y + dy, z + dz);
}

Vec3i Vec3i::offset(const Vec3i& other) const
{
    return offset(other.x, other.y, other.z);
}

Vec3i Vec3i::subtract(const Vec3i& other) const
{
    return offset(-other.x, -other.y, -other.z);
}

Vec3i Vec3i::multiply(int scale) const
{
    return Vec3i(x * scale, y * scale, z * scale);
}

Vec3i Vec3i::cross(const Vec3i& other) const
{
    return Vec3i(y * other.z - z * other.y,
                 z * other.x - x * other.z,
                 x * other.y - y * other.x);
}

double Vec3i::dist(int x, int y, int z) const
{
    double dx = (double)(this->x - x);
    double dy = (double)(this->y - y);
    double dz = (double)(this->z - z);

    return sqrt(dx * dx + dy * dy + dz * dz);
}

double Vec3i::dist(const Vec3i& other) const
{
    return dist(other.x, other.y, other.z);
}

double Vec3i::distSqr(double x, double y, double z) const
{
    double dx = (double)this->x - x;
    double dy = (double)this->y - y;
    double dz = (double)this->z - z;

    return dx * dx + dy * dy + dz * dz;
}

double Vec3i::distSqrToCenter(double x, double y, double z) const
{
    double dx = (double)this->x + 0.5 - x;
    double dy = (double)this->y + 0.5 - y;
    double dz = (double)this->z + 0.5 - z;

    return dx * dx + dy * dy + dz * dz;
}

double Vec3i::distSqr(const Vec3i& other) const
{
    return distSqr(other.x, other.y, other.z);
}
