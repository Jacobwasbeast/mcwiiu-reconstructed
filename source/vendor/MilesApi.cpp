#include "vendor/MilesApi.hpp"

#include <stddef.h>
#include <string.h>

#include "vendor/MilesFilters.hpp"
#include "vendor/MilesMixer.hpp"
#include "vendor/MilesRib.hpp"
#include "vendor/MilesTime.hpp"

typedef int (*MSS_PROVIDER_STARTUP_CALLBACK)();

extern "C" void* platform_alloc(unsigned int uSize);
extern "C" void platform_free(void* pBlock);

static const unsigned int MILES_API_ENUMERATE_FIRST = 0xFFFFFFFF;

static const unsigned int MILES_LOCK_STATE_RELEASED = 1;
static const unsigned int MILES_LOCK_STATE_ACQUIRED = 2;

static const int MSS_PROVIDER_ALREADY_STARTED = 2;

static const int MILES_API_MUTEX_KIND = 2;

static const char MILES_API_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\genericmss.cpp";
static const char MILES_API_MUTEX_NAME[] = "Miles mutex";
static const char MILES_API_MUTEX_BLOCK_NAME[] = "Miles mutex block";
static const char MILES_API_MUTEX_LABEL[] = "Miles API mutex";

static const char MILES_API_MIXER_LIBRARY[] = "Miles Mixer";
static const char MILES_API_MIXER_INTERFACE[] = "MSS mixer services";
static const char MILES_API_MIXER_STARTUP[] = "MIXER_startup";
static const char MILES_API_CODEC_INTERFACE[] = "ASI codec";
static const char MILES_API_CODEC_STARTUP[] = "ASI_startup";
static const char MILES_API_FILTER_INTERFACE[] = "MSS pipeline filter";
static const char MILES_API_FILTER_STARTUP[] = "FLT_startup";

int MSS_api_started = 0;

AIL_MEM_ALLOC_CALLBACK AIL_mem_alloc_handler = platform_alloc;
AIL_MEM_FREE_CALLBACK AIL_mem_free_handler = platform_free;

rrMutex MSS_api_mutex;
MilesProfileSite MSS_api_lock_wait_site;
MilesProfileSite MSS_api_lock_site;
MilesProfileSite MSS_api_unlock_site;
MilesProfileSite MSS_api_mutex_name_site;

char AIL_error[256];
int AIL_preference[37];

extern "C" void InMilesMutex()
{
    if (MSS_api_started == 0)
    {
        return;
    }

    if (rrMutexLockTimeout(&MSS_api_mutex, 0) == 0)
    {
        if (MSS_profile != NULL)
        {
            MSS_profile->pZoneEnter(MSS_profile, 0, 0, 0, 1, MILES_API_FILE, 88,
                                    &MSS_api_lock_wait_site, MILES_API_MUTEX_BLOCK_NAME);
        }

        rrMutexLock(&MSS_api_mutex);

        if (MSS_profile == NULL)
        {
            return;
        }

        MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_API_FILE, 90);
    }

    if (MSS_profile != NULL)
    {
        MSS_profile->pLockState(MSS_profile, &MSS_api_mutex, MILES_LOCK_STATE_ACQUIRED,
                                MILES_API_FILE, 92, &MSS_api_lock_site, MILES_API_MUTEX_NAME);
    }
}

extern "C" void OutMilesMutex()
{
    if (MSS_api_started == 0)
    {
        return;
    }

    if (MSS_profile != NULL)
    {
        MSS_profile->pLockState(MSS_profile, &MSS_api_mutex, MILES_LOCK_STATE_RELEASED,
                                MILES_API_FILE, 100, &MSS_api_unlock_site, MILES_API_MUTEX_NAME);
    }

    rrMutexUnlock(&MSS_api_mutex);
}

extern "C" void AIL_lock_mutex()
{
    InMilesMutex();
}

extern "C" void AIL_unlock_mutex()
{
    OutMilesMutex();
}

extern "C" int AIL_API_background()
{
    return 0;
}

extern "C" void AIL_API_set_error(const char* pError)
{
    if (pError == NULL)
    {
        AIL_error[0] = '\0';
        return;
    }

    strcpy(AIL_error, pError);
}

extern "C" void* AIL_API_mem_alloc_lock(unsigned int uSize)
{
    return AIL_mem_alloc_handler(uSize);
}

extern "C" void AIL_API_mem_free_lock(void* pBlock)
{
    AIL_mem_free_handler(pBlock);
}

extern "C" void AIL_API_startup()
{
    unsigned int uIndex;
    unsigned int uProvider;
    MSS_PROVIDER_STARTUP_CALLBACK pMixerStartup;
    MSS_PROVIDER_STARTUP_CALLBACK pCodecStartup;
    MSS_PROVIDER_STARTUP_CALLBACK pFilterStartup;
    int iResult;

    AIL_preference[0] = 1;
    AIL_preference[1] = 0x10;
    AIL_preference[2] = 1;
    AIL_preference[3] = 0x40;
    AIL_preference[4] = 1;
    AIL_preference[5] = 0x83;
    AIL_preference[6] = 4;
    AIL_preference[7] = 0x40;
    AIL_preference[8] = 0xC;
    AIL_preference[9] = 0x20;
    AIL_preference[10] = 500;
    AIL_preference[11] = 1;
    AIL_preference[12] = 0;
    AIL_preference[13] = 0;
    AIL_preference[14] = 0;
    AIL_preference[15] = 0x2000;
    AIL_preference[16] = 100;
    AIL_preference[17] = 0;
    AIL_preference[18] = 0xC000;
    AIL_preference[19] = -1;
    AIL_preference[20] = 0;
    AIL_preference[21] = 8;
    AIL_preference[22] = 0x78;
    AIL_preference[23] = 0x7F;
    AIL_preference[24] = 1;
    AIL_preference[25] = 0;
    AIL_preference[26] = 2;
    AIL_preference[27] = 0;
    AIL_preference[28] = 0x600;
    AIL_preference[29] = 0x40;
    AIL_preference[30] = 0x78;
    AIL_preference[31] = 0;
    AIL_preference[32] = 1;
    AIL_preference[33] = 0;
    AIL_preference[34] = 1;
    AIL_preference[35] = 1;
    AIL_preference[36] = 0x8000;

    AIL_error[0] = '\0';

    rrGetTime();

    MSS_api_started = 1;
    rrMutexCreate(&MSS_api_mutex, MILES_API_MUTEX_KIND);

    if (MSS_profile != NULL)
    {
        MSS_profile->pLockName(MSS_profile, &MSS_api_mutex, &MSS_api_mutex_name_site,
                               MILES_API_MUTEX_LABEL);
    }

    RIB_load_static_provider_library(MIX_RIB_MAIN, MILES_API_MIXER_LIBRARY);

    uIndex = MILES_API_ENUMERATE_FIRST;
    while (RIB_enumerate_providers(MILES_API_MIXER_INTERFACE, &uIndex, &uProvider) != 0)
    {
        if (RIB_request_interface_entry(uProvider, MILES_API_MIXER_INTERFACE, 0,
                                        MILES_API_MIXER_STARTUP, &pMixerStartup) == 0)
        {
            pMixerStartup();
        }
    }

    uIndex = MILES_API_ENUMERATE_FIRST;
    while (RIB_enumerate_providers(MILES_API_CODEC_INTERFACE, &uIndex, &uProvider) != 0)
    {
        if (RIB_request_interface_entry(uProvider, MILES_API_CODEC_INTERFACE, 0,
                                        MILES_API_CODEC_STARTUP, &pCodecStartup) == 0)
        {
            iResult = pCodecStartup();
            if (iResult != 0 && iResult != MSS_PROVIDER_ALREADY_STARTED)
            {
                RIB_free_provider_library(uProvider);
            }
        }
    }

    FLT_init_list();

    uIndex = MILES_API_ENUMERATE_FIRST;
    while (RIB_enumerate_providers(MILES_API_FILTER_INTERFACE, &uIndex, &uProvider) != 0)
    {
        if (RIB_request_interface_entry(uProvider, MILES_API_FILTER_INTERFACE, 0,
                                        MILES_API_FILTER_STARTUP, &pFilterStartup) == 0)
        {
            iResult = pFilterStartup();
            if (iResult != 0 && iResult != MSS_PROVIDER_ALREADY_STARTED)
            {
                RIB_free_provider_library(uProvider);
            }
        }
    }

    MilesAsyncStartup();
}
