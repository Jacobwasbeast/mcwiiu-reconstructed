#include "vendor/__N_16_chat_VctMain_cpp_2fb29e39.hpp"

#include "vendor/VCTAudio.hpp"

#include <coreinit/time.h>

struct VCTResult
{
    unsigned int status;
    unsigned int reserved;
    unsigned int payload;
};

unsigned int g_vctAudioPacketCount = 0;

extern unsigned int vct_handle_audio(unsigned short connectionId, unsigned char *data, unsigned int dataSize, long long receiveTick);

int __N_16_chat_VctMain_cpp_2fb29e39::VCTi_HandleData(unsigned short connectionId, unsigned char *data, unsigned int dataSize, VCTResult *result)
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
