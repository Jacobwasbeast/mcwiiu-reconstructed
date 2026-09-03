#ifndef SOURCE_PLATFORM_WIIU_FFLICOORDINATE_HPP
#define SOURCE_PLATFORM_WIIU_FFLICOORDINATE_HPP

#include <stdint.h>

struct FFLVec3
{
    float x;
    float y;
    float z;
};

struct FFLVec4
{
    float x;
    float y;
    float z;
    float w;
};

struct FFLiSnorm10_10_10_2
{
    uint32_t value;
};

struct FFLiSnorm8_8_8_8
{
    int8_t x;
    int8_t y;
    int8_t z;
    int8_t w;
};

enum FFLCoordinateType
{
    FFLCoordinateX = 0,
    FFLCoordinateY = 1,
    FFLCoordinateZ = 2
};

class FFLiCoordinate
{
public:
    FFLiCoordinate();
    ~FFLiCoordinate();

    static float FlipValue(FFLVec3* value, float component);
    void TransformImpl(FFLVec3* value) const;
    void Transform(FFLVec3* value) const;
    void Transform(FFLVec4* value) const;
    static float FlipValue(FFLVec4* value, float component);
    void TransformWithoutScale(FFLVec3* value) const;
    static uint32_t FlipValue(FFLiSnorm10_10_10_2* value, uint32_t component);
    void TransformImpl(FFLiSnorm10_10_10_2* value) const;
    void TransformWithoutScale(FFLiSnorm10_10_10_2* value) const;
    static int32_t FlipValue(FFLiSnorm8_8_8_8* value, int8_t component);
    void TransformImpl(FFLiSnorm8_8_8_8* value) const;
    void TransformWithoutScale(FFLiSnorm8_8_8_8* value) const;
    bool IsDefault() const;
    void Set(FFLCoordinateType source, FFLCoordinateType destination);
    void TransformWithoutScale(FFLVec4* value) const;
    void SetScale(float scale);

private:
    uint8_t mOutputX;
    uint8_t mOutputY;
    uint8_t mOutputZ;
    uint8_t mFlipX;
    uint8_t mFlipY;
    uint8_t mFlipZ;
    uint8_t mReserved[2];
    float mScale;
};

#endif
