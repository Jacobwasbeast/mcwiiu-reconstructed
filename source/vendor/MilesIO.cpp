#include "vendor/MilesIO.hpp"
#include "vendor/MilesMarker.hpp"

#include "game/client/audio/EventSystem.hpp"
#include "game/core/PODHandleArray.hpp"
#include "game/util/LegacyUtilityFunctions.hpp"

#include <stddef.h>
#include <string.h>

#include "vendor/MilesApi.hpp"
#include "vendor/MilesAsync.hpp"
#include "vendor/MilesBank.hpp"
#include "vendor/MilesCoreApi.hpp"
#include "vendor/MilesMem.hpp"
#include "vendor/MilesRR.hpp"

static const unsigned int MILES_IO_WHOLE_FILE = 0xFFFFFFFF;
static const unsigned int MILES_IO_SEEK_SET = 0;
static const unsigned int MILES_IO_SEEK_CURRENT = 1;
static const unsigned int MILES_IO_SEEK_END = 2;

static const int CONTAINER_NAME_SIZE = 260;
static const int AUDITION_STATUS_CONNECTED = 2;
static const int AUDITION_SLEEP_MILLISECONDS = 0;

struct AuditionBankHeader
{
    int magic;
    char name[1];
};

static const char NO_AUDITION_BANK_NAME_ERROR[] = "Can't audition without a sound bank name.";

static const char MILES_IO_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\io.cpp";

AIL_FILE_OPEN_CALLBACK AIL_file_open = DefaultFileOpen;
AIL_FILE_CLOSE_CALLBACK AIL_file_close = DefaultFileClose;
AIL_FILE_SEEK_CALLBACK AIL_file_seek = DefaultFileSeek;
AIL_FILE_READ_CALLBACK AIL_file_read = DefaultFileRead;

unsigned int DefaultFileOpen(const char* pFileName, unsigned int* puFileHandle)
{
    MilesFile* pFile;
    const char* pCursor;
    const char* pNumber;
    int bNegative;
    char* pOutput;
    char szName[512];
    unsigned int uLength;
    unsigned int uOffset;
    unsigned int uSize;
    int iCharacter;
    int iOpened;

    pFile = (MilesFile*)AIL_mem_alloc_lock_info((int)sizeof(MilesFile), MILES_IO_FILE, 71);
    if (pFile == NULL)
    {
        return 0;
    }

    memset(pFile, 0, sizeof(MilesFile));

    uLength = MILES_IO_WHOLE_FILE;
    uOffset = 0;

    if (*pFileName == '*')
    {
        pCursor = pFileName;
        pOutput = szName;

        for (;;)
        {
            iCharacter = (int)*++pCursor;
            if (iCharacter == '\0' || iCharacter == '*')
            {
                break;
            }

            *pOutput = (char)iCharacter;
            pOutput = pOutput + 1;
        }

        *pOutput = '\0';

        if (*pCursor == '*')
        {
            pNumber = pCursor + 1;
            iCharacter = (int)*pNumber;
            bNegative = iCharacter == '-';
            if (bNegative)
            {
                pNumber = pCursor + 2;
                iCharacter = (int)*pNumber;
            }

            uLength = 0;
            while ((unsigned int)(iCharacter - '0') < 10)
            {
                pNumber = pNumber + 1;
                uLength = uLength * 10 + (unsigned int)(iCharacter - '0');
                iCharacter = (int)*pNumber;
            }

            if (bNegative)
            {
                uLength = (unsigned int)(-(int)uLength);
            }

            if (iCharacter == '*')
            {
                pCursor = pNumber + 1;
                iCharacter = (int)*pCursor;
                bNegative = iCharacter == '-';
                if (bNegative)
                {
                    pCursor = pNumber + 2;
                    iCharacter = (int)*pCursor;
                }

                uOffset = 0;
                while ((unsigned int)(iCharacter - '0') < 10)
                {
                    pCursor = pCursor + 1;
                    uOffset = uOffset * 10 + (unsigned int)(iCharacter - '0');
                    iCharacter = (int)*pCursor;
                }

                if (bNegative)
                {
                    uOffset = (unsigned int)(-(int)uOffset);
                }
            }
        }

        iOpened = Platform_OpenFile(pFile, szName);
    }
    else
    {
        iOpened = Platform_OpenFile(pFile, pFileName);
    }

    if (iOpened == 0)
    {
        AIL_mem_free_lock(pFile);
        return 0;
    }

    uSize = pFile->uSize;
    if (uOffset < uSize)
    {
        if (uLength > uSize - uOffset)
        {
            uLength = uSize - uOffset;
        }
    }
    else
    {
        uOffset = uSize;
        uLength = 0;
    }

    pFile->uLength = uLength;

    if (uOffset != MILES_IO_WHOLE_FILE)
    {
        pFile->uBaseOffset = uOffset;
    }

    *puFileHandle = (unsigned int)pFile;
    return 1;
}

void DefaultFileClose(unsigned int uFileHandle)
{
    MilesFile* pFile;

    pFile = (MilesFile*)uFileHandle;
    if (pFile == NULL)
    {
        return;
    }

    if (pFile->iDoNotClose == 0)
    {
        Platform_CloseFile(pFile);
    }

    AIL_mem_free_lock(pFile);
}

int DefaultFileSeek(unsigned int uFileHandle, int iOffset, unsigned int uType)
{
    MilesFile* pFile;
    unsigned int uLength;
    unsigned int uPosition;

    pFile = (MilesFile*)uFileHandle;
    if (pFile == NULL)
    {
        return 0;
    }

    if (uType == MILES_IO_SEEK_END)
    {
        uLength = pFile->uLength;
        uPosition = uLength + (unsigned int)iOffset;
    }
    else
    {
        uPosition = (unsigned int)iOffset;
        if (uType == MILES_IO_SEEK_CURRENT)
        {
            uPosition = pFile->uPosition + uPosition;
        }

        uLength = pFile->uLength;
    }

    pFile->uPosition = uPosition;

    if (uPosition > uLength)
    {
        pFile->uPosition = uLength;
        uPosition = uLength;
    }

    Platform_SeekFromBeginning(pFile, uPosition);
    return (int)pFile->uPosition;
}

int DefaultFileRead(unsigned int uFileHandle, void* pBuffer, unsigned int uBytes)
{
    MilesFile* pFile;
    int iBytesRead;

    pFile = (MilesFile*)uFileHandle;
    if (pFile == NULL)
    {
        return 0;
    }

    if (pFile->uPosition + uBytes > pFile->uLength)
    {
        uBytes = pFile->uLength - pFile->uPosition;
    }

    iBytesRead = Platform_ReadFile(pFile, pBuffer, uBytes);
    pFile->uPosition = pFile->uPosition + (unsigned int)iBytesRead;
    return iBytesRead;
}

void CloseAsync(MilesAsyncRead* pRead)
{
    if (pRead->uFileHandle == 0)
    {
        return;
    }

    pRead->iLength = 0;
    pRead->iLengthHigh = 0;
    pRead->uFlags = 0;

    MilesAsyncFileRead(pRead);
    MilesAsyncFileStatus(pRead, MILES_IO_WHOLE_FILE);
}

PODHandleArray<SoundBank*> g_soundBanks;
EventSystem* g_eventSystem = NULL;

MilesContainerFreeAllCallback g_milesContainerFreeAllCallback = Default_FreeAll;
MilesContainerGetPresetCallback g_milesContainerGetPresetCallback = Default_GetPreset;
MilesContainerGetEnvironmentCallback g_milesContainerGetEnvironmentCallback = Default_GetEnvironment;
MilesContainerGetSoundCallback g_milesContainerGetSoundCallback = Default_GetSound;
MilesContainerGetEventCallback g_milesContainerGetEventCallback = Default_GetEvent;
MilesContainerGetMarkerListCallback g_milesContainerGetMarkerListCallback = Default_GetMarkerList;
MilesContainerGetLoadedCountCallback g_milesContainerGetLoadedCountCallback = Default_GetLoadedCount;
MilesContainerGetMemUsageCallback g_milesContainerGetMemUsageCallback = Default_GetMemUsage;
MilesContainerGetLoadedNameCallback g_milesContainerGetLoadedNameCallback = Default_GetLoadedName;
MilesAuditionAsyncQueuedCallback g_milesAuditionAsyncQueuedCallback = NULL;
MilesAuditionAsyncLoadCallback g_milesAuditionAsyncLoadCallback = NULL;
MilesAuditionAsyncCancelCallback g_milesAuditionAsyncCancelCallback = NULL;
MilesAuditionSetPersistCallback g_milesAuditionSetPersistCallback = NULL;
MilesAuditionRampStateCallback g_milesAuditionRampStateCallback = NULL;
MilesAuditionStatusCallback g_milesAuditionStatusCallback = NULL;
MilesAuditionPumpCallback g_milesAuditionPumpCallback = NULL;
MilesAuditionOpenBankCallback g_milesAuditionOpenBankCallback = NULL;
MilesAuditionOpenCompleteCallback g_milesAuditionOpenCompleteCallback = NULL;
MilesAuditionCloseBankCallback g_milesAuditionCloseBankCallback = NULL;
MilesAuditionSuppressCallback g_milesAuditionSuppressCallback = NULL;
MilesAuditionFrameCallback g_milesAuditionFrameStartCallback = NULL;
MilesAuditionFrameCallback g_milesAuditionFrameEndCallback = NULL;
MilesAuditionFrameCallback g_milesAuditionDefragStartCallback = NULL;
MilesAuditionEventCallback g_milesAuditionEventCallback = NULL;
MilesAuditionSoundCallback g_milesAuditionSoundCallback = NULL;
MilesAuditionSoundIdCallback g_milesAuditionSoundCompleteCallback = NULL;
MilesAuditionSoundIdCallback g_milesAuditionSoundPlayingCallback = NULL;
MilesAuditionSoundFlagsCallback g_milesAuditionSoundFlagsCallback = NULL;
MilesAuditionSoundLimitedCallback g_milesAuditionSoundLimitedCallback = NULL;
MilesAuditionControlCallback g_milesAuditionControlCallback = NULL;
MilesAuditionSoundBusCallback g_milesAuditionSoundBusCallback = NULL;
MilesAuditionAsyncErrorCallback g_milesAuditionAsyncErrorCallback = NULL;
MilesAuditionAsyncCompleteCallback g_milesAuditionAsyncCompleteCallback = NULL;
MilesAuditionListenerPositionCallback g_milesAuditionListenerPositionCallback = NULL;
MilesAuditionSoundPositionCallback g_milesAuditionSoundPositionCallback = NULL;
MilesAuditionSendCPUCallback g_milesAuditionSendCPUCallback = NULL;
MilesAuditionCountCallback g_milesAuditionUpdateDataCountCallback = NULL;
MilesAuditionCountCallback g_milesAuditionSendCountCallback = NULL;
MilesAuditionHandleSystemLoadCallback g_milesAuditionHandleSystemLoadCallback = NULL;
MilesAuditionVarStateCallback g_milesAuditionVarStateCallback = NULL;
MilesAuditionSoundStateCallback g_milesAuditionSoundStateCallback = NULL;
MilesAuditionClearStateCallback g_milesAuditionClearStateCallback = NULL;
MilesAuditionCompletionEventCallback g_milesAuditionCompletionEventCallback = NULL;
MilesAuditionAddRampCallback g_milesAuditionAddRampCallback = NULL;

SoundBank* GetContainer(const char* name, const char** assetName)
{
    const char* separator = strchr(name, '/');
    if (separator == NULL)
    {
        return NULL;
    }

    const unsigned int length = static_cast<unsigned int>(separator - name);
    char bankName[CONTAINER_NAME_SIZE];
    if (length == 0)
    {
        bankName[0] = '\0';
    }
    else
    {
        strncpy(bankName, name, length);
        bankName[length] = '\0';
    }

    g_soundBanks.Reacquire();
    int count = g_soundBanks.Count();
    SoundBank** banks = g_soundBanks.Data();
    while (count != 0)
    {
        if (reinterpret_cast<SoundBankHeader*>(*banks)->magic != AUDIO_BANK_MAGIC)
        {
            if (AIL_stricmp(AIL_get_soundbank_name(*banks), bankName) == 0)
            {
                *assetName = separator + 1;
                return *banks;
            }
        }

        --count;
        ++banks;
    }

    *assetName = separator + 1;
    return NULL;
}

int Default_GetSound(const char* name, char* path, _MILESBANKSOUNDINFO* info)
{
    const char* assetName = NULL;
    SoundBank* bank = GetContainer(name, &assetName);
    if (bank != NULL)
    {
        return AIL_sound_asset_info(bank, name, path, info);
    }

    return 0;
}

int Default_GetLoadedCount()
{
    return g_soundBanks.Count();
}

const char* Default_GetLoadedName(int index)
{
    g_soundBanks.Reacquire();
    SoundBank* bank = g_soundBanks[index];
    if (reinterpret_cast<AuditionBankHeader*>(bank)->magic == AUDIO_BANK_MAGIC)
    {
        return reinterpret_cast<AuditionBankHeader*>(bank)->name;
    }

    return AIL_get_soundbank_name(bank);
}

int Default_GetMemUsage()
{
    int usage = 0;

    g_soundBanks.Reacquire();
    int count = g_soundBanks.Count();
    SoundBank** banks = g_soundBanks.Data();
    while (count != 0)
    {
        if (reinterpret_cast<SoundBankHeader*>(*banks)->magic == SOUND_BANK_MAGIC)
        {
            usage += AIL_get_soundbank_mem_usage(*banks);
        }

        --count;
        ++banks;
    }

    return usage;
}

void Default_FreeAll()
{
    g_soundBanks.Reacquire();
    int count = g_soundBanks.Count();
    SoundBank** banks = g_soundBanks.Data();
    while (count != 0)
    {
        if (reinterpret_cast<SoundBankHeader*>(*banks)->magic == SOUND_BANK_MAGIC)
        {
            AIL_close_soundbank(*banks);
        }
        else
        {
            Audition_CloseBank(*banks);
        }

        --count;
        ++banks;
    }

    g_soundBanks.FreeMem();
}

void* Default_GetEnvironment(const char* name)
{
    const char* assetName = NULL;
    SoundBank* bank = GetContainer(name, &assetName);
    if (bank != NULL)
    {
        return AIL_find_environment_preset(bank, name);
    }

    return NULL;
}

const unsigned char* Default_GetEvent(const char* name)
{
    const char* assetName = NULL;
    SoundBank* bank = GetContainer(name, &assetName);
    const unsigned char* event = NULL;
    if (bank != NULL)
    {
        AIL_get_event_contents(bank, name, &event);
    }

    return event;
}

const MilesMarkerList* Default_GetMarkerList(const char* name)
{
    const char* assetName = NULL;
    SoundBank* bank = GetContainer(name, &assetName);
    if (bank != NULL)
    {
        return AIL_get_marker_list(bank, name);
    }

    return 0;
}

void* Default_GetPreset(const char* name)
{
    const char* assetName = NULL;
    SoundBank* bank = GetContainer(name, &assetName);
    if (bank != NULL)
    {
        return AIL_find_sound_preset(bank, name);
    }

    return NULL;
}

void EvalFunc(void* instance, const char* name, int index, void* value, int flags)
{
    if (instance != NULL)
    {
        SoundInstanceData* data = static_cast<SoundInstanceData*>(instance);
        Handle context = data->m_context;
        Var_LookupWithContext(data->m_eventSystem, context, name, flags, value);
        return;
    }

    Handle context = g_eventSystem->m_variables;
    Var_LookupWithContext(g_eventSystem, context, name, flags, value);
}

extern "C" int Container_GetSound(const char* name, char* path, _MILESBANKSOUNDINFO* info)
{
    return g_milesContainerGetSoundCallback(name, path, info);
}

extern "C" void* Container_GetPreset(const char* name)
{
    return g_milesContainerGetPresetCallback(name);
}

extern "C" void Audition_AsyncQueued(unsigned long long eventId, int auditionId, const char* name)
{
    if (g_milesAuditionAsyncQueuedCallback)
        g_milesAuditionAsyncQueuedCallback(eventId, auditionId, name);
}

extern "C" void Audition_AsyncLoad(int auditionId, int size)
{
    if (g_milesAuditionAsyncLoadCallback)
        g_milesAuditionAsyncLoadCallback(auditionId, size);
}

extern "C" void Audition_AsyncCancel(int auditionId)
{
    if (g_milesAuditionAsyncCancelCallback)
        g_milesAuditionAsyncCancelCallback(auditionId);
}

extern "C" void Audition_SetPersist(unsigned long long eventId, const char* name, const char* preset)
{
    if (g_milesAuditionSetPersistCallback)
        g_milesAuditionSetPersistCallback(eventId, name, preset);
}

extern "C" void Audition_RampState(const char* name, unsigned long long eventId, int target)
{
    if (g_milesAuditionRampStateCallback)
        g_milesAuditionRampStateCallback(name, eventId, target);
}

extern "C" void* Container_GetEnvironment(const char* name)
{
    return g_milesContainerGetEnvironmentCallback(name);
}

extern "C" const unsigned char* Container_GetEvent(const char* name)
{
    return g_milesContainerGetEventCallback(name);
}

extern "C" const MilesMarkerList* Container_GetMarkerList(const char* name)
{
    return g_milesContainerGetMarkerListCallback(name);
}

extern "C" int Audition_Status()
{
    if (g_milesAuditionStatusCallback)
        return g_milesAuditionStatusCallback(0);
    return 0;
}

extern "C" int Audition_Pump()
{
    if (g_milesAuditionPumpCallback)
        return g_milesAuditionPumpCallback(0);
    return 0;
}

extern "C" SoundBank* Audition_OpenBank(const char* name)
{
    SoundBank* bank = NULL;
    if (g_milesAuditionOpenBankCallback)
        bank = g_milesAuditionOpenBankCallback(name);
    return bank;
}

extern "C" int Audition_OpenComplete(SoundBank* bank)
{
    if (g_milesAuditionOpenCompleteCallback)
        return g_milesAuditionOpenCompleteCallback(bank);
    return 0;
}

extern "C" void Audition_CloseBank(SoundBank* bank)
{
    if (g_milesAuditionCloseBankCallback)
        g_milesAuditionCloseBankCallback(bank);
}

extern "C" void Audition_Suppress(int suppress)
{
    if (g_milesAuditionSuppressCallback)
        g_milesAuditionSuppressCallback(suppress);
}

extern "C" void Audition_FrameStart()
{
    if (g_milesAuditionFrameStartCallback)
        g_milesAuditionFrameStartCallback();
}

extern "C" void Audition_FrameEnd()
{
    if (g_milesAuditionFrameEndCallback)
        g_milesAuditionFrameEndCallback();
}

extern "C" void Audition_DefragStart()
{
    if (g_milesAuditionDefragStartCallback)
        g_milesAuditionDefragStartCallback();
}

extern "C" void Audition_Event(const char* name, unsigned long long serial,
                               unsigned long long context, int fromBank, const void* initBlock,
                               int initBlockSize)
{
    if (g_milesAuditionEventCallback)
        g_milesAuditionEventCallback(name, serial, context, fromBank, initBlock, initBlockSize);
}

extern "C" void Audition_Sound(float volume, float pitch, unsigned long long serial,
                               unsigned long long soundId, const char* name, const char* labels,
                               int delay)
{
    if (g_milesAuditionSoundCallback)
        g_milesAuditionSoundCallback(volume, pitch, serial, soundId, name, labels, delay);
}

extern "C" void Audition_SoundComplete(unsigned long long soundId)
{
    if (g_milesAuditionSoundCompleteCallback)
        g_milesAuditionSoundCompleteCallback(soundId);
}

extern "C" void Audition_SoundPlaying(unsigned long long soundId)
{
    if (g_milesAuditionSoundPlayingCallback)
        g_milesAuditionSoundPlayingCallback(soundId);
}

extern "C" void Audition_SoundFlags(unsigned long long soundId, unsigned char flags)
{
    if (g_milesAuditionSoundFlagsCallback)
        g_milesAuditionSoundFlagsCallback(soundId, flags);
}

extern "C" void Audition_SoundLimited(unsigned long long soundId, const char* limit)
{
    if (g_milesAuditionSoundLimitedCallback)
        g_milesAuditionSoundLimitedCallback(soundId, limit);
}

extern "C" void Audition_Control(unsigned long long serial, const char* name, int action,
                                 unsigned long long context)
{
    if (g_milesAuditionControlCallback)
        g_milesAuditionControlCallback(serial, name, action, context);
}

extern "C" void Audition_SoundBus(unsigned long long soundId, int bus)
{
    if (g_milesAuditionSoundBusCallback)
        g_milesAuditionSoundBusCallback(soundId, bus);
}

extern "C" void Audition_AsyncError(int auditionId)
{
    if (g_milesAuditionAsyncErrorCallback)
        g_milesAuditionAsyncErrorCallback(auditionId);
}

extern "C" void Audition_AsyncComplete(int auditionId, int size)
{
    if (g_milesAuditionAsyncCompleteCallback)
        g_milesAuditionAsyncCompleteCallback(auditionId, size);
}

extern "C" void Audition_ListenerPosition(float x, float y, float z)
{
    if (g_milesAuditionListenerPositionCallback)
        g_milesAuditionListenerPositionCallback(x, y, z);
}

extern "C" void Audition_SoundPosition(float x, float y, float z, unsigned long long soundId)
{
    if (g_milesAuditionSoundPositionCallback)
        g_milesAuditionSoundPositionCallback(x, y, z, soundId);
}

extern "C" void Audition_SendCPU(_DIG_DRIVER* driver)
{
    if (g_milesAuditionSendCPUCallback)
        g_milesAuditionSendCPUCallback(driver);
}

extern "C" void Audition_UpdateDataCount(int count)
{
    if (g_milesAuditionUpdateDataCountCallback)
        g_milesAuditionUpdateDataCountCallback(count);
}

extern "C" void Audition_SendCount(int count)
{
    if (g_milesAuditionSendCountCallback)
        g_milesAuditionSendCountCallback(count);
}

extern "C" void Audition_HandleSystemLoad(int used, int total)
{
    if (g_milesAuditionHandleSystemLoadCallback)
        g_milesAuditionHandleSystemLoadCallback(used, total);
}

extern "C" void Audition_VarState(const char* name, unsigned long long soundId, int isInteger,
                                  const void* value)
{
    if (g_milesAuditionVarStateCallback)
        g_milesAuditionVarStateCallback(name, soundId, isInteger, value);
}

extern "C" void Audition_SoundState(float volume, float lowPass, float blendVolume,
                                    float blendPitch, float levelA, float levelB, float rate,
                                    float levelC, unsigned long long soundId)
{
    if (g_milesAuditionSoundStateCallback)
        g_milesAuditionSoundStateCallback(volume, lowPass, blendVolume, blendPitch, levelA, levelB,
                                          rate, levelC, soundId);
}

extern "C" void Audition_ClearState()
{
    if (g_milesAuditionClearStateCallback)
        g_milesAuditionClearStateCallback();
}

extern "C" void Audition_CompletionEvent(unsigned long long completionEvent,
                                         unsigned long long soundId)
{
    if (g_milesAuditionCompletionEventCallback)
        g_milesAuditionCompletionEventCallback(completionEvent, soundId);
}

extern "C" void Audition_AddRamp(unsigned long long soundId, int target, const char* name,
                                 const char* labels, int global, unsigned long long eventId)
{
    if (g_milesAuditionAddRampCallback)
        g_milesAuditionAddRampCallback(soundId, target, name, labels, global, eventId);
}

extern "C" SoundBank* MilesAddSoundBankInternal(const char* fileName, const char* bankName)
{
    if (Audition_Status() == AUDITION_STATUS_CONNECTED)
    {
        if (bankName == NULL)
        {
            AIL_set_error(NO_AUDITION_BANK_NAME_ERROR);
            return NULL;
        }

        SoundBank* bank = Audition_OpenBank(bankName);
        if (bank != NULL)
        {
            g_soundBanks.Reacquire();
            g_soundBanks.Push(bank);
            AIL_unlock_mutex();
            while (Audition_OpenComplete(bank) == 0)
            {
                AIL_sleep(AUDITION_SLEEP_MILLISECONDS);
            }

            AIL_lock_mutex();
            return bank;
        }
    }
    else
    {
        SoundBank* bank = AIL_open_soundbank(fileName, bankName);
        if (bank != NULL)
        {
            g_soundBanks.Reacquire();
            g_soundBanks.Push(bank);
            return bank;
        }
    }

    return NULL;
}
