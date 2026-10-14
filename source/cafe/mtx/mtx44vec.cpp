#include <cafe/mtx.h>

#include <math.h>

#ifdef __ghs__
#include <ppc_ps.h>
#endif

extern "C"
{
void ASM_MTX44MultVec(const Mtx44 m, const Vec* src, Vec* dst)
{
#ifdef __ghs__
    f32x2 xy = __PSQ_L(&src->x, 0, 0);
    f32x2 z1 = __PSQ_L(&src->z, 1, 0);
    f32x2 w;
    f32x2 vx;
    f32x2 vy;
    f32x2 vz;

    w = __PS_MADD(z1, __PSQ_L(&m[3][2], 0, 0), __PS_MUL(xy, __PSQ_L(&m[3][0], 0, 0)));
    w = __PS_SUM0(w, w, w);
    w = __PS_DIV(__PS_MERGE11(z1, z1), __PS_MERGE00(w, w));
    vx = __PS_MADD(z1, __PSQ_L(&m[0][2], 0, 0), __PS_MUL(xy, __PSQ_L(&m[0][0], 0, 0)));
    vy = __PS_MADD(z1, __PSQ_L(&m[1][2], 0, 0), __PS_MUL(xy, __PSQ_L(&m[1][0], 0, 0)));
    vz = __PS_MADD(z1, __PSQ_L(&m[2][2], 0, 0), __PS_MUL(xy, __PSQ_L(&m[2][0], 0, 0)));
    vx = __PS_SUM1(vy, __PS_SUM0(vx, vx, vx), vy);
    vz = __PS_SUM0(vz, vz, vz);
    __PSQ_ST(&dst->x, __PS_MUL(vx, w), 0, 0);
    __PSQ_ST(&dst->z, __PS_MUL(vz, w), 1, 0);
#else
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
#endif
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
#ifdef __ghs__
    f32x2 xy = __PSQ_L(&src->x, 0, 0);
    f32x2 z1 = __PSQ_L(&src->z, 1, 0);
    f32x2 vx = __PS_MUL(__PSQ_L(&m[0][0], 0, 0), xy);
    f32x2 vy = __PS_MUL(__PSQ_L(&m[1][0], 0, 0), xy);
    f32x2 vz = __PS_MUL(__PSQ_L(&m[2][0], 0, 0), xy);

    vx = __PS_SUM0(vx, vx, vx);
    vy = __PS_SUM0(vy, vy, vy);
    vz = __PS_SUM0(vz, vz, vz);
    __PSQ_ST(&dst->x, __PS_MADD(__PSQ_L(&m[0][2], 0, 0), z1, vx), 1, 0);
    __PSQ_ST(&dst->y, __PS_MADD(__PSQ_L(&m[1][2], 0, 0), z1, vy), 1, 0);
    __PSQ_ST(&dst->z, __PS_MADD(__PSQ_L(&m[2][2], 0, 0), z1, vz), 1, 0);
#else
    Vec vTmp;

    vTmp.x = (m[0][2] * src->z) + ((m[0][0] * src->x) + (m[0][1] * src->y));
    vTmp.y = (m[1][2] * src->z) + ((m[1][0] * src->x) + (m[1][1] * src->y));
    vTmp.z = (m[2][2] * src->z) + ((m[2][0] * src->x) + (m[2][1] * src->y));
    dst->x = vTmp.x;
    dst->y = vTmp.y;
    dst->z = vTmp.z;
#endif
}
}
