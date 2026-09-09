#include "game/client/audio/PlatformSoundEngineImpl.hpp"

#include <stdio.h>
#include <string.h>

#include "platform/wiiu/C4JThreadImpl.hpp"

struct _SAMPLE
{
    unsigned char ucReserved[454];
    bool bOutputToDRC;
};

struct _MILESSOUNDINSTANCEINFO
{
    int iSoundIndex;
    int iSoundSerial;
    int iSoundBankIndex;
    int iSoundBankSerial;
    int iEventIndex;
    int iEventSerial;
    _SAMPLE* pSample;
    _STREAM* pStream;
    void* pSoundInstance;
    int iSoundInstanceSize;
    unsigned int uiType;
    unsigned int uiPaused;
    unsigned int uiSampleRate;
    unsigned int uiLength;
    unsigned int uiPosition;
    const char* pName;
    int iActive;
};

typedef int (*RIB_MAIN_FUNC)(unsigned int uiProvider, int iUpDown, void* pUserData, void* pRegisterCallback, void* pUnregisterCallback);

void AXInit();
void MIXInit();

int BinkADec_RIB_Main(unsigned int uiProvider, int iUpDown, void* pUserData, void* pRegisterCallback, void* pUnregisterCallback);
unsigned int RIB_load_static_provider_library(RIB_MAIN_FUNC pMain, const char* pLibraryName);

unsigned int file_open_callback(const char* pFileName, unsigned int* puiFileHandle);
void file_close_callback(unsigned int uiFileHandle);
void file_seek_callback(unsigned int uiFileHandle, int iOffset, unsigned int uiType);
int file_read_callback(unsigned int uiFileHandle, void* pBuffer, unsigned int uiBytes);

void AIL_startup();
void AIL_shutdown();
const char* AIL_last_error();
void AIL_set_file_callbacks(unsigned int (*pOpen)(const char*, unsigned int*), void (*pClose)(unsigned int), void (*pSeek)(unsigned int, int, unsigned int), int (*pRead)(unsigned int, void*, unsigned int));
unsigned int AIL_output_ax(int iDevice);
_DIG_DRIVER* AIL_open_generic_digital_driver(unsigned int uiFrequency, int iBits, int iChannels, unsigned int uiFlags, unsigned int uiProvider);
void AIL_close_digital_driver(_DIG_DRIVER* pDriver);
void AIL_set_3D_rolloff_factor(_DIG_DRIVER* pDriver, float fRolloffFactor);
void AIL_set_listener_3D_position(_DIG_DRIVER* pDriver, float fX, float fY, float fZ);
void AIL_set_listener_3D_orientation(_DIG_DRIVER* pDriver, float fFaceX, float fFaceY, float fFaceZ, float fUpX, float fUpY, float fUpZ);
void AIL_register_falloff_function_callback(_SAMPLE* pSample, float (*pCallback)(_SAMPLE*, float, float, float, float));
void AIL_set_sample_is_3D(_SAMPLE* pSample, int iIs3D);
void AIL_set_sample_3D_distances(_SAMPLE* pSample, float fMaxDistance, float fMinDistance, int iAutoWetAttenuation);
void AIL_set_sample_3D_position(_SAMPLE* pSample, float fX, float fY, float fZ);
void AIL_set_sample_playback_rate_factor(_SAMPLE* pSample, float fFactor);
void AIL_set_sample_volume_levels(_SAMPLE* pSample, float fLeftLevel, float fRightLevel);
_STREAM* AIL_open_stream(_DIG_DRIVER* pDriver, const char* pFileName, int iStreamMem);
void AIL_start_stream(_STREAM* pStream);
void AIL_pause_stream(_STREAM* pStream, int iOnOff);
void AIL_close_stream(_STREAM* pStream);
int AIL_stream_status(_STREAM* pStream);
_SAMPLE* AIL_stream_sample_handle(_STREAM* pStream);
unsigned int AIL_enqueue_event_start();
unsigned int AIL_enqueue_event_buffer(unsigned int* puiQueue, void* pData, int iDataSize, int iFlags);
void AIL_enqueue_event_end_named(unsigned int uiQueue, const char* pEventName);

void MilesSetEventErrorCallback(void (*pCallback)(long long, const char*));
int MilesStartupEventSystem(_DIG_DRIVER* pDriver, int iEventMemory, int iFlags, int iStreamMemory);
SoundBank* MilesAddSoundBank(const char* pFileName, int iFlags);
void MilesEnqueueEventByName(const char* pEventName);
void MilesBeginEventQueueProcessing();
void MilesCompleteEventQueueProcessing();
int MilesEnumerateSoundInstances(SoundBank* pSoundBank, unsigned int* puiNext, unsigned int uiTypeMask, const char* pLabels, int iEventIndex, int iEventSerial, _MILESSOUNDINSTANCEINFO* pInfo);

static char szSoundPath[] = "WiiU/Sound/";

extern "C" void* MEMAllocFromDefaultHeapEx(unsigned int uiSize, int iAlignment);
extern "C" void MEMFreeToDefaultHeap(void* pBlock);

struct MilesMutex;

struct MilesDriver
{
    unsigned char ucReserved[0x6C];
    void (*pReportMutex)(MilesDriver*, MilesMutex*, int, const char*, int, void*, const char*);
};

extern int g_milesMutexEnabled;
extern MilesDriver* g_milesDriver;
extern MilesMutex g_milesMutex;
extern unsigned char g_milesMutexDiagnostic;

void rrMutexUnlock(MilesMutex* pMutex);

void OutMilesMutex()
{
    if (g_milesMutexEnabled != 0)
    {
        if (g_milesDriver != NULL)
        {
            g_milesDriver->pReportMutex(g_milesDriver, &g_milesMutex, 1,
                                        "C:\\devel\\projects\\mss\\src\\sdk\\shared\\genericmss.cpp",
                                        100, &g_milesMutexDiagnostic, "Miles mutex");
        }

        rrMutexUnlock(&g_milesMutex);
    }
}

void* platform_alloc(unsigned int uiSize)
{
    return MEMAllocFromDefaultHeapEx(uiSize, 0x40);
}

void platform_free(void* pBlock)
{
    MEMFreeToDefaultHeap(pBlock);
}

PlatformSoundEngineImpl::PlatformSoundEngineImpl()
{
    m_pDigitalDriver = NULL;
    m_pStream = NULL;
    m_pStreamDRC = NULL;
}

PlatformSoundEngineImpl::~PlatformSoundEngineImpl()
{
}

int PlatformSoundEngineImpl::initAX(void* pArg)
{
    AXInit();
    return 0;
}

void PlatformSoundEngineImpl::ErrorCallback(long long iErrorCode, const char* pErrorText)
{
    AIL_last_error();
}

float PlatformSoundEngineImpl::custom_falloff_function(_SAMPLE* pSample, float fDistance, float fRolloffFactor, float fMinDistance, float fMaxDistance)
{
    float fVolume;

    if (fMaxDistance == 10000.0f)
    {
        return 1.0f;
    }

    fVolume = 1.0f - fDistance / fMaxDistance;

    if (fVolume < 0.0f)
    {
        return 0.0f;
    }

    if (fVolume > 1.0f)
    {
        return 1.0f;
    }

    return fVolume;
}

void PlatformSoundEngineImpl::init(char* pSoundBankPath, SoundBankLoadedCallback pSoundBankLoaded)
{
    C4JThreadImpl* pAXInitThread;
    unsigned int uiProvider;
    char szSoundBankFile[256];

    RIB_load_static_provider_library(BinkADec_RIB_Main, "BinkADec");

    pAXInitThread = new C4JThreadImpl(initAX, NULL, "AXInitThread", 0);
    pAXInitThread->SetProcessor(2);
    pAXInitThread->Run();
    pAXInitThread->WaitForCompletion(-1);
    delete pAXInitThread;

    MIXInit();

    AIL_set_file_callbacks(file_open_callback, file_close_callback, file_seek_callback, file_read_callback);
    AIL_startup();

    uiProvider = AIL_output_ax(3);

    m_pDigitalDriver = AIL_open_generic_digital_driver(32000, 16, 128, 0, uiProvider);

    if (m_pDigitalDriver == NULL)
    {
        AIL_shutdown();
        return;
    }

    MilesSetEventErrorCallback(ErrorCallback);
    AIL_set_3D_rolloff_factor(m_pDigitalDriver, 1.0f);

    if (MilesStartupEventSystem(m_pDigitalDriver, 0x20000, 0, 0x40000) == 0)
    {
        AIL_close_digital_driver(m_pDigitalDriver);
        AIL_shutdown();
        return;
    }

    sprintf(szSoundBankFile, "%s", szSoundPath);
    strcat(szSoundBankFile, "Minecraft.msscmp");

    m_pSoundBank = MilesAddSoundBank(szSoundBankFile, 0);

    if (m_pSoundBank == NULL)
    {
        AIL_last_error();
        AIL_close_digital_driver(m_pDigitalDriver);
        AIL_shutdown();
        return;
    }

    MilesEnqueueEventByName("Minecraft/CacheSounds");
}

void PlatformSoundEngineImpl::tick(void* pUserData, float fListenerX, float fListenerY, float fListenerZ, float fListenerForwardX, float fListenerForwardY, float fListenerForwardZ, ConfigureSoundCallback pConfigureSound, UpdateSoundCallback pUpdateSound, RouteToDRCCallback pRouteToDRC, bool* pbSystemMusicPlaying)
{
    _MILESSOUNDINSTANCEINFO info;
    unsigned int uiNext;

    *pbSystemMusicPlaying = false;

    AIL_set_listener_3D_position(m_pDigitalDriver, fListenerX, fListenerY, fListenerZ);
    AIL_set_listener_3D_orientation(m_pDigitalDriver, fListenerForwardX, fListenerForwardY, fListenerForwardZ, 0.0f, 1.0f, 0.0f);

    MilesBeginEventQueueProcessing();

    uiNext = 0xFFFFFFFF;

    while (MilesEnumerateSoundInstances(NULL, &uiNext, 0, NULL, 0, 0, &info) != 0)
    {
        info.pSample->bOutputToDRC = pRouteToDRC(pUserData, info.pSoundInstance);

        if (info.uiType == 1)
        {
            bool bIs3D;
            float fMinDistance;
            float fPlaybackRate;
            float fX;
            float fY;
            float fZ;
            float fVolume;

            pConfigureSound(pUserData, info.pSoundInstance, &bIs3D, &fMinDistance, &fPlaybackRate, &fX, &fY, &fZ, &fVolume);

            AIL_register_falloff_function_callback(info.pSample, custom_falloff_function);

            if (fPlaybackRate != 0.0f)
            {
                AIL_set_sample_playback_rate_factor(info.pSample, fPlaybackRate);
            }

            if (bIs3D)
            {
                AIL_set_sample_is_3D(info.pSample, 1);
                AIL_set_sample_3D_distances(info.pSample, fMinDistance, 1.0f, 0);
                AIL_set_sample_3D_position(info.pSample, fX, fY, fZ);
            }
            else
            {
                AIL_set_sample_is_3D(info.pSample, 0);
            }

            AIL_set_sample_volume_levels(info.pSample, fVolume, fVolume);

            pUpdateSound(pUserData, info.pSoundInstance, &bIs3D, 0, &fX, &fY, &fZ, &fVolume);
        }
        else if (info.uiType == 4)
        {
            bool bIs3D;
            float fX;
            float fY;
            float fZ;
            float fVolume;

            pUpdateSound(pUserData, info.pSoundInstance, &bIs3D, 2, &fX, &fY, &fZ, &fVolume);
        }
        else
        {
            bool bIs3D;
            float fX;
            float fY;
            float fZ;
            float fVolume;

            pUpdateSound(pUserData, info.pSoundInstance, &bIs3D, 1, &fX, &fY, &fZ, &fVolume);

            if (bIs3D)
            {
                AIL_set_sample_3D_position(info.pSample, fX, fY, fZ);
            }

            AIL_set_sample_volume_levels(info.pSample, fVolume, fVolume);
        }
    }

    MilesCompleteEventQueueProcessing();
}

void PlatformSoundEngineImpl::startSound(void* pSoundData, int iSoundDataSize, char* pEventName)
{
    unsigned int uiQueue;

    uiQueue = AIL_enqueue_event_start();
    AIL_enqueue_event_buffer(&uiQueue, pSoundData, iSoundDataSize, 0);
    AIL_enqueue_event_end_named(uiQueue, pEventName);
}

bool PlatformSoundEngineImpl::openStream(char* pFileName)
{
    m_pStreamDRC = AIL_open_stream(m_pDigitalDriver, pFileName, 0);

    if (m_pStreamDRC != NULL)
    {
        m_pStream = AIL_open_stream(m_pDigitalDriver, pFileName, 0);

        if (m_pStream != NULL)
        {
            return true;
        }
    }

    return false;
}

void PlatformSoundEngineImpl::startStream(void* pUserData, ConfigureSoundCallback pConfigureSound)
{
    _SAMPLE* pSample;
    _SAMPLE* pSampleDRC;
    bool bIs3D;
    float fMinDistance;
    float fPlaybackRate;
    float fX;
    float fY;
    float fZ;
    float fVolume;

    pConfigureSound(pUserData, NULL, &bIs3D, &fMinDistance, &fPlaybackRate, &fX, &fY, &fZ, &fVolume);

    pSample = AIL_stream_sample_handle(m_pStream);
    pSampleDRC = AIL_stream_sample_handle(m_pStreamDRC);
    pSampleDRC->bOutputToDRC = true;

    AIL_register_falloff_function_callback(pSample, custom_falloff_function);

    if (bIs3D)
    {
        AIL_set_sample_is_3D(pSample, 1);
        AIL_set_sample_3D_distances(pSample, fMinDistance, 1.0f, 0);
        AIL_set_sample_3D_position(pSample, fX, fY, fZ);
        AIL_set_sample_is_3D(pSampleDRC, 1);
        AIL_set_sample_3D_distances(pSampleDRC, fMinDistance, 1.0f, 0);
        AIL_set_sample_3D_position(pSampleDRC, fX, fY, fZ);
    }
    else
    {
        AIL_set_sample_is_3D(pSample, 0);
        AIL_set_sample_is_3D(pSampleDRC, 0);
    }

    AIL_set_sample_playback_rate_factor(pSample, fPlaybackRate);
    AIL_set_sample_volume_levels(pSample, fVolume, fVolume);
    AIL_start_stream(m_pStream);

    AIL_set_sample_playback_rate_factor(pSampleDRC, fPlaybackRate);
    AIL_set_sample_volume_levels(pSampleDRC, fVolume, fVolume);
    AIL_start_stream(m_pStreamDRC);
}

void PlatformSoundEngineImpl::setStreamVol(float fVolume)
{
    _SAMPLE* pSample;
    _SAMPLE* pSampleDRC;

    pSample = AIL_stream_sample_handle(m_pStream);
    AIL_set_sample_volume_levels(pSample, fVolume, fVolume);

    pSampleDRC = AIL_stream_sample_handle(m_pStreamDRC);
    pSampleDRC->bOutputToDRC = true;
    AIL_set_sample_volume_levels(pSampleDRC, fVolume, fVolume);
}

void PlatformSoundEngineImpl::stopStream()
{
    AIL_pause_stream(m_pStream, 1);
    AIL_close_stream(m_pStream);
    m_pStream = NULL;

    AIL_pause_stream(m_pStreamDRC, 1);
    AIL_close_stream(m_pStreamDRC);
    m_pStreamDRC = NULL;
}

void PlatformSoundEngineImpl::updateStreamPos(void* pUserData, UpdateSoundCallback pUpdateSound)
{
    bool bIs3D;
    float fX;
    float fY;
    float fZ;
    float fVolume;

    pUpdateSound(pUserData, NULL, &bIs3D, 1, &fX, &fY, &fZ, &fVolume);

    if (bIs3D)
    {
        AIL_set_sample_3D_position(AIL_stream_sample_handle(m_pStream), fX, fY, fZ);
    }
}

bool PlatformSoundEngineImpl::isStreamComplete()
{
    if (m_pStream != NULL && AIL_stream_status(m_pStream) == 2)
    {
        AIL_close_stream(m_pStream);
        m_pStream = NULL;
        AIL_close_stream(m_pStreamDRC);
        m_pStreamDRC = NULL;
        return true;
    }

    return false;
}

void PlatformSoundEngineImpl::shutdown()
{
    AIL_shutdown();
}
