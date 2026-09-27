#include <cafe/mtx.h>

#include <math.h>

extern "C"
{
void ASM_VECAdd(const Vec* a, const Vec* b, Vec* ab)
{
    ab->x = a->x + b->x;
    ab->y = a->y + b->y;
    ab->z = a->z + b->z;
}

void ASM_VECCrossProduct(const Vec* a, const Vec* b, Vec* axb)
{
    Vec vTmp;

    vTmp.x = (a->y * b->z) - (a->z * b->y);
    vTmp.y = (a->z * b->x) - (a->x * b->z);
    vTmp.z = (a->x * b->y) - (a->y * b->x);
    axb->x = vTmp.x;
    axb->y = vTmp.y;
    axb->z = vTmp.z;
}

f32 ASM_VECDistance(const Vec* a, const Vec* b)
{
    return sqrtf(ASM_VECSquareDistance(a, b));
}

f32 ASM_VECDotProduct(const Vec* a, const Vec* b)
{
    f32 dot;

    dot = (a->z * b->z) + ((a->x * b->x) + (a->y * b->y));
    return dot;
}

f32 ASM_VECMag(const Vec* v)
{
    return sqrtf(ASM_VECSquareMag(v));
}

void ASM_VECNormalize(const Vec* src, Vec* unit)
{
    f32 mag;

    mag = (src->z * src->z) + ((src->x * src->x) + (src->y * src->y));

    mag = 1.0f / sqrtf(mag);
    unit->x = src->x * mag;
    unit->y = src->y * mag;
    unit->z = src->z * mag;
}

void ASM_VECScale(const Vec* src, Vec* dst, f32 scale)
{
    dst->x = (src->x * scale);
    dst->y = (src->y * scale);
    dst->z = (src->z * scale);
}

f32 ASM_VECSquareDistance(const Vec* a, const Vec* b)
{
    Vec diff;

    diff.x = a->x - b->x;
    diff.y = a->y - b->y;
    diff.z = a->z - b->z;
    return (diff.z * diff.z) + ((diff.x * diff.x) + (diff.y * diff.y));
}

f32 ASM_VECSquareMag(const Vec* v)
{
    f32 sqmag;

    sqmag = v->z * v->z + ((v->x * v->x) + (v->y * v->y));
    return sqmag;
}

void ASM_VECSubtract(const Vec* a, const Vec* b, Vec* a_b)
{
    a_b->x = a->x - b->x;
    a_b->y = a->y - b->y;
    a_b->z = a->z - b->z;
}

void C_VECAdd(const Vec* a, const Vec* b, Vec* ab)
{
    ab->x = a->x + b->x;
    ab->y = a->y + b->y;
    ab->z = a->z + b->z;
}

void C_VECCrossProduct(const Vec* a, const Vec* b, Vec* axb)
{
    Vec vTmp;

    vTmp.x = (a->y * b->z) - (a->z * b->y);
    vTmp.y = (a->z * b->x) - (a->x * b->z);
    vTmp.z = (a->x * b->y) - (a->y * b->x);
    axb->x = vTmp.x;
    axb->y = vTmp.y;
    axb->z = vTmp.z;
}

f32 C_VECDistance(const Vec* a, const Vec* b)
{
    return sqrtf(C_VECSquareDistance(a, b));
}

f32 C_VECDotProduct(const Vec* a, const Vec* b)
{
    f32 dot;

    dot = (a->z * b->z) + ((a->x * b->x) + (a->y * b->y));
    return dot;
}

void C_VECHalfAngle(const Vec* a, const Vec* b, Vec* half)
{
    Vec aTmp;
    Vec bTmp;
    Vec hTmp;

    aTmp.x = -a->x;
    aTmp.y = -a->y;
    aTmp.z = -a->z;
    bTmp.x = -b->x;
    bTmp.y = -b->y;
    bTmp.z = -b->z;

    C_VECNormalize(&aTmp, &aTmp);
    C_VECNormalize(&bTmp, &bTmp);
    C_VECAdd(&aTmp, &bTmp, &hTmp);

    if (C_VECDotProduct(&hTmp, &hTmp) > 0.0f)
    {
        C_VECNormalize(&hTmp, half);
        return;
    }
    *half = hTmp;
}

f32 C_VECMag(const Vec* v)
{
    return sqrtf(C_VECSquareMag(v));
}

void C_VECNormalize(const Vec* src, Vec* unit)
{
    f32 mag;

    mag = (src->z * src->z) + ((src->x * src->x) + (src->y * src->y));

    mag = 1.0f / sqrtf(mag);
    unit->x = src->x * mag;
    unit->y = src->y * mag;
    unit->z = src->z * mag;
}

void C_VECReflect(const Vec* src, const Vec* normal, Vec* dst)
{
    f32 cosA;
    Vec uI;
    Vec uN;

    uI.x = -src->x;
    uI.y = -src->y;
    uI.z = -src->z;

    C_VECNormalize(&uI, &uI);
    C_VECNormalize(normal, &uN);

    cosA = C_VECDotProduct(&uI, &uN);
    dst->x = (2.0f * uN.x * cosA) - uI.x;
    dst->y = (2.0f * uN.y * cosA) - uI.y;
    dst->z = (2.0f * uN.z * cosA) - uI.z;
    C_VECNormalize(dst, dst);
}

void C_VECScale(const Vec* src, Vec* dst, f32 scale)
{
    dst->x = (src->x * scale);
    dst->y = (src->y * scale);
    dst->z = (src->z * scale);
}

f32 C_VECSquareDistance(const Vec* a, const Vec* b)
{
    Vec diff;

    diff.x = a->x - b->x;
    diff.y = a->y - b->y;
    diff.z = a->z - b->z;
    return (diff.z * diff.z) + ((diff.x * diff.x) + (diff.y * diff.y));
}

f32 C_VECSquareMag(const Vec* v)
{
    f32 sqmag;

    sqmag = v->z * v->z + ((v->x * v->x) + (v->y * v->y));
    return sqmag;
}

void C_VECSubtract(const Vec* a, const Vec* b, Vec* a_b)
{
    a_b->x = a->x - b->x;
    a_b->y = a->y - b->y;
    a_b->z = a->z - b->z;
}

void PSVECAdd(const Vec* a, const Vec* b, Vec* ab)
{
    ab->x = a->x + b->x;
    ab->y = a->y + b->y;
    ab->z = a->z + b->z;
}

void PSVECCrossProduct(const Vec* a, const Vec* b, Vec* axb)
{
    Vec vTmp;

    vTmp.x = (a->y * b->z) - (a->z * b->y);
    vTmp.y = (a->z * b->x) - (a->x * b->z);
    vTmp.z = (a->x * b->y) - (a->y * b->x);
    axb->x = vTmp.x;
    axb->y = vTmp.y;
    axb->z = vTmp.z;
}

f32 PSVECDistance(const Vec* a, const Vec* b)
{
    return sqrtf(PSVECSquareDistance(a, b));
}

f32 PSVECDotProduct(const Vec* a, const Vec* b)
{
    f32 dot;

    dot = (a->z * b->z) + ((a->x * b->x) + (a->y * b->y));
    return dot;
}

f32 PSVECMag(const Vec* v)
{
    return sqrtf(PSVECSquareMag(v));
}

void PSVECNormalize(const Vec* src, Vec* unit)
{
    f32 mag;

    mag = (src->z * src->z) + ((src->x * src->x) + (src->y * src->y));

    mag = 1.0f / sqrtf(mag);
    unit->x = src->x * mag;
    unit->y = src->y * mag;
    unit->z = src->z * mag;
}

void PSVECScale(const Vec* src, Vec* dst, f32 scale)
{
    dst->x = (src->x * scale);
    dst->y = (src->y * scale);
    dst->z = (src->z * scale);
}

f32 PSVECSquareDistance(const Vec* a, const Vec* b)
{
    Vec diff;

    diff.x = a->x - b->x;
    diff.y = a->y - b->y;
    diff.z = a->z - b->z;
    return (diff.z * diff.z) + ((diff.x * diff.x) + (diff.y * diff.y));
}

f32 PSVECSquareMag(const Vec* v)
{
    f32 sqmag;

    sqmag = v->z * v->z + ((v->x * v->x) + (v->y * v->y));
    return sqmag;
}

void PSVECSubtract(const Vec* a, const Vec* b, Vec* a_b)
{
    a_b->x = a->x - b->x;
    a_b->y = a->y - b->y;
    a_b->z = a->z - b->z;
}
}
