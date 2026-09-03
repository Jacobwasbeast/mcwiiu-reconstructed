#include "platform/wiiu/FFLiCoordinate.hpp"

FFLiCoordinate::FFLiCoordinate()
    : mOutputX(1), mOutputY(2), mOutputZ(0), mFlipX(0), mFlipY(0), mFlipZ(0), mScale(1.0f)
{
}

FFLiCoordinate::~FFLiCoordinate()
{
}

float FFLiCoordinate::FlipValue(FFLVec3*, float component)
{
    return -component;
}

float FFLiCoordinate::FlipValue(FFLVec4*, float component)
{
    return -component;
}

void FFLiCoordinate::TransformImpl(FFLVec3* value) const
{
    float input[3];
    float output[3];
    input[0] = value->x;
    input[1] = value->y;
    input[2] = value->z;
    output[mOutputZ] = mFlipZ ? -input[0] : input[0];
    output[mOutputX] = mFlipX ? -input[1] : input[1];
    output[mOutputY] = mFlipY ? -input[2] : input[2];
    value->x = output[0];
    value->y = output[1];
    value->z = output[2];
}

void FFLiCoordinate::Transform(FFLVec3* value) const
{
    TransformImpl(value);
    value->x *= mScale;
    value->y *= mScale;
    value->z *= mScale;
}

void FFLiCoordinate::Transform(FFLVec4* value) const
{
    FFLVec3 vector;
    vector.x = value->x;
    vector.y = value->y;
    vector.z = value->z;
    Transform(&vector);
    value->x = vector.x;
    value->y = vector.y;
    value->z = vector.z;
}

void FFLiCoordinate::TransformWithoutScale(FFLVec3* value) const
{
    TransformImpl(value);
}

uint32_t FFLiCoordinate::FlipValue(FFLiSnorm10_10_10_2*, uint32_t component)
{
    if (component == 0x200)
        return 0x1ff;
    if (component == 0x1ff)
        return 0x200;
    return (0x400U - component) & 0x3ffU;
}

void FFLiCoordinate::TransformImpl(FFLiSnorm10_10_10_2* value) const
{
    uint32_t input[3];
    uint32_t output[3];
    const uint32_t packed = value->value;
    input[0] = packed & 0x3ffU;
    input[1] = (packed >> 10) & 0x3ffU;
    input[2] = (packed >> 20) & 0x3ffU;
    output[mOutputZ] = mFlipZ ? FlipValue(value, input[0]) : input[0];
    output[mOutputX] = mFlipX ? FlipValue(value, input[1]) : input[1];
    output[mOutputY] = mFlipY ? FlipValue(value, input[2]) : input[2];
    value->value = ((output[2] & 0x3ffU) << 20) | ((output[1] & 0x3ffU) << 10) |
                   (output[0] & 0x3ffU) | (packed & 0xc0000000U);
}

void FFLiCoordinate::TransformWithoutScale(FFLiSnorm10_10_10_2* value) const
{
    TransformImpl(value);
}

int32_t FFLiCoordinate::FlipValue(FFLiSnorm8_8_8_8*, int8_t component)
{
    if (component == -128)
        component = 127;
    return (int8_t)(-component);
}

void FFLiCoordinate::TransformImpl(FFLiSnorm8_8_8_8* value) const
{
    int8_t input[3];
    int8_t output[3];
    input[0] = value->x;
    input[1] = value->y;
    input[2] = value->z;
    output[mOutputZ] = mFlipZ ? (int8_t)FlipValue(value, input[0]) : input[0];
    output[mOutputX] = mFlipX ? (int8_t)FlipValue(value, input[1]) : input[1];
    output[mOutputY] = mFlipY ? (int8_t)FlipValue(value, input[2]) : input[2];
    value->x = output[0];
    value->y = output[1];
    value->z = output[2];
}

void FFLiCoordinate::TransformWithoutScale(FFLiSnorm8_8_8_8* value) const
{
    TransformImpl(value);
}

bool FFLiCoordinate::IsDefault() const
{
    return mOutputX == 1 && mOutputY == 2 && mOutputZ == 0 &&
           mFlipX == 0 && mFlipY == 0 && mFlipZ == 0 && mScale == 1.0f;
}

void FFLiCoordinate::Set(FFLCoordinateType source, FFLCoordinateType destination)
{
    mOutputX = (uint8_t)source;
    mOutputY = (uint8_t)destination;
    mOutputZ = (uint8_t)(3 - (int)source - (int)destination);
    mFlipX = 0;
    mFlipY = 0;
    mFlipZ = 0;
}

void FFLiCoordinate::TransformWithoutScale(FFLVec4* value) const
{
    FFLVec3 vector;
    vector.x = value->x;
    vector.y = value->y;
    vector.z = value->z;
    TransformWithoutScale(&vector);
    value->x = vector.x;
    value->y = vector.y;
    value->z = vector.z;
}

void FFLiCoordinate::SetScale(float scale)
{
    mScale = scale;
}
