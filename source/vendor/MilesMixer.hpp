#ifndef SOURCE_VENDOR_MILES_MIXER_HPP
#define SOURCE_VENDOR_MILES_MIXER_HPP

#include "vendor/MilesRib.hpp"

int MSS_mixer_property(unsigned int uProperty, void* pBefore, const void* pNewValue, void* pAfter);

extern "C" void MSS_mixer_startup();
extern "C" void MSS_mixer_shutdown();
extern "C" void MSS_mixer_flush(void* pDestination, int iLength);

extern "C" void MSS_mixer_merge(void*, unsigned int*, unsigned short*, void**, unsigned short*,
                                int*, int*, unsigned int, int, int, unsigned int);
extern "C" void MSS_mixer_copy(void* pSource, unsigned int uLength, unsigned short* pDestination,
                               unsigned int uFlags, int iSuppressSwap);
extern "C" void MSS_mixer_mc_copy(void*, int, void*, unsigned int, unsigned int, int);
extern "C" void MSS_mixer_adpcm_decode(unsigned int*, unsigned int*, int, int, unsigned int, int*);

extern "C" int MIX_RIB_MAIN(unsigned int uProvider, int iUpDown,
                            RIB_ALLOC_PROVIDER_HANDLE_CALLBACK pAllocProviderHandle,
                            RIB_REGISTER_INTERFACE_CALLBACK pRegisterInterface,
                            RIB_UNREGISTER_INTERFACE_CALLBACK pUnregisterInterface);

#endif
