#include "platform/wiiu/FFLiMiiDataCore.hpp"

struct FFLiSwapEndianDesc
{
    unsigned int type;
    unsigned int count;
};

int FFLiSwapEndianGroup__FPvPC18FFLiSwapEndianDescUi(void* data, const FFLiSwapEndianDesc* descriptions, unsigned int count);

static const FFLiSwapEndianDesc g_miiDataCoreSwapEndianDescriptions[7] =
{
    { 2, 1 },
    { 0, 8 },
    { 0, 10 },
    { 0, 2 },
    { 1, 11 },
    { 0, 2 },
    { 1, 12 }
};

void FFLiMiiDataCore::SwapEndian()
{
    FFLiSwapEndianGroup__FPvPC18FFLiSwapEndianDescUi(this, g_miiDataCoreSwapEndianDescriptions, 7);
}
