#include "vendor/VCTAudio.hpp"
#include <cmath>

static int s_vad_54 = 0;
static int s_vad_58 = 0;
static int s_vad_5c = 0;
static unsigned int s_vadClampGain = 0x31;
static unsigned int s_vadEnableInvalidation = 0;
static unsigned short s_vadActiveGain = 0x15e;
static unsigned short s_vadInactiveGain = 0x118;
static unsigned int s_vad_6c = 0;
static unsigned int s_vadReleaseTime = 5;
static unsigned int s_vad_74 = 0x2d0;
static unsigned char s_vadActivity = 0;

static unsigned int s_vadBss[4] = {0x1000000, 0x1000000, 0x1000000, 0x1000000};

unsigned int VCT_GetVADReleaseTime()
{
    return s_vadReleaseTime;
}

void VCT_ResetVAD()
{
    for (int i = 0; i < 4; ++i)
    {
        s_vadBss[i] = 0x1000000;
    }
    s_vad_5c = 0;
    s_vadActivity = 0;
    s_vad_6c = 0;
    s_vad_54 = 0;
    s_vad_58 = 0;
}

unsigned char VCT_GetVADActivity()
{
    return s_vadActivity;
}

unsigned int VCTi_GetVADStatus(unsigned int energy, unsigned int threshold, unsigned char* pStatus)
{
    s_vad_6c = threshold >> 8;
    if (pStatus != 0)
    {
        *pStatus = (unsigned char)(threshold >> 8);
    }
    if (s_vadClampGain <= energy)
    {
        if (s_vadActivity == 0)
        {
            unsigned int sum = 0;
            for (int i = 0; i < 4; ++i)
            {
                sum += s_vadBss[i];
            }
            double dVar = std::sqrt((double)(sum >> 2));
            int val = (int)dVar;
            if (threshold == 0 || threshold < (unsigned int)((val * s_vadActiveGain) / 100))
            {
                s_vadBss[s_vad_58] = energy;
                s_vad_58 = (s_vad_58 + 1) & 3;
                return 0;
            }
            s_vadBss[0] = energy;
            s_vadActivity = 1;
            s_vad_58 = 1;
            s_vad_5c = (int)((dVar * s_vadInactiveGain) / 100.0);
            return 1;
        }
        else
        {
            if (energy < (unsigned int)s_vad_5c)
            {
                s_vad_54++;
                if (s_vadReleaseTime < (unsigned int)s_vad_54)
                {
                    s_vad_58 = 0;
                    s_vad_54 = 0;
                    s_vadActivity = 0;
                    return 3;
                }
                return 2;
            }
            s_vad_54 = 0;
            return 1;
        }
    }
    return 0;
}

void vct_init_vad()
{
    s_vadReleaseTime = 5;
    unsigned int frameLength = VCTi_GetAudioFrameLength();
    s_vad_74 = frameLength * 5;
    VCT_ResetVAD();
}

unsigned short VCT_GetVADActiveGain()
{
    return s_vadActiveGain;
}

unsigned short VCT_GetVADInactiveGain()
{
    return s_vadInactiveGain;
}

unsigned int VCT_GetVADClampGain()
{
    return s_vadClampGain;
}

unsigned int VCT_IsEnableVADInvalidation()
{
    return s_vadEnableInvalidation;
}
