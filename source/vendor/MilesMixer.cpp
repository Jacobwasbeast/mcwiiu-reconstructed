#include "vendor/MilesMixer.hpp"

#include <stddef.h>
#include <string.h>

static const char MIX_INTERFACE_NAME[] = "MSS mixer services";
static const char MIX_PROVIDER_NAME[] = "Default Miles Mixer";

static const unsigned int MIX_PROVIDER_VERSION = 0x130;

struct MIX_COPY_CHANNEL
{
    void* pData;
    unsigned int uLength;
    int iChannels;
    int iDestinationOffset;
};

static const RIB_INTERFACE_ENTRY MIX_interface[] = {
    { 0, "PROVIDER_property", (void*)MSS_mixer_property, 0 },
    { 0, "MIXER_startup", (void*)MSS_mixer_startup, 0 },
    { 0, "MIXER_startup", (void*)MSS_mixer_startup, 0 },
    { 0, "MIXER_shutdown", (void*)MSS_mixer_shutdown, 0 },
    { 0, "MIXER_flush", (void*)MSS_mixer_flush, 0 },
    { 0, "MIXER_merge", (void*)MSS_mixer_merge, 0 },
    { 0, "MIXER_copy", (void*)MSS_mixer_copy, 0 },
    { 0, "MIXER_mc_copy", (void*)MSS_mixer_mc_copy, 0 },
    { 0, "MIXER_adpcm_decode", (void*)MSS_mixer_adpcm_decode, 0 }
};

int MSS_mixer_property(unsigned int uProperty, void* pBefore, const void*, void*)
{
    if (uProperty == RIB_PROVIDER_NAME)
    {
        if (pBefore != NULL)
        {
            *(const char**)pBefore = MIX_PROVIDER_NAME;
            return 1;
        }
    }
    else
    {
        if (uProperty != RIB_PROVIDER_VERSION)
        {
            return 0;
        }

        if (pBefore != NULL)
        {
            *(unsigned int*)pBefore = MIX_PROVIDER_VERSION;
            return 1;
        }
    }

    return 0;
}

extern "C" void MSS_mixer_startup()
{
}

extern "C" void MSS_mixer_shutdown()
{
}

extern "C" void MSS_mixer_flush(void* pDestination, int iLength)
{
    memset(pDestination, 0, iLength);
}

extern "C" void MSS_mixer_copy(void* pSource, unsigned int uLength, unsigned short* pDestination,
                               unsigned int uFlags, int iSuppressSwap)
{
    MIX_COPY_CHANNEL stereo;
    MIX_COPY_CHANNEL mono;
    unsigned short* pSample;
    int iCount;

    if ((uFlags & 2) != 0)
    {
        stereo.pData = pSource;
        stereo.uLength = uLength;
        stereo.iChannels = 2;
        stereo.iDestinationOffset = 0;

        MSS_mixer_mc_copy(&stereo, 1, pDestination, uFlags,
                          (unsigned int)((int)uLength >> 3), 2);
    }
    else
    {
        mono.pData = pSource;
        mono.uLength = uLength;
        mono.iChannels = 1;
        mono.iDestinationOffset = 0;

        MSS_mixer_mc_copy(&mono, 1, pDestination, uFlags,
                          (unsigned int)((int)uLength >> 2), 1);
    }

    if (iSuppressSwap != 0 || (uFlags & 1) == 0)
    {
        return;
    }

    pSample = pDestination;

    for (iCount = (int)uLength >> 1; iCount != 0; iCount = iCount - 2)
    {
        *pSample = (unsigned short)((*pSample >> 8) | (*pSample << 8));
        pSample = pSample + 1;
    }
}

extern "C" int MIX_RIB_MAIN(unsigned int uProvider, int iUpDown,
                            RIB_ALLOC_PROVIDER_HANDLE_CALLBACK,
                            RIB_REGISTER_INTERFACE_CALLBACK pRegisterInterface,
                            RIB_UNREGISTER_INTERFACE_CALLBACK pUnregisterInterface)
{
    if (iUpDown != 0)
    {
        pRegisterInterface(uProvider, MIX_INTERFACE_NAME,
                           (int)(sizeof(MIX_interface) / sizeof(MIX_interface[0])), MIX_interface);
    }
    else
    {
        pUnregisterInterface(uProvider, NULL, 0, NULL);
    }

    return 1;
}
