#include "vendor/__N_15_FFLiRawMask_cpp_1637d14a.hpp"
#include "platform/wiiu/FFLiDatabaseDefault.hpp"

#include <string.h>

int FFLiiGetEyeRotateOffset(int value);
int FFLiiGetEyebrowRotateOffset(int value);
float FFLiiGetAdjustedEyeH(float value, int adjust);
float FFLiiGetAdjustedMouthH(float value, int adjust);

namespace
{
    static int charValue(const FFLiCharInfo* info, int offset)
    {
        int value;
        memcpy(&value, info->data + offset, sizeof(value));
        return value;
    }

    static float anglePart(int value)
    {
        int remainder = value - (((value >> 31) & 31) + value & ~31);
        return (float)remainder * 0.015625f;
    }
}

void __N_15_FFLiRawMask_cpp_1637d14a::CalcRawMask(RawMasks* rawMasks, const FFLiCharInfo* charInfo, int rotation, int eyeHeightAdjust, int mouthHeightAdjust)
{
    static float eyeScale = 3.5323312282562256f;
    static float faceScale = 4.629278182983398f;
    static float eyebrowScale = 0.8896146416664124f;
    static float mouthScale = 1.7792292833328247f;
    static float eyeBase = 1.0760942697525024f;
    static float eyeOffset = 18.451274871826172f;
    static float eyebrowOffset = 16.549802780151367f;
    static float mouthOffset = 29.258849f;
    static float chinOffset = 31.763554f;
    static float lowerFaceOffset = 15.309928894042969f;
    const float origin = 176.0f;
    const float unit = 0.015625f;
    const float eyeWidth = 0.4f;
    const float faceWidth = 1.0f;
    const float faceHeight = 5.34375f;
    const float eyeHeight = 4.5f;
    const float eyeDepth = 0.12f;
    const float browWidth = 0.64f;
    const float browHeight = 11.25f;
    const float mouthWidth = 5.0625f;
    const float mouthHeight = 6.1875f;
    const float scale = 9.0f;
    const float center = 32.0f;
    const float bottom = 360.0f;
    const float zero = 0.0f;
    const int rotationValue = rotation;
    const float rotationScale = ((float)rotationValue - origin) * unit;
    const float faceX = ((float)charValue(charInfo, 0x34) - origin) * eyebrowScale;
    const float faceY = faceScale * ((float)charValue(charInfo, 0x28) - origin) + 1.0f;
    const float faceZ = 5.34375f * faceY;
    const float faceW = 4.5f * faceY;
    const float eyeX = (float)charValue(charInfo, 0x2c) * 0.12f + 0.6399999857f;
    const float eyeY = ((float)charValue(charInfo, 0x38) - origin) * eyeBase + eyeOffset;
    const float eyeRotation = anglePart(charValue(charInfo, 0x20) + FFLiiGetEyeRotateOffset(charValue(charInfo, 0x20)));
    const float browX = ((float)charValue(charInfo, 0x50) - origin) * eyebrowScale;
    const float browY = eyeScale * ((float)charValue(charInfo, 0x44) - origin) + 4.62927818298f;
    const float browZ = 0.64f * browY;
    const float browW = ((float)charValue(charInfo, 0x54) - origin) * eyeBase + eyebrowOffset;
    const float browRotation = anglePart(charValue(charInfo, 0x3c) + FFLiiGetEyebrowRotateOffset(charValue(charInfo, 0x3c)));
    const float mouthX = eyeScale * ((float)charValue(charInfo, 0x6c) - origin) + 4.62927818298f;
    const float mouthY = ((float)charValue(charInfo, 0x74) - origin) * eyeBase + mouthOffset;
    const float mouthZ = 5.0625f * mouthX;
    const float mouthW = 6.1875f * mouthX;
    const float chinX = ((float)charValue(charInfo, 0x70) - origin) * 0.12f + 0.6399999857f;
    const float chinY = ((float)charValue(charInfo, 0x84) - origin) * eyeScale + 4.62927818298f;
    const float chinZ = ((float)charValue(charInfo, 0x88) - origin) * eyeBase + chinOffset;
    const float lowerX = ((float)charValue(charInfo, 0xa4) - origin) * mouthScale + lowerFaceOffset;
    const float depthA = ((float)charValue(charInfo, 0xa0) - origin);
    const float depthB = ((float)charValue(charInfo, 0xa8) - origin);
    const float scaleFace = rotationScale;
    const float eyeAdjustedA = faceZ * scaleFace;
    const float eyeAdjustedB = faceW * scaleFace;
    const float faceCenter = 32.0f - faceX;
    const float adjustedEye = FFLiiGetAdjustedEyeH(faceZ * faceW * scaleFace, eyeHeightAdjust);
    const float adjustedEye2 = FFLiiGetAdjustedEyeH(faceZ * faceW * scaleFace, mouthHeightAdjust);
    const float adjustedMouth = FFLiiGetAdjustedMouthH(eyeScale * mouthX * ((float)charValue(charInfo, 0x70) * 0.12f + 0.64f) * scaleFace, charValue(charInfo, 100));
    (void)faceWidth; (void)faceHeight; (void)eyeHeight; (void)eyeDepth; (void)browWidth; (void)browHeight; (void)mouthWidth; (void)mouthHeight; (void)scale; (void)center; (void)bottom; (void)zero; (void)faceCenter; (void)eyeAdjustedA; (void)eyeAdjustedB; (void)depthA; (void)depthB;
    rawMasks->values[0] = (32.0f - faceX) * scaleFace;
    rawMasks->values[1] = eyeY * scaleFace;
    rawMasks->values[2] = faceZ * scaleFace;
    rawMasks->values[3] = adjustedEye;
    rawMasks->values[4] = eyeRotation;
    rawMasks->values[5] = 1.0e-38f;
    rawMasks->values[6] = (faceX + 32.0f) * scaleFace;
    rawMasks->values[7] = rawMasks->values[1]; rawMasks->values[8] = rawMasks->values[2]; rawMasks->values[9] = adjustedEye2; rawMasks->values[10] = 360.0f - eyeRotation;
    rawMasks->values[11] = 2.0e-38f;
    rawMasks->values[12] = (32.0f - browX) * scaleFace; rawMasks->values[13] = browW * scaleFace; rawMasks->values[14] = browY * scaleFace; rawMasks->values[15] = eyeScale * browY * faceW * scaleFace; rawMasks->values[16] = browRotation;
    rawMasks->values[17] = 1.0e-38f; rawMasks->values[18] = (browX + 32.0f) * scaleFace; rawMasks->values[19] = rawMasks->values[13]; rawMasks->values[20] = rawMasks->values[14]; rawMasks->values[21] = rawMasks->values[15]; rawMasks->values[22] = 360.0f - browRotation;
    rawMasks->values[23] = 1.0e-38f;
    rawMasks->values[24] = (32.0f - mouthX) * scaleFace; rawMasks->values[25] = mouthY * scaleFace; rawMasks->values[26] = mouthZ * scaleFace; rawMasks->values[27] = adjustedMouth; rawMasks->values[28] = 0.0f; rawMasks->values[29] = (mouthX + 32.0f) * scaleFace; rawMasks->values[30] = rawMasks->values[25]; rawMasks->values[31] = rawMasks->values[26]; rawMasks->values[32] = rawMasks->values[27]; rawMasks->values[33] = 1.0e-38f;
    rawMasks->values[34] = (32.0f - chinX) * scaleFace; rawMasks->values[35] = 1.0e-38f; rawMasks->values[36] = (chinX + 32.0f) * scaleFace; rawMasks->values[37] = chinY * scaleFace; rawMasks->values[38] = chinZ * scaleFace; rawMasks->values[39] = lowerX * scaleFace; rawMasks->values[40] = rawMasks->values[39]; rawMasks->values[41] = 1.0e-38f;
    rawMasks->values[42] = (chinX + 32.0f) * scaleFace; rawMasks->values[43] = chinY * scaleFace; rawMasks->values[44] = chinZ * scaleFace; rawMasks->values[45] = lowerX * scaleFace; rawMasks->values[46] = 0.0f; rawMasks->values[47] = 0.0f;
}
