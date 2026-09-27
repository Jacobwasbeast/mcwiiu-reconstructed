#include <cafe/mtx.h>

#include <math.h>

extern "C"
{
void ASM_MTX44MultVec(const Mtx44 m, const Vec* src, Vec* dst)
{
    Vec vTmp;
    f32 w;

    vTmp.x = m[0][0] * src->x + m[0][1] * src->y + m[0][2] * src->z + m[0][3];
    vTmp.y = m[1][0] * src->x + m[1][1] * src->y + m[1][2] * src->z + m[1][3];
    vTmp.z = m[2][0] * src->x + m[2][1] * src->y + m[2][2] * src->z + m[2][3];
    w = m[3][0] * src->x + m[3][1] * src->y + m[3][2] * src->z + m[3][3];
    w = 1.0f / w;

    dst->x = vTmp.x * w;
    dst->y = vTmp.y * w;
    dst->z = vTmp.z * w;
}

void ASM_MTX44MultVecArray(const Mtx44 m, const Vec* srcBase, Vec* dstBase, u32 count)
{
    u32 i;
    Vec vTmp;
    f32 w;

    for (i = 0; i < count; i++)
    {
        vTmp.x = m[0][0] * srcBase->x + m[0][1] * srcBase->y + m[0][2] * srcBase->z + m[0][3];
        vTmp.y = m[1][0] * srcBase->x + m[1][1] * srcBase->y + m[1][2] * srcBase->z + m[1][3];
        vTmp.z = m[2][0] * srcBase->x + m[2][1] * srcBase->y + m[2][2] * srcBase->z + m[2][3];
        w = m[3][0] * srcBase->x + m[3][1] * srcBase->y + m[3][2] * srcBase->z + m[3][3];
        w = 1.0f / w;
        dstBase->x = vTmp.x * w;
        dstBase->y = vTmp.y * w;
        dstBase->z = vTmp.z * w;
        srcBase++;
        dstBase++;
    }
}

void ASM_MTX44MultVecArraySR(const Mtx44 m, const Vec* srcBase, Vec* dstBase, u32 count)
{
    u32 i;
    Vec vTmp;

    for (i = 0; i < count; i++)
    {
        vTmp.x = (m[0][2] * srcBase->z) + ((m[0][0] * srcBase->x) + (m[0][1] * srcBase->y));
        vTmp.y = (m[1][2] * srcBase->z) + ((m[1][0] * srcBase->x) + (m[1][1] * srcBase->y));
        vTmp.z = (m[2][2] * srcBase->z) + ((m[2][0] * srcBase->x) + (m[2][1] * srcBase->y));
        dstBase->x = vTmp.x;
        dstBase->y = vTmp.y;
        dstBase->z = vTmp.z;
        srcBase++;
        dstBase++;
    }
}

void ASM_MTX44MultVecSR(const Mtx44 m, const Vec* src, Vec* dst)
{
    Vec vTmp;

    vTmp.x = (m[0][2] * src->z) + ((m[0][0] * src->x) + (m[0][1] * src->y));
    vTmp.y = (m[1][2] * src->z) + ((m[1][0] * src->x) + (m[1][1] * src->y));
    vTmp.z = (m[2][2] * src->z) + ((m[2][0] * src->x) + (m[2][1] * src->y));
    dst->x = vTmp.x;
    dst->y = vTmp.y;
    dst->z = vTmp.z;
}
}
