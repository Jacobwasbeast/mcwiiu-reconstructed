#include "vendor/MilesApi.hpp"

#include <stddef.h>
#include <string.h>

#include "game/client/audio/FalloffGraph.hpp"
#include "vendor/MilesEventExec.hpp"
#include "game/util/LegacyUtilityFunctions.hpp"
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

_DIG_DRIVER* MSS_primary_digital_driver = NULL;

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

extern "C" void* MSS_alloc_info(unsigned int uSize, unsigned int, const char* pOwnerName,
                                int iOwnerLine)
{
    return AIL_mem_alloc_lock_info(uSize, pOwnerName, iOwnerLine);
}

extern "C" void MSS_free_info(void* pBlock)
{
    AIL_mem_free_lock(pBlock);
}

extern "C" _DIG_DRIVER* AIL_open_generic_digital_driver(unsigned int uFrequency, int iBits,
                                                        unsigned int uSpeakerConfig,
                                                        unsigned int uFlags,
                                                        RADSS_OPEN_DRIVER_CALLBACK pOutput)
{
    _DIG_DRIVER* pResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_open_digital_driver(%ld, %ld, %ld, 0x%lX)", uFrequency, iBits,
                      uSpeakerConfig, uFlags);
    }

    InMilesMutex();
    pResult = AIL_API_open_generic_digital_driver(uFrequency, iBits, uSpeakerConfig, uFlags,
                                                  pOutput);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresuhex((unsigned int)pResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;

    if (pResult != NULL)
    {
        pResult->requestedSpeakerConfig = uSpeakerConfig;
    }

    return pResult;
}

extern "C" void AIL_close_digital_driver(_DIG_DRIVER* pDriver)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_close_digital_driver(0x%lX)", pDriver);
    }

    AIL_API_close_digital_driver(pDriver);

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" _SAMPLE* AIL_allocate_sample_handle(_DIG_DRIVER* pDriver)
{
    _SAMPLE* pResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_allocate_sample_handle(0x%lX)", pDriver);
    }

    InMilesMutex();
    pResult = AIL_API_allocate_sample_handle(pDriver);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresuhex((unsigned int)pResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return pResult;
}

extern "C" _SAMPLE* AIL_allocate_bus(_DIG_DRIVER* pDriver)
{
    _SAMPLE* pResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_allocate_bus(0x%lX)", pDriver);
    }

    InMilesMutex();
    pResult = AIL_API_allocate_bus(pDriver);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresuhex((unsigned int)pResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return pResult;
}

extern "C" void AIL_set_sample_bus(_SAMPLE* pSample, unsigned int uBus)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_bus(0x%lX,%d)", pSample, uBus);
    }

    InMilesMutex();

    if (uBus < DRIVER_BUS_COUNT && pSample->driver->busSamples[uBus] != NULL)
    {
        pSample->reverbBus = uBus;
    }

    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" _SAMPLE* AIL_bus_sample_handle(_DIG_DRIVER* pDriver, int iBus)
{
    _SAMPLE* pResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_bus_sample_handle(0x%lX,%d)", pDriver, iBus);
    }

    InMilesMutex();
    pResult = AIL_API_bus_sample_handle(pDriver, iBus);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresuhex((unsigned int)pResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return pResult;
}

extern "C" void AIL_release_sample_handle(_SAMPLE* pSample)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_release_sample_handle(0x%lX)", pSample);
    }

    InMilesMutex();
    AIL_API_release_sample_handle(pSample);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" int AIL_init_sample(_SAMPLE* pSample, unsigned int uFormat)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_init_sample(0x%lX,%ld)", pSample, uFormat);
    }

    InMilesMutex();
    iResult = AIL_API_init_sample(pSample, uFormat);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int AIL_set_sample_file(_SAMPLE* pSample, const void* pFile, int iBlock)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_file(0x%lX,0x%lX,%ld)", pSample, pFile, iBlock);
    }

    InMilesMutex();
    iResult = AIL_API_set_sample_file(pSample, pFile, iBlock);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int AIL_set_named_sample_file(_SAMPLE* pSample, const char* pFileName, const void* pFile,
                                         int iSize, int iBlock)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_named_sample_file(0x%lX,%s,0x%lX,%ld,%ld)", pSample, pFileName,
                      pFile, iSize, iBlock);
    }

    InMilesMutex();
    iResult = AIL_API_set_named_sample_file(pSample, pFileName, pFile, iSize, iBlock);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int AIL_set_digital_driver_processor(_DIG_DRIVER* pDriver, int iStage, int iProvider)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_digital_driver_processor(0x%lX,%ld,%ld)", pDriver, iStage,
                      iProvider);
    }

    InMilesMutex();
    iResult = AIL_API_set_digital_driver_processor(pDriver, iStage, iProvider);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" void AIL_set_sample_address(_SAMPLE* pSample, const void* pStart, unsigned int uLength)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_address(0x%lX,0x%lX,%lu)", pSample, pStart, uLength);
    }

    InMilesMutex();
    AIL_API_set_sample_address(pSample, pStart, uLength);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_adpcm_block_size(_SAMPLE* pSample, unsigned int uBlockSize)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_adpcm_block_size(0x%lX,%lu)", pSample, uBlockSize);
    }

    InMilesMutex();
    AIL_API_set_sample_adpcm_block_size(pSample, uBlockSize);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_start_sample(_SAMPLE* pSample)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_start_sample(0x%lX)", pSample);
    }

    InMilesMutex();
    AIL_API_start_sample(pSample);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_stop_sample(_SAMPLE* pSample)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_stop_sample(0x%lX)", pSample);
    }

    InMilesMutex();
    AIL_API_stop_sample(pSample);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_resume_sample(_SAMPLE* pSample)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_resume_sample(0x%lX)", pSample);
    }

    InMilesMutex();
    AIL_API_resume_sample(pSample);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_playback_rate_factor(_SAMPLE* pSample, float fFactor)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_playback_rate_factor(0x%lX,%f)", pSample, fFactor);
    }

    InMilesMutex();
    AIL_API_set_sample_playback_rate_factor(pSample, fFactor);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_playback_delay(_SAMPLE* pSample, int iDelay)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_playback_delay(0x%lX,%ld)", pSample, iDelay);
    }

    InMilesMutex();
    AIL_API_set_sample_playback_delay(pSample, iDelay);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" int AIL_set_sample_is_3D(_SAMPLE* pSample, int iEnabled)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_is_3D(0x%lX, %d )", pSample, iEnabled);
    }

    InMilesMutex();
    iResult = AIL_API_set_sample_is_3D(pSample, iEnabled);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" void AIL_set_speaker_configuration(_DIG_DRIVER* pDriver, const float* pSpeakerPositions,
                                              int iChannels, float fAzimuth)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_speaker_configuration(0x%lX,0x%lX,%ld,%f)", pDriver,
                      pSpeakerPositions, iChannels, fAzimuth);
    }

    InMilesMutex();
    AIL_API_set_speaker_configuration(pDriver, pSpeakerPositions, iChannels, fAzimuth);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_speaker_reverb_levels(_DIG_DRIVER* pDriver, const float* pWetLevels,
                                              const float* pDryLevels, const int* pSpeakers,
                                              int iCount)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_speaker_reverb_levels(0x%lX,0x%lX,0x%lX,0x%lX,%d)", pDriver,
                      pWetLevels, pDryLevels, pSpeakers, iCount);
    }

    InMilesMutex();
    AIL_API_set_speaker_reverb_levels(pDriver, pWetLevels, pDryLevels, pSpeakers, iCount);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_volume_levels(_SAMPLE* pSample, float fLeftLevel, float fRightLevel)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_volume_levels(0x%lX,%f,%f)", pSample, fLeftLevel,
                      fRightLevel);
    }

    InMilesMutex();
    AIL_API_set_sample_volume_levels(pSample, fLeftLevel, fRightLevel);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_loop_count(_SAMPLE* pSample, int iLoopCount)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_loop_count(0x%lX,%ld)", pSample, iLoopCount);
    }

    InMilesMutex();
    AIL_API_set_sample_loop_count(pSample, iLoopCount);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_loop_block(_SAMPLE* pSample, int iLoopStart, int iLoopEnd)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_loop_block(0x%lX,%ld,%ld)", pSample, iLoopStart, iLoopEnd);
    }

    InMilesMutex();
    AIL_API_set_sample_loop_block(pSample, iLoopStart, iLoopEnd);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" int AIL_sample_status(_SAMPLE* pSample)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_status(0x%lX)", pSample);
    }

    iResult = AIL_API_sample_status(pSample);

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" float AIL_sample_playback_rate_factor(_SAMPLE* pSample)
{
    float fResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_playback_rate_factor(0x%lX)", pSample);
    }

    fResult = AIL_API_sample_playback_rate_factor(pSample);

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresfloat(fResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return fResult;
}

extern "C" void AIL_sample_volume_levels(_SAMPLE* pSample, float* pLeftLevel, float* pRightLevel)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_volume_levels(0x%lX,0x%lX,0x%lX)", pSample, pLeftLevel,
                      pRightLevel);
    }

    AIL_API_sample_volume_levels(pSample, pLeftLevel, pRightLevel);

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_digital_master_volume_level(_DIG_DRIVER* pDriver, float fLevel)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_digital_master_volume_level(0x%lX,%f)", pDriver, fLevel);
    }

    InMilesMutex();
    AIL_API_set_digital_master_volume_level(pDriver, fLevel);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_digital_master_reverb_levels(_DIG_DRIVER* pDriver, int iBus,
                                                     float fWetLevel, float fDryLevel)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_digital_master_reverb_levels(0x%lX,%ld,%f,%f)", pDriver, iBus,
                      fWetLevel, fDryLevel);
    }

    InMilesMutex();
    AIL_API_set_digital_master_reverb_levels(pDriver, iBus, fWetLevel, fDryLevel);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_digital_master_reverb(_DIG_DRIVER* pDriver, int iBus, float fReverbTime,
                                              float fReverbPredelay, float fReverbDamping)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_digital_master_reverb(0x%lX,%ld,%f,%f,%f)", pDriver, iBus,
                      fReverbTime, fReverbPredelay, fReverbDamping);
    }

    InMilesMutex();
    AIL_API_set_digital_master_reverb(pDriver, iBus, fReverbTime, fReverbPredelay, fReverbDamping);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" int AIL_minimum_sample_buffer_size(_DIG_DRIVER* pDriver, int iPlaybackRate, int iFormat)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_minimum_sample_buffer_size(0x%lX,%ld,%ld)", pDriver, iPlaybackRate,
                      iFormat);
    }

    InMilesMutex();
    iResult = AIL_API_minimum_sample_buffer_size(pDriver, iPlaybackRate, iFormat);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int AIL_sample_buffer_available(_SAMPLE* pSample)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_buffer_available(0x%lX)", pSample);
    }

    InMilesMutex();
    iResult = AIL_API_sample_buffer_available(pSample);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int AIL_set_sample_buffer_count(_SAMPLE* pSample, int iCount)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_buffer_count(0x%lX)", pSample);
    }

    InMilesMutex();
    iResult = AIL_API_set_sample_buffer_count(pSample, iCount);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int AIL_load_sample_buffer(_SAMPLE* pSample, int iBuffer, const void* pData,
                                      unsigned int uLength)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_load_sample_buffer(0x%lX,%ld,0x%lX,%lu)", pSample, iBuffer, pData,
                      uLength);
    }

    InMilesMutex();
    iResult = AIL_API_load_sample_buffer(pSample, iBuffer, pData, uLength);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int AIL_sample_granularity(_SAMPLE* pSample)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_granularity(0x%lX)", pSample);
    }

    iResult = AIL_API_sample_granularity(pSample);

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" void AIL_set_sample_position(_SAMPLE* pSample, unsigned int uPosition)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_position(0x%lX,%lu)", pSample, uPosition);
    }

    InMilesMutex();
    AIL_API_set_sample_position(pSample, uPosition);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" unsigned int AIL_sample_position(_SAMPLE* pSample)
{
    unsigned int uResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_position(0x%lX)", pSample);
    }

    uResult = AIL_API_sample_position(pSample);

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(uResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return uResult;
}

extern "C" AIL_FALLOFF_CALLBACK AIL_register_falloff_function_callback(
    _SAMPLE* pSample, AIL_FALLOFF_CALLBACK pCallback)
{
    AIL_FALLOFF_CALLBACK pResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_register_falloff_function_callback(0x%lX,0x%lX)", pSample, pCallback);
    }

    InMilesMutex();
    pResult = AIL_API_register_falloff_function_callback(pSample, pCallback);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresuhex((unsigned int)pResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return pResult;
}

extern "C" _STREAM* AIL_open_stream(_DIG_DRIVER* pDriver, const char* pFileName, int iStreamMem)
{
    _STREAM* pResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_open_stream(0x%lX, %s, %ld)", pDriver, pFileName, iStreamMem);
    }

    InMilesMutex();
    pResult = AIL_API_open_stream(pDriver, pFileName, iStreamMem);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresuhex((unsigned int)pResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return pResult;
}

extern "C" void AIL_close_stream(_STREAM* pStream)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_close_stream(0x%lX)", pStream);
    }

    InMilesMutex();
    AIL_API_close_stream(pStream);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" _SAMPLE* AIL_stream_sample_handle(_STREAM* pStream)
{
    _SAMPLE* pResult = NULL;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_stream_sample_handle(0x%lX)", pStream);
    }

    if (pStream != NULL)
    {
        pResult = pStream->sample;
    }

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresuhex((unsigned int)pResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return pResult;
}

extern "C" void AIL_start_stream(_STREAM* pStream)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_start_stream(0x%lX)", pStream);
    }

    InMilesMutex();
    AIL_API_start_stream(pStream);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_pause_stream(_STREAM* pStream, int iPause)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_pause_stream(0x%lX, %ld)", pStream, iPause);
    }

    InMilesMutex();
    AIL_API_pause_stream(pStream, iPause);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_stream_loop_block(_STREAM* pStream, int iLoopStart, int iLoopEnd)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_stream_loop_block(0x%lX,%ld,%ld)", pStream, iLoopStart, iLoopEnd);
    }

    InMilesMutex();
    AIL_API_set_stream_loop_block(pStream, iLoopStart, iLoopEnd);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_stream_loop_count(_STREAM* pStream, int iLoopCount)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_stream_loop_count(0x%lX, %ld)", pStream, iLoopCount);
    }

    InMilesMutex();
    AIL_API_set_stream_loop_count(pStream, iLoopCount);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" int AIL_stream_status(_STREAM* pStream)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_stream_status(0x%lX)", pStream);
    }

    iResult = AIL_API_stream_status(pStream);

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" void AIL_set_stream_position(_STREAM* pStream, int iPosition)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_stream_position(0x%lX, %ld)", pStream, iPosition);
    }

    InMilesMutex();
    AIL_API_set_stream_position(pStream, iPosition);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" int AIL_stream_position(_STREAM* pStream)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_stream_position(0x%lX)", pStream);
    }

    InMilesMutex();
    iResult = AIL_API_stream_position(pStream);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" void AIL_set_sample_ms_position(_SAMPLE* pSample, unsigned int uMilliseconds)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_ms_position(0x%lX,%lu)", pSample, uMilliseconds);
    }

    InMilesMutex();
    AIL_API_set_sample_ms_position(pSample, uMilliseconds);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_stream_ms_position(_STREAM* pStream, unsigned int uMilliseconds)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_stream_ms_position(0x%lX,%lu)", pStream, uMilliseconds);
    }

    InMilesMutex();
    AIL_API_set_stream_ms_position(pStream, uMilliseconds);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" int AIL_WAV_info(const void* pData, _AILSOUNDINFO* pInfo)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_WAV_info(0x%lX,0x%lX)", pData, pInfo);
    }

    iResult = AIL_API_WAV_info(pData, pInfo);

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int MilesEnumerateSoundInstances(EventSystem* pEventSystem, unsigned int* puNext,
                                            unsigned int uTypeMask, const char* pLabels,
                                            int iEventIndex, int iEventSerial,
                                            SoundInstanceInfo* pInfo)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("AIL_enumerate_sound_instances(0x%lX,%d,%s,%ld,0x%lX)", puNext, uTypeMask,
                      pLabels, iEventIndex, iEventSerial, pInfo);
    }

    InMilesMutex();
    iResult = MilesEnumerateSoundInstancesInternal(pEventSystem, puNext, uTypeMask, pLabels,
                                                   iEventIndex, iEventSerial, pInfo);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int MilesBeginEventQueueProcessing()
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("AIL_begin_event_queue_processing()");
    }

    InMilesMutex();
    iResult = MilesBeginEventQueueProcessingInternal();
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" int MilesCompleteEventQueueProcessing()
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("AIL_complete_event_queue_processing()");
    }

    InMilesMutex();
    iResult = MilesCompleteEventQueueProcessingInternal();
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" unsigned int MilesStartupEventSystem(_DIG_DRIVER* pDriver, int iEventMemory, int iFlags,
                                                int iStreamMemory)
{
    unsigned int uResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("AIL_startup_event_system(0x%lX,%d,0x%lX", "%d)", pDriver, iEventMemory,
                      iFlags, iStreamMemory);
    }

    InMilesMutex();
    uResult = MilesStartupEventSystemInternal(pDriver, iEventMemory, iFlags, iStreamMemory);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresuint(uResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return uResult;
}

extern "C" SoundBank* MilesAddSoundBank(const char* pFileName, const char* pBankName)
{
    SoundBank* pResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("AIL_add_soundbank(%s)", pFileName);
    }

    InMilesMutex();
    pResult = MilesAddSoundBankInternal(pFileName, pBankName);
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresuint((unsigned int)pResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return pResult;
}

extern "C" unsigned long long MilesEnqueueEventByName(const char* pEventName)
{
    unsigned long long ullResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("AIL_enqueue_event_by_name(%s)", pEventName);
    }

    ullResult = MilesEnqueueEventByNameInternal(pEventName);

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint64(static_cast<unsigned int>(ullResult >> 32),
                    static_cast<unsigned int>(ullResult));
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return ullResult;
}

extern "C" _DIG_DRIVER* AIL_primary_digital_driver(_DIG_DRIVER* pDriver)
{
    _DIG_DRIVER* pPrevious;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_primary_digital_driver(0x%lX)", pDriver);
    }

    InMilesMutex();
    pPrevious = MSS_primary_digital_driver;
    if (pDriver != NULL)
    {
        MSS_primary_digital_driver = pDriver;
    }
    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresuhex((unsigned int)pPrevious);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return pPrevious;
}

extern "C" void AIL_set_sample_3D_distances(_SAMPLE* pSample, float fMaxDistance,
                                            float fMinDistance, int iAutomaticWetAttenuation)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_3D_distances(0x%lX,%f,%f,%ld)", pSample, fMaxDistance,
                      fMinDistance, iAutomaticWetAttenuation);
    }

    InMilesMutex();
    AIL_API_set_sample_3D_distances(pSample, fMaxDistance, fMinDistance, iAutomaticWetAttenuation);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_room_type(_DIG_DRIVER* pDriver, int iBus, int iRoomType)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_room_type(0x%lX,%ld,%d)", pDriver, iBus, iRoomType);
    }

    InMilesMutex();
    AIL_API_set_room_type(pDriver, iBus, iRoomType);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_3D_rolloff_factor(_DIG_DRIVER* pDriver, float fRolloffFactor)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_3D_rolloff_factor(0x%lX,%f)", pDriver, fRolloffFactor);
    }

    InMilesMutex();
    AIL_API_set_3D_rolloff_factor(pDriver, fRolloffFactor);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_3D_doppler_factor(_DIG_DRIVER* pDriver, float fDopplerFactor)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_3D_doppler_factor(0x%lX,%f)", pDriver, fDopplerFactor);
    }

    InMilesMutex();
    AIL_API_set_3D_doppler_factor(pDriver, fDopplerFactor);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_3D_position(_SAMPLE* pSample, float fX, float fY, float fZ)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_3D_position(0x%lX,%f,%f,%f)", pSample, fX, fY, fZ);
    }

    InMilesMutex();
    AIL_API_set_sample_3D_position(pSample, fX, fY, fZ);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_listener_3D_position(_DIG_DRIVER* pDriver, float fX, float fY, float fZ)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_listener_3D_position(0x%lX,%f,%f,%f)", pDriver, fX, fY, fZ);
    }

    InMilesMutex();
    AIL_API_set_listener_3D_position(pDriver, fX, fY, fZ);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_3D_volume_falloff(_SAMPLE* pSample, const FalloffGraphPoint* pPoints,
                                                 int iPointCount)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_3D_volume_falloff(0x%lX,0x%lX,%d)", pSample, pPoints,
                      iPointCount);
    }

    InMilesMutex();
    AIL_API_set_sample_3D_volume_falloff(pSample, pPoints, iPointCount);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_3D_lowpass_falloff(_SAMPLE* pSample,
                                                  const FalloffGraphPoint* pPoints, int iPointCount)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_3D_lowpass_falloff(0x%lX,0x%lX,%d)", pSample, pPoints,
                      iPointCount);
    }

    InMilesMutex();
    AIL_API_set_sample_3D_lowpass_falloff(pSample, pPoints, iPointCount);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_3D_exclusion_falloff(_SAMPLE* pSample,
                                                    const FalloffGraphPoint* pPoints,
                                                    int iPointCount)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_3D_exclusion_falloff(0x%lX,0x%lX,%d)", pSample, pPoints,
                      iPointCount);
    }

    InMilesMutex();
    AIL_API_set_sample_3D_exclusion_falloff(pSample, pPoints, iPointCount);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_listener_3D_orientation(_DIG_DRIVER* pDriver, float fForwardX,
                                                float fForwardY, float fForwardZ, float fUpX,
                                                float fUpY, float fUpZ)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_listener_3D_orientation(0x%lX,%f,%f,%f,%f,%f,%f)", pDriver,
                      fForwardX, fForwardY, fForwardZ, fUpX, fUpY, fUpZ);
    }

    InMilesMutex();
    AIL_API_set_listener_3D_orientation(pDriver, fForwardX, fForwardY, fForwardZ, fUpX, fUpY, fUpZ);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_close_filter(FLT_INSTANCE* pFilter)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_close_filter(0x%lX)", pFilter);
    }

    InMilesMutex();
    AIL_API_close_filter(pFilter);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_file_callbacks(AIL_FILE_OPEN_CALLBACK pOpen, AIL_FILE_CLOSE_CALLBACK pClose,
                                       AIL_FILE_SEEK_CALLBACK pSeek, AIL_FILE_READ_CALLBACK pRead)
{
    if (pOpen == NULL)
    {
        pOpen = DefaultFileOpen;
    }

    AIL_file_open = pOpen;

    if (pClose == NULL)
    {
        pClose = DefaultFileClose;
    }

    AIL_file_close = pClose;

    if (pSeek == NULL)
    {
        pSeek = DefaultFileSeek;
    }

    AIL_file_seek = pSeek;

    if (pRead == NULL)
    {
        pRead = DefaultFileRead;
    }

    AIL_file_read = pRead;
}

extern "C" RADSS_OPEN_DRIVER_CALLBACK AIL_get_default_output()
{
    return RADSS_AXInstallDriver1(0, 0);
}

extern "C" RADSS_OPEN_DRIVER_CALLBACK AIL_output_ax(int iDevice)
{
    return RADSS_AXInstallDriver1(iDevice, 0);
}
