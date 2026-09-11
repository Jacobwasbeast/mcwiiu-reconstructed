#include "game/client/render/Renderer.hpp"

#include "platform/wiiu/FFLiManager.hpp"
#include "platform/wiiu/FFLiCoordinate.hpp"
#include "platform/wiiu/FFLiUtilShader.hpp"
#include "platform/wiiu/WiiU_GX2.hpp"
#include "platform/wiiu/CafeThreadLocal.hpp"

#include <coreinit/thread.h>
#include <map>
#include <coreinit/memdefaultheap.h>
#include <coreinit/memexpheap.h>
#include <coreinit/memfrmheap.h>
#include <coreinit/cache.h>
#include <coreinit/debug.h>
#include <dmae/mem.h>
#include <dmae/sync.h>
#include <png.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>

struct _OVERLAPPED;
struct FFLResourceDesc;

struct D3DXIMAGE_INFO
{
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int MipLevels;
    unsigned int Format;
    unsigned int ResourceType;
    unsigned int ImageFileFormat;
};

struct _Vec3
{
    float x;
    float y;
    float z;
};

extern "C"
{
void GX2Init(unsigned int* attributes);
unsigned int GX2GetSystemTVScanMode();
void GX2CalcTVSize(unsigned int renderMode, unsigned int format, unsigned int bufferingMode, unsigned int* size,
                   unsigned int* scaleNeeded);
void GX2CalcDRCSize(unsigned int renderMode, unsigned int format, unsigned int bufferingMode, unsigned int* size,
                    unsigned int* scaleNeeded);
void GX2SetTVBuffer(void* buffer, unsigned int size, unsigned int renderMode, unsigned int format,
                    unsigned int bufferingMode);
void GX2SetDRCBuffer(void* buffer, unsigned int size, unsigned int renderMode, unsigned int format,
                     unsigned int bufferingMode);
void GX2SetTVEnable(int bEnable);
void GX2SetDRCEnable(int bEnable);
void GX2Invalidate(unsigned int mode, void* buffer, unsigned int size);
void GX2Flush();
void GX2DrawDone();
void GX2SwapScanBuffers();
void GX2SetSwapInterval(unsigned int interval);
unsigned long long GX2GetLastSubmittedTimeStamp();
void GX2WaitTimeStamp(unsigned long long timeStamp);
void GX2CopyColorBufferToScanBuffer(_GX2ColorBuffer* buffer, unsigned int target);
void GX2SetupContextStateEx(_GX2ContextState* state, int bProfilingEnable);
void GX2SetContextState(_GX2ContextState* state);
void GX2SetShaderModeEx(unsigned int mode, unsigned int vertexGprs, unsigned int vertexStackEntries,
                        unsigned int geometryGprs, unsigned int geometryStackEntries, unsigned int pixelGprs,
                        unsigned int pixelStackEntries);
void GX2SetViewport(float x, float y, float width, float height, float nearZ, float farZ);
void GX2SetScissor(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
void GX2SetColorBuffer(_GX2ColorBuffer* buffer, unsigned int target);
void GX2SetDepthBuffer(_GX2DepthBuffer* buffer);
void GX2CalcSurfaceSizeAndAlignment(_GX2Surface* surface);
void GX2InitColorBufferRegs(_GX2ColorBuffer* buffer);
void GX2InitTextureRegs(_GX2Texture* texture);
void GX2CalcDepthBufferHiZInfo(_GX2DepthBuffer* buffer, unsigned int* size, unsigned int* alignment);
void GX2InitDepthBufferHiZEnable(_GX2DepthBuffer* buffer, int bEnable);
void GX2InitSampler(_GX2Sampler* sampler, unsigned int clampMode, unsigned int minMagFilter);
void GX2InitSamplerClamping(_GX2Sampler* sampler, unsigned int clampX, unsigned int clampY, unsigned int clampZ);
void GX2InitSamplerXYFilter(_GX2Sampler* sampler, unsigned int magFilter, unsigned int minFilter,
                            unsigned int maxAnisotropicRatio);
void GX2InitSamplerZMFilter(_GX2Sampler* sampler, unsigned int zFilter, unsigned int mipFilter);
unsigned int GX2CalcFetchShaderSizeEx(unsigned int numAttributes, unsigned int tessellationMode,
                                      unsigned int tessellationType);
void GX2InitFetchShaderEx(_GX2FetchShader* fetchShader, void* buffer, unsigned int numAttributes,
                          const void* attributes, unsigned int tessellationMode, unsigned int tessellationType);
void GX2SetFetchShader(const _GX2FetchShader* fetchShader);
void GX2SetVertexShader(const _GX2VertexShader* shader);
void GX2SetPixelShader(const _GX2PixelShader* shader);
void GX2SetVertexUniformReg(unsigned int offset, unsigned int count, const void* values);
void GX2SetPixelUniformReg(unsigned int offset, unsigned int count, const void* values);
void GX2SetVertexTexture(const _GX2Texture* texture, unsigned int unit);
void GX2SetVertexSampler(const _GX2Sampler* sampler, unsigned int unit);
void GX2SetPixelTexture(const _GX2Texture* texture, unsigned int unit);
void GX2SetPixelSampler(const _GX2Sampler* sampler, unsigned int unit);
void GX2SetAttribBuffer(unsigned int index, unsigned int size, unsigned int stride, const void* buffer);
void GX2DrawEx(unsigned int mode, unsigned int count, unsigned int offset, unsigned int numInstances);
void GX2SetDepthStencilControl(int bDepthTest, int bDepthWrite, unsigned int depthCompare, int bStencilTest,
                               int bBackfaceStencil, unsigned int frontStencilCompare,
                               unsigned int frontStencilZPass, unsigned int frontStencilZFail,
                               unsigned int frontStencilFail, unsigned int backStencilCompare,
                               unsigned int backStencilZPass, unsigned int backStencilZFail,
                               unsigned int backStencilFail);
void GX2SetCullOnlyControl(unsigned int frontFace, int bCullFront, int bCullBack);
void GX2SetPolygonControl(unsigned int frontFace, int bCullFront, int bCullBack, int bPolygonMode,
                          unsigned int polygonModeFront, unsigned int polygonModeBack, int bPolygonOffsetFront,
                          int bPolygonOffsetBack, int bPointLineOffset);
void GX2SetPolygonOffset(float frontOffset, float frontScale, float backOffset, float backScale, float clamp);
void GX2SetColorControl(unsigned int logicOp, unsigned char blendEnableMask, int bMultiWriteEnable,
                        int bColorBufferEnable);
void GX2SetBlendControl(unsigned int target, unsigned int colourSource, unsigned int colourDestination,
                        unsigned int colourCombine, int bSeparateAlphaBlend, unsigned int alphaSource,
                        unsigned int alphaDestination, unsigned int alphaCombine);
void GX2SetBlendConstantColor(float red, float green, float blue, float alpha);
void GX2SetAlphaTest(int bAlphaTest, unsigned int compare, float reference);
void GX2SetStencilMask(unsigned char frontMask, unsigned char frontWriteMask, unsigned char frontReference,
                       unsigned char backMask, unsigned char backWriteMask, unsigned char backReference);
void GX2SetTargetChannelMasks(unsigned int mask0, unsigned int mask1, unsigned int mask2, unsigned int mask3,
                              unsigned int mask4, unsigned int mask5, unsigned int mask6, unsigned int mask7);
unsigned int GX2GetSurfaceMipPitch(const _GX2Surface* surface, unsigned int level);
}

void _GX2InitColorBuffer(_GX2ColorBuffer* buffer, unsigned int width, unsigned int height,
                         _GX2SurfaceFormat format, _GX2AAMode aaMode)
{
    buffer->surface.dim = 1;
    buffer->surface.width = width;
    buffer->surface.height = height;
    buffer->surface.depth = 1;
    buffer->surface.numMips = 1;
    buffer->surface.format = format;
    buffer->surface.aaMode = aaMode;
    buffer->surface.use = 0;
    buffer->surface.imagePtr = NULL;
    buffer->surface.mipPtr = NULL;
    buffer->surface.tileMode = 0;
    buffer->surface.swizzle = 0;
    buffer->surface.alignment = 0;
    buffer->surface.pitch = 0;
    buffer->viewMip = 0;
    buffer->viewFirstSlice = 0;
    buffer->viewNumSlices = 1;
    buffer->aaBuffer = NULL;
    buffer->aaSize = 0;
    GX2CalcSurfaceSizeAndAlignment(&buffer->surface);
    GX2InitColorBufferRegs(buffer);
}

void GX2InitColorBufferFTV(_GX2ColorBuffer* buffer, unsigned int width, unsigned int height,
                           _GX2SurfaceFormat format, _GX2AAMode aaMode);
void GX2InitDepthBuffer(_GX2DepthBuffer* buffer, unsigned int width, unsigned int height,
                        _GX2SurfaceFormat format, _GX2AAMode aaMode);
void GX2InitTexture(_GX2Texture* texture, unsigned int width, unsigned int height, unsigned int depth,
                    unsigned int numMips, _GX2SurfaceFormat format, _GX2SurfaceDim dim);

void OSFastMutex_Init(OSFastMutex* mutex, const char* name);
void OSFastMutex_Lock(OSFastMutex* mutex);
void OSFastMutex_Unlock(OSFastMutex* mutex);
void ASM_MTXIdentity(float (*matrix)[4]);
void ASM_MTX44Identity(float (*matrix)[4]);
void ASM_MTX44Concat(const float (*left)[4], const float (*right)[4], float (*result)[4]);
void ASM_MTX44Trans(float (*matrix)[4], float x, float y, float z);
void ASM_MTX44Scale(float (*matrix)[4], float x, float y, float z);
void ASM_MTX44RotAxisRad(float (*matrix)[4], const _Vec3* axis, float radians);
void ASM_MTX44Transpose(const float (*source)[4], float (*destination)[4]);
void ASM_MTX44MultVecSR(const float (*matrix)[4], const _Vec3* source, _Vec3* destination);
void ASM_VECNormalize(const _Vec3* source, _Vec3* destination)
{
    const float half = 0.5f;
    const float three = 3.0f;
    asm volatile(
        "lfs 9,%[half]\n"
        "psq_l 2,0(%[src]),0,0\n"
        "ps_mul 4,2,2\n"
        "psq_l 3,8(%[src]),1,0\n"
        "ps_madd 5,3,3,4\n"
        "ps_sum0 6,5,3,4\n"
        "lfs 10,%[three]\n"
        "frsqrte 1,6\n"
        "fmuls 7,1,1\n"
        "fmuls 8,1,9\n"
        "fnmsubs 7,7,6,10\n"
        "fmuls 1,7,8\n"
        "ps_muls0 2,2,1\n"
        "psq_st 2,0(%[dst]),0,0\n"
        "ps_muls0 3,3,1\n"
        "psq_st 3,8(%[dst]),1,0\n"
        :
        : [src] "b" (source), [dst] "b" (destination), [half] "m" (half), [three] "m" (three)
        : "fr1", "fr2", "fr3", "fr4", "fr5", "fr6", "fr7", "fr8", "fr9", "fr10", "memory");
}

void ASM_MTX44RotAxisRad(float (*matrix)[4], const _Vec3* axis, float radians)
{
    const float sine = sinf(radians);
    const float cosine = cosf(radians);
    const float oneMinusCosine = 1.0f - cosine;
    matrix[0][0] = cosine + axis->x * axis->x * oneMinusCosine;
    matrix[0][1] = axis->x * axis->y * oneMinusCosine - axis->z * sine;
    matrix[0][2] = axis->x * axis->z * oneMinusCosine + axis->y * sine;
    matrix[0][3] = 0.0f;
    matrix[1][0] = axis->y * axis->x * oneMinusCosine + axis->z * sine;
    matrix[1][1] = cosine + axis->y * axis->y * oneMinusCosine;
    matrix[1][2] = axis->y * axis->z * oneMinusCosine - axis->x * sine;
    matrix[1][3] = 0.0f;
    matrix[2][0] = axis->z * axis->x * oneMinusCosine - axis->y * sine;
    matrix[2][1] = axis->z * axis->y * oneMinusCosine + axis->x * sine;
    matrix[2][2] = cosine + axis->z * axis->z * oneMinusCosine;
    matrix[2][3] = 0.0f;
    matrix[3][0] = 0.0f;
    matrix[3][1] = 0.0f;
    matrix[3][2] = 0.0f;
    matrix[3][3] = 1.0f;
}

void ASM_MTX44MultVec(const _Mat44* matrix, FFLVec4* source, FFLVec4* result);
void ASM_MTX44Transpose(const _Mat44* source, _Mat44* destination);
void ASM_MTX44Concat(const _Mat44* left, const _Mat44* right, _Mat44* result);
void ASM_MTX34To44(const _Mat34* source, _Mat44* destination);
void ASM_MTXConcat(const _Mat34* left, const _Mat34* right, _Mat34* result);
void ASM_MTXIdentity(float (*matrix)[4]);
void ASM_MTXRotTrig(double sine, double cosine, _Mat34* matrix, unsigned int axis);
void ASM_MTXRotRad(double angle, _Mat34* matrix, unsigned int axis);
int C_MTX44Inverse(const float (*source)[4], float (*destination)[4])
{
    float work[4][4];
    memcpy(work, source, sizeof(work));
    ASM_MTX44Identity(destination);
    for (unsigned int column = 0; column < 4; ++column)
    {
        unsigned int pivot = column;
        float largest = 0.0f;
        for (unsigned int row = column; row < 4; ++row)
        {
            const float magnitude = fabsf(work[row][column]);
            if (largest < magnitude)
            {
                pivot = row;
                largest = magnitude;
            }
        }
        if (largest == 0.0f)
            return 0;
        if (pivot != column)
        {
            for (unsigned int element = 0; element < 4; ++element)
            {
                const float value = work[column][element];
                work[column][element] = work[pivot][element];
                work[pivot][element] = value;
                const float inverse = destination[column][element];
                destination[column][element] = destination[pivot][element];
                destination[pivot][element] = inverse;
            }
        }
        const float scale = 1.0f / work[column][column];
        for (unsigned int element = 0; element < 4; ++element)
        {
            work[column][element] *= scale;
            destination[column][element] *= scale;
        }
        for (unsigned int row = 0; row < 4; ++row)
        {
            if (row == column)
                continue;
            const float factor = work[row][column];
            for (unsigned int element = 0; element < 4; ++element)
            {
                work[row][element] = -(work[column][element] * factor - work[row][element]);
                destination[row][element] = -(destination[column][element] * factor - destination[row][element]);
            }
        }
    }
    return 1;
}

void C_MTXPerspective(float (*matrix)[4], float fieldOfView, float aspect, float nearDistance, float farDistance);
void C_MTXOrtho(float (*matrix)[4], float top, float bottom, float left, float right, float nearDistance,
                float farDistance);
int C_MTX44Inverse(const float (*source)[4], float (*destination)[4]);

void ASM_MTX44MultVec(const _Mat44* matrix, FFLVec4* source, FFLVec4* result)
{
    for (int row = 0; row < 4; ++row)
    {
        float value = matrix->value[row][0] * source->x
            + matrix->value[row][1] * source->y
            + matrix->value[row][2] * source->z
            + matrix->value[row][3] * source->w;
        if (row == 0) result->x = value;
        if (row == 1) result->y = value;
        if (row == 2) result->z = value;
        if (row == 3) result->w = value;
    }
}

void ASM_MTX44MultVecSR(const float (*matrix)[4], const _Vec3* source, _Vec3* destination)
{
    destination->x = matrix[0][0] * source->x + matrix[0][1] * source->y + matrix[0][2] * source->z + matrix[0][3];
    destination->y = matrix[1][0] * source->x + matrix[1][1] * source->y + matrix[1][2] * source->z + matrix[1][3];
    destination->z = matrix[2][0] * source->x + matrix[2][1] * source->y + matrix[2][2] * source->z + matrix[2][3];
}

void ASM_MTX44Transpose(const float (*source)[4], float (*destination)[4])
{
    float transposed[4][4];
    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            transposed[row][column] = source[column][row];
    memcpy(destination, transposed, sizeof(transposed));
}

void ASM_MTX44Transpose(const _Mat44* source, _Mat44* destination)
{
    ASM_MTX44Transpose(source->value, destination->value);
}

void ASM_MTX44Concat(const float (*left)[4], const float (*right)[4], float (*result)[4])
{
    float product[4][4];
    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            product[row][column] = left[row][0] * right[0][column] + left[row][1] * right[1][column]
                + left[row][2] * right[2][column] + left[row][3] * right[3][column];
    memcpy(result, product, sizeof(product));
}

void ASM_MTX44Concat(const _Mat44* left, const _Mat44* right, _Mat44* result)
{
    ASM_MTX44Concat(left->value, right->value, result->value);
}

void ASM_MTX44Concat(float* left, float* right, float* result) asm("ASM_MTX44Concat");
void ASM_MTX44Concat(float* left, float* right, float* result)
{
    ASM_MTX44Concat(reinterpret_cast<const float (*)[4]>(left), reinterpret_cast<const float (*)[4]>(right),
                    reinterpret_cast<float (*)[4]>(result));
}

int C_MTX44Inverse(float* source, float* destination) asm("C_MTX44Inverse");
int C_MTX44Inverse(float* source, float* destination)
{
    return C_MTX44Inverse(reinterpret_cast<const float (*)[4]>(source), reinterpret_cast<float (*)[4]>(destination));
}

void ASM_MTX44Identity(float (*matrix)[4])
{
    memset(matrix, 0, sizeof(float) * 16);
    matrix[0][0] = 1.0f;
    matrix[1][1] = 1.0f;
    matrix[2][2] = 1.0f;
    matrix[3][3] = 1.0f;
}

void ASM_MTXIdentity(float (*matrix)[4])
{
    memset(matrix, 0, sizeof(float) * 12);
    matrix[0][0] = 1.0f;
    matrix[1][1] = 1.0f;
    matrix[2][2] = 1.0f;
}

void ASM_MTX44Trans(float (*matrix)[4], float x, float y, float z)
{
    matrix[0][3] = x;
    matrix[1][3] = y;
    matrix[2][3] = z;
}

void ASM_MTX44Scale(float (*matrix)[4], float x, float y, float z)
{
    memset(matrix, 0, sizeof(float) * 16);
    matrix[0][0] = x;
    matrix[1][1] = y;
    matrix[2][2] = z;
    matrix[3][3] = 1.0f;
}

void ASM_MTX34To44(const _Mat34* source, _Mat44* destination)
{
    memcpy(destination->value, source->value, sizeof(source->value));
    destination->value[3][0] = 0.0f;
    destination->value[3][1] = 0.0f;
    destination->value[3][2] = 0.0f;
    destination->value[3][3] = 1.0f;
}

void ASM_MTXConcat(const _Mat34* left, const _Mat34* right, _Mat34* result)
{
    _Mat34 product;
    for (int row = 0; row < 3; ++row)
    {
        product.value[row][0] = left->value[row][0] * right->value[0][0]
            + left->value[row][1] * right->value[1][0] + left->value[row][2] * right->value[2][0];
        product.value[row][1] = left->value[row][0] * right->value[0][1]
            + left->value[row][1] * right->value[1][1] + left->value[row][2] * right->value[2][1];
        product.value[row][2] = left->value[row][0] * right->value[0][2]
            + left->value[row][1] * right->value[1][2] + left->value[row][2] * right->value[2][2];
        product.value[row][3] = left->value[row][0] * right->value[0][3]
            + left->value[row][1] * right->value[1][3] + left->value[row][2] * right->value[2][3]
            + left->value[row][3];
    }
    *result = product;
}

void ASM_MTXTrans(double x, double y, double z, _Mat34* matrix)
{
    memset(matrix, 0, sizeof(*matrix));
    matrix->value[0][0] = 1.0f;
    matrix->value[1][1] = 1.0f;
    matrix->value[2][2] = 1.0f;
    matrix->value[0][3] = (float)x;
    matrix->value[1][3] = (float)y;
    matrix->value[2][3] = (float)z;
}

void ASM_MTXScale(double x, double y, double z, _Mat34* matrix)
{
    memset(matrix, 0, sizeof(*matrix));
    matrix->value[0][0] = (float)x;
    matrix->value[1][1] = (float)y;
    matrix->value[2][2] = (float)z;
}

static void StoreRotationPair(_Mat34* matrix, unsigned int offset, float first, float second)
{
    unsigned char* bytes = reinterpret_cast<unsigned char*>(matrix);
    memcpy(bytes + offset, &first, sizeof(first));
    memcpy(bytes + offset + sizeof(first), &second, sizeof(second));
}

static void StoreRotationSingle(_Mat34* matrix, unsigned int offset, float value)
{
    memcpy(reinterpret_cast<unsigned char*>(matrix) + offset, &value, sizeof(value));
}

void ASM_MTXRotTrig(double sine, double cosine, _Mat34* matrix, unsigned int axis)
{
    const float s = static_cast<float>(sine);
    const float c = static_cast<float>(cosine);
    const float ns = -s;
    const unsigned int effectiveAxis = axis | 0x20u;
    const float zero = 0.0f;
    const float one = 1.0f;
    if (effectiveAxis == 0x78u)
    {
        StoreRotationSingle(matrix, 0, one);
        StoreRotationPair(matrix, 4, zero, zero);
        StoreRotationPair(matrix, 12, zero, zero);
        StoreRotationPair(matrix, 20, c, ns);
        StoreRotationPair(matrix, 28, zero, zero);
        StoreRotationPair(matrix, 36, s, c);
        StoreRotationSingle(matrix, 44, zero);
    }
    else if (effectiveAxis == 0x79u)
    {
        StoreRotationPair(matrix, 0, c, zero);
        StoreRotationPair(matrix, 8, s, zero);
        StoreRotationPair(matrix, 16, zero, one);
        StoreRotationPair(matrix, 24, zero, zero);
        StoreRotationPair(matrix, 32, ns, zero);
        StoreRotationPair(matrix, 40, c, zero);
    }
    else if (effectiveAxis == 0x7Au)
    {
        StoreRotationPair(matrix, 0, c, zero);
        StoreRotationPair(matrix, 8, zero, zero);
        StoreRotationPair(matrix, 16, s, c);
        StoreRotationPair(matrix, 24, zero, zero);
        StoreRotationPair(matrix, 32, zero, zero);
        StoreRotationPair(matrix, 40, one, zero);
    }
}

void ASM_MTXRotRad(double angle, _Mat34* matrix, unsigned int axis)
{
    ASM_MTXRotTrig((double)sinf((float)angle), (double)cosf((float)angle), matrix, axis);
}

void C_MTXPerspective(float (*matrix)[4], float fieldOfView, float aspect, float nearDistance, float farDistance)
{
    const float cotangent = 1.0f / tanf(fieldOfView * 0.5f);
    const float reciprocalDepth = 1.0f / (farDistance - nearDistance);
    memset(matrix, 0, sizeof(float) * 16);
    matrix[0][0] = cotangent / aspect;
    matrix[1][1] = cotangent;
    matrix[2][2] = -(farDistance + nearDistance) * reciprocalDepth;
    matrix[2][3] = -(2.0f * farDistance * nearDistance) * reciprocalDepth;
    matrix[3][2] = -1.0f;
    matrix[3][3] = 0.0f;
}

void C_MTXOrtho(float (*matrix)[4], float top, float bottom, float left, float right, float nearDistance,
                float farDistance)
{
    const float reciprocalWidth = 1.0f / (right - left);
    const float reciprocalHeight = 1.0f / (top - bottom);
    const float reciprocalDepth = 1.0f / (farDistance - nearDistance);
    memset(matrix, 0, sizeof(float) * 16);
    matrix[0][0] = 2.0f * reciprocalWidth;
    matrix[1][1] = 2.0f * reciprocalHeight;
    matrix[2][2] = -2.0f * reciprocalDepth;
    matrix[2][3] = -(farDistance + nearDistance) * reciprocalDepth;
    matrix[3][0] = -(right + left) * reciprocalWidth;
    matrix[3][1] = -(top + bottom) * reciprocalHeight;
    matrix[3][2] = 0.0f;
    matrix[3][3] = 1.0f;
}

void C_MTXLookAt(_Mat34* matrix, const float* eye, const float* target, const float* up)
{
    _Vec3 forward = {target[0] - eye[0], target[1] - eye[1], target[2] - eye[2]};
    const float forwardLength = sqrtf(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    forward.x /= forwardLength;
    forward.y /= forwardLength;
    forward.z /= forwardLength;

    _Vec3 side = {up[1] * forward.z - up[2] * forward.y,
                  up[2] * forward.x - up[0] * forward.z,
                  up[0] * forward.y - up[1] * forward.x};
    const float sideLength = sqrtf(side.x * side.x + side.y * side.y + side.z * side.z);
    side.x /= sideLength;
    side.y /= sideLength;
    side.z /= sideLength;

    const _Vec3 correctedUp = {forward.y * side.z - forward.z * side.y,
                               forward.z * side.x - forward.x * side.z,
                               forward.x * side.y - forward.y * side.x};
    matrix->value[0][0] = side.x;
    matrix->value[0][1] = side.y;
    matrix->value[0][2] = side.z;
    matrix->value[0][3] = -(eye[0] * side.x + eye[1] * side.y + eye[2] * side.z);
    matrix->value[1][0] = correctedUp.x;
    matrix->value[1][1] = correctedUp.y;
    matrix->value[1][2] = correctedUp.z;
    matrix->value[1][3] = -(eye[0] * correctedUp.x + eye[1] * correctedUp.y + eye[2] * correctedUp.z);
    matrix->value[2][0] = forward.x;
    matrix->value[2][1] = forward.y;
    matrix->value[2][2] = forward.z;
    matrix->value[2][3] = -(eye[0] * forward.x + eye[1] * forward.y + eye[2] * forward.z);
}

unsigned int FFLGetWorkSize(FFLResourceDesc* resourceDesc);
int FFLInitRes(void* workMemory, FFLResourceDesc* resourceDesc);
void FFLInitResGPUStep();
void FFLExit();
void* MakeIcon(_GX2Texture* texture, IconShaderType shaderType, FFLDataSource dataSource, FFLStoreData* storeData,
             int index);

void user_write_data_init(void* buffer, int size);
void user_write_data(png_struct_def* png, unsigned char* data, unsigned int size);
void user_flush_data(png_struct_def* png);

extern int FFLGetBufferSizeDefaultShader(const char* name);
extern int FFLInitDefaultShader(void* shader, void* buffer, const char* name);
extern void FFLSetDefaultShader(void* shader);
extern void FFLDestroyDefaultShader(void* shader);
extern void InitShader__FP11PlainShaderPCv(PlainShader* shader, const void* data);
extern void DestroyShader__FP11PlainShader(PlainShader* shader);
extern int GetVertexUniformLocation__FPC16_GX2VertexShaderPCc(const _GX2VertexShader* shader,
                                                              const char* name);
extern int GetPixelUniformLocation__FPC15_GX2PixelShaderPCc(const _GX2PixelShader* shader,
                                                            const char* name);
extern unsigned int GetPixelSamplerLocation__FPC15_GX2PixelShaderPCc(const _GX2PixelShader* shader,
                                                                      const char* name);
extern const unsigned char gFFLIconShaderData[];
int user_write_data_bytes_written();

int CreateFileA(const char* fileName, unsigned int desiredAccess, unsigned int shareMode,
                void* securityAttributes, unsigned int creationDisposition, unsigned int flagsAndAttributes,
                int templateFile);
unsigned int GetFileSize(int file, unsigned int* fileSizeHigh);
int ReadFile(int file, void* buffer, unsigned int numberOfBytesToRead, unsigned int* numberOfBytesRead,
             _OVERLAPPED* overlapped);
int CloseHandle(int handle);

bool InputManager_IsShowingOnscreenKeyboard();
bool InputManager_IsShowingErrorViewer();

namespace nn
{
    namespace swkbd
    {
        void DrawTV();
        void DrawDRC();
    }

    namespace erreula
    {
        void DrawTV();
        void DrawDRC();
    }
}

#include "platform/wiiu/RendererShaders.inc"

const _GX2AttribStream gAttribStreamStandard[5] = {
    {0x00000000, 0x00000000, 0x00000000, 0x00000813, 0x00000000, 0x00000000, 0x00010205, 0x00000003},
    {0x00000001, 0x00000000, 0x00000014, 0x0000000A, 0x00000000, 0x00000000, 0x00010203, 0x00000002},
    {0x00000002, 0x00000000, 0x00000018, 0x0000020A, 0x00000000, 0x00000000, 0x00010203, 0x00000002},
    {0x00000003, 0x00000000, 0x0000000C, 0x0000080D, 0x00000000, 0x00000000, 0x00010404, 0x00000003},
    {0x00000004, 0x00000000, 0x0000001C, 0x00000307, 0x00000000, 0x00000000, 0x00010404, 0x00000003},
};
const _GX2AttribStream gAttribStreamQuad[2] = {
    {0x00000000, 0x00000000, 0x00000000, 0x0000030E, 0x00000000, 0x00000000, 0x00010203, 0x00000003},
    {0x00000001, 0x00000000, 0x00000008, 0x0000030E, 0x00000000, 0x00000000, 0x00010203, 0x00000003},
};
const _GX2AttribStream gAttribStreamPosition[1] = {
    {0x00000000, 0x00000000, 0x00000000, 0x0000080D, 0x00000000, 0x00000000, 0x00010405, 0x00000003},
};
const _GX2AttribStream gAttribStreamParticle[3] = {
    {0x00000000, 0x00000000, 0x00000000, 0x0000080D, 0x00000000, 0x00000000, 0x00010405, 0x00000003},
    {0x00000001, 0x00000000, 0x00000008, 0x0000080D, 0x00000000, 0x00000000, 0x00010405, 0x00000003},
    {0x00000002, 0x00000000, 0x00000010, 0x0000000A, 0x00000000, 0x00000000, 0x00010203, 0x00000002},
};

static const unsigned int sPrimitiveModes[7] = { 4, 6, 5, 0x13, 2, 3, 0 };

Renderer* gRenderer = NULL;
static OSFastMutex sTotalAllocMutex;
static int sTotalCommandBufferBytes = 0;
static unsigned long long sPreviousSubmitTimeStamp = 0;
static unsigned long long sLastSubmitTimeStamp = 0;
static void* sFFLWorkMemory = NULL;
static FFLResourceDesc sFFLResourceDesc;
static _GX2Sampler sMiiIconSampler;
static unsigned char* sUserWriteDataStart;
static unsigned char* sUserWriteDataCurrent;
static unsigned char* sUserWriteDataEnd;
static unsigned int sIconShaderType;
static void* sIconDefaultShaderBuffer;
static PlainShader sIconDefaultShader;
static PlainShader sIconExtShader;

void user_write_data_init(void* buffer, int size)
{
    sUserWriteDataStart = (unsigned char*)buffer;
    sUserWriteDataCurrent = (unsigned char*)buffer;
    sUserWriteDataEnd = (unsigned char*)buffer + size;
}

void user_write_data(png_struct_def*, unsigned char* data, unsigned int size)
{
    unsigned int available = (unsigned int)(sUserWriteDataEnd - sUserWriteDataCurrent);
    unsigned int count = size < available ? size : available;
    memcpy(sUserWriteDataCurrent, data, count);
    sUserWriteDataCurrent += count;
}

void user_flush_data(png_struct_def*)
{
}

int user_write_data_bytes_written()
{
    return (int)(sUserWriteDataCurrent - sUserWriteDataStart);
}

int InitIconShader__F14IconShaderType(IconShaderType shaderType)
{
    sIconShaderType = shaderType;
    if (shaderType == ICON_SHADER_TYPE_DEFAULT)
    {
        if (sIconDefaultShaderBuffer == NULL)
        {
            sIconDefaultShaderBuffer = malloc((size_t)FFLGetBufferSizeDefaultShader("Gfx2") + 0x100);
            if (sIconDefaultShaderBuffer == NULL)
            {
                OSPanic(__FILE__, 0x54, "Invalid pointer");
            }
        }

        unsigned char* aligned = (unsigned char*)(((uintptr_t)sIconDefaultShaderBuffer + 0xffu) & ~0xffu);
        if (aligned == NULL)
        {
            OSPanic(__FILE__, 0x5a, "Invalid pointer");
        }
        int result = FFLInitDefaultShader(&sIconDefaultShader, aligned, "Gfx2");
        if (result != 0)
        {
            OSPanic(__FILE__, 0x5d, "A default shader cannot be set up");
        }
        FFLSetDefaultShader(&sIconDefaultShader);
        return result;
    }

    if (shaderType != ICON_SHADER_TYPE_PLAIN)
    {
        return shaderType;
    }

    FFLiManager* manager = FFLiManager::GetInstance();
    void* contextState = manager->Allocate(0xa100, 0x100);
    GX2SetupContextStateEx((_GX2ContextState*)contextState, 1);
    GX2SetContextState((_GX2ContextState*)contextState);
    InitShader__FP11PlainShaderPCv(&sIconExtShader, gFFLIconShaderData);
    GX2InitSampler((_GX2Sampler*)&sIconExtShader.pixelSampler, 1, 1);
    sIconExtShader.vertexMatrix = (unsigned int)GetVertexUniformLocation__FPC16_GX2VertexShaderPCc(
        sIconExtShader.vertexShader, "u_mvp");
    sIconExtShader.pixelMode = (unsigned int)GetPixelUniformLocation__FPC15_GX2PixelShaderPCc(
        sIconExtShader.pixelShader, "u_mode");
    sIconExtShader.pixelColor1 = (unsigned int)GetPixelUniformLocation__FPC15_GX2PixelShaderPCc(
        sIconExtShader.pixelShader, "u_const1");
    sIconExtShader.pixelColor2 = (unsigned int)GetPixelUniformLocation__FPC15_GX2PixelShaderPCc(
        sIconExtShader.pixelShader, "u_const2");
    sIconExtShader.pixelColor3 = (unsigned int)GetPixelUniformLocation__FPC15_GX2PixelShaderPCc(
        sIconExtShader.pixelShader, "u_const3");
    return (int)GetPixelSamplerLocation__FPC15_GX2PixelShaderPCc(sIconExtShader.pixelShader, "s_texture");
}

void FinalizeDefaultShader()
{
    FFLDestroyDefaultShader(&sIconDefaultShader);
}

void FinalizeExtShader()
{
    DestroyShader__FP11PlainShader(&sIconExtShader);
}

void FinalizeIconShader__Fv()
{
    if (sIconShaderType == ICON_SHADER_TYPE_DEFAULT)
    {
        FinalizeDefaultShader();
        return;
    }
    if (sIconShaderType != ICON_SHADER_TYPE_PLAIN)
    {
        OSPanic(__FILE__, 0x73, "Invalid Condition");
        return;
    }
    FinalizeExtShader();
}

static Renderer sRenderer;

namespace
{
std::map<OSThread*, Renderer::Context*> s_rendererTlsContexts;

Renderer::Context*& currentRendererContext()
{
    return CafeThreadLocal::slot(s_rendererTlsContexts);
}
}

Renderer::Renderer()
{
    gRenderer = this;
}

void Renderer::Tick()
{
    CBuffTick();
}

void Renderer::UpdateGamma(unsigned short gamma)
{
    mGamma = gamma;
}

void Renderer::InitialiseContext(bool bReset)
{
    currentRendererContext() = new Context(this);
}

void Renderer::InitialiseContext()
{
    InitialiseContext(false);
}

Renderer::Context& Renderer::getContext()
{
    return *currentRendererContext();
}

void Renderer::SetupShaders()
{
    mVertexShaders = new _GX2VertexShader*[C4JRender::eVertexType_Count];
    mFetchShaders = new _GX2FetchShader*[C4JRender::eVertexType_Count];
    mVertexStrides = new unsigned int[C4JRender::eVertexType_Count];

    mVertexStrides[0] = 0x20;
    mFetchShaders[0] = new _GX2FetchShader;
    mVertexStrides[1] = 0x10;
    mFetchShaders[1] = new _GX2FetchShader;
    mVertexStrides[2] = 0x20;
    mFetchShaders[2] = new _GX2FetchShader;
    mVertexStrides[3] = 0x20;
    mFetchShaders[3] = new _GX2FetchShader;
    mVertexStrides[4] = 8;
    mFetchShaders[4] = new _GX2FetchShader;
    mVertexStrides[5] = 8;
    mFetchShaders[5] = new _GX2FetchShader;
    mVertexStrides[6] = 8;
    mFetchShaders[6] = new _GX2FetchShader;
    mVertexStrides[7] = 0x14;
    mFetchShaders[7] = new _GX2FetchShader;
    mVertexStrides[8] = 0x20;
    mFetchShaders[8] = new _GX2FetchShader;
    mVertexStrides[9] = 0x20;
    mFetchShaders[9] = new _GX2FetchShader;

    mVertexShaders[0] = &gVertexShaderStandard;
    mVertexShaders[1] = &gVertexShaderQuad;
    mVertexShaders[2] = &gVertexShaderStandardLit;
    mVertexShaders[3] = &gVertexShaderTerrain;
    mVertexShaders[4] = &gVertexShaderPosition;
    mVertexShaders[5] = &gVertexShaderPositionAlpha;
    mVertexShaders[6] = &gVertexShaderPositionLerp;
    mVertexShaders[7] = &gVertexShaderParticle;
    mVertexShaders[8] = &gVertexShaderLayered;
    mVertexShaders[9] = &gVertexShaderLayeredLit;

    mClearVertexShader = &gVertexShaderClear;
    mResolveVertexShader = &gVertexShaderScreenQuad;

    mPixelShaders = new _GX2PixelShader*[C4JRender::ePixelShaderType_Count];
    mPixelShaders[0] = &gPixelShaderTextured;
    mPixelShaders[1] = &gPixelShaderTexturedAlpha;
    mPixelShaders[2] = &gPixelShaderTexturedMask;
    mPixelShaders[3] = &gPixelShaderTexturedLerp;
    mPixelShaders[4] = &gPixelShaderTexturedFog;
    mPixelShaders[5] = &gPixelShaderTexturedForceLOD;
    mPixelShaders[6] = &gPixelShaderUntextured;
    mPixelShaders[7] = &gPixelShaderOrderIndependentAlpha;
    mPixelShaders[8] = &gPixelShaderLayered;
    mPixelShaders[9] = &gPixelShaderTexturedShadow;
    mPixelShaders[10] = &gPixelShaderTexturedGlow;
    mPixelShaders[11] = &gPixelShaderTexturedBlend;
    mPixelShaders[12] = &gPixelShaderTexturedModulate;
    mPixelShaders[13] = &gPixelShaderTexturedAdd;

    mClearPixelShader = &gPixelShaderClear;
    mCopyPixelShader = &gPixelShaderCopy;
    mGammaPixelShader = &gPixelShaderGamma;
    mAlphaAccumulatePixelShader = &gPixelShaderAlphaAccumulate;
    mAlphaResolvePixelShader = &gPixelShaderAlphaResolve;

    GX2InitFetchShaderEx(mFetchShaders[0], MemoryAllocMEM2(GX2CalcFetchShaderSizeEx(5, 0, 0), 0x100), 5,
                         gAttribStreamStandard, 0, 0);
    GX2InitFetchShaderEx(mFetchShaders[1], MemoryAllocMEM2(GX2CalcFetchShaderSizeEx(2, 0, 0), 0x100), 2,
                         gAttribStreamQuad, 0, 0);
    GX2InitFetchShaderEx(mFetchShaders[2], MemoryAllocMEM2(GX2CalcFetchShaderSizeEx(5, 0, 0), 0x100), 5,
                         gAttribStreamStandard, 0, 0);
    GX2InitFetchShaderEx(mFetchShaders[3], MemoryAllocMEM2(GX2CalcFetchShaderSizeEx(5, 0, 0), 0x100), 5,
                         gAttribStreamStandard, 0, 0);
    GX2InitFetchShaderEx(mFetchShaders[4], MemoryAllocMEM2(GX2CalcFetchShaderSizeEx(1, 0, 0), 0x100), 1,
                         gAttribStreamPosition, 0, 0);
    GX2InitFetchShaderEx(mFetchShaders[5], MemoryAllocMEM2(GX2CalcFetchShaderSizeEx(1, 0, 0), 0x100), 1,
                         gAttribStreamPosition, 0, 0);
    GX2InitFetchShaderEx(mFetchShaders[6], MemoryAllocMEM2(GX2CalcFetchShaderSizeEx(1, 0, 0), 0x100), 1,
                         gAttribStreamPosition, 0, 0);
    GX2InitFetchShaderEx(mFetchShaders[7], MemoryAllocMEM2(GX2CalcFetchShaderSizeEx(3, 0, 0), 0x100), 3,
                         gAttribStreamParticle, 0, 0);
    GX2InitFetchShaderEx(mFetchShaders[8], MemoryAllocMEM2(GX2CalcFetchShaderSizeEx(5, 0, 0), 0x100), 5,
                         gAttribStreamStandard, 0, 0);
    GX2InitFetchShaderEx(mFetchShaders[9], MemoryAllocMEM2(GX2CalcFetchShaderSizeEx(5, 0, 0), 0x100), 5,
                         gAttribStreamStandard, 0, 0);
}

void Renderer::Initialise(bool bFirstTime)
{
    unsigned int size;
    unsigned int scaleNeeded;
    unsigned int hiZSize;
    unsigned int hiZAlignment;

    if (bFirstTime)
    {
        InitialiseContext(true);

        mCommandBufferSlots = new short[0x800000];
        mCommandBuffers = new CommandBuffer*[16000];
        mCommandBufferMatrices = new float[16000 * 16];
        mCommandBufferIds = new int[16000];
        mCommandBufferVertexTypes = new unsigned char[16000];
        mCommandBufferPixelShaderTypes = new int[16000];
        mCommandBufferPrimitiveTypes = new unsigned char[16000];

        memset(mCommandBufferSlots, 0xFF, 0x1000000);
        memset(mCommandBuffers, 0, 64000);
        memset(mCommandBufferIds, 0, 64000);
        memset(mCommandBufferVertexTypes, 0, 16000);
        memset(mCommandBufferPixelShaderTypes, 0, 64000);
        memset(mCommandBufferPrimitiveTypes, 0, 16000);

        mNumCommandBuffers = 0;
        mScreenGrabRequested = false;
        mDRCFrameStarted = false;
        mNumRetiredCommandBuffers = 0;

        SetupShaders();

        unsigned int initAttributes[5];
        initAttributes[0] = 1;
        initAttributes[1] = 2;
        initAttributes[2] = 0x400000;
        initAttributes[3] = (unsigned int)MemoryAllocMEM2(0x400000, 0x40);
        initAttributes[4] = 0;
        GX2Init(initAttributes);

        float black[4];
        black[0] = 0.0f;
        black[1] = 0.0f;
        black[2] = 0.0f;
        black[3] = 0.0f;
        SetClearColour(black);

        mForegroundAcquired = true;
    }

    mDisplayEnabled = false;
    MemoryInit();

    mScreenWidth[0] = 0x500;
    mScreenHeight[0] = 0x2D0;
    mHiDef = true;
    if (GX2GetSystemTVScanMode() - 1U < 3)
    {
        mHiDef = false;
    }

    GX2CalcTVSize(3, GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, 2, &size, &scaleNeeded);
    mTVScanBuffer = MemoryAllocBucket(size, 0x1000);
    GX2Invalidate(0x40, mTVScanBuffer, size);
    GX2SetTVBuffer(mTVScanBuffer, size, 3, GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, 2);

    GX2CalcDRCSize(1, GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, 2, &size, &scaleNeeded);
    mDRCScanBuffer = MemoryAllocBucket(size, 0x1000);
    GX2Invalidate(0x40, mTVScanBuffer, size);
    GX2SetDRCBuffer(mDRCScanBuffer, size, 1, GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, 2);

    mColourBuffer.surface.use = 3;
    _GX2InitColorBuffer(&mColourBuffer, mScreenWidth[0], mScreenHeight[0],
                        GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, GX2_AA_MODE1X);
    void* colourImage = MemoryAllocMEM1(mColourBuffer.surface.imageSize, mColourBuffer.surface.alignment, false);
    GX2Invalidate(0x40, colourImage, mColourBuffer.surface.imageSize);
    mColourBuffer.surface.imagePtr = colourImage;

    GX2InitTexture(&mColourTexture, mScreenWidth[0], mScreenHeight[0], 1, 1,
                   GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, GX2_SURFACE_DIM_TEXTURE_2D);
    GX2CalcSurfaceSizeAndAlignment(&mColourTexture.surface);
    GX2InitTextureRegs(&mColourTexture);
    mColourTexture.surface.imagePtr = colourImage;
    mColourTexture.surface.mipPtr = NULL;
    if (mColourTexture.surface.numMips > 1)
    {
        mColourTexture.surface.mipPtr = (unsigned char*)colourImage + mColourTexture.surface.mipOffset[0];
    }

    GX2InitDepthBuffer(&mDepthBuffer, mScreenWidth[0], mScreenHeight[0], GX2_SURFACE_FORMAT_UNORM_D24_S8,
                       GX2_AA_MODE1X);
    void* depthImage = MemoryAllocMEM1(mDepthBuffer.surface.imageSize, mDepthBuffer.surface.alignment, false);
    GX2Invalidate(0x40, depthImage, mDepthBuffer.surface.imageSize);
    mDepthBuffer.surface.imagePtr = depthImage;
    GX2CalcDepthBufferHiZInfo(&mDepthBuffer, &hiZSize, &hiZAlignment);
    mDepthBuffer.hiZPtr = MemoryAllocMEM1(hiZSize, hiZAlignment, false);
    GX2InitDepthBufferHiZEnable(&mDepthBuffer, mDepthBuffer.hiZPtr != NULL);
    GX2InitDepthBufferHiZEnable(&mDepthBuffer, true);

    mScreenWidth[1] = 0x356;
    mDRCColourBuffer.surface.use = 3;
    mScreenHeight[1] = 0x1E0;
    _GX2InitColorBuffer(&mDRCColourBuffer, 0x356, 0x1E0, GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, GX2_AA_MODE1X);
    void* drcImage = MemoryAllocMEM1(mDRCColourBuffer.surface.imageSize, mDRCColourBuffer.surface.alignment, false);
    GX2Invalidate(0x40, drcImage, mDRCColourBuffer.surface.imageSize);
    mDRCColourBuffer.surface.imagePtr = drcImage;

    GX2InitTexture(&mDRCTexture, mScreenWidth[1], mScreenHeight[1], 1, 1, GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8,
                   GX2_SURFACE_DIM_TEXTURE_2D);
    GX2CalcSurfaceSizeAndAlignment(&mDRCTexture.surface);
    GX2InitTextureRegs(&mDRCTexture);
    mDRCTexture.surface.imagePtr = drcImage;
    mDRCTexture.surface.mipPtr = NULL;
    if (mDRCTexture.surface.numMips > 1)
    {
        mDRCTexture.surface.mipPtr = (unsigned char*)drcImage + mDRCTexture.surface.mipOffset[0];
    }

    GX2InitSamplerClamping(&mDRCSampler, 2, 2, 2);
    GX2InitSamplerXYFilter(&mDRCSampler, 1, 1, 0);
    GX2InitSamplerZMFilter(&mDRCSampler, 1, 2);

    GX2InitDepthBuffer(&mDRCDepthBuffer, mScreenWidth[1], mScreenHeight[1], GX2_SURFACE_FORMAT_UNORM_D24_S8,
                       GX2_AA_MODE1X);
    mDRCDepthBuffer.surface.imagePtr = depthImage;

    mAlphaColourBuffer.surface.use = 3;
    mScreen = 0;
    _GX2InitColorBuffer(&mAlphaColourBuffer, mScreenWidth[0], mScreenHeight[0],
                        GX2_SURFACE_FORMAT_FLOAT_R16_G16_B16_A16, GX2_AA_MODE1X);
    void* alphaImage = MemoryAllocMEM1(0x1600000, mAlphaColourBuffer.surface.alignment, true);
    GX2Invalidate(0x40, alphaImage, mAlphaColourBuffer.surface.imageSize);
    mAlphaColourBuffer.surface.imagePtr = alphaImage;

    GX2InitTexture(&mAlphaTexture, mScreenWidth[0], mScreenHeight[0], 1, 1,
                   GX2_SURFACE_FORMAT_FLOAT_R16_G16_B16_A16, GX2_SURFACE_DIM_TEXTURE_2D);
    mAlphaTexture.surface.imagePtr = alphaImage;
    mAlphaTexture.surface.mipPtr = NULL;
    if (mAlphaTexture.surface.numMips > 1)
    {
        mAlphaTexture.surface.mipPtr = (unsigned char*)alphaImage + mAlphaTexture.surface.mipOffset[0];
    }

    GX2InitSamplerClamping(&mAlphaSampler, 2, 2, 2);
    GX2InitSamplerXYFilter(&mAlphaSampler, 0, 0, 0);
    GX2InitSamplerZMFilter(&mAlphaSampler, 1, 2);

    GX2InitColorBufferFTV(&mTVColourBuffer, mScreenWidth[0], mScreenHeight[0],
                          GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, GX2_AA_MODE1X);
    mTVColourBuffer.surface.imagePtr = alphaImage;

    GX2InitTexture(&mTVTexture, mScreenWidth[0], mScreenHeight[0], 1, 1,
                   GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, GX2_SURFACE_DIM_TEXTURE_2D);
    mTVTexture.surface.imagePtr = alphaImage;
    mTVTexture.surface.mipPtr = NULL;
    if (mTVTexture.surface.numMips > 1)
    {
        mTVTexture.surface.mipPtr = (unsigned char*)alphaImage + mTVTexture.surface.mipOffset[0];
    }

    unsigned int thumbnailSize = 0x200;
    for (int thumbnail = 0; thumbnail < 4; ++thumbnail)
    {
        mThumbnailColourBuffer[thumbnail].surface.use = 3;
        _GX2InitColorBuffer(&mThumbnailColourBuffer[thumbnail], thumbnailSize, thumbnailSize,
                            GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, GX2_AA_MODE1X);
        mThumbnailColourBuffer[thumbnail].surface.tileMode = 1;
        GX2CalcSurfaceSizeAndAlignment(&mThumbnailColourBuffer[thumbnail].surface);
        GX2InitColorBufferRegs(&mThumbnailColourBuffer[thumbnail]);
        void* thumbnailImage = MemoryAllocBucket(mThumbnailColourBuffer[thumbnail].surface.imageSize,
                                                 mThumbnailColourBuffer[thumbnail].surface.alignment);
        GX2Invalidate(0x40, thumbnailImage, mThumbnailColourBuffer[thumbnail].surface.imageSize);
        mThumbnailColourBuffer[thumbnail].surface.imagePtr = thumbnailImage;

        GX2InitTexture(&mThumbnailTexture[thumbnail], thumbnailSize, thumbnailSize, 1, 1,
                       GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, GX2_SURFACE_DIM_TEXTURE_2D);
        mThumbnailTexture[thumbnail].surface.tileMode = 1;
        GX2CalcSurfaceSizeAndAlignment(&mThumbnailTexture[thumbnail].surface);
        GX2InitTextureRegs(&mThumbnailTexture[thumbnail]);
        mThumbnailTexture[thumbnail].surface.imagePtr = thumbnailImage;
        mThumbnailTexture[thumbnail].surface.mipPtr = NULL;
        if (mThumbnailTexture[thumbnail].surface.numMips > 1)
        {
            mThumbnailTexture[thumbnail].surface.mipPtr =
                (unsigned char*)thumbnailImage + mThumbnailTexture[thumbnail].surface.mipOffset[0];
        }

        thumbnailSize >>= 1;
    }

    if (bFirstTime)
    {
        mContextState = (_GX2ContextState*)MemoryAllocMEM2(0xA100, 0x100);
        GX2SetupContextStateEx(mContextState, true);
        memset(mTextures, 0, sizeof(mTextures));
        mDefaultTexture = TextureCreate();
        TextureBind(mDefaultTexture);

        unsigned char* white = new unsigned char[0x400];
        memset(white, 0xFF, 0x400);
        TextureData(0x10, 0x10, white, 0, C4JRender::eTextureFormat_RGBA8);
        delete[] white;

        mFrameCount = 0;
        mCapturingThumbnail = false;
        mGamma = 0x2AAA;
        OSFastMutex_Init(&mCommandBufferMutex, "commandBufferCS");
        OSFastMutex_Init(&sTotalAllocMutex, "totalAllocCS");
        mNextCommandBufferId = 0;
        mAllowStaticCommandBufferIds = true;
    }
    else
    {
        GX2SetContextState(mContextState);
    }

    GX2SetViewport(0.0f, 0.0f, (float)mScreenWidth[0], (float)mScreenHeight[0], 0.0f, 1.0f);
    GX2SetScissor(0, 0, mScreenWidth[0], mScreenHeight[0]);
    GX2SetSwapInterval(1);
    GX2SetShaderModeEx(0, 0x30, 0x40, 0, 0, 200, 0xC0);

    float one[4];
    one[0] = 1.0f;
    one[1] = 1.0f;
    one[2] = 1.0f;
    one[3] = 1.0f;
    memcpy(mColour, one, sizeof(mColour));
    memset(mColourAdd, 0, sizeof(mColourAdd));
    memcpy(mTexColour[0], one, sizeof(mTexColour[0]));
    memcpy(mTexColour[1], one, sizeof(mTexColour[1]));
    memcpy(mTexColour[2], one, sizeof(mTexColour[2]));
    memcpy(mTexColour[3], one, sizeof(mTexColour[3]));
    mPixelUniformsDirty = true;
    memset(mLayeredParams, 0, sizeof(mLayeredParams));
    memset(mLayeredTextureSize, 0, sizeof(mLayeredTextureSize));
    mLayeredParamsDirty = true;

    GX2SetViewport(0.0f, 0.0f, (float)mScreenWidth[0], (float)mScreenHeight[0], 0.0f, 1.0f);
    GX2SetScissor(0, 0, mScreenWidth[0], mScreenHeight[0]);
    StateSetVertexTextureUV(0.0f, 0.0f);
    GX2SetVertexTexture(&mTextures[mDefaultTexture].mTexture, 0);
    GX2SetVertexSampler(&mTextures[mDefaultTexture].mSampler, 0);
    mTextureStateDirty = false;
    mCurrentVertexShader = -1;
    mCurrentPixelShader = -1;
    mCommandBufferVertsDrawn = 0;
    mCommandBufferVertsDrawnLastFrame = 0;
}

void Renderer::Initialise()
{
    Initialise(true);
}

_GX2ColorBuffer* Renderer::GetColorBuffer()
{
    return &mColourBuffer;
}

_GX2DepthBuffer* Renderer::GetDepthStencilBuffer()
{
    return &mDepthBuffer;
}

_GX2ColorBuffer* Renderer::GetColorBufferDRC()
{
    return &mDRCColourBuffer;
}

_GX2DepthBuffer* Renderer::GetDepthStencilBufferDRC()
{
    return &mDRCDepthBuffer;
}

void Renderer::StartFrameInternal(int screen, bool bAlphaTest)
{
    mCurrentVertexShader = -1;
    mCurrentPixelShader = -1;
    TextureBindVertex(-1);
    TextureBind(-1);
    StateSetColour(1.0f, 1.0f, 1.0f, 1.0f);
    StateSetDepthFunc(3);
    StateSetDepthMask(true);
    StateSetStencil(7, 0, 0, 0);
    StateSetBlendEnable(true);
    StateSetBlendFunc(4, 5);
    StateSetBlendFactor(0xFFFFFFFF);
    StateSetAlphaTestEnable(false);
    StateSetAlphaFunc(4, 0.1f);
    StateSetFaceCull(true);
    StateSetLineWidth(1.0f);
    StateSetWriteEnable(true, true, true, true);
    StateSetDepthTestEnable(false);
    StateSetAlphaTestEnable(bAlphaTest);

    if (screen == 0)
    {
        GX2SetColorBuffer(&mColourBuffer, 0);
        GX2SetDepthBuffer(&mDepthBuffer);
        mScreen = 0;
    }
    else
    {
        GX2SetColorBuffer(&mDRCColourBuffer, 0);
        GX2SetDepthBuffer(&mDRCDepthBuffer);
        mScreen = screen;
    }

    GX2SetViewport(0.0f, 0.0f, (float)mScreenWidth[mScreen], (float)mScreenHeight[mScreen], 0.0f, 1.0f);
    GX2SetScissor(0, 0, mScreenWidth[mScreen], mScreenHeight[mScreen]);
}

void Renderer::StartFrame()
{
    StartFrameInternal(0, true);
}

void Renderer::ResetRenderState()
{
    StartFrameInternal(0, false);
}

void Renderer::GammaCorrect()
{
    Context& context = getContext();

    GX2SetVertexShader(mResolveVertexShader);
    GX2SetPixelShader(mGammaPixelShader);
    GX2SetDepthStencilControl(0, 0, 7, 1, 1, 7, 2, 0, 0, 7, 2, 0, 0);
    GX2SetCullOnlyControl(0, 0, 0);
    GX2SetColorControl(0xCC, 0, 0, 1);
    GX2SetPixelSampler(&mAlphaSampler, 0);

    float quad[4];
    quad[0] = 0.0f;
    quad[1] = 0.0f;
    quad[2] = 1.0f;
    quad[3] = 1.0f;
    GX2SetVertexUniformReg(0x80, 4, quad);

    float gamma[4];
    gamma[0] = 1.0f / ((float)mGamma * 3.0517578125e-05f * 1.5f + 0.5f);
    gamma[1] = 0.0f;
    gamma[2] = 0.0f;
    gamma[3] = 0.0f;
    GX2SetPixelUniformReg(0x18, 4, gamma);

    GX2SetViewport(0.0f, 0.0f, (float)mScreenWidth[0], (float)mScreenHeight[0], 0.0f, 1.0f);
    GX2SetPixelTexture(&mColourTexture, 0);
    GX2SetColorBuffer(&mTVColourBuffer, 0);
    GX2DrawEx(6, 4, 0, 1);

    GX2SetDepthStencilControl(context.mDepthTestEnable, context.mDepthMask, context.mDepthFunc, 1, 1,
                              context.mStencilFunc, 2, 0, 0, context.mStencilFunc, 2, 0, 0);
    GX2SetPolygonControl(0, 0, context.mFaceCull, 0, 2, 2, 1, 1, 0);
    GX2SetColorControl(0xCC, context.mBlendEnable, 0, 1);
    GX2SetBlendControl(0, context.mBlendSource, context.mBlendDestination, 0, 0, 0, 0, 0);
    GX2SetAlphaTest(1, 6, context.mAlphaTestEnable ? context.mAlphaTestRef : 0.0f);
    StateSetViewport(mCurrentViewport);

    if (mScreen == 0)
    {
        GX2SetColorBuffer(&mColourBuffer, 0);
    }

    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
}

void Renderer::CopyFrameToDRC()
{
    Context& context = getContext();

    GX2SetVertexShader(mResolveVertexShader);
    GX2SetPixelShader(mCopyPixelShader);
    GX2SetDepthStencilControl(0, 0, 7, 1, 1, 7, 2, 0, 0, 7, 2, 0, 0);
    GX2SetCullOnlyControl(0, 0, 0);
    GX2SetColorControl(0xCC, 0, 0, 1);
    GX2SetPixelSampler(&mDRCSampler, 0);

    float quad[4];
    quad[0] = 0.0f;
    quad[1] = 0.0f;
    quad[2] = 1.0f;
    quad[3] = 1.0f;
    GX2SetVertexUniformReg(0x80, 4, quad);

    GX2SetViewport(0.0f, 0.0f, (float)mScreenWidth[1], (float)mScreenHeight[1], 0.0f, 1.0f);
    GX2SetPixelTexture(&mTVTexture, 0);
    GX2SetColorBuffer(&mDRCColourBuffer, 0);
    GX2DrawEx(6, 4, 0, 1);

    GX2SetDepthStencilControl(context.mDepthTestEnable, context.mDepthMask, context.mDepthFunc, 1, 1,
                              context.mStencilFunc, 2, 0, 0, context.mStencilFunc, 2, 0, 0);
    GX2SetPolygonControl(0, 0, context.mFaceCull, 0, 2, 2, 1, 1, 0);
    GX2SetColorControl(0xCC, context.mBlendEnable, 0, 1);
    GX2SetBlendControl(0, context.mBlendSource, context.mBlendDestination, 0, 0, 0, 0, 0);
    GX2SetAlphaTest(1, 6, context.mAlphaTestEnable ? context.mAlphaTestRef : 0.0f);
    StateSetViewport(mCurrentViewport);

    if (mScreen == 0)
    {
        GX2SetColorBuffer(&mColourBuffer, 0);
    }

    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
}

void Renderer::RenderOnScreenKeyboard(bool bDRC)
{
    if (!InputManager_IsShowingOnscreenKeyboard() && !InputManager_IsShowingErrorViewer())
    {
        return;
    }

    GX2SetContextState(NULL);

    _GX2ColorBuffer* colourBuffer;
    if (bDRC)
    {
        colourBuffer = &mDRCColourBuffer;
    }
    else
    {
        colourBuffer = &mTVColourBuffer;
    }

    unsigned int format = colourBuffer->surface.format;
    colourBuffer->surface.format = format | 0x400;
    GX2InitColorBufferRegs(colourBuffer);
    GX2SetColorBuffer(colourBuffer, 0);

    if (InputManager_IsShowingOnscreenKeyboard())
    {
        if (bDRC)
        {
            nn::swkbd::DrawDRC();
        }
        else
        {
            nn::swkbd::DrawTV();
        }
    }

    if (InputManager_IsShowingErrorViewer())
    {
        if (bDRC)
        {
            nn::erreula::DrawDRC();
        }
        else
        {
            nn::erreula::DrawTV();
        }
    }

    colourBuffer->surface.format = format;
    GX2InitColorBufferRegs(colourBuffer);
    GX2SetColorBuffer(colourBuffer, 0);
}

void Renderer::StartDRCFrame(bool bCopyFrame)
{
    GammaCorrect();

    if (bCopyFrame)
    {
        CopyFrameToDRC();
    }

    RenderOnScreenKeyboard(false);
    StartFrameInternal(1, true);
    mDRCFrameStarted = true;
}

void Renderer::DoScreenGrabOnNextPresent()
{
    mScreenGrabRequested = true;
}

void Renderer::TEMP_PatchStateFromIggy()
{
    StateSetViewport(mCurrentViewport);
    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
}

void Renderer::Present()
{
    Context& context = getContext();

    if (mDRCFrameStarted)
    {
        RenderOnScreenKeyboard(true);
    }
    else
    {
        RenderOnScreenKeyboard(false);
        GammaCorrect();
    }

    mDRCFrameStarted = false;
    GX2Flush();
    GX2CopyColorBufferToScanBuffer(&mDRCColourBuffer, 4);
    GX2SetContextState(mContextState);
    GX2CopyColorBufferToScanBuffer(&mTVColourBuffer, 1);
    GX2SwapScanBuffers();
    GX2SetContextState(mContextState);
    GX2Flush();

    sPreviousSubmitTimeStamp = sLastSubmitTimeStamp;
    sLastSubmitTimeStamp = GX2GetLastSubmittedTimeStamp();

    if (mFrameCount > 0)
    {
        GX2WaitTimeStamp(sPreviousSubmitTimeStamp);
    }

    if (!mDisplayEnabled)
    {
        GX2SetTVEnable(1);
        GX2SetDRCEnable(1);
        mDisplayEnabled = true;
    }

    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
    ++mFrameCount;

    context.mDynamicVertexOffset = 0;
    context.mDynamicVertexBufferIndex = (context.mDynamicVertexBufferIndex + 1) % 3;

    mCommandBufferVertsDrawnLastFrame = mCommandBufferVertsDrawn;
    mCommandBufferVertsDrawn = 0;
}

void Renderer::Clear(int flags)
{
    Context& context = getContext();

    GX2SetVertexShader(mClearVertexShader);
    GX2SetPixelShader(mClearPixelShader);
    GX2SetStencilMask(0, 0, 0, 0, 0, 0);
    GX2SetDepthStencilControl(1, flags & 1, 7, 1, 1, 7, 2, 0, 0, 7, 2, 0, 0);
    GX2SetCullOnlyControl(0, 0, 0);
    GX2SetColorControl(0xCC, 0, 0, 1);
    GX2SetAlphaTest(0, 6, 0.0f);
    GX2SetTargetChannelMasks((flags & 2) != 0 ? 0xF : 0, 0, 0, 0, 0, 0, 0, 0);
    GX2SetPixelUniformReg(0x10, 4, mClearColour);
    GX2DrawEx(6, 4, 0, 1);

    GX2SetDepthStencilControl(context.mDepthTestEnable, context.mDepthMask, context.mDepthFunc, 1, 1,
                              context.mStencilFunc, 2, 0, 0, context.mStencilFunc, 2, 0, 0);
    GX2SetPolygonControl(0, 0, context.mFaceCull, 0, 2, 2, 1, 1, 0);
    GX2SetColorControl(0xCC, context.mBlendEnable, 0, 1);
    GX2SetBlendControl(0, context.mBlendSource, context.mBlendDestination, 0, 0, 0, 0, 0);
    GX2SetAlphaTest(1, 6, context.mAlphaTestEnable ? context.mAlphaTestRef : 0.0f);
    GX2SetTargetChannelMasks(0xF, 0, 0, 0, 0, 0, 0, 0);
    GX2SetStencilMask(0, 255, 0, 0, 255, 0);
    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
}

void Renderer::SetClearColour(const float* colour)
{
    mClearColour[0] = colour[0];
    mClearColour[1] = colour[1];
    mClearColour[2] = colour[2];
    mClearColour[3] = colour[3];
}

bool Renderer::IsWidescreen()
{
    return true;
}

bool Renderer::IsHiDef()
{
    return mHiDef;
}

void Renderer::ConvertLinearToPng(ImageFileBuffer* buffer, unsigned char* pixels, unsigned int width,
                                  unsigned int height)
{
    int capacity = (int)((height * width * 24) / 5) + 0x100;
    void* png = malloc(capacity);
    int written = 0;
    SaveTextureDataToMemory(png, capacity, &written, width, height, (int*)pixels);
    buffer->mFormat = 0;
    buffer->mData = png;
    buffer->mSize = written;
}

void Renderer::CaptureThumbnail(ImageFileBuffer* buffer)
{
    Context& context = getContext();

    float left = 0.0f;
    float right = 1.0f;
    float top = 0.0f;
    float bottom = 1.0f;

    switch (mCurrentViewport)
    {
    case C4JRender::eViewportType_FullScreen:
        break;
    case C4JRender::eViewportType_TopHalf:
        bottom = 0.5f;
        break;
    case C4JRender::eViewportType_BottomHalf:
        top = 0.5f;
        break;
    case C4JRender::eViewportType_LeftHalf:
    case C4JRender::eViewportType_RightHalf:
        right = 0.5f;
        break;
    case C4JRender::eViewportType_TopLeftQuarter:
        right = 0.5f;
        bottom = 0.5f;
        break;
    case C4JRender::eViewportType_TopRightQuarter:
        left = 0.5f;
        bottom = 0.5f;
        break;
    case C4JRender::eViewportType_BottomLeftQuarter:
        right = 0.5f;
        top = 0.5f;
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        left = 0.5f;
        top = 0.5f;
        break;
    default:
        break;
    }

    float aspect = IsWidescreen() ? 1.7777777910232544f : 1.3333333730697632f;
    float height = bottom - top;
    float scaledLeft = left * aspect;
    float scaledRight = right * aspect;
    float width = scaledRight - scaledLeft;

    if (width < height)
    {
        float inset = (height - width) * 0.5f;
        top = top + inset;
        height = (bottom - inset) - top;
    }
    else
    {
        float inset = (width - height) * 0.5f;
        scaledLeft = scaledLeft + inset;
        scaledRight = scaledRight - inset;
    }

    GX2SetVertexShader(mResolveVertexShader);
    GX2SetPixelShader(mCopyPixelShader);
    GX2SetDepthStencilControl(0, 0, 7, 1, 1, 7, 2, 0, 0, 7, 2, 0, 0);
    GX2SetCullOnlyControl(0, 0, 0);
    GX2SetColorControl(0xCC, 0, 0, 1);
    GX2SetPixelSampler(&mAlphaSampler, 0);

    unsigned int size = 0x200;
    float quad[4];
    quad[0] = scaledLeft / aspect;
    quad[1] = top;
    quad[2] = scaledRight / aspect - scaledLeft / aspect;
    quad[3] = height;
    GX2SetVertexUniformReg(0x80, 4, quad);
    GX2SetViewport(0.0f, 0.0f, (float)(int)size, (float)(int)size, 0.0f, 1.0f);
    GX2SetScissor(0, 0, size, size);
    GX2SetPixelTexture(&mColourTexture, 0);
    GX2SetColorBuffer(&mThumbnailColourBuffer[0], 0);
    GX2Flush();
    GX2Invalidate(0x12, mColourTexture.surface.imagePtr, mColourTexture.surface.imageSize);
    GX2DrawEx(6, 4, 0, 1);

    for (int level = 1; level < 4; ++level)
    {
        size >>= 1;
        quad[0] = 0.0f;
        quad[1] = 0.0f;
        quad[2] = 1.0f;
        quad[3] = 1.0f;
        GX2SetVertexUniformReg(0x80, 4, quad);
        GX2SetViewport(0.0f, 0.0f, (float)(int)size, (float)(int)size, 0.0f, 1.0f);
        GX2SetScissor(0, 0, size, size);
        GX2SetPixelTexture(&mThumbnailTexture[level - 1], 0);
        GX2SetColorBuffer(&mThumbnailColourBuffer[level], 0);
        GX2Flush();
        GX2Invalidate(0x12, mThumbnailTexture[level - 1].surface.imagePtr,
                      mThumbnailTexture[level - 1].surface.imageSize);
        GX2DrawEx(6, 4, 0, 1);
    }

    GX2DrawDone();
    GX2Invalidate(0x10, mThumbnailTexture[3].surface.imagePtr, mThumbnailTexture[3].surface.imageSize);
    ConvertLinearToPng(buffer, (unsigned char*)mThumbnailTexture[3].surface.imagePtr, 0x40, 0x40);

    GX2SetDepthStencilControl(context.mDepthTestEnable, context.mDepthMask, context.mDepthFunc, 1, 1,
                              context.mStencilFunc, 2, 0, 0, context.mStencilFunc, 2, 0, 0);
    GX2SetPolygonControl(0, 0, context.mFaceCull, 0, 2, 2, 1, 1, 0);
    GX2SetColorControl(0xCC, context.mBlendEnable, 0, 1);
    GX2SetBlendControl(0, context.mBlendSource, context.mBlendDestination, 0, 0, 0, 0, 0);
    GX2SetAlphaTest(1, 6, context.mAlphaTestEnable ? context.mAlphaTestRef : 0.0f);
    StateSetViewport(mCurrentViewport);
    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
}

void Renderer::CaptureScreen(ImageFileBuffer* buffer, _XSOCIAL_PREVIEWIMAGE* previewImage)
{
}

void Renderer::BeginConditionalSurvey(int survey)
{
}

void Renderer::EndConditionalSurvey()
{
}

void Renderer::BeginConditionalRendering(int survey)
{
}

void Renderer::EndConditionalRendering()
{
}

void Renderer::BeginOrderIndependentAlpha()
{
    Context& context = getContext();

    GX2SetColorBuffer(&mAlphaColourBuffer, 0);
    GX2SetColorBuffer(&mColourBuffer, 1);
    GX2SetVertexShader(mClearVertexShader);
    GX2SetPixelShader(mAlphaAccumulatePixelShader);
    GX2SetTargetChannelMasks(0xF, 8, 0, 0, 0, 0, 0, 0);
    GX2SetDepthStencilControl(0, 0, 7, 1, 1, 7, 2, 0, 0, 7, 2, 0, 0);
    GX2SetCullOnlyControl(0, 0, 0);
    GX2SetColorControl(0xCC, 0, 0, 1);
    GX2SetAlphaTest(0, 6, 0.0f);
    GX2DrawEx(6, 4, 0, 1);
    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
    GX2SetBlendControl(0, 1, 1, 0, 0, 0, 0, 0);
    GX2SetBlendControl(1, 0, 5, 0, 0, 0, 0, 0);
    GX2SetColorControl(0xCC, 3, 0, 1);
    GX2SetDepthStencilControl(1, 0, 3, 1, 1, 7, 2, 0, 0, 7, 2, 0, 0);
    GX2SetCullOnlyControl(0, 0, 1);
    context.mOrderIndependentAlpha = true;
}

void Renderer::EndOrderIndependentAlpha()
{
    Context& context = getContext();

    context.mOrderIndependentAlpha = false;
    GX2Invalidate(0x10, mAlphaColourBuffer.surface.imagePtr, mAlphaColourBuffer.surface.imageSize);
    GX2SetColorBuffer(&mColourBuffer, 0);
    GX2SetVertexShader(mResolveVertexShader);
    GX2SetPixelShader(mAlphaResolvePixelShader);
    GX2SetBlendControl(0, 7, 6, 0, 0, 0, 0, 0);
    GX2SetDepthStencilControl(0, 0, 7, 1, 1, 7, 2, 0, 0, 7, 2, 0, 0);
    GX2SetCullOnlyControl(0, 0, 0);
    GX2SetColorControl(0xCC, 3, 0, 1);

    float quad[4];
    quad[0] = context.mViewportScale[2];
    quad[1] = context.mViewportScale[3];
    quad[2] = context.mViewportScale[0];
    quad[3] = context.mViewportScale[1];
    GX2SetVertexUniformReg(0x80, 4, quad);

    GX2SetPixelTexture(&mAlphaTexture, 0);
    GX2SetPixelSampler(&mAlphaSampler, 0);
    GX2DrawEx(6, 4, 0, 1);

    GX2SetDepthStencilControl(context.mDepthTestEnable, context.mDepthMask, context.mDepthFunc, 1, 1,
                              context.mStencilFunc, 2, 0, 0, context.mStencilFunc, 2, 0, 0);
    GX2SetPolygonControl(0, 0, context.mFaceCull, 0, 2, 2, 1, 1, 0);
    GX2SetColorControl(0xCC, context.mBlendEnable, 0, 1);
    GX2SetBlendControl(0, context.mBlendSource, context.mBlendDestination, 0, 0, 0, 0, 0);
    GX2SetAlphaTest(1, 6, context.mAlphaTestEnable ? context.mAlphaTestRef : 0.0f);
    mCurrentPixelShader = -1;
    mCurrentVertexShader = -1;
}

void Renderer::ReleaseForeground()
{
    GX2Flush();
    GX2DrawDone();
    MemoryRelease();
    mForegroundAcquired = false;
    mDisplayEnabled = false;
    OSMemoryBarrier();
}

void Renderer::AcquiredForeground()
{
    mForegroundAcquired = true;
    OSMemoryBarrier();
    if (mContextState == 0)
        Initialise(true);
    else
        Initialise(false);
}

void Renderer::WiiUWaitDrawDone()
{
    if (!mForegroundAcquired)
    {
        return;
    }

    GX2DrawDone();
}

int Renderer::GetFramesSinceBoot()
{
    return mFrameCount;
}

void Renderer::BeginEvent(const wchar_t* name)
{
}

void Renderer::EndEvent()
{
}

int Renderer::GetNumCommandBufferVertsDrawnLastFrame()
{
    return mCommandBufferVertsDrawnLastFrame;
}

int Renderer::GetNumVertsInCommandBuffer(int id)
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    int slot = mCommandBufferSlots[id];
    int numVertices = 0;
    if (slot >= 0)
    {
        numVertices = mCommandBuffers[slot]->getNumVertices();
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
    return numVertices;
}

void Renderer::MatrixMode(int mode)
{
    Context& context = getContext();
    context.mMatrixMode = mode;
}

void Renderer::MatrixSetIdentity()
{
    Context& context = getContext();
    ASM_MTX44Identity(context.mMatrixStack[context.mMatrixMode][context.mMatrixStackDepth[context.mMatrixMode]]);
    context.mMatrixDirty[context.mMatrixMode] = true;
}

void Renderer::MultWithStack(float (*matrix)[4])
{
    Context& context = getContext();
    float (*top)[4] = context.mMatrixStack[context.mMatrixMode][context.mMatrixStackDepth[context.mMatrixMode]];
    ASM_MTX44Concat(matrix, top, top);
    context.mMatrixDirty[context.mMatrixMode] = true;
}

void Renderer::MatrixTranslate(float x, float y, float z)
{
    float matrix[4][4];
    ASM_MTX44Trans(matrix, x, y, z);
    ASM_MTX44Transpose(matrix, matrix);
    MultWithStack(matrix);
}

void Renderer::MatrixRotate(float angle, float x, float y, float z)
{
    _Vec3 axis;
    axis.x = x;
    axis.y = y;
    axis.z = z;

    float matrix[4][4];
    ASM_MTX44RotAxisRad(matrix, &axis, angle);
    ASM_MTX44Transpose(matrix, matrix);
    MultWithStack(matrix);
}

void Renderer::MatrixScale(float x, float y, float z)
{
    float matrix[4][4];
    ASM_MTX44Scale(matrix, x, y, z);
    MultWithStack(matrix);
}

void Renderer::MatrixPerspective(float fieldOfView, float aspect, float nearDistance, float farDistance)
{
    float matrix[4][4];
    C_MTXPerspective(matrix, fieldOfView, aspect, nearDistance, farDistance);
    ASM_MTX44Transpose(matrix, matrix);
    MultWithStack(matrix);
}

void Renderer::MatrixOrthogonal(float left, float right, float bottom, float top, float nearDistance,
                                float farDistance)
{
    float matrix[4][4];
    C_MTXOrtho(matrix, top, bottom, left, right, nearDistance, farDistance);
    ASM_MTX44Transpose(matrix, matrix);
    MultWithStack(matrix);
}

void Renderer::MatrixPop()
{
    Context& context = getContext();
    --context.mMatrixStackDepth[context.mMatrixMode];
    context.mMatrixDirty[context.mMatrixMode] = true;
}

void Renderer::MatrixPush()
{
    Context& context = getContext();
    float (*top)[4] = context.mMatrixStack[context.mMatrixMode][context.mMatrixStackDepth[context.mMatrixMode]];
    memcpy(top + 4, top, 0x40);
    ++context.mMatrixStackDepth[context.mMatrixMode];
}

void Renderer::MatrixMult(float* matrix)
{
    float local[4][4];
    memcpy(local, matrix, 0x40);
    MultWithStack(local);
}

float* Renderer::MatrixGet(int mode)
{
    Context& context = getContext();
    return &context.mMatrixStack[mode][context.mMatrixStackDepth[mode]][0][0];
}

void Renderer::Set_matrixDirty()
{
    Context& context = getContext();

    ASM_MTXIdentity(context.mMatrixStack[0][0]);
    ASM_MTXIdentity(context.mMatrixStack[1][0]);
    ASM_MTXIdentity(context.mMatrixStack[3][0]);
    ASM_MTXIdentity(context.mMatrixStack[2][0]);

    context.mMatrixDirty[2] = true;
    mCurrentVertexShader = -1;
    context.mMatrixDirty[1] = true;
    mCurrentPixelShader = -1;
    context.mMatrixDirty[3] = true;
    context.mMatrixDirty[0] = true;
}

void* Renderer::MemoryAllocMEM1(unsigned int size, unsigned int alignment, bool bReserved)
{
    if (alignment < 4)
    {
        alignment = 4;
    }

    if (!bReserved)
    {
        return MEMAllocFromExpHeapEx(mMEM1Heap, size, alignment);
    }

    if (mReservedMEM1 == NULL)
    {
        mReservedMEM1 = MEMAllocFromExpHeapEx(mMEM1Heap, size, alignment);
    }

    return mReservedMEM1;
}

void Renderer::MemoryFreeMEM1(void* memory)
{
    MEMFreeToExpHeap(mMEM1Heap, memory);
}

void* Renderer::MemoryAllocMEM2(unsigned int size, unsigned int alignment)
{
    if (alignment < 4)
    {
        alignment = 4;
    }

    return MEMAllocFromDefaultHeapEx(size, alignment);
}

void Renderer::MemoryFreeMEM2(void* memory)
{
    MEMFreeToDefaultHeap(memory);
}

void* Renderer::MemoryAllocBucket(unsigned int size, unsigned int alignment)
{
    return MEMAllocFromExpHeapEx(mBucketHeap, size, alignment);
}

void Renderer::MemoryFreeBucket(void* memory)
{
    MEMFreeToExpHeap(mBucketHeap, memory);
}

void Renderer::MemoryInit()
{
    MEMHeapHandle heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
    unsigned int size = MEMGetAllocatableSizeForFrmHeapEx(heap, 4);
    mMEM1Heap = MEMCreateExpHeapEx(MEMAllocFromFrmHeapEx(heap, size, 4), size, 0);

    heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_FG);
    size = MEMGetAllocatableSizeForFrmHeapEx(heap, 4);
    mBucketHeap = MEMCreateExpHeapEx(MEMAllocFromFrmHeapEx(heap, size, 4), size, 0);
}

void Renderer::MemoryRelease()
{
    MEMDestroyExpHeap(mMEM1Heap);
    MEMFreeToFrmHeap(MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1), MEM_FRM_HEAP_FREE_ALL);
    MEMDestroyExpHeap(mBucketHeap);
    MEMFreeToFrmHeap(MEMGetBaseHeapHandle(MEM_BASE_HEAP_FG), MEM_FRM_HEAP_FREE_ALL);
}

void Renderer::StateSetColour(float red, float green, float blue, float alpha)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL)
    {
        mColour[3] = alpha;
        mPixelUniformsDirty = true;
        mColour[0] = red;
        mColour[2] = blue;
        mColour[1] = green;
    }
    else if (context.mRecordingBuffer->mRecordState)
    {
        context.mRecordingBuffer->SetColor(red, green, blue, alpha);
    }
}

void Renderer::StateSetColourAdd(float red, float green, float blue, float alpha)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL)
    {
        mColourAdd[3] = alpha;
        mPixelUniformsDirty = true;
        mColourAdd[0] = red;
        mColourAdd[2] = blue;
        mColourAdd[1] = green;
    }
}

void Renderer::StateSetTexColour(int stage, float red, float green, float blue, float alpha)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL && (unsigned int)stage < 4)
    {
        mTexColour[stage][0] = red;
        mTexColour[stage][1] = green;
        mTexColour[stage][2] = blue;
        mTexColour[stage][3] = alpha;
        mPixelUniformsDirty = true;
    }
}

void Renderer::StateSetDepthMask(bool bDepthMask)
{
    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer == NULL)
    {
        context.mDepthMask = bDepthMask;
        GX2SetDepthStencilControl(context.mDepthTestEnable, bDepthMask, context.mDepthFunc, 1, 1,
                                  context.mStencilFunc, 2, 0, 0, context.mStencilFunc, 2, 0, 0);
    }
    else if (buffer->mRecordState)
    {
        buffer->SetDepthMask(bDepthMask);
    }
}

void Renderer::StateSetBlendEnable(bool bBlend)
{
    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer == NULL)
    {
        context.mBlendEnable = bBlend;
        GX2SetColorControl(0xCC, bBlend, 0, 1);
    }
    else if (buffer->mRecordState)
    {
        buffer->SetBlendEnable(bBlend);
    }
}

void Renderer::StateSetBlendFunc(int source, int destination)
{
    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer == NULL)
    {
        context.mBlendSource = source;
        context.mBlendDestination = destination;
        GX2SetBlendControl(0, source, destination, 0, 0, 0, 0, 0);
    }
    else if (buffer->mRecordState)
    {
        buffer->SetBlendFunc(source, destination);
    }
}

void Renderer::StateSetBlendFactor(unsigned int factor)
{
    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer == NULL)
    {
        GX2SetBlendConstantColor((float)(factor & 0xFF) / 255.0f, (float)((factor >> 8) & 0xFF) / 255.0f,
                                 (float)((factor >> 16) & 0xFF) / 255.0f, (float)(factor >> 24) / 255.0f);
    }
    else if (buffer->mRecordState)
    {
        buffer->SetBlendFactor(factor);
    }
}

void Renderer::StateSetAlphaFunc(int alphaFunc, float reference)
{
    Context& context = getContext();
    context.mAlphaTestRef = reference;
    GX2SetAlphaTest(1, 6, context.mAlphaTestEnable ? reference : 0.0f);
}

void Renderer::StateSetDepthFunc(int depthFunc)
{
    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer == NULL)
    {
        context.mDepthFunc = depthFunc;
        GX2SetDepthStencilControl(context.mDepthTestEnable, context.mDepthMask, depthFunc, 1, 1,
                                  context.mStencilFunc, 2, 0, 0, context.mStencilFunc, 2, 0, 0);
    }
    else if (buffer->mRecordState)
    {
        buffer->SetDepthFunc(depthFunc);
    }
}

void Renderer::StateSetFaceCull(bool bFaceCull)
{
    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer == NULL)
    {
        context.mFaceCull = bFaceCull;
        GX2SetPolygonControl(0, 0, bFaceCull, 0, 2, 2, 1, 1, 0);
    }
    else if (buffer->mRecordState)
    {
        buffer->SetFaceCull(bFaceCull);
    }
}

void Renderer::StateSetFaceCullCW(bool bClockwise)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL)
    {
        GX2SetPolygonControl(bClockwise ? 0 : 1, 0, context.mFaceCull, 0, 2, 2, 1, 1, 0);
    }
}

void Renderer::StateSetLineWidth(float width)
{
}

void Renderer::StateSetWriteEnable(bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
    getContext();
    GX2SetTargetChannelMasks((bRed ? 1 : 0) | (bGreen ? 2 : 0) | (bBlue ? 4 : 0) | (bAlpha ? 8 : 0), 0, 0, 0, 0,
                             0, 0, 0);
}

void Renderer::StateSetDepthTestEnable(bool bDepthTest)
{
    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer == NULL)
    {
        context.mDepthTestEnable = bDepthTest;
        GX2SetDepthStencilControl(bDepthTest, context.mDepthMask, context.mDepthFunc, 1, 1,
                                  context.mStencilFunc, 2, 0, 0, context.mStencilFunc, 2, 0, 0);
    }
    else if (buffer->mRecordState)
    {
        buffer->SetDepthTestEnable(bDepthTest);
    }
}

void Renderer::StateSetAlphaTestEnable(bool bAlphaTest)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL)
    {
        context.mAlphaTestEnable = bAlphaTest;
        GX2SetAlphaTest(1, 6, bAlphaTest ? context.mAlphaTestRef : 0.0f);
    }
}

void Renderer::StateSetDepthSlopeAndBias(float slope, float bias)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL)
    {
        GX2SetPolygonControl(0, 0, context.mFaceCull, 0, 2, 2, 1, 1, 0);
        GX2SetPolygonOffset(bias, slope, bias, slope, 0.0f);
    }
}

void Renderer::StateSetFogEnable(bool bFog)
{
    Context& context = getContext();
    context.mFogEnable = bFog;
}

void Renderer::StateSetFogMode(int mode)
{
    Context& context = getContext();
    context.mFogMode = mode;
}

void Renderer::StateSetFogNearDistance(float nearDistance)
{
    Context& context = getContext();
    context.mFogNearDistance = nearDistance;
}

void Renderer::StateSetFogFarDistance(float farDistance)
{
    Context& context = getContext();
    context.mFogFarDistance = farDistance;
}

void Renderer::StateSetFogDensity(float density)
{
    Context& context = getContext();
    context.mFogDensity = density;
}

void Renderer::StateSetFogColour(float red, float green, float blue)
{
    getContext();
    mFogColour[1] = green;
    mFogColour[3] = 1.0f;
    mPixelUniformsDirty = true;
    mFogColour[0] = red;
    mFogColour[2] = blue;
}

void Renderer::StateSetLightingEnable(bool bLighting)
{
    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer != NULL && buffer->mRecordState)
    {
        buffer->SetLightingEnable(bLighting);
        return;
    }

    context.mLightingEnable = bLighting;
}

void Renderer::StateSetVertexTextureUV(float u, float v)
{
    float uniform[4];
    uniform[0] = u - 1.0f;
    uniform[1] = v - 1.0f;
    uniform[2] = 0.0f;
    uniform[3] = 0.0f;
    GX2SetVertexUniformReg(0x40, 4, uniform);
}

void Renderer::StateSetTextureScaleUI(float scaleU, float scaleV, float offsetU, float offsetV)
{
    float uniform[4];
    uniform[0] = scaleU;
    uniform[1] = scaleV;
    uniform[2] = offsetU;
    uniform[3] = offsetV;
    GX2SetVertexUniformReg(0x84, 4, uniform);
}

void Renderer::StateSetScalingGridUI(float left, float right, float top, float bottom, float innerLeft,
                                     float innerRight, float innerTop, float innerBottom, float scaleU,
                                     float scaleV)
{
    float edges[4];
    float inner[4];
    float scale[4];

    edges[0] = left;
    edges[1] = right;
    edges[2] = top;
    edges[3] = bottom;
    inner[0] = innerLeft;
    inner[1] = innerRight;
    inner[2] = innerTop;
    inner[3] = innerBottom;
    scale[0] = scaleU;
    scale[1] = scaleV;
    scale[2] = 0.0f;
    scale[3] = 0.0f;

    GX2SetVertexUniformReg(0x88, 4, edges);
    GX2SetVertexUniformReg(0x8C, 4, inner);
    GX2SetVertexUniformReg(0x90, 4, scale);
}

void Renderer::StateSetLightColour(int light, float red, float green, float blue)
{
    if (light >= 2)
    {
        return;
    }

    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer == NULL)
    {
        context.mLightColour[light][0] = red;
        context.mLightColour[light][2] = blue;
        context.mLightColour[light][1] = green;
        context.mLightingDirty = true;
        context.mLightColour[light][3] = 1.0f;
    }
    else if (buffer->mRecordState)
    {
        buffer->SetLightColour(light, red, green, blue);
    }
}

void Renderer::StateSetLightAmbientColour(float red, float green, float blue)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL)
    {
        context.mLightAmbientColour[0] = red;
        context.mLightAmbientColour[2] = blue;
        context.mLightingDirty = true;
        context.mLightAmbientColour[3] = 1.0f;
        context.mLightAmbientColour[1] = green;
    }
    else if (context.mRecordingBuffer->mRecordState)
    {
        context.mRecordingBuffer->SetLightAmbientColour(red, green, blue);
    }
}

void Renderer::StateSetLightDirection(int light, float x, float y, float z)
{
    if (light >= 2)
    {
        return;
    }

    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer == NULL)
    {
        _Vec3 direction;
        direction.x = x;
        direction.y = y;
        direction.z = z;

        float transposed[4][4];
        ASM_MTX44Transpose(context.mMatrixStack[0][context.mMatrixStackDepth[0]], transposed);
        ASM_MTX44MultVecSR(transposed, &direction, &direction);
        ASM_VECNormalize(&direction, &direction);

        context.mLightDirection[light][0] = direction.x;
        context.mLightingDirty = true;
        context.mLightDirection[light][1] = direction.y;
        context.mLightDirection[light][3] = 0.0f;
        context.mLightDirection[light][2] = direction.z;
    }
    else if (buffer->mRecordState)
    {
        buffer->SetLightDirection(light, x, y, z);
    }
}

void Renderer::StateSetLightEnable(int light, bool bEnable)
{
    if (light >= 2)
    {
        return;
    }

    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer == NULL)
    {
        context.mLightEnable[light] = bEnable;
        context.mLightingDirty = true;
    }
    else if (buffer->mRecordState)
    {
        buffer->SetLightEnable(light, bEnable);
    }
}

void Renderer::StateSetViewport(C4JRender::eViewportType viewport)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        return;
    }

    mCurrentViewport = viewport;

    float screenWidth = (float)mScreenWidth[mScreen];
    float screenHeight = (float)mScreenHeight[mScreen];
    float x = 0.0f;
    float y = 0.0f;
    float width = screenWidth;
    float height = screenHeight;

    switch (viewport)
    {
    case C4JRender::eViewportType_FullScreen:
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = 1.0f / screenWidth;
        context.mViewportScale[3] = 1.0f / screenHeight;
        break;
    case C4JRender::eViewportType_TopHalf:
        height = screenHeight * 0.5f;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = 1.0f / screenWidth;
        context.mViewportScale[3] = 1.0f / screenHeight;
        break;
    case C4JRender::eViewportType_BottomHalf:
        height = screenHeight * 0.5f;
        y = height;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = 1.0f / screenWidth;
        context.mViewportScale[3] = height / screenHeight;
        break;
    case C4JRender::eViewportType_LeftHalf:
        width = screenWidth * 0.5f;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = 1.0f / screenWidth;
        context.mViewportScale[3] = 1.0f / screenHeight;
        break;
    case C4JRender::eViewportType_RightHalf:
        width = screenWidth * 0.5f;
        x = width;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = width / screenWidth;
        context.mViewportScale[3] = 1.0f / screenHeight;
        break;
    case C4JRender::eViewportType_TopLeftQuarter:
        width = screenWidth * 0.5f;
        height = screenHeight * 0.5f;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = 1.0f / screenWidth;
        context.mViewportScale[3] = 1.0f / screenHeight;
        break;
    case C4JRender::eViewportType_TopRightQuarter:
        width = screenWidth * 0.5f;
        height = screenHeight * 0.5f;
        x = width;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = width / screenWidth;
        context.mViewportScale[3] = 1.0f / screenHeight;
        break;
    case C4JRender::eViewportType_BottomLeftQuarter:
        width = screenWidth * 0.5f;
        height = screenHeight * 0.5f;
        y = height;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = 1.0f / screenWidth;
        context.mViewportScale[3] = height / screenHeight;
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        width = screenWidth * 0.5f;
        height = screenHeight * 0.5f;
        x = width;
        y = height;
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = width / screenWidth;
        context.mViewportScale[3] = height / screenHeight;
        break;
    default:
        context.mViewportScale[0] = width / screenWidth;
        context.mViewportScale[1] = height / screenHeight;
        context.mViewportScale[2] = x / screenWidth;
        context.mViewportScale[3] = y / screenHeight;
        break;
    }

    GX2SetViewport(x, y, width, height, 0.0f, 1.0f);
    GX2SetScissor((unsigned int)x, (unsigned int)y, (unsigned int)width, (unsigned int)height);
}

void Renderer::StateGetScissorRect(unsigned int& left, unsigned int& top, unsigned int& right,
                                   unsigned int& bottom)
{
    Context& context = getContext();
    left = context.mScissorRect[0];
    top = context.mScissorRect[1];
    right = context.mScissorRect[2];
    bottom = context.mScissorRect[3];
}

void Renderer::StateSetScissorRect(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom)
{
    Context& context = getContext();
    context.mScissorRect[2] = right;
    context.mScissorRect[0] = left;
    context.mScissorRect[1] = top;
    context.mScissorRect[3] = bottom;
    GX2SetScissor(left, top, right - left, bottom - top);
}

void Renderer::StateSetEnableViewportClipPlanes(bool bEnable)
{
}

void Renderer::Mtx44Vec4Transform(float (*matrix)[4], _Vec4* source, _Vec4* destination)
{
    destination->x = matrix[0][0] * source->x + matrix[0][1] * source->y + matrix[0][2] * source->z +
                     matrix[0][3] * source->w;
    destination->y = matrix[1][0] * source->x + matrix[1][1] * source->y + matrix[1][2] * source->z +
                     matrix[1][3] * source->w;
    destination->z = matrix[2][0] * source->x + matrix[2][1] * source->y + matrix[2][2] * source->z +
                     matrix[2][3] * source->w;
    destination->w = matrix[3][0] * source->x + matrix[3][1] * source->y + matrix[3][2] * source->z +
                     matrix[3][3] * source->w;
}

void Renderer::StateSetTexGenCol(int coordinate, float x, float y, float z, float w, bool bEyeLinear)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        return;
    }

    _Vec4 plane;
    plane.x = x;
    plane.y = y;
    plane.z = z;
    plane.w = w;

    if (bEyeLinear)
    {
        float modelView[4][4];
        memcpy(modelView, MatrixGet(0), 0x40);
        ASM_MTX44Transpose(modelView, modelView);

        float inverse[4][4];
        C_MTX44Inverse(modelView, inverse);

        float transposed[4][4];
        ASM_MTX44Transpose(inverse, transposed);

        _Vec4 transformed;
        Mtx44Vec4Transform(transposed, &plane, &transformed);
        plane = transformed;
    }

    int matrix = bEyeLinear ? 0 : 1;
    context.mTexGenMatrix[matrix][0][coordinate] = plane.x;
    context.mTexGenMatrix[matrix][1][coordinate] = plane.y;
    context.mTexGenMatrix[matrix][2][coordinate] = plane.z;
    context.mTexGenMatrix[matrix][3][coordinate] = plane.w;

    int other = bEyeLinear ? 1 : 0;
    context.mTexGenMatrix[other][0][coordinate] = 0.0f;
    context.mTexGenMatrix[other][1][coordinate] = 0.0f;
    context.mTexGenMatrix[other][2][coordinate] = 0.0f;
    context.mTexGenMatrix[other][3][coordinate] = 0.0f;
}

void Renderer::StateSetStencil(int stencilFunc, unsigned char reference, unsigned char compareMask,
                               unsigned char writeMask)
{
    Context& context = getContext();
    context.mStencilFunc = stencilFunc;

    if (context.mRecordingBuffer == NULL)
    {
        GX2SetDepthStencilControl(context.mDepthTestEnable, context.mDepthMask, context.mDepthFunc, 1, 1,
                                  stencilFunc, 2, 0, 0, stencilFunc, 2, 0, 0);
        GX2SetStencilMask(compareMask, writeMask, reference, compareMask, writeMask, reference);
    }
}

void Renderer::StateSetForceLOD(int lod)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL)
    {
        context.mForceLOD = lod;
    }
}

void Renderer::StateSetLerpColour(float red, float green, float blue, float alpha)
{
    Context& context = getContext();

    if (context.mRecordingBuffer == NULL)
    {
        mLerpColour[3] = alpha;
        mPixelUniformsDirty = true;
        mLerpColour[0] = red;
        mLerpColour[2] = blue;
        mLerpColour[1] = green;
    }
}

void Renderer::SetLayeredColour(int layer, float red, float green, float blue)
{
    Context& context = getContext();
    context.mLayeredColour[layer][0] = red;
    context.mLayeredColour[layer][1] = green;
    context.mLayeredColour[layer][2] = blue;
    context.mLayeredColour[layer][3] = 1.0f;
}

void Renderer::SetLayeredParam(int layer, int slot, float offset, float scale)
{
    Context& context = getContext();
    mLayeredParams[slot][0] = context.mLayeredColour[layer][0];
    mLayeredParams[slot][1] = context.mLayeredColour[layer][1];
    mLayeredParams[slot][3] = offset + scale;
    mLayeredParams[slot][2] = context.mLayeredColour[layer][2];
    mLayeredParamsDirty = true;
}

void Renderer::SetLayeredTextureSize(int width, int height)
{
    getContext();
    mLayeredParamsDirty = true;
    mLayeredTextureSize[0] = (float)width;
    mLayeredTextureSize[1] = (float)height;
}

void Renderer::StateSetBoneMatrixData(int bone)
{
    Context& context = getContext();
    memcpy(context.mBoneMatrix[bone], MatrixGet(0), 0x40);

    if (context.mNumBoneMatrices < bone + 1)
    {
        context.mNumBoneMatrices = bone + 1;
    }
}

void Renderer::StateSetBoneMatrixNull(int bone)
{
    Context& context = getContext();
    memset(context.mBoneMatrix[bone], 0, 0x40);

    if (context.mNumBoneMatrices < bone + 1)
    {
        context.mNumBoneMatrices = bone + 1;
    }
}

void Renderer::StateSetVertexAlphaDistance(float distance)
{
    Context& context = getContext();
    context.mVertexAlphaDistance = distance;
}

void Renderer::StateSetVertexAlpha(float alpha)
{
    Context& context = getContext();

    if (context.mVertexAlpha != alpha)
    {
        context.mVertexAlpha = alpha;
        GX2SetVertexUniformReg(0xA0, 4, &context.mVertexAlpha);
    }
}

void Renderer::UpdateFogState()
{
    Context& context = getContext();

    float uniform[4];
    uniform[0] = 0.0f;
    uniform[1] = 0.0f;
    uniform[2] = 0.0f;
    uniform[3] = 0.0f;

    if (!context.mFogEnable)
    {
        uniform[2] = 0.0f;
        GX2SetVertexUniformReg(0x44, 4, uniform);
        return;
    }

    if (context.mFogMode == 1)
    {
        uniform[0] = context.mFogFarDistance;
        uniform[1] = context.mFogNearDistance;
        uniform[2] = 1.0f;
        uniform[3] = context.mVertexAlphaDistance;
    }
    else
    {
        uniform[0] = context.mFogDensity;
        if (context.mFogMode == 2)
        {
            uniform[2] = 2.0f;
        }
        else
        {
            uniform[2] = 3.0f;
        }
        uniform[3] = context.mVertexAlphaDistance;
    }

    GX2SetVertexUniformReg(0x44, 4, uniform);
}

void Renderer::UpdateLightingState()
{
    Context& context = getContext();

    if (!context.mLightingDirty || !context.mLightingEnable)
    {
        return;
    }

    if (!context.mLightEnable[0])
    {
        memset(context.mLightDirection[0], 0, 16);
        memset(context.mLightColour[0], 0, 16);
    }

    if (!context.mLightEnable[1])
    {
        memset(context.mLightDirection[1], 0, 16);
        memset(context.mLightColour[1], 0, 16);
    }

    GX2SetVertexUniformReg(0x48, 0x14, context.mLightDirection);
    context.mLightingDirty = false;
}

void Renderer::UpdateViewportState()
{
}

void Renderer::UpdateTexGenState()
{
    Context& context = getContext();
    GX2SetVertexUniformReg(0x5C, 0x10, context.mTexGenMatrix[0]);
    GX2SetVertexUniformReg(0x6C, 0x10, context.mTexGenMatrix[1]);
}

void Renderer::StateUpdate()
{
    Context& context = getContext();
    StateSetFaceCull(context.mFaceCull);
    StateSetDepthMask(context.mDepthMask);
    StateSetDepthTestEnable(context.mDepthTestEnable);
    StateSetAlphaTestEnable(context.mAlphaTestEnable);
}

int Renderer::GetMaxTextures()
{
    return 512;
}

int Renderer::TextureCreate()
{
    for (int texture = 0; texture < 512; ++texture)
    {
        if (!mTextures[texture].mUsed)
        {
            mTextures[texture].mTexture.surface.imagePtr = NULL;
            mTextures[texture].mTexture.surface.mipPtr = NULL;
            mTextures[texture].mFlags = 0;
            mTextures[texture].mUsed = true;
            mTextures[texture].mLevels = 1;
            GX2InitSampler(&mTextures[texture].mSampler, 0, 0);
            return texture;
        }
    }

    return -1;
}

void Renderer::TextureFree(int texture)
{
    if (mTextures[texture].mTexture.surface.imagePtr != NULL)
    {
        MemoryFreeMEM2(mTextures[texture].mTexture.surface.imagePtr);
    }

    if (mTextures[texture].mTexture.surface.mipPtr != NULL)
    {
        MemoryFreeMEM2(mTextures[texture].mTexture.surface.mipPtr);
    }

    mTextures[texture].mTexture.surface.imagePtr = NULL;
    mTextures[texture].mTexture.surface.mipPtr = NULL;
    mTextures[texture].mUsed = false;
}

void Renderer::TextureBind(int texture)
{
    TextureBind(0, texture);
}

void Renderer::UpdateTextureState(int stage)
{
    Context& context = getContext();
    Texture& texture = mTextures[context.mBoundTexture[stage]];

    if (texture.mTexture.surface.imagePtr == NULL)
    {
        return;
    }

    if (context.mTextureIsVertex[stage])
    {
        GX2SetVertexTexture(&texture.mTexture, stage);
        GX2SetVertexSampler(&texture.mSampler, stage);
    }
    else
    {
        GX2SetPixelTexture(&texture.mTexture, stage);
        GX2SetPixelSampler(&texture.mSampler, stage);
    }
}

void Renderer::TextureBind(int stage, int texture)
{
    if (texture == -1)
    {
        texture = mDefaultTexture;
    }

    Context& context = getContext();
    CommandBuffer* buffer = context.mRecordingBuffer;

    if (buffer != NULL && buffer->mRecordState)
    {
        buffer->BindTexture(texture);
    }

    context.mBoundTexture[stage] = texture;
    context.mTextureIsVertex[stage] = false;
    context.mTextureIsMask[stage] = false;
    UpdateTextureState(stage);
}

void Renderer::TextureBindMask(int stage)
{
    Context& context = getContext();
    context.mTextureIsMask[stage] = true;
}

void Renderer::TextureBindVertex(int texture)
{
    if (texture == -1)
    {
        texture = mDefaultTexture;
    }

    Context& context = getContext();
    context.mBoundTexture[0] = texture;
    context.mTextureIsMask[0] = false;
    context.mTextureIsVertex[0] = true;
    UpdateTextureState(0);
}

void Renderer::TextureSetTextureLevels(int levels)
{
    Context& context = getContext();
    mTextures[context.mBoundTexture[0]].mLevels = levels;
}

int Renderer::TextureGetTextureLevels()
{
    Context& context = getContext();
    return mTextures[context.mBoundTexture[0]].mLevels;
}

void Renderer::TextureData(int width, int height, void* data, int level, C4JRender::eTextureFormat format)
{
    Context& context = getContext();
    Texture& texture = mTextures[context.mBoundTexture[0]];
    texture.mFormat = format;

    if (level == 0)
    {
        GX2InitTexture(&texture.mTexture, width, height, 1, texture.mLevels,
                       GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, GX2_SURFACE_DIM_TEXTURE_2D);
        texture.mTexture.surface.tileMode = 1;
        GX2CalcSurfaceSizeAndAlignment(&texture.mTexture.surface);
        GX2InitTextureRegs(&texture.mTexture);

        void* image = NULL;
        void* mipImage = NULL;

        if (texture.mTexture.surface.imageSize != 0)
        {
            image = MemoryAllocMEM2(texture.mTexture.surface.imageSize, texture.mTexture.surface.alignment);
        }

        if (texture.mTexture.surface.mipSize != 0)
        {
            mipImage = MemoryAllocMEM2(texture.mTexture.surface.mipSize, texture.mTexture.surface.alignment);
        }

        texture.mTexture.surface.imagePtr = image;

        if (texture.mTexture.surface.numMips > 1 && mipImage == NULL)
        {
            mipImage = (unsigned char*)image + texture.mTexture.surface.mipOffset[0];
        }

        texture.mTexture.surface.mipPtr = mipImage;

        unsigned int sourcePitch = texture.mTexture.surface.width * 4;
        unsigned int destinationPitch = texture.mTexture.surface.pitch * 4;
        unsigned char* destination = (unsigned char*)image;
        unsigned char* source = (unsigned char*)data;

        for (unsigned int row = 0; row < texture.mTexture.surface.height; ++row)
        {
            memcpy(destination, source, sourcePitch);
            source += sourcePitch;
            destination += destinationPitch;
        }

        GX2Invalidate(0x40, texture.mTexture.surface.imagePtr, texture.mTexture.surface.imageSize);
    }
    else
    {
        unsigned int mipOffset = 0;
        if (level != 1)
        {
            mipOffset = texture.mTexture.surface.mipOffset[level - 1];
        }

        unsigned int rows = texture.mTexture.surface.height >> level;
        unsigned int sourcePitch = (texture.mTexture.surface.width >> level) * 4;
        unsigned int destinationPitch = GX2GetSurfaceMipPitch(&texture.mTexture.surface, level) * 4;
        unsigned char* destination = (unsigned char*)texture.mTexture.surface.mipPtr + mipOffset;
        unsigned char* source = (unsigned char*)data;

        for (unsigned int row = 0; row < rows; ++row)
        {
            memcpy(destination, source, sourcePitch);
            source += sourcePitch;
            destination += destinationPitch;
        }

        GX2Invalidate(0x40, (unsigned char*)texture.mTexture.surface.mipPtr + mipOffset,
                      destinationPitch * rows);
    }
}

void Renderer::TextureDataUpdate(int x, int y, int width, int height, void* data, int level)
{
    Context& context = getContext();
    Texture& texture = mTextures[context.mBoundTexture[0]];

    if (x != 0 || y != 0)
    {
        return;
    }

    unsigned int levelWidth = texture.mTexture.surface.width >> level;
    if ((unsigned int)width != levelWidth)
    {
        return;
    }

    unsigned int levelHeight = texture.mTexture.surface.height >> level;
    if ((unsigned int)height != levelHeight)
    {
        return;
    }

    if (level == 0)
    {
        unsigned int sourcePitch = texture.mTexture.surface.width * 4;
        unsigned int destinationPitch = texture.mTexture.surface.pitch * 4;
        unsigned char* destination = (unsigned char*)texture.mTexture.surface.imagePtr;
        unsigned char* source = (unsigned char*)data;

        if (sourcePitch == destinationPitch)
        {
            GX2Invalidate(0x40, destination, texture.mTexture.surface.imageSize);
            GX2Invalidate(0x40, source, texture.mTexture.surface.imageSize);
            DMAETimeStamp timestamp = DMAECopyMem(destination, source,
                                                  texture.mTexture.surface.imageSize >> 2,
                                                  DMAE_SWAP_NONE);
            DMAEWaitDone(timestamp);
            return;
        }

        for (unsigned int row = 0; row < texture.mTexture.surface.height; ++row)
        {
            memcpy(destination, source, sourcePitch);
            destination += destinationPitch;
            source += sourcePitch;
        }

        GX2Invalidate(0x40, texture.mTexture.surface.imagePtr, texture.mTexture.surface.imageSize);
        return;
    }

    unsigned int mipOffset = 0;
    if (level != 1)
    {
        mipOffset = texture.mTexture.surface.mipOffset[level - 1];
    }

    unsigned char* destination = (unsigned char*)texture.mTexture.surface.mipPtr + mipOffset;
    unsigned int sourcePitch = levelWidth * 4;
    unsigned int destinationPitch = GX2GetSurfaceMipPitch(&texture.mTexture.surface, level) * 4;
    unsigned char* source = (unsigned char*)data;
    unsigned char* row = destination;

    for (unsigned int line = 0; line < levelHeight; ++line)
    {
        memcpy(row, source, sourcePitch);
        source += sourcePitch;
        row += destinationPitch;
    }

    GX2Invalidate(0x40, destination, destinationPitch * levelHeight);
}

void Renderer::TextureSetParam(int parameter, int value)
{
    Context& context = getContext();

    if (parameter == 0)
    {
        return;
    }

    Texture& texture = mTextures[context.mBoundTexture[0]];
    unsigned int flags = texture.mFlags;

    if (parameter == 1)
    {
        flags &= ~4u;
        texture.mFlags = flags;
        if (value == 1)
        {
            flags |= 4u;
            texture.mFlags = flags;
        }
    }
    else if (parameter == 2)
    {
        flags &= ~8u;
        texture.mFlags = flags;
        if (value == 1)
        {
            flags |= 8u;
            texture.mFlags = flags;
        }
    }
    else if (parameter == 3)
    {
        flags &= ~1u;
        texture.mFlags = flags;
        if (value == 0)
        {
            flags |= 1u;
            texture.mFlags = flags;
        }
    }
    else if (parameter == 4)
    {
        flags &= ~2u;
        texture.mFlags = flags;
        if (value == 0)
        {
            flags |= 2u;
            texture.mFlags = flags;
        }
    }

    GX2InitSamplerClamping(&texture.mSampler, (flags & 1) != 0 ? 2 : 0, (flags & 2) != 0 ? 2 : 0, 2);
    flags = mTextures[context.mBoundTexture[0]].mFlags;
    GX2InitSamplerXYFilter(&mTextures[context.mBoundTexture[0]].mSampler, (flags >> 3) & 1, (flags >> 2) & 1, 0);
    GX2InitSamplerZMFilter(&mTextures[context.mBoundTexture[0]].mSampler, 1, 2);
    UpdateTextureState(0);
}

void Renderer::TextureDynamicUpdateStart()
{
}

void Renderer::TextureDynamicUpdateEnd()
{
}

int Renderer::LoadTextureData(const char* fileName, D3DXIMAGE_INFO* info, int** data)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = 1;

    int file = CreateFileA(fileName, 0x80000000, 0, NULL, 3, 0x80, 0);
    if (file == -1)
    {
        return -1;
    }

    unsigned int fileSize = GetFileSize(file, NULL);
    void* fileData = malloc(fileSize);
    ReadFile(file, fileData, fileSize, NULL, NULL);
    png_image_begin_read_from_memory(&image, fileData, fileSize);
    CloseHandle(file);

    if ((image.warning_or_error & 3) >= 2)
    {
        return -1;
    }

    image.format = 0x23;
    *data = (int*)new unsigned char[image.height * image.width * 4];

    if (*data == NULL || !png_image_finish_read(&image, NULL, *data, 0, NULL))
    {
        free(fileData);
        return -1;
    }

    free(fileData);
    info->Width = image.width;
    info->Height = image.height;
    return 0;
}

int Renderer::LoadTextureData(unsigned char* fileData, unsigned int fileSize, D3DXIMAGE_INFO* info, int** data)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = 1;

    png_image_begin_read_from_memory(&image, fileData, fileSize);

    if ((image.warning_or_error & 3) >= 2)
    {
        return -1;
    }

    image.format = 0x23;
    *data = (int*)new unsigned char[image.height * image.width * 4];

    if (*data == NULL || !png_image_finish_read(&image, NULL, *data, 0, NULL))
    {
        return -1;
    }

    info->Width = image.width;
    info->Height = image.height;
    return 0;
}

int Renderer::SaveTextureData(const char* fileName, D3DXIMAGE_INFO* info, int* data)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.height = info->Height;
    image.width = info->Width;
    image.format = 3;
    image.version = 1;
    png_image_write_to_file(&image, fileName, 0, data, 0, NULL);
    return 0;
}

int Renderer::SaveTextureDataToMemory(void* destination, int destinationSize, int* bytesWritten, int width,
                                      int height, int* data)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = 1;
    image.format = 3;
    image.width = width;
    image.height = height;
    user_write_data_init(destination, destinationSize);
    png_image_write_to_stdio(&image, NULL, 0, data, 0, NULL,
                             reinterpret_cast<void*>(user_write_data), reinterpret_cast<void*>(user_flush_data));
    *bytesWritten = user_write_data_bytes_written();
    return 0;
}

void Renderer::TextureGetStats()
{
}

_GX2Texture* Renderer::TextureGetTexture(int texture)
{
    if ((unsigned int)texture < 512 && mTextures[texture].mUsed)
    {
        return &mTextures[texture].mTexture;
    }

    return NULL;
}

C4JRender::ePixelShaderType Renderer::FinalisePixelShaderType(C4JRender::ePixelShaderType pixelShaderType)
{
    Context& context = getContext();

    if (pixelShaderType != C4JRender::ePixelShaderType_Default)
    {
        return pixelShaderType;
    }

    if (context.mOrderIndependentAlpha)
    {
        return C4JRender::ePixelShaderType_OrderIndependentAlpha;
    }

    if (context.mBoundTexture[1] == mDefaultTexture)
    {
        return C4JRender::ePixelShaderType_Default;
    }

    if (context.mBoundTexture[2] == mDefaultTexture)
    {
        if (context.mTextureIsMask[1])
        {
            return C4JRender::ePixelShaderType_LayeredColourMask;
        }

        return C4JRender::ePixelShaderType_DefaultTwoLayers;
    }

    if (context.mBoundTexture[3] == mDefaultTexture)
    {
        return C4JRender::ePixelShaderType_DefaultThreeLayers;
    }

    return C4JRender::ePixelShaderType_DefaultFourLayers;
}

void Renderer::TextureSetTexture(int texture, _GX2Texture& source)
{
    mTextures[texture].mTexture = source;
}

void Renderer::TextureSetSampler(int texture, _GX2Sampler& source)
{
    mTextures[texture].mSampler = source;
}

void* Renderer::GetDynamicVertexMemory(C4JRender::eVertexType vertexType, int numVertices)
{
    Context& context = getContext();

    unsigned int offset = context.mDynamicVertexOffset;
    unsigned char* buffer = (unsigned char*)context.mDynamicVertexBuffer[context.mDynamicVertexBufferIndex];
    unsigned int used = offset + ((mVertexStrides[vertexType] * numVertices + 0x1F) & 0xFFFFFFE0);
    context.mDynamicVertexOffset = used;

    if (used < 0x200000)
    {
        return buffer + offset;
    }

    return NULL;
}

void Renderer::DrawVertexSetup(C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType pixelShaderType,
                               C4JRender::ePrimitiveType primitiveType)
{
    Context& context = getContext();

    int vertexShader = vertexType;
    if (vertexType == C4JRender::eVertexType_Default && context.mLightingEnable)
    {
        vertexShader = C4JRender::eVertexType_DefaultLighting;
    }

    if (vertexShader != mCurrentVertexShader)
    {
        GX2SetFetchShader(mFetchShaders[vertexShader]);
        GX2SetVertexShader(mVertexShaders[vertexShader]);
        mCurrentVertexShader = vertexShader;
    }

    C4JRender::ePixelShaderType finalPixelShader = FinalisePixelShaderType(pixelShaderType);

    if (finalPixelShader == C4JRender::ePixelShaderType_LayeredColourMask &&
        (mCurrentPixelShader != C4JRender::ePixelShaderType_LayeredColourMask || mLayeredParamsDirty))
    {
        GX2SetPixelUniformReg(0x24, 0x80, mLayeredTextureSize);
        mLayeredParamsDirty = false;
    }

    if (finalPixelShader != mCurrentPixelShader || mPixelUniformsDirty)
    {
        GX2SetPixelUniformReg(0, 0x20, mColour);
        mPixelUniformsDirty = false;
    }

    if (finalPixelShader != mCurrentPixelShader)
    {
        GX2SetPixelShader(mPixelShaders[finalPixelShader]);
        mCurrentPixelShader = finalPixelShader;
    }

    if (context.mMatrixDirty[0])
    {
        GX2SetVertexUniformReg(0, 0x10, MatrixGet(0));
        context.mMatrixDirty[0] = false;
    }

    if (context.mMatrixDirty[1])
    {
        GX2SetVertexUniformReg(0x20, 0x10, MatrixGet(1));
        context.mMatrixDirty[1] = false;
    }

    if (context.mMatrixDirty[2])
    {
        GX2SetVertexUniformReg(0x30, 0x10, MatrixGet(2));
        context.mMatrixDirty[2] = false;
    }

    float identity[4][4];
    ASM_MTX44Identity(identity);
    GX2SetVertexUniformReg(0x10, 0x10, identity);

    UpdateFogState();
    UpdateViewportState();
    UpdateLightingState();
    UpdateTexGenState();
}

void Renderer::DrawVertices(C4JRender::ePrimitiveType primitiveType, int numVertices, void* vertices,
                            C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType pixelShaderType,
                            unsigned char* layers)
{
    Context& context = getContext();

    if (context.mRecordingBuffer != NULL)
    {
        int recordedVertexType = vertexType;
        if (vertexType == C4JRender::eVertexType_Default && context.mLightingEnable)
        {
            recordedVertexType = C4JRender::eVertexType_DefaultLighting;
        }

        context.mRecordingVertexType = (C4JRender::eVertexType)recordedVertexType;
        context.mRecordingPrimitiveType = primitiveType;
        context.mRecordingPixelShaderType = pixelShaderType;
        context.mRecordingBuffer->AddVertices(mVertexStrides[recordedVertexType], numVertices, vertices, context,
                                              layers);
        return;
    }

    void* destination = GetDynamicVertexMemory(vertexType, numVertices);
    if (destination == NULL)
    {
        return;
    }

    DrawVertexSetup(vertexType, pixelShaderType, primitiveType);

    unsigned int size = numVertices * mVertexStrides[vertexType];
    memcpy(destination, vertices, size);
    GX2Invalidate(0x40, destination, size);
    StateUpdate();
    GX2SetAttribBuffer(0, size, mVertexStrides[vertexType], destination);
    GX2DrawEx(sPrimitiveModes[primitiveType], numVertices, 0, 1);
}

bool Renderer::InitFFL()
{
    unsigned int workSize = FFLGetWorkSize(&sFFLResourceDesc);
    sFFLWorkMemory = malloc(workSize);

    if (FFLInitRes(sFFLWorkMemory, &sFFLResourceDesc) != 0)
    {
        return false;
    }

    FFLInitResGPUStep();
    return true;
}

void Renderer::ShutdownFFL()
{
    FFLExit();
    free(sFFLWorkMemory);
}

void* Renderer::CreateMiiIconTexture(FFLDataSource dataSource, FFLStoreData* storeData, int index, int* texture)
{
    OSReport("Setup FFL...\n");

    int created = gRenderer->TextureCreate();
    _GX2Texture* miiTexture = gRenderer->TextureGetTexture(created);
    void* result = MakeIcon(miiTexture, (IconShaderType)0, dataSource, storeData, index);
    GX2Invalidate(0x42, miiTexture->surface.imagePtr, miiTexture->surface.imageSize);
    GX2InitSampler(&sMiiIconSampler, 2, 1);
    gRenderer->TextureSetTexture(created, *miiTexture);
    *texture = created;
    return result;
}

void Renderer::FreeMiiIconTexture(int texture, void* memory)
{
    free(memory);
    mTextures[texture].mTexture.surface.imagePtr = NULL;
    mTextures[texture].mUsed = false;
    mTextures[texture].mTexture.surface.mipPtr = NULL;
}

void Renderer::CBuffLockStaticCreations()
{
    mAllowStaticCommandBufferIds = false;
}

int Renderer::CBuffCreate(int count)
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    int first = mNextCommandBufferId;
    for (;;)
    {
        if (first > 0x7FFFFF)
        {
            OSFastMutex_Unlock(&mCommandBufferMutex);
            return -1;
        }

        bool bFree = true;
        for (int index = 0; index < count; ++index)
        {
            if (mCommandBufferSlots[first + index] != -1)
            {
                bFree = false;
                break;
            }
        }

        if (bFree)
        {
            break;
        }

        ++first;
    }

    for (int index = 0; index < count; ++index)
    {
        mCommandBufferSlots[first + index] = -2;
    }

    if (mAllowStaticCommandBufferIds)
    {
        mNextCommandBufferId = first + count;
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
    return first;
}

void Renderer::DeleteInternalBuffer(int slot)
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    int retired = mNumRetiredCommandBuffers + 1;
    int retiredSlot = 16000 - retired;
    mCommandBuffers[retiredSlot] = mCommandBuffers[slot];
    mNumRetiredCommandBuffers = retired;
    memcpy(&mCommandBufferMatrices[retiredSlot * 16], &mCommandBufferMatrices[slot * 16], 0x40);

    int last = mNumCommandBuffers - 1;
    mNumCommandBuffers = last;

    if (last != 0)
    {
        mCommandBuffers[slot] = mCommandBuffers[last];
        memcpy(&mCommandBufferMatrices[slot * 16], &mCommandBufferMatrices[last * 16], 0x40);
        mCommandBufferVertexTypes[slot] = mCommandBufferVertexTypes[mNumCommandBuffers];
        mCommandBufferPixelShaderTypes[slot] = mCommandBufferPixelShaderTypes[mNumCommandBuffers];
        mCommandBufferPrimitiveTypes[slot] = mCommandBufferPrimitiveTypes[mNumCommandBuffers];
        int id = mCommandBufferIds[mNumCommandBuffers];
        mCommandBufferIds[slot] = id;
        mCommandBufferSlots[id] = (short)slot;
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
}

void Renderer::CBuffDelete(int firstId, int count)
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    for (int index = 0; index < count; ++index)
    {
        if (mCommandBufferSlots[firstId + index] >= 0)
        {
            DeleteInternalBuffer(mCommandBufferSlots[firstId + index]);
        }

        mCommandBufferSlots[firstId + index] = -1;
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
}

void Renderer::CBuffStart(int id, bool bRecordState)
{
    Context& context = getContext();

    CommandBuffer* buffer = new CommandBuffer(bRecordState);
    context.mMatrixMode = 3;
    context.mRecordingBuffer = buffer;
    context.mMatrixStackDepth[3] = 0;
    context.mRecordingId = id;
    ASM_MTX44Identity(context.mMatrixStack[3][0]);
    context.mMatrixDirty[3] = true;
}

void Renderer::CBuffClear(int id)
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    if (mCommandBufferSlots[id] >= 0)
    {
        DeleteInternalBuffer(mCommandBufferSlots[id]);
        mCommandBufferSlots[id] = -2;
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
}

int Renderer::CBuffSize(int id)
{
    if (id == -1)
    {
        return sTotalCommandBufferBytes < 0 ? 0 : sTotalCommandBufferBytes;
    }

    OSFastMutex_Lock(&mCommandBufferMutex);

    int size = 0;
    int slot = mCommandBufferSlots[id];
    if (slot >= 0)
    {
        size = mCommandBuffers[slot]->mVerticesCapacity;
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
    return size;
}

void Renderer::CBuffEnd()
{
    Context& context = getContext();

    OSFastMutex_Lock(&mCommandBufferMutex);

    if (context.mDeferredMode)
    {
        context.mDeferredBuffers.push_back(DeferredCBuff(context.mRecordingId, context.mRecordingBuffer,
                                                         context.mRecordingVertexType,
                                                         context.mRecordingPixelShaderType,
                                                         context.mRecordingPrimitiveType,
                                                         context.mMatrixStack[3][0]));
        context.mMatrixMode = 0;
        context.mRecordingBuffer->EndRecording(mVertexStrides[context.mRecordingVertexType]);
        context.mRecordingBuffer = NULL;
        OSFastMutex_Unlock(&mCommandBufferMutex);
        return;
    }

    if (mCommandBufferSlots[context.mRecordingId] >= 0)
    {
        DeleteInternalBuffer(mCommandBufferSlots[context.mRecordingId]);
    }

    if (mNumCommandBuffers + mNumRetiredCommandBuffers + 10 > 16000)
    {
        OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\MinecraftConsoles-Maintenance-"
                "LegacyPlatforms\\4JLibraries_Source\\WiiU\\Render\\RendererCBuff.cpp",
                749, "Out of command buffers");
    }

    int slot = mNumCommandBuffers;
    mNumCommandBuffers = slot + 1;
    mCommandBufferIds[slot] = context.mRecordingId;
    mCommandBufferSlots[context.mRecordingId] = (short)slot;
    mCommandBufferVertexTypes[slot] = (unsigned char)context.mRecordingVertexType;
    mCommandBufferPixelShaderTypes[slot] = context.mRecordingPixelShaderType;
    mCommandBufferPrimitiveTypes[slot] = (unsigned char)context.mRecordingPrimitiveType;
    mCommandBuffers[slot] = context.mRecordingBuffer;
    memcpy(&mCommandBufferMatrices[slot * 16], context.mMatrixStack[3][0], 0x40);

    context.mMatrixMode = 0;
    context.mRecordingBuffer->EndRecording(mVertexStrides[context.mRecordingVertexType]);
    context.mRecordingBuffer = NULL;
    OSFastMutex_Unlock(&mCommandBufferMutex);
}

bool Renderer::CBuffCall(int id, bool bApplyMatrix, unsigned char layerMask)
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    int slot = mCommandBufferSlots[id];
    if (slot < 0)
    {
        OSFastMutex_Unlock(&mCommandBufferMutex);
        return false;
    }

    Context& context = getContext();
    int vertexType = mCommandBufferVertexTypes[slot];
    unsigned char primitiveType = mCommandBufferPrimitiveTypes[slot];

    if (!bApplyMatrix)
    {
        mCommandBuffers[slot]->Render((C4JRender::eVertexType)vertexType, context, primitiveType, false,
                                      layerMask);
        OSFastMutex_Unlock(&mCommandBufferMutex);
        return true;
    }

    if (context.mMatrixDirty[0])
    {
        GX2SetVertexUniformReg(0, 0x10, MatrixGet(0));
        context.mMatrixDirty[0] = false;
    }

    if (context.mMatrixDirty[1])
    {
        GX2SetVertexUniformReg(0x20, 0x10, MatrixGet(1));
        context.mMatrixDirty[1] = false;
    }

    if (context.mMatrixDirty[2])
    {
        GX2SetVertexUniformReg(0x30, 0x10, MatrixGet(2));
        context.mMatrixDirty[2] = false;
    }

    if (context.mNumBoneMatrices > 0)
    {
        GX2SetVertexUniformReg(0xA4, context.mNumBoneMatrices << 4, context.mBoneMatrix);
        context.mNumBoneMatrices = 0;
    }

    UpdateFogState();
    UpdateLightingState();

    if (vertexType == C4JRender::eVertexType_Default && context.mLightingEnable)
    {
        vertexType = C4JRender::eVertexType_DefaultLighting;
    }
    else if (vertexType == C4JRender::eVertexType_DefaultLighting && !context.mLightingEnable)
    {
        vertexType = C4JRender::eVertexType_Default;
    }
    else if (vertexType == C4JRender::eVertexType_Entity && context.mLightingEnable)
    {
        vertexType = C4JRender::eVertexType_EntityLighting;
    }
    else if (vertexType == C4JRender::eVertexType_EntityLighting && !context.mLightingEnable)
    {
        vertexType = C4JRender::eVertexType_Entity;
    }

    UpdateViewportState();
    UpdateTexGenState();

    if (vertexType != mCurrentVertexShader)
    {
        GX2SetFetchShader(mFetchShaders[vertexType]);
        GX2SetVertexShader(mVertexShaders[vertexType]);
        mCurrentVertexShader = vertexType;
    }

    C4JRender::ePixelShaderType pixelShaderType =
        (C4JRender::ePixelShaderType)mCommandBufferPixelShaderTypes[slot];

    if (context.mForceLOD >= 0)
    {
        float forceLOD[4];
        forceLOD[0] = (float)context.mForceLOD;
        forceLOD[1] = 0.0f;
        forceLOD[2] = 0.0f;
        forceLOD[3] = 0.0f;
        pixelShaderType = C4JRender::ePixelShaderType_TerrainForceLod;
        GX2SetPixelUniformReg(0x20, 4, forceLOD);
    }

    C4JRender::ePixelShaderType finalPixelShader = FinalisePixelShaderType(pixelShaderType);

    if (finalPixelShader != mCurrentPixelShader || mPixelUniformsDirty)
    {
        GX2SetPixelUniformReg(0, 0x20, mColour);
        mPixelUniformsDirty = false;
    }

    if (finalPixelShader == C4JRender::ePixelShaderType_LayeredColourMask &&
        (mCurrentPixelShader != C4JRender::ePixelShaderType_LayeredColourMask || mLayeredParamsDirty))
    {
        GX2SetPixelUniformReg(0x24, 0x80, mLayeredTextureSize);
        mLayeredParamsDirty = false;
    }

    if (finalPixelShader != mCurrentPixelShader)
    {
        GX2SetPixelShader(mPixelShaders[finalPixelShader]);
        mCurrentPixelShader = finalPixelShader;
    }

    mCommandBuffers[slot]->Render((C4JRender::eVertexType)vertexType, context, primitiveType, bApplyMatrix,
                                  layerMask);

    MultWithStack((float (*)[4])&mCommandBufferMatrices[slot * 16]);
    ASM_MTX44Identity(context.mMatrixStack[3][0]);
    context.mMatrixDirty[3] = true;

    OSFastMutex_Unlock(&mCommandBufferMutex);
    return true;
}

void Renderer::CBuffTick()
{
    OSFastMutex_Lock(&mCommandBufferMutex);

    int remaining = mNumRetiredCommandBuffers;
    int examined = 0;
    int slot = 15999;

    while (examined < remaining)
    {
        CommandBuffer* buffer = mCommandBuffers[slot];

        if (buffer->IsBusy())
        {
            ++examined;
            --slot;
            continue;
        }

        delete buffer;
        remaining = mNumRetiredCommandBuffers - 1;
        mNumRetiredCommandBuffers = remaining;
        mCommandBuffers[slot] = NULL;

        if (remaining == 0)
        {
            ++examined;
            --slot;
            break;
        }

        mCommandBuffers[slot] = mCommandBuffers[15999 - remaining];
    }

    OSFastMutex_Unlock(&mCommandBufferMutex);
}

void Renderer::CBuffDeferredModeStart()
{
    Context& context = getContext();
    context.mDeferredMode = true;
}

void Renderer::CBuffDeferredModeEnd()
{
    Context& context = getContext();

    if (!context.mDeferredMode)
    {
        return;
    }

    OSFastMutex_Lock(&mCommandBufferMutex);
    context.mDeferredMode = false;

    for (unsigned int index = 0; index < context.mDeferredBuffers.size(); ++index)
    {
        const DeferredCBuff& deferred = context.mDeferredBuffers[index];

        if (mCommandBufferSlots[deferred.mId] >= 0)
        {
            DeleteInternalBuffer(mCommandBufferSlots[deferred.mId]);
        }

        if (mNumCommandBuffers + mNumRetiredCommandBuffers + 10 > 16000)
        {
            OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\MinecraftConsoles-Maintenance-"
                    "LegacyPlatforms\\4JLibraries_Source\\WiiU\\Render\\RendererCBuff.cpp",
                    709, "Out of command buffers");
        }

        int slot = mNumCommandBuffers;
        mNumCommandBuffers = slot + 1;
        mCommandBufferIds[slot] = deferred.mId;
        mCommandBufferSlots[deferred.mId] = (short)slot;
        mCommandBufferVertexTypes[slot] = (unsigned char)deferred.mVertexType;
        mCommandBufferPixelShaderTypes[slot] = deferred.mPixelShaderType;
        mCommandBufferPrimitiveTypes[slot] = (unsigned char)deferred.mPrimitiveType;
        mCommandBuffers[slot] = deferred.mBuffer;
        memcpy(&mCommandBufferMatrices[slot * 16], deferred.mMatrix, 0x40);
    }

    context.mDeferredBuffers.clear();
    OSFastMutex_Unlock(&mCommandBufferMutex);
}

Renderer::Context::Context(Renderer* renderer)
{
    ASM_MTXIdentity(mMatrixStack[0][0]);
    mMatrixStackDepth[0] = 0;
    ASM_MTXIdentity(mMatrixStack[1][0]);
    mMatrixStackDepth[1] = 0;
    ASM_MTXIdentity(mMatrixStack[2][0]);
    mMatrixStackDepth[2] = 0;
    ASM_MTXIdentity(mMatrixStack[3][0]);
    mMatrixStackDepth[3] = 0;

    mTextureIsVertex[0] = false;
    mBoundTexture[3] = 0;
    mAlphaTestEnable = false;
    mAlphaTestRef = 1.0f;
    mTextureIsMask[2] = false;
    mTextureIsMask[3] = false;
    mTextureIsMask[1] = false;
    mTextureIsMask[0] = false;
    mTextureIsVertex[2] = false;
    mTextureIsVertex[3] = false;
    mTextureIsVertex[1] = false;
    mMatrixMode = 0;
    mBoundTexture[0] = 0;
    mFaceCull = true;
    mDeferredMode = false;
    mDepthTestEnable = true;
    mBoundTexture[1] = 0;
    mBoundTexture[2] = 0;
    mDepthFunc = 7;
    mStencilFunc = 7;
    mFogEnable = false;
    mRecordingBuffer = NULL;
    mLightingEnable = false;
    mDepthMask = true;
    mForceLOD = -1;

    memset(mLightDirection, 0, 80);

    mLightingDirty = false;
    mDynamicVertexBuffer[0] = renderer->MemoryAllocMEM2(0x200000, 0x20);
    mDynamicVertexBuffer[1] = renderer->MemoryAllocMEM2(0x200000, 0x20);
    mDynamicVertexBuffer[2] = renderer->MemoryAllocMEM2(0x200000, 0x20);

    mViewportScale[1] = 1.0f;
    mVertexAlpha = 0.0f;
    mViewportScale[2] = 0.0f;
    mVertexAlphaDistance = 0.0f;
    mViewportScale[3] = 0.0f;
    mViewportScale[0] = 1.0f;
    mOrderIndependentAlpha = false;
    mDynamicVertexOffset = 0;
    mDynamicVertexBufferIndex = 0;
}

Renderer::DeferredCBuff::DeferredCBuff(int id, CommandBuffer* buffer, int vertexType,
                                       C4JRender::ePixelShaderType pixelShaderType, int primitiveType,
                                       float (*matrix)[4])
{
    mPixelShaderType = pixelShaderType;
    mVertexType = vertexType;
    mBuffer = buffer;
    mId = id;
    mPrimitiveType = primitiveType;
    memcpy(mMatrix, matrix, 0x40);
}

Renderer::CommandBuffer::CommandBuffer(bool bRecordState)
{
    mRecordState = bRecordState;
    mVerticesUsed = 0;
    mVertices = (unsigned char*)malloc(0x1000);
    mVerticesCapacity = 0x1000;

    OSFastMutex_Lock(&sTotalAllocMutex);
    sTotalCommandBufferBytes += mVerticesCapacity;
    OSFastMutex_Unlock(&sTotalAllocMutex);

    mLastRenderedFrame = 0;
}

Renderer::CommandBuffer::~CommandBuffer()
{
    free(mVertices);

    OSFastMutex_Lock(&sTotalAllocMutex);
    sTotalCommandBufferBytes -= mVerticesCapacity;
    OSFastMutex_Unlock(&sTotalAllocMutex);
}

void Renderer::CommandBuffer::StartRecording()
{
}

void Renderer::CommandBuffer::EndRecording(unsigned int vertexStride)
{
    if (mVerticesUsed != 0)
    {
        GX2Invalidate(0x40, mVertices, mVerticesUsed);
        return;
    }

    free(mVertices);
    mVertices = NULL;
}

void Renderer::CommandBuffer::Render(C4JRender::eVertexType vertexType, Context& context, int primitiveType,
                                     bool bApplyMatrix, unsigned char layerMask)
{
    if (mVertices == NULL)
    {
        return;
    }

    unsigned int batchOffset[32];
    int batchCount[32];
    unsigned int numBatches = 0;
    bool bMatrixPushed = false;
    int shaderVertexType = vertexType;

    for (unsigned int index = 0; index < mCommands.size(); ++index)
    {
        const Command& command = mCommands[index];

        if (command.mType == eCommandType_Matrix)
        {
            if (shaderVertexType == C4JRender::eVertexType_Terrain)
            {
                if (!bApplyMatrix && command.mMatrix[13] == context.mLastQuadUniform[2] &&
                    command.mMatrix[14] == context.mLastQuadUniform[3])
                {
                    continue;
                }

                GX2SetVertexUniformReg(0x48, 4, &command.mMatrix[12]);
                context.mLastQuadUniform[2] = command.mMatrix[13];
                context.mLastQuadUniform[3] = command.mMatrix[14];
            }
            else
            {
                GX2SetVertexUniformReg(0x10, 0x10, command.mMatrix);
                bMatrixPushed = true;
            }
        }
        else if (command.mType == eCommandType_Vertices)
        {
            if (mRecordState)
            {
                gRenderer->UpdateLightingState();

                if (shaderVertexType == C4JRender::eVertexType_Default && context.mLightingEnable)
                {
                    shaderVertexType = C4JRender::eVertexType_DefaultLighting;
                }
                else if (shaderVertexType == C4JRender::eVertexType_DefaultLighting &&
                         !context.mLightingEnable)
                {
                    shaderVertexType = C4JRender::eVertexType_Default;
                }
                else if (shaderVertexType == C4JRender::eVertexType_Entity && context.mLightingEnable)
                {
                    shaderVertexType = C4JRender::eVertexType_EntityLighting;
                }
                else if (shaderVertexType == C4JRender::eVertexType_EntityLighting &&
                         !context.mLightingEnable)
                {
                    shaderVertexType = C4JRender::eVertexType_Entity;
                }

                if (shaderVertexType != gRenderer->mCurrentVertexShader)
                {
                    GX2SetFetchShader(gRenderer->mFetchShaders[shaderVertexType]);
                    GX2SetVertexShader(gRenderer->mVertexShaders[shaderVertexType]);
                    gRenderer->mCurrentVertexShader = shaderVertexType;
                }
            }

            if (shaderVertexType == C4JRender::eVertexType_Terrain)
            {
                if (numBatches != 0 &&
                    command.mVertices.mOffset == batchOffset[numBatches - 1] +
                    batchCount[numBatches - 1] * gRenderer->mVertexStrides[C4JRender::eVertexType_Terrain])
                {
                    batchCount[numBatches - 1] += command.mVertices.mCount;
                }
                else
                {
                    batchOffset[numBatches] = command.mVertices.mOffset;
                    batchCount[numBatches] = command.mVertices.mCount;
                    ++numBatches;
                }
            }
            else
            {
                GX2SetAttribBuffer(0, mVerticesUsed, gRenderer->mVertexStrides[shaderVertexType], mVertices);
                GX2DrawEx(sPrimitiveModes[primitiveType], command.mVertices.mCount,
                          command.mVertices.mOffset / gRenderer->mVertexStrides[shaderVertexType], 1);
                gRenderer->mCommandBufferVertsDrawn += command.mVertices.mCount;
            }
        }
    }

    GX2SetAttribBuffer(0, mVerticesUsed, gRenderer->mVertexStrides[shaderVertexType], mVertices);

    for (unsigned int batch = 0; batch < numBatches; ++batch)
    {
        GX2DrawEx(sPrimitiveModes[primitiveType], batchCount[batch],
                  batchOffset[batch] / gRenderer->mVertexStrides[shaderVertexType], 1);
        gRenderer->mCommandBufferVertsDrawn += batchCount[batch];
    }

    if (bMatrixPushed)
    {
        float identity[4][4];
        ASM_MTX44Identity(identity);
        GX2SetVertexUniformReg(0x10, 0x10, identity);
    }

    mLastRenderedFrame = gRenderer->mFrameCount;
}

bool Renderer::CommandBuffer::IsBusy()
{
    return gRenderer->mFrameCount - 3 <= mLastRenderedFrame;
}

void Renderer::CommandBuffer::AddMatrix(const float* matrix)
{
    Command command;
    command.mType = eCommandType_Matrix;
    memcpy(command.mMatrix, matrix, 0x40);
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::AddVertices(unsigned int vertexStride, unsigned int numVertices, void* vertices,
                                          Context& context, unsigned char* layers)
{
    if (context.mMatrixDirty[3])
    {
        AddMatrix(gRenderer->MatrixGet(3));
        context.mMatrixDirty[3] = false;
    }

    Command command;
    unsigned int offset = mVerticesUsed;
    command.mType = eCommandType_Vertices;
    mVerticesUsed = offset + numVertices * vertexStride;
    command.mVertices.mOffset = offset;
    command.mVertices.mCount = numVertices;

    if (mVerticesCapacity < mVerticesUsed)
    {
        OSFastMutex_Lock(&sTotalAllocMutex);
        sTotalCommandBufferBytes -= mVerticesCapacity;
        OSFastMutex_Unlock(&sTotalAllocMutex);

        mVerticesCapacity = (mVerticesUsed + 0xFFF) & 0xFFFFF000;
        mVertices = (unsigned char*)realloc(mVertices, mVerticesCapacity);

        OSFastMutex_Lock(&sTotalAllocMutex);
        sTotalCommandBufferBytes += mVerticesCapacity;
        OSFastMutex_Unlock(&sTotalAllocMutex);
    }

    if (layers == NULL)
    {
        memcpy(mVertices + offset, vertices, numVertices * vertexStride);
        command.mVertices.mLayer = 0;
        mCommands.push_back(command);
        return;
    }

    unsigned int quadSize = vertexStride * 4;
    unsigned int destination = offset;

    for (unsigned char layer = 0; layer < 7; ++layer)
    {
        unsigned int count = 0;

        for (unsigned int quad = 0; quad < numVertices / 4; ++quad)
        {
            if (layers[quad] == layer)
            {
                memcpy(mVertices + destination, (unsigned char*)vertices + quad * quadSize, quadSize);
                count += 4;
                destination += quadSize;
            }
        }

        if (count > 0)
        {
            command.mVertices.mLayer = layer;
            command.mVertices.mOffset = offset;
            command.mVertices.mCount = count;
            mCommands.push_back(command);
        }

        offset = destination;
    }
}

void Renderer::CommandBuffer::BindTexture(int texture)
{
    Command command;
    command.mType = eCommandType_BindTexture;
    command.mTexture = texture;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetColor(float red, float green, float blue, float alpha)
{
    Command command;
    command.mColour[1] = green;
    command.mColour[2] = blue;
    command.mColour[0] = red;
    command.mType = eCommandType_Colour;
    command.mColour[3] = alpha;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetDepthFunc(int depthFunc)
{
    Command command;
    command.mType = eCommandType_DepthFunc;
    command.mDepthFunc = depthFunc;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetDepthMask(bool bDepthMask)
{
    Command command;
    command.mType = eCommandType_DepthMask;
    command.mEnable = bDepthMask;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetDepthTestEnable(bool bDepthTest)
{
    Command command;
    command.mType = eCommandType_DepthTestEnable;
    command.mEnable = bDepthTest;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetLightingEnable(bool bLighting)
{
    Command command;
    command.mType = eCommandType_LightingEnable;
    command.mEnable = bLighting;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetLightEnable(int light, bool bEnable)
{
    Command command;
    command.mType = eCommandType_LightEnable;
    command.mLightEnable.mIndex = light;
    command.mLightEnable.mEnable = bEnable;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetLightDirection(int light, float x, float y, float z)
{
    Command command;
    command.mType = eCommandType_LightDirection;
    command.mLight.mIndex = light;

    Context& context = gRenderer->getContext();

    _Vec3 direction;
    direction.x = x;
    direction.y = y;
    direction.z = z;

    float transposed[4][4];
    ASM_MTX44Transpose(context.mMatrixStack[3][context.mMatrixStackDepth[3]], transposed);
    ASM_MTX44MultVecSR(transposed, &direction, &direction);
    ASM_VECNormalize(&direction, &direction);

    command.mLight.mValue[1] = direction.y;
    command.mLight.mValue[0] = direction.x;
    command.mLight.mValue[2] = direction.z;
    command.mLight.mValue[3] = 0.0f;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetLightColour(int light, float red, float green, float blue)
{
    Command command;
    command.mLight.mValue[0] = red;
    command.mLight.mValue[1] = green;
    command.mType = eCommandType_LightColour;
    command.mLight.mValue[2] = blue;
    command.mLight.mIndex = light;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetLightAmbientColour(float red, float green, float blue)
{
    Command command;
    command.mLight.mValue[0] = red;
    command.mLight.mValue[2] = blue;
    command.mType = eCommandType_LightAmbientColour;
    command.mLight.mValue[1] = green;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetBlendEnable(bool bBlend)
{
    Command command;
    command.mType = eCommandType_BlendEnable;
    command.mEnable = bBlend;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetBlendFunc(int source, int destination)
{
    Command command;
    command.mType = eCommandType_BlendFunc;
    command.mBlendFunc.mSource = source;
    command.mBlendFunc.mDestination = destination;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetBlendFactor(unsigned int factor)
{
    Command command;
    command.mType = eCommandType_BlendFactor;
    command.mBlendFactor = factor;
    mCommands.push_back(command);
}

void Renderer::CommandBuffer::SetFaceCull(bool bFaceCull)
{
    Command command;
    command.mType = eCommandType_FaceCull;
    command.mEnable = bFaceCull;
    mCommands.push_back(command);
}

int Renderer::CommandBuffer::getNumVertices()
{
    if (mVertices == NULL)
    {
        return 0;
    }

    int numVertices = 0;

    for (unsigned int index = 0; index < mCommands.size(); ++index)
    {
        if (mCommands[index].mType == eCommandType_Vertices)
        {
            numVertices += mCommands[index].mVertices.mCount;
        }
    }

    return numVertices;
}
