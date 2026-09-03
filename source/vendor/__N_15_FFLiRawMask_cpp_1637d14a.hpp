#ifndef VENDOR_N_15_FFLIRAWMASK_CPP_1637D14A_HPP
#define VENDOR_N_15_FFLIRAWMASK_CPP_1637D14A_HPP

struct FFLiCharInfo;

class __N_15_FFLiRawMask_cpp_1637d14a
{
public:
    struct RawMasks
    {
        float values[48];
    };

    static void CalcRawMask(RawMasks* rawMasks, const FFLiCharInfo* charInfo, int rotation, int eyeHeightAdjust, int mouthHeightAdjust);
};

#endif
