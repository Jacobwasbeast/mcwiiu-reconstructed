#include "platform/wiiu/FFLiMiiDataOfficial.hpp"

void FFLiSwapEndianArrayImpl__tm__3_Us__FPZ1ZUi_v(unsigned short* values, unsigned int count);

void FFLiMiiDataOfficial::SwapEndian()
{
    FFLiMiiDataCore::SwapEndian();
    FFLiSwapEndianArrayImpl__tm__3_Us__FPZ1ZUi_v(dataWords, 10);
}
