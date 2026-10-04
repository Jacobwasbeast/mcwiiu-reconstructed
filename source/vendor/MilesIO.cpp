#include "vendor/MilesIO.hpp"

#include "game/client/audio/EventSystem.hpp"
#include "game/core/PODHandleArray.hpp"
#include "game/util/LegacyUtilityFunctions.hpp"

#include <stddef.h>
#include <string.h>

#include "vendor/MilesAsync.hpp"
#include "vendor/MilesMem.hpp"
#include "vendor/MilesRR.hpp"

static const unsigned int MILES_IO_WHOLE_FILE = 0xFFFFFFFF;
static const unsigned int MILES_IO_SEEK_SET = 0;
static const unsigned int MILES_IO_SEEK_CURRENT = 1;
static const unsigned int MILES_IO_SEEK_END = 2;
static const int CONTAINER_NAME_SIZE = 260;

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

MilesContainerGetSoundCallback g_milesContainerGetSoundCallback = Default_GetSound;
MilesContainerGetPresetCallback g_milesContainerGetPresetCallback = Default_GetPreset;
MilesAuditionAsyncQueuedCallback g_milesAuditionAsyncQueuedCallback = NULL;
MilesAuditionAsyncLoadCallback g_milesAuditionAsyncLoadCallback = NULL;
MilesAuditionAsyncCancelCallback g_milesAuditionAsyncCancelCallback = NULL;
MilesAuditionSetPersistCallback g_milesAuditionSetPersistCallback = NULL;
MilesAuditionRampStateCallback g_milesAuditionRampStateCallback = NULL;

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
