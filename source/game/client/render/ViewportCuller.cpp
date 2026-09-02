#include "game/client/render/ViewportCuller.hpp"

#include "game/util/AABB.hpp"

bool ViewportCuller::Face::inFront(double x, double y, double z, double radius)
{
    return mNormalX * x + mNormalY * y + mNormalZ * z > mDistance - radius;
}

bool ViewportCuller::Face::inFront(double x0, double y0, double z0, double x1, double y1, double z1)
{
    double nx0 = mNormalX * x0;
    double ny0 = mNormalY * y0;
    double nz0 = mNormalZ * z0;

    if (nx0 + ny0 + nz0 > mDistance)
    {
        return true;
    }

    double nx1 = mNormalX * x1;

    if (nx1 + ny0 + nz0 > mDistance)
    {
        return true;
    }

    double ny1 = mNormalY * y1;

    if (nx0 + ny1 + nz0 > mDistance)
    {
        return true;
    }

    if (nx1 + ny1 + nz0 > mDistance)
    {
        return true;
    }

    double nz1 = mNormalZ * z1;

    if (nx0 + ny0 + nz1 > mDistance)
    {
        return true;
    }

    if (nx1 + ny0 + nz1 > mDistance)
    {
        return true;
    }

    if (nx0 + ny1 + nz1 > mDistance)
    {
        return true;
    }

    if (nx1 + ny1 + nz1 > mDistance)
    {
        return true;
    }

    return false;
}

bool ViewportCuller::Face::fullyInFront(double x0, double y0, double z0, double x1, double y1, double z1)
{
    double nx0 = mNormalX * x0;
    double ny0 = mNormalY * y0;
    double nz0 = mNormalZ * z0;

    if (nx0 + ny0 + nz0 < mDistance)
    {
        return false;
    }

    double nx1 = mNormalX * x1;

    if (nx1 + ny0 + nz0 < mDistance)
    {
        return false;
    }

    double ny1 = mNormalY * y1;

    if (nx0 + ny1 + nz0 < mDistance)
    {
        return false;
    }

    if (nx1 + ny1 + nz0 < mDistance)
    {
        return false;
    }

    double nz1 = mNormalZ * z1;

    if (nx0 + ny0 + nz1 < mDistance)
    {
        return false;
    }

    if (nx1 + ny0 + nz1 < mDistance)
    {
        return false;
    }

    if (nx0 + ny1 + nz1 < mDistance)
    {
        return false;
    }

    if (nx1 + ny1 + nz1 < mDistance)
    {
        return false;
    }

    return true;
}

ViewportCuller::~ViewportCuller()
{
}

bool ViewportCuller::isVisible(AABB* boundingBox)
{
    return cubeInFrustum(boundingBox->mMinX,
                         boundingBox->mMinY,
                         boundingBox->mMinZ,
                         boundingBox->mMaxX,
                         boundingBox->mMaxY,
                         boundingBox->mMaxZ);
}

double ViewportCuller::_max(double x, double y, double z)
{
    if (x > y)
    {
        return x > z ? z : x;
    }

    return z > y ? z : y;
}

bool ViewportCuller::cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    x0 -= mXOff;
    y0 -= mYOff;
    z0 -= mZOff;
    x1 -= mXOff;
    y1 -= mYOff;
    z1 -= mZOff;

    double halfX = (x1 - x0) * 0.5;
    double halfY = (y1 - y0) * 0.5;
    double halfZ = (z1 - z0) * 0.5;

    double centerX = x0 + halfX;
    double centerY = y0 + halfY;
    double centerZ = z0 + halfZ;

    double radius = _max(halfX, halfY, halfZ) * 1.5;

    for (int i = 0; i < 6; ++i)
    {
        if (!mFaces[i].inFront(centerX, centerY, centerZ, radius))
        {
            return false;
        }
    }

    for (int i = 0; i < 6; ++i)
    {
        if (!mFaces[i].inFront(x0, y0, z0, x1, y1, z1))
        {
            return false;
        }
    }

    return true;
}

bool ViewportCuller::cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    x0 -= mXOff;
    y0 -= mYOff;
    z0 -= mZOff;
    x1 -= mXOff;
    y1 -= mYOff;
    z1 -= mZOff;

    double halfX = (x1 - x0) * 0.5;
    double halfY = (y1 - y0) * 0.5;
    double halfZ = (z1 - z0) * 0.5;

    double centerX = x0 + halfX;
    double centerY = y0 + halfY;
    double centerZ = z0 + halfZ;

    double radius = _max(halfX, halfY, halfZ) * 1.5;

    for (int i = 0; i < 6; ++i)
    {
        if (!mFaces[i].inFront(centerX, centerY, centerZ, radius))
        {
            return false;
        }
    }

    for (int i = 0; i < 6; ++i)
    {
        if (!mFaces[i].fullyInFront(x0, y0, z0, x1, y1, z1))
        {
            return false;
        }
    }

    return true;
}

void ViewportCuller::prepare(double x, double y, double z)
{
    mXOff = x;
    mYOff = y;
    mZOff = z;
}
