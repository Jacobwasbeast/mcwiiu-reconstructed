#include "vendor/VCTAudio.hpp"

#include <coreinit/time.h>

struct VCTResult
{
    unsigned int status;
    unsigned int reserved;
    unsigned int payload;
};

namespace
{

unsigned int g_vctAudioPacketCount = 0;

int VCTi_HandleData(unsigned short connectionId, unsigned char* data, unsigned int dataSize, VCTResult* result)
{
    if (data != 0)
    {
        unsigned int signature = 0;
        signature = (static_cast<unsigned int>(data[0]) << 24) |
                    (static_cast<unsigned int>(data[1]) << 16) |
                    (static_cast<unsigned int>(data[2]) << 8) |
                    static_cast<unsigned int>(data[3]);
        if (signature != 0x5f564354U)
        {
            return 2;
        }
        if ((g_vctMode != 0) && (g_vctInitialized != 0))
        {
            result->status = 0;
            result->payload = 0;
            const long long receiveTick = OSGetTime();
            if ((data[4] & 0xf0) == 0x40)
            {
                ++g_vctAudioPacketCount;
                vct_handle_audio(connectionId, data, dataSize, receiveTick);
                return 0;
            }
        }
    }
    return 0;
}

}

unsigned char VCT_IsInitialized()
{
    return g_vctInitialized;
}

bool VCT_HandleData(unsigned short connectionId, unsigned char* data, unsigned int dataSize)
{
    VCTResult result;
    return VCTi_HandleData(connectionId, data, dataSize, &result) != 2;
}

unsigned int VCT_GetAudioFrameLength()
{
    return VCTi_GetAudioFrameLength();
}

unsigned int VCT_GetAudioDataSize()
{
    return VCTi_GetAudioDataSize();
}

unsigned int VCT_GetAudioScale(const void* pAudio, unsigned int uiSize)
{
    if (pAudio == 0 || uiSize == 0)
    {
        return 0;
    }
    return HPFFilter2(pAudio, uiSize);
}
