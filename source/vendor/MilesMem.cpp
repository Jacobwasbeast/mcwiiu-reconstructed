#include "vendor/MilesMem.hpp"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "game/client/audio/FalloffGraph.hpp"
#include "game/core/PODHandleArray.hpp"
#include "vendor/MilesApi.hpp"
#include "vendor/MilesAsync.hpp"
#include "vendor/MilesTime.hpp"

typedef void (*AIL_LOG_CALLBACK)(const char* pText, int iDepth);

static const char MILES_DEBUG_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\mssdbg.c";

void __debugbreak();

static const char AIL_indent[] = "                                ";

static const int AIL_LOG_PREFIX_LENGTH = 15;
static const unsigned int AIL_LOG_MAXIMUM_HOURS = 99;

int AIL_debug_enabled = 0;
int AIL_debug_nested = 0;
int AIL_debug_depth = 0;

static int AIL_log_start_ms = 0;
static AIL_LOG_CALLBACK AIL_log_callback = NULL;
static FILE* AIL_log_file = NULL;

MilesProfileSite AIL_debug_log_site;
MilesProfileSite MSS_alloc_site;
MilesProfileSite MSS_free_site;

static int ShouldLog()
{
    if (AIL_debug_enabled == 0)
    {
        return 0;
    }

    if (AIL_debug_depth != 1 && AIL_debug_nested == 0)
    {
        return 0;
    }

    return AIL_API_background() == 0;
}

extern "C" void AIL_debug_log(const char* pFormat, ...)
{
    va_list arguments;
    char szTimestamp[16];
    char szBuffer[512];
    char* pText;
    char* pLine;
    const char* pMessageFormat;
    const char* pDescription;
    unsigned int uElapsed;
    unsigned int uHours;

    if (AIL_debug_enabled == 0)
    {
        return;
    }

    va_start(arguments, pFormat);

    pText = szBuffer + AIL_LOG_PREFIX_LENGTH;
    pMessageFormat = pFormat;

    if (*pFormat == '!' || *pFormat == '@')
    {
        if (AIL_debug_depth > 1)
        {
            strcpy(pText, &AIL_indent[sizeof(AIL_indent) - AIL_debug_depth * 2]);
            pText = pText + AIL_debug_depth * 2 - 1;
        }

        pMessageFormat = pFormat + 1;
    }

    vsnprintf(pText, (sizeof(szBuffer) - 1) - (size_t)(pText - szBuffer), pMessageFormat,
              arguments);
    va_end(arguments);

    szBuffer[sizeof(szBuffer) - 1] = '\0';

    if (MSS_profile != NULL)
    {
        pDescription = MSS_profile->pDescribe(MSS_profile, szBuffer + AIL_LOG_PREFIX_LENGTH);
        MSS_profile->pMessage(MSS_profile, 0, 1, &AIL_debug_log_site, "%s", pDescription);
    }

    if (AIL_log_file == NULL && AIL_log_callback == NULL && MSS_profile != NULL)
    {
        return;
    }

    uElapsed = (unsigned int)(AIL_ms_count() - AIL_log_start_ms);

    uHours = uElapsed / 3600000;
    if (uHours > AIL_LOG_MAXIMUM_HOURS)
    {
        uHours = AIL_LOG_MAXIMUM_HOURS;
    }

    pLine = szBuffer + AIL_LOG_PREFIX_LENGTH;

    if (*pFormat == '@' || *pFormat == '!')
    {
        if (*pFormat == '@' && AIL_debug_depth == 1)
        {
            sprintf(szTimestamp, "[%.02d:%.02d:%.02d.%.03d] ", (int)uHours,
                    (int)((uElapsed / 60000) % 60), (int)((uElapsed / 1000) % 60),
                    (int)(uElapsed % 1000));
            memcpy(szBuffer, szTimestamp, AIL_LOG_PREFIX_LENGTH);
        }
        else
        {
            memset(szBuffer, ' ', AIL_LOG_PREFIX_LENGTH);
        }

        pLine = szBuffer;
    }

    if (AIL_log_file != NULL)
    {
        fprintf(AIL_log_file, "%s\n", pLine);
        return;
    }

    if (AIL_log_callback != NULL)
    {
        AIL_log_callback(pLine, AIL_debug_depth);
    }
}

extern "C" void AIL_set_error(const char* pError)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (ShouldLog() != 0)
    {
        if (pError == NULL)
        {
            AIL_debug_log("@AIL_set_error(NULL)");
        }
        else
        {
            AIL_debug_log("@AIL_set_error(%s)", pError);
        }
    }

    AIL_API_set_error(pError);

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void* AIL_mem_alloc_lock_info(int iSize, const char* pOwnerName, int iOwnerLine)
{
    void* pBlock;
    unsigned int uCallStack;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (ShouldLog() != 0)
    {
        AIL_debug_log("@AIL_mem_alloc_lock(%ld) from: %s, line: %d ", iSize, pOwnerName,
                      iOwnerLine);
    }

    pBlock = AIL_API_mem_alloc_lock((unsigned int)iSize);

    if (MSS_profile != NULL)
    {
        uCallStack = MSS_profile->pCallStack(MSS_profile, 0, 0);
        MSS_profile->pAlloc(MSS_profile, pBlock, 0, iSize, pOwnerName, iOwnerLine, &MSS_alloc_site,
                            "Miles alloc: %t", uCallStack);
    }

    if (ShouldLog() != 0)
    {
        outresuhex((unsigned int)pBlock);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return pBlock;
}

extern "C" void AIL_mem_free_lock(void* pBlock)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (ShouldLog() != 0)
    {
        AIL_debug_log("@AIL_mem_free_lock(0x%lX)", pBlock);
    }

    if (MSS_profile != NULL)
    {
        MSS_profile->pFree(MSS_profile, pBlock, MILES_DEBUG_FILE, 440, &MSS_free_site);
    }

    AIL_API_mem_free_lock(pBlock);

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void outresuhex(unsigned int uResult)
{
    AIL_debug_log("!-Result = 0x%lX", uResult);
}

extern "C" int AIL_ms_count()
{
    return (int)rrTimeToMillis(rrGetTime());
}

extern "C" void outresint(int iResult)
{
    AIL_debug_log("!-Result = %d", iResult);
}

struct HandleEntry
{
    int size;
    unsigned int generation;
    union
    {
        void* data;
        int nextFree;
    };
};

static const unsigned int HANDLE_GUARD = 0xDEADBEEF;
static const int HANDLE_BLOCK_OVERHEAD = 0x17;
static const unsigned int HANDLE_BLOCK_ALIGNMENT_MASK = 0xFFFFFFF0;
static const int HANDLE_TAG_FREE = 1;

static int sHandleFreeHead = 0;
static unsigned int sHandleGeneration = 1;
static int sHandleBreakOnError = 0;
static unsigned int sHandleCount = 0;
static unsigned char* sHandleTableEnd = NULL;
static unsigned char* sHandleHeapCursor = NULL;
static int sHandleNeedsDefrag = 0;

void* Handle_Get(Handle handle)
{
    if (handle.index != 0 && handle.index <= sHandleCount)
    {
        const HandleEntry* entry = reinterpret_cast<const HandleEntry*>(
            sHandleTableEnd - handle.index * sizeof(HandleEntry));
        if (entry->generation == handle.generation)
        {
            return entry->data;
        }
    }

    return NULL;
}

Handle Handle_AllocateInfo(int size, const char* file, int line)
{
    Handle handle;
    HandleEntry* entry;
    unsigned char* data;
    int index;
    unsigned int generation;
    int tableSize;
    int blockSize;

    blockSize = (size + HANDLE_BLOCK_OVERHEAD) & HANDLE_BLOCK_ALIGNMENT_MASK;
    tableSize = sHandleCount * sizeof(HandleEntry);

    if (sHandleFreeHead == 0)
    {
        tableSize = tableSize + sizeof(HandleEntry);
    }

    if (blockSize > (sHandleTableEnd - sHandleHeapCursor) - tableSize)
    {
        return Handle();
    }

    data = sHandleHeapCursor;

    if (sHandleFreeHead == 0)
    {
        index = sHandleCount + 1;
        sHandleCount = index;
    }
    else
    {
        index = -sHandleFreeHead;
        entry = reinterpret_cast<HandleEntry*>(sHandleTableEnd - index * sizeof(HandleEntry));
        sHandleFreeHead = sHandleFreeHead + entry->nextFree;
    }

    memset(data, 0, size);
    *reinterpret_cast<unsigned int*>(data + size) = HANDLE_GUARD;

    sHandleHeapCursor = sHandleHeapCursor + blockSize;
    reinterpret_cast<int*>(sHandleHeapCursor)[-1] = 0;

    generation = sHandleGeneration;
    sHandleGeneration = sHandleGeneration + 1;

    if (sHandleGeneration == 0)
    {
        sHandleGeneration = 1;
    }

    entry = reinterpret_cast<HandleEntry*>(sHandleTableEnd - index * sizeof(HandleEntry));
    entry->size = size;
    entry->generation = generation;
    entry->data = data;
    reinterpret_cast<int*>(data)[-1] = -index;

    handle.index = index;
    handle.generation = generation;
    return handle;
}

void Handle_Free(Handle handle)
{
    HandleEntry* entry;
    unsigned int index;
    int size;

    index = handle.index;

    if (index == 0)
    {
        return;
    }

    if (index > sHandleCount)
    {
        if (sHandleBreakOnError != 0)
        {
            __debugbreak();
        }

        return;
    }

    entry = reinterpret_cast<HandleEntry*>(sHandleTableEnd - index * sizeof(HandleEntry));
    size = entry->size;

    if (entry->generation != handle.generation)
    {
        if (sHandleBreakOnError != 0)
        {
            __debugbreak();
        }

        return;
    }

    if (*reinterpret_cast<unsigned int*>(static_cast<unsigned char*>(entry->data) + size) !=
            HANDLE_GUARD &&
        sHandleBreakOnError != 0)
    {
        __debugbreak();
    }

    entry->generation = 0;
    reinterpret_cast<int*>(entry->data)[-1] = HANDLE_TAG_FREE;
    *static_cast<unsigned int*>(entry->data) =
        (size + HANDLE_BLOCK_OVERHEAD) & HANDLE_BLOCK_ALIGNMENT_MASK;

    if (index == sHandleCount)
    {
        sHandleCount = sHandleCount - 1;
        sHandleNeedsDefrag = 1;
        return;
    }

    if (sHandleFreeHead == 0)
    {
        entry->nextFree = index;
    }
    else
    {
        entry->nextFree = sHandleFreeHead + index;
    }

    sHandleNeedsDefrag = 1;
    sHandleFreeHead = -static_cast<int>(index);
}

static const int SAMPLE_STATUS_FREE = 1;
static const int SAMPLE_STATUS_DONE = 2;

extern "C" void AIL_API_end_sample(_SAMPLE* pSample)
{
    int* pStatus;

    if (pSample == NULL)
    {
        return;
    }

    pStatus = pSample->driver->sampleStatus;

    if (pStatus[pSample->index] == SAMPLE_STATUS_FREE ||
        pStatus[pSample->index] == SAMPLE_STATUS_DONE)
    {
        return;
    }

    pStatus[pSample->index] = SAMPLE_STATUS_DONE;
    pSample->starved = 1;

    if (pSample->endOfBuffer != NULL)
    {
        pSample->endOfBuffer(pSample);
    }

    if (pSample->endOfSample != NULL)
    {
        pSample->endOfSample(pSample);
    }
}

extern "C" int AIL_API_sample_3D_position(_SAMPLE* pSample, float* pX, float* pY, float* pZ)
{
    if (pSample == NULL)
    {
        return 0;
    }

    if (pX != NULL)
    {
        *pX = pSample->x;
    }

    if (pY != NULL)
    {
        *pY = pSample->y;
    }

    if (pZ != NULL)
    {
        *pZ = pSample->z;
    }

    return pSample->is3D;
}

extern "C" void AIL_API_listener_3D_position(_LISTENER* pListener, float* pX, float* pY, float* pZ)
{
    if (pListener == NULL)
    {
        return;
    }

    if (pX != NULL)
    {
        *pX = pListener->x;
    }

    if (pY != NULL)
    {
        *pY = pListener->y;
    }

    if (pZ != NULL)
    {
        *pZ = pListener->z;
    }
}

extern "C" void AIL_end_sample(_SAMPLE* pSample)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_end_sample(0x%lX)", pSample);
    }

    InMilesMutex();
    AIL_API_end_sample(pSample);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" int AIL_sample_3D_position(_SAMPLE* pSample, float* pX, float* pY, float* pZ)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_3D_position(0x%lX,0x%lX,0x%lX,0x%lX)", pSample, pX, pY, pZ);
    }

    iResult = AIL_API_sample_3D_position(pSample, pX, pY, pZ);

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" void AIL_listener_3D_position(_LISTENER* pListener, float* pX, float* pY, float* pZ)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) &&
        AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_listener_3D_position(0x%lX,0x%lX,0x%lX,0x%lX)", pListener, pX, pY,
                      pZ);
    }

    AIL_API_listener_3D_position(pListener, pX, pY, pZ);

    AIL_debug_depth = AIL_debug_depth - 1;
}
