#include "game/core/FrustumCuller.hpp"

#include "game/core/Frustum.hpp"
#include "game/core/FrustumData.hpp"
#include "game/util/AABB.hpp"

FrustumCuller::FrustumCuller(FrustumData* frustumData)
{
    if (frustumData == NULL)
    {
        frustumData = Frustum::getFrustum();
    }

    this->frustumData = frustumData;
}

FrustumCuller::~FrustumCuller()
{
}

bool FrustumCuller::isVisible(AABB* boundingBox)
{
    return cubeInFrustum(boundingBox->mMinX, boundingBox->mMinY, boundingBox->mMinZ,
                         boundingBox->mMaxX, boundingBox->mMaxY, boundingBox->mMaxZ);
}

bool FrustumCuller::cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    x0 -= xOffset;
    y0 -= yOffset;
    z0 -= zOffset;
    x1 -= xOffset;
    y1 -= yOffset;
    z1 -= zOffset;

    for (int i = 0; i < 6; i++)
    {
        float* plane = frustumData->frustum[i];

        if (frustumData->discriminant(plane, x0, y0, z0) <= 0.0 &&
            frustumData->discriminant(plane, x1, y0, z0) <= 0.0 &&
            frustumData->discriminant(plane, x0, y1, z0) <= 0.0 &&
            frustumData->discriminant(plane, x1, y1, z0) <= 0.0 &&
            frustumData->discriminant(plane, x0, y0, z1) <= 0.0 &&
            frustumData->discriminant(plane, x1, y0, z1) <= 0.0 &&
            frustumData->discriminant(plane, x0, y1, z1) <= 0.0 &&
            frustumData->discriminant(plane, x1, y1, z1) <= 0.0)
        {
            return false;
        }
    }

    return true;
}

bool FrustumCuller::cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    x0 -= xOffset;
    y0 -= yOffset;
    z0 -= zOffset;
    x1 -= xOffset;
    y1 -= yOffset;
    z1 -= zOffset;

    for (int i = 0; i < 6; i++)
    {
        float* plane = frustumData->frustum[i];

        if (frustumData->discriminant(plane, x0, y0, z0) <= 0.0 ||
            frustumData->discriminant(plane, x1, y0, z0) <= 0.0 ||
            frustumData->discriminant(plane, x0, y1, z0) <= 0.0 ||
            frustumData->discriminant(plane, x1, y1, z0) <= 0.0 ||
            frustumData->discriminant(plane, x0, y0, z1) <= 0.0 ||
            frustumData->discriminant(plane, x1, y0, z1) <= 0.0 ||
            frustumData->discriminant(plane, x0, y1, z1) <= 0.0 ||
            frustumData->discriminant(plane, x1, y1, z1) <= 0.0)
        {
            return false;
        }
    }

    return true;
}

void FrustumCuller::prepare(double x, double y, double z)
{
    xOffset = x;
    yOffset = y;
    zOffset = z;
}
