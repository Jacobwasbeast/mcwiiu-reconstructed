#include "vendor/MilesMem.hpp"

#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "game/client/audio/FalloffGraph.hpp"
#include "game/core/PODHandleArray.hpp"
#include "game/util/LegacyUtilityFunctions.hpp"
#include "vendor/MilesApi.hpp"
#include "vendor/MilesAsync.hpp"
#include "vendor/MilesEventError.hpp"
#include "vendor/MilesFilters.hpp"
#include "vendor/MilesIO.hpp"
#include "vendor/MilesRib.hpp"
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

namespace
{
const int PRESET_MAGIC = 0x50535420;
const int AUDIO_BANK_NAME_OFFSET = 8;
const int SOUND_ASSET_PATH_MARKER = '*';
const int SOUND_ASSET_INFO_OFFSET = 0xc;
const int SOUND_ASSET_INFO_SIZE = 0x2c;
const int SOUND_ASSET_NAME_OFFSET = 4;
const int SPEAKER_COUNT = 8;
const int CHANNEL_MATRIX_SIZE = 64;
const unsigned int PRESET_FLAG_SPEAKER_SCALES = 0x20000;
const unsigned int PRESET_FLAG_CHANNEL_LEVELS = 0x10000;
const int PRESET_CHANNEL_LIMIT = 8;
const unsigned int PRESET_CHANNEL_MASK = 0xffff;
const int PRESET_CHANNEL_SHIFT = 16;
const int PRESET_NO_INDEX = -1;
const int PRESET_PAIR_WORDS = 2;
const int PRESET_FALLOFF_HEADER_WORDS = 6;
const int PRESET_FALLOFF_POINT_WORDS = 8;
const int PRESET_PROCESSOR_ENTRY_WORDS = 3;
const int PRESET_PROCESSOR_HEADER_WORDS = 4;
const int PRESET_CHANNEL_LEVEL_WORDS = 5;
const unsigned int PRESET_COMPRESSOR_BUS_LIMIT = 3;
const int SAMPLE_STAGE_BASE = 1;
const int SAMPLE_REVERB_BUS_COMPRESSOR = 4;
const int GRAPH_MODE_LINEAR = 0;
const int GRAPH_MODE_STEP = 2;
const int ALL_CHANNELS = -1;
const double PRESET_DECIBEL_BASE = 10.0;
const float PRESET_DECIBELS_PER_DECADE = 20.0f;
const double PRESET_VOLUME_EXPONENT = 0.6;
const double PRESET_SEMITONE_BASE = 2.0;
const float PRESET_SEMITONES_PER_OCTAVE = 12.0f;
const float PRESET_UNIT = 1.0f;
const float PRESET_ZERO = 0.0f;
const float PRESET_HALF = 0.5f;
const float PRESET_DEFAULT_CONE_ANGLE = 360.0f;
const float PRESET_DEFAULT_MAXIMUM_DISTANCE = 200.0f;
const float GRAPH_TANGENT_DIVISOR = 3.0f;
const float GRAPH_CUBIC_COEFFICIENT = 3.0f;
const char PRESET_COMPRESSOR_FILTER[] = "Compressor Filter";
const char PRESET_COMPRESSOR_INPUT[] = "Input";
const char PRESET_FILTER_ERROR[] = "Sound Preset Error - Filter Not Found.";
const char PRESET_NOT_FOUND_ERROR[] = "Preset Not Found.";

enum PresetRecordType
{
    PRESET_CHANNEL_ROUTING = 0,
    PRESET_POSITION = 1,
    PRESET_CONE = 2,
    PRESET_DISTANCES = 3,
    PRESET_ORIENTATION = 4,
    PRESET_VELOCITY = 5,
    PRESET_VALUE = 6,
    PRESET_REVERB = 7,
    PRESET_LOW_PASS = 8,
    PRESET_OCCLUSION = 9,
    PRESET_EXCLUSION = 10,
    PRESET_OBSTRUCTION = 11,
    PRESET_PITCH = 12,
    PRESET_VOLUME_PAN = 13,
    PRESET_FALLOFF = 14,
    PRESET_PROCESSOR = 15,
    PRESET_CHANNEL_LEVEL = 16
};

union PresetValue
{
    int integer;
    float number;
};

struct PresetPair
{
    PresetValue value;
    int expression;
};

struct PresetHeader
{
    int magic;
    int reserved0;
    unsigned int flags;
    int size;
};

struct PresetProcessorEntry
{
    int nameOffset;
    PresetValue value;
    int expression;
};

PresetPair* PresetPairs(int* record)
{
    return reinterpret_cast<PresetPair*>(record + 1);
}

void ResolvePair(PresetHeader* header, void* instance, PresetEvalFunc evaluate, PresetPair* pair,
                 int flags)
{
    if (pair->expression != 0)
    {
        evaluate(instance, reinterpret_cast<char*>(header) + pair->expression, PRESET_NO_INDEX,
                 &pair->value, flags);
    }
}

int* PresetEnd(PresetHeader* header)
{
    return reinterpret_cast<int*>(reinterpret_cast<char*>(header) + header->size);
}

int* PresetBegin(PresetHeader* header)
{
    return reinterpret_cast<int*>(header + 1);
}

void FillChannelMatrix(int* sources, int* destinations)
{
    for (int row = 0; row < PRESET_CHANNEL_LIMIT; ++row)
    {
        for (int column = 0; column < PRESET_CHANNEL_LIMIT; ++column)
        {
            sources[row * PRESET_CHANNEL_LIMIT + column] = column;
            destinations[row * PRESET_CHANNEL_LIMIT + column] = row;
        }
    }
}
}

extern "C" const char* AIL_get_soundbank_name(SoundBank* bank)
{
    SoundBankHeader* header = reinterpret_cast<SoundBankHeader*>(bank);
    if (header->magic == SOUND_BANK_MAGIC)
    {
        return header->name;
    }

    if (header->magic == AUDIO_BANK_MAGIC)
    {
        return reinterpret_cast<const char*>(header) + AUDIO_BANK_NAME_OFFSET;
    }

    return NULL;
}

extern "C" void* AIL_find_sound_preset(SoundBank* bank, const char* name)
{
    SoundBankHeader* header = reinterpret_cast<SoundBankHeader*>(bank);
    if (header != NULL && name != NULL && header->magic == SOUND_BANK_MAGIC)
    {
        SoundBank::AssetEntry* entry = reinterpret_cast<SoundBank::AssetEntry*>(
            static_cast<uintptr_t>(FindAsset(bank, name, header->presets, header->presetCount)));
        if (entry == NULL)
        {
            AIL_set_error(PRESET_NOT_FOUND_ERROR);
            return NULL;
        }

        return reinterpret_cast<char*>(header) + entry->value;
    }

    return NULL;
}

extern "C" int AIL_sound_asset_info(SoundBank* bank, const char* name, char* path,
                                    _MILESBANKSOUNDINFO* info)
{
    SoundBankHeader* header = reinterpret_cast<SoundBankHeader*>(bank);
    SoundBank::AssetEntry* entry = reinterpret_cast<SoundBank::AssetEntry*>(
        static_cast<uintptr_t>(FindAsset(bank, name, header->sounds, header->soundCount)));
    if (entry == NULL)
    {
        if (path != NULL)
        {
            *path = '\0';
        }
        return 0;
    }

    char* asset = reinterpret_cast<char*>(header) + entry->value;
    if (info != NULL)
    {
        memcpy(info, asset + SOUND_ASSET_INFO_OFFSET, SOUND_ASSET_INFO_SIZE);
    }

    const char* assetName = asset + *reinterpret_cast<int*>(asset + SOUND_ASSET_NAME_OFFSET);
    size_t length = strlen(assetName);
    int prefixLength = header->pathLength;
    if (path != NULL)
    {
        *path = SOUND_ASSET_PATH_MARKER;
        memcpy(path + 1, header->path, header->pathLength);
        strcpy(path + header->pathLength + 1, assetName);
    }

    return prefixLength + length + 2;
}

extern "C" float evaluate_graph(float position, const GraphPoint* points, unsigned int count)
{
    if (count == 0)
    {
        return PRESET_ZERO;
    }

    if (count != 1)
    {
        const int last = count - 1;
        if (last >= 0)
        {
            int index = last;
            for (unsigned int remaining = count; remaining != 0; --remaining, --index)
            {
                if (points[index].x <= position)
                {
                    break;
                }
            }

            if (index >= 0 && points[index].x <= position)
            {
                const GraphPoint* current = points + index;
                if (index == last)
                {
                    return current->y;
                }

                const GraphPoint* next = current + 1;
                if (current->outMode != GRAPH_MODE_STEP && next->inMode != GRAPH_MODE_STEP)
                {
                    const float width = next->x - current->x;
                    const float height = next->y - current->y;
                    float outX = current->outTangentX;
                    float outY = current->outTangentY;
                    if (current->outMode == GRAPH_MODE_LINEAR)
                    {
                        outX = width;
                        outY = height;
                    }

                    float inX = next->inTangentX;
                    float inY = next->inTangentY;
                    if (next->inMode == GRAPH_MODE_LINEAR)
                    {
                        inX = current->x - next->x;
                        inY = current->y - next->y;
                    }

                    const float inSlope =
                        width * ((next->y - (next->y - inY / GRAPH_TANGENT_DIVISOR)) /
                                 (next->x - (next->x - inX / GRAPH_TANGENT_DIVISOR)));
                    const float outSlope =
                        ((current->y + outY / GRAPH_TANGENT_DIVISOR) - current->y) /
                        ((current->x + outX / GRAPH_TANGENT_DIVISOR) - current->x);
                    const float outScaled = width * outSlope;
                    const float offset = position - current->x;
                    const float inverseSquare = PRESET_UNIT / (width * width);
                    return offset *
                               (offset * (((((height + height + height) - outScaled) - outScaled) - inSlope) *
                                              inverseSquare +
                                          offset * ((((outScaled + inSlope) - height) - height) * inverseSquare /
                                                    width)) +
                                outSlope) +
                           current->y;
                }

                return current->y;
            }
        }
    }

    return points[0].y;
}

extern "C" int AIL_resolve_raw_sound_preset(void* preset, void* instance, PresetEvalFunc evaluate)
{
    PresetHeader* header = static_cast<PresetHeader*>(preset);
    if (header == NULL || header->magic != PRESET_MAGIC)
    {
        return 0;
    }

    int* record = PresetBegin(header);
    int* end = PresetEnd(header);
    while (record != end)
    {
        PresetPair* pairs = PresetPairs(record);
        switch (record[0])
        {
        case PRESET_POSITION:
        case PRESET_CONE:
            ResolvePair(header, instance, evaluate, pairs, 1);
            ResolvePair(header, instance, evaluate, pairs + 1, 1);
            ResolvePair(header, instance, evaluate, pairs + 2, 1);
            record += 7;
            break;
        case PRESET_DISTANCES:
            ResolvePair(header, instance, evaluate, pairs, 1);
            ResolvePair(header, instance, evaluate, pairs + 1, 1);
            ResolvePair(header, instance, evaluate, pairs + 2, 0);
            record += 7;
            break;
        case PRESET_ORIENTATION:
            ResolvePair(header, instance, evaluate, pairs, 1);
            ResolvePair(header, instance, evaluate, pairs + 1, 1);
            ResolvePair(header, instance, evaluate, pairs + 2, 1);
            ResolvePair(header, instance, evaluate, pairs + 3, 1);
            ResolvePair(header, instance, evaluate, pairs + 4, 1);
            ResolvePair(header, instance, evaluate, pairs + 5, 1);
            record += 13;
            break;
        case PRESET_VELOCITY:
            ResolvePair(header, instance, evaluate, pairs, 1);
            ResolvePair(header, instance, evaluate, pairs + 1, 1);
            ResolvePair(header, instance, evaluate, pairs + 2, 1);
            ResolvePair(header, instance, evaluate, pairs + 3, 1);
            record += 9;
            break;
        case PRESET_VALUE:
        case PRESET_LOW_PASS:
        case PRESET_OCCLUSION:
        case PRESET_EXCLUSION:
        case PRESET_OBSTRUCTION:
        case PRESET_PITCH:
            ResolvePair(header, instance, evaluate, pairs, 1);
            record += 3;
            break;
        case PRESET_REVERB:
            ResolvePair(header, instance, evaluate, pairs, 1);
            ResolvePair(header, instance, evaluate, pairs + 1, 1);
            record += 5;
            break;
        case PRESET_VOLUME_PAN:
            ResolvePair(header, instance, evaluate, pairs, 1);
            ResolvePair(header, instance, evaluate, pairs + 1, 1);
            ResolvePair(header, instance, evaluate, pairs + 2, 1);
            ResolvePair(header, instance, evaluate, pairs + 3, 1);
            ResolvePair(header, instance, evaluate, pairs + 4, 1);
            record += 11;
            break;
        case PRESET_FALLOFF:
            record += (record[4] + record[5] + record[3]) * PRESET_FALLOFF_POINT_WORDS +
                      PRESET_FALLOFF_HEADER_WORDS;
            break;
        case PRESET_PROCESSOR:
        {
            PresetProcessorEntry* entries = reinterpret_cast<PresetProcessorEntry*>(record + 4);
            for (int index = 0; index < record[3]; ++index)
            {
                if (entries[index].expression != 0)
                {
                    evaluate(instance, reinterpret_cast<char*>(header) + entries[index].expression,
                             PRESET_NO_INDEX, &entries[index].value, 1);
                }
            }
            record += record[3] * PRESET_PROCESSOR_ENTRY_WORDS + PRESET_PROCESSOR_HEADER_WORDS;
            break;
        }
        default:
            record += 3;
            break;
        }
    }

    return 1;
}

extern "C" int AIL_apply_raw_sound_preset(_SAMPLE* sample, void* preset)
{
    PresetHeader* header = static_cast<PresetHeader*>(preset);
    if (sample == NULL || header == NULL || header->magic != PRESET_MAGIC)
    {
        return 0;
    }

    int speakers[SPEAKER_COUNT];
    float scales[SPEAKER_COUNT];
    int sources[CHANNEL_MATRIX_SIZE];
    int destinations[CHANNEL_MATRIX_SIZE];
    float levels[CHANNEL_MATRIX_SIZE];

    if ((header->flags & PRESET_FLAG_SPEAKER_SCALES) != 0)
    {
        for (int speaker = 0; speaker < SPEAKER_COUNT; ++speaker)
        {
            speakers[speaker] = speaker;
        }
        AIL_sample_speaker_scale_factors(sample, speakers, scales, SPEAKER_COUNT);
    }

    if ((header->flags & PRESET_FLAG_CHANNEL_LEVELS) != 0)
    {
        FillChannelMatrix(sources, destinations);
        AIL_sample_channel_levels(sample, sources, destinations, levels, CHANNEL_MATRIX_SIZE);
    }

    int* record = PresetBegin(header);
    int* end = PresetEnd(header);
    while (record != end)
    {
        PresetPair* pairs = PresetPairs(record);
        int words = PRESET_PAIR_WORDS + 1;
        switch (record[0])
        {
        case PRESET_CHANNEL_ROUTING:
            if (record[1] == 0)
            {
                AIL_set_sample_channel_levels(sample, NULL, NULL, NULL, 0);
            }
            else if (record[1] == 1)
            {
                AIL_set_sample_is_3D(sample, 1);
            }
            break;
        case PRESET_POSITION:
            AIL_set_sample_3D_position(sample, pairs[0].value.number, pairs[1].value.number,
                                       pairs[2].value.number);
            words = 7;
            break;
        case PRESET_CONE:
            AIL_set_sample_3D_cone(
                sample, pairs[0].value.number, pairs[1].value.number,
                pow(PRESET_DECIBEL_BASE, pairs[2].value.number / PRESET_DECIBELS_PER_DECADE));
            words = 7;
            break;
        case PRESET_DISTANCES:
            AIL_set_sample_3D_distances(sample, pairs[1].value.number, pairs[0].value.number,
                                        pairs[2].value.integer);
            words = 7;
            break;
        case PRESET_ORIENTATION:
            AIL_set_sample_3D_orientation(sample, pairs[0].value.number, pairs[1].value.number,
                                          pairs[2].value.number, pairs[3].value.number,
                                          pairs[4].value.number, pairs[5].value.number);
            words = 13;
            break;
        case PRESET_VELOCITY:
            AIL_set_sample_3D_velocity(sample, pairs[0].value.number, pairs[1].value.number,
                                       pairs[2].value.number, pairs[3].value.number);
            words = 9;
            break;
        case PRESET_VALUE:
            sample->presetValue = pairs[0].value.number;
            break;
        case PRESET_REVERB:
            AIL_set_sample_reverb_levels(
                sample, pow(PRESET_DECIBEL_BASE, pairs[1].value.number / PRESET_DECIBELS_PER_DECADE),
                pow(PRESET_DECIBEL_BASE, pairs[0].value.number / PRESET_DECIBELS_PER_DECADE));
            words = 5;
            break;
        case PRESET_LOW_PASS:
        {
            float cutOff = pairs[0].value.number / static_cast<float>(sample->playbackRate / 2);
            if (cutOff > PRESET_UNIT)
            {
                cutOff = PRESET_UNIT;
            }
            AIL_set_sample_low_pass_cut_off(sample, cutOff, ALL_CHANNELS);
            break;
        }
        case PRESET_OCCLUSION:
            AIL_set_sample_occlusion(sample, pairs[0].value.number);
            break;
        case PRESET_EXCLUSION:
            AIL_set_sample_exclusion(sample, pairs[0].value.number);
            break;
        case PRESET_OBSTRUCTION:
            AIL_set_sample_obstruction(sample, pairs[0].value.number);
            break;
        case PRESET_PITCH:
            AIL_set_sample_playback_rate_factor(
                sample, pow(PRESET_SEMITONE_BASE, pairs[0].value.number / PRESET_SEMITONES_PER_OCTAVE));
            break;
        case PRESET_VOLUME_PAN:
        {
            float volume =
                pow(pow(PRESET_DECIBEL_BASE, pairs[0].value.number / PRESET_DECIBELS_PER_DECADE),
                    PRESET_VOLUME_EXPONENT);
            float fade =
                pow(pow(PRESET_DECIBEL_BASE, pairs[3].value.number / PRESET_DECIBELS_PER_DECADE),
                    PRESET_VOLUME_EXPONENT);
            float center =
                pow(pow(PRESET_DECIBEL_BASE, pairs[4].value.number / PRESET_DECIBELS_PER_DECADE),
                    PRESET_VOLUME_EXPONENT);
            AIL_set_sample_51_volume_pan(sample, volume, pairs[1].value.number, pairs[2].value.number,
                                         fade, center);
            words = 11;
            break;
        }
        case PRESET_FALLOFF:
            FalloffGraph_Set(sample, record[1], reinterpret_cast<FalloffGraph*>(record + 1));
            words = (record[4] + record[5] + record[3]) * PRESET_FALLOFF_POINT_WORDS +
                    PRESET_FALLOFF_HEADER_WORDS;
            break;
        case PRESET_PROCESSOR:
        {
            const int stage = record[1] + SAMPLE_STAGE_BASE;
            PresetProcessorEntry* entries = reinterpret_cast<PresetProcessorEntry*>(record + 4);
            int count = record[3];
            if (record[2] == 0)
            {
                AIL_set_sample_processor(sample, stage, 0);
                words = count * PRESET_PROCESSOR_ENTRY_WORDS + PRESET_PROCESSOR_HEADER_WORDS;
                break;
            }

            const char* filterName = reinterpret_cast<char*>(header) + record[2];
            int provider = 0;
            if (AIL_find_filter(filterName, &provider) == 0)
            {
                AIL_set_error(PRESET_FILTER_ERROR);
                return 0;
            }

            if (AIL_sample_processor(sample, stage) != provider)
            {
                if (sample->reverbBus != SAMPLE_REVERB_BUS_COMPRESSOR ||
                    strcmp(filterName, PRESET_COMPRESSOR_FILTER) != 0)
                {
                    AIL_set_sample_processor(sample, stage, provider);
                }
                else
                {
                    _DIG_DRIVER* driver = sample->driver;
                    int bus = 0;
                    if (driver->busSamples[0] != sample)
                    {
                        if (driver->busSamples[1] == sample)
                        {
                            bus = 1;
                        }
                        else if (driver->busSamples[2] == sample)
                        {
                            bus = 2;
                        }
                        else if (driver->busSamples[3] == sample)
                        {
                            bus = 3;
                        }
                    }

                    unsigned int inputBus = 0;
                    for (int index = 0; index < count; ++index)
                    {
                        if (strcmp(reinterpret_cast<char*>(header) + entries[index].nameOffset,
                                   PRESET_COMPRESSOR_INPUT) == 0)
                        {
                            inputBus = static_cast<unsigned int>(entries[index].value.number);
                        }
                    }

                    if (inputBus > PRESET_COMPRESSOR_BUS_LIMIT)
                    {
                        inputBus = 0;
                    }
                    AIL_install_bus_compressor(driver, bus, stage, inputBus);
                    count = record[3];
                }
            }

            for (int index = 0; index < count; ++index)
            {
                int value = entries[index].value.integer;
                AIL_sample_stage_property(sample, stage, reinterpret_cast<char*>(header) + entries[index].nameOffset,
                                          ALL_CHANNELS, 0, &value, 0);
                count = record[3];
            }

            words = count * PRESET_PROCESSOR_ENTRY_WORDS + PRESET_PROCESSOR_HEADER_WORDS;
            break;
        }
        case PRESET_CHANNEL_LEVEL:
        {
            const unsigned int packed = record[1];
            if ((packed & PRESET_CHANNEL_MASK) < PRESET_CHANNEL_LIMIT &&
                (packed >> PRESET_CHANNEL_SHIFT) < PRESET_CHANNEL_LIMIT)
            {
                levels[(packed & PRESET_CHANNEL_MASK) * PRESET_CHANNEL_LIMIT +
                       (packed >> PRESET_CHANNEL_SHIFT)] = pairs[1].value.number;
            }
            words = PRESET_CHANNEL_LEVEL_WORDS;
            break;
        }
        default:
            break;
        }

        record += words;
    }

    if ((header->flags & PRESET_FLAG_SPEAKER_SCALES) != 0)
    {
        AIL_set_sample_speaker_scale_factors(sample, speakers, scales, SPEAKER_COUNT);
    }

    if ((header->flags & PRESET_FLAG_CHANNEL_LEVELS) != 0)
    {
        AIL_set_sample_channel_levels(sample, sources, destinations, levels, CHANNEL_MATRIX_SIZE);
    }

    return 1;
}

extern "C" int AIL_unapply_raw_sound_preset(_SAMPLE* sample, void* preset)
{
    PresetHeader* header = static_cast<PresetHeader*>(preset);
    if (sample == NULL || header == NULL || header->magic != PRESET_MAGIC)
    {
        return 0;
    }

    int* record = PresetBegin(header);
    int* end = PresetEnd(header);
    while (record != end)
    {
        int words = PRESET_PAIR_WORDS + 1;
        switch (record[0])
        {
        case PRESET_CHANNEL_ROUTING:
            AIL_set_sample_is_3D(sample, 0);
            AIL_set_sample_channel_levels(sample, NULL, NULL, NULL, 0);
            break;
        case PRESET_POSITION:
            AIL_set_sample_3D_position(sample, PRESET_ZERO, PRESET_ZERO, PRESET_ZERO);
            words = 7;
            break;
        case PRESET_CONE:
            AIL_set_sample_3D_cone(sample, PRESET_DEFAULT_CONE_ANGLE, PRESET_DEFAULT_CONE_ANGLE,
                                   PRESET_UNIT);
            words = 7;
            break;
        case PRESET_DISTANCES:
            AIL_set_sample_3D_distances(sample, PRESET_DEFAULT_MAXIMUM_DISTANCE, PRESET_ZERO, 1);
            words = 7;
            break;
        case PRESET_ORIENTATION:
            AIL_set_sample_3D_orientation(sample, PRESET_UNIT, PRESET_ZERO, PRESET_ZERO, PRESET_ZERO,
                                          PRESET_UNIT, PRESET_ZERO);
            words = 13;
            break;
        case PRESET_VELOCITY:
            AIL_set_sample_3D_velocity(sample, PRESET_UNIT, PRESET_ZERO, PRESET_ZERO, PRESET_ZERO);
            words = 9;
            break;
        case PRESET_VALUE:
            sample->presetValue = PRESET_ZERO;
            break;
        case PRESET_REVERB:
            AIL_set_sample_reverb_levels(sample, PRESET_UNIT, PRESET_ZERO);
            words = 5;
            break;
        case PRESET_LOW_PASS:
            AIL_set_sample_low_pass_cut_off(sample, PRESET_UNIT, ALL_CHANNELS);
            break;
        case PRESET_OCCLUSION:
            AIL_set_sample_occlusion(sample, PRESET_ZERO);
            break;
        case PRESET_EXCLUSION:
            AIL_set_sample_exclusion(sample, PRESET_ZERO);
            break;
        case PRESET_OBSTRUCTION:
            AIL_set_sample_obstruction(sample, PRESET_ZERO);
            break;
        case PRESET_PITCH:
            AIL_set_sample_playback_rate_factor(sample, PRESET_UNIT);
            break;
        case PRESET_VOLUME_PAN:
            AIL_set_sample_51_volume_pan(sample, PRESET_UNIT, PRESET_HALF, PRESET_HALF, PRESET_UNIT,
                                         PRESET_UNIT);
            words = 11;
            break;
        case PRESET_FALLOFF:
            FalloffGraph_Set(sample, record[1], NULL);
            words = (record[4] + record[5] + record[3]) * PRESET_FALLOFF_POINT_WORDS +
                    PRESET_FALLOFF_HEADER_WORDS;
            break;
        case PRESET_PROCESSOR:
            if (record[2] != 0)
            {
                AIL_set_sample_processor(sample, record[1] + SAMPLE_STAGE_BASE, 0);
            }
            words = record[3] * PRESET_PROCESSOR_ENTRY_WORDS + PRESET_PROCESSOR_HEADER_WORDS;
            break;
        case PRESET_CHANNEL_LEVEL:
            words = PRESET_CHANNEL_LEVEL_WORDS;
            break;
        default:
            break;
        }

        record += words;
    }

    if ((header->flags & PRESET_FLAG_SPEAKER_SCALES) != 0)
    {
        int speakers[SPEAKER_COUNT];
        float scales[SPEAKER_COUNT];
        for (int speaker = 0; speaker < SPEAKER_COUNT; ++speaker)
        {
            speakers[speaker] = speaker;
            scales[speaker] = PRESET_UNIT;
        }
        AIL_set_sample_speaker_scale_factors(sample, speakers, scales, SPEAKER_COUNT);
    }

    if ((header->flags & PRESET_FLAG_CHANNEL_LEVELS) != 0)
    {
        int sources[CHANNEL_MATRIX_SIZE];
        int destinations[CHANNEL_MATRIX_SIZE];
        float levels[CHANNEL_MATRIX_SIZE];
        for (int index = 0; index < CHANNEL_MATRIX_SIZE; ++index)
        {
            levels[index] = PRESET_ZERO;
        }
        FillChannelMatrix(sources, destinations);
        for (int channel = 0; channel < PRESET_CHANNEL_LIMIT; ++channel)
        {
            levels[channel * PRESET_CHANNEL_LIMIT + channel] = PRESET_UNIT;
        }
        AIL_set_sample_channel_levels(sample, sources, destinations, levels, CHANNEL_MATRIX_SIZE);
    }

    return 1;
}

namespace
{
const char MILES_FILTER_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\flt.cpp";
const char MILES_WAVEFILE_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\wavefile.cpp";

const unsigned int RIB_FUNCTION = 0;
const unsigned int RIB_PROPERTY = 1;
const unsigned int RIB_ENTRY_NOT_FOUND = 0xFFFFFFFF;
const unsigned int RIB_ENUMERATE_FIRST = 0xFFFFFFFF;

const char ASI_STREAM_INTERFACE[] = "ASI stream";
const char ASI_CODEC_INTERFACE[] = "ASI codec";
const char MIXER_SERVICES_INTERFACE[] = "MSS mixer services";
const char PIPELINE_FILTER_INTERFACE[] = "MSS pipeline filter";
const char VOICE_FILTER_INTERFACE[] = "MSS voice filter";
const char FILTER_SAMPLE_INTERFACE[] = "Pipeline filter sample services";
const char COMPRESSOR_NOT_FOUND_ERROR[] = "Compressor Filter Not Found";
const char BUS_INDEX_ERROR[] = "Both indices must refer to installed busses.";
const char OUT_OF_MEMORY_ERROR[] = "Out of memory";
const char NOT_A_FILTER_ERROR[] = "Not a filter provider";
const char MATRIX_FILTER_ERROR[] = "Matrix filter already associated with driver";
const char FILTER_LOAD_ERROR[] = "Error loading filter";
const char CHANNEL_BUFFER_ERROR[] = "Channel buffer allocation failed.";

const int SPEAKER_CONFIG_COUNT = 10;
const int SPEAKER_ID_COUNT = 18;
const int SPEAKER_ID_LAST = 17;
const int SPEAKER_UNMAPPED = -1;

const signed char SPEAKER_CHANNEL_MAP[SPEAKER_CONFIG_COUNT][SPEAKER_ID_COUNT] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, -1, -1, -1, -1, -1, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, -1, -1, 2, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 2, -1, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 2, 3, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 2, 3, 4, 5, -1, -1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 2, 3, 4, 5, -1, -1, -1, 6, 7, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 2, 3, 4, 5, -1, -1, 6, 7, 8, -1, -1, -1, -1, -1, -1, -1}};

const unsigned int SPEAKER_TRAIT_LEFT = 0x01;
const unsigned int SPEAKER_TRAIT_RIGHT = 0x02;
const unsigned int SPEAKER_TRAIT_FRONT = 0x04;
const unsigned int SPEAKER_TRAIT_BACK = 0x08;
const unsigned int SPEAKER_TRAIT_SIDE = 0x10;
const unsigned int SPEAKER_TRAIT_CENTER = 0x20;
const unsigned int SPEAKER_TRAIT_TOP = 0x40;
const unsigned int SPEAKER_TRAIT_LOW_FREQUENCY = 0x80;
const unsigned int SPEAKER_TRAIT_SHARED = SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_RIGHT |
                                          SPEAKER_TRAIT_CENTER | SPEAKER_TRAIT_TOP |
                                          SPEAKER_TRAIT_LOW_FREQUENCY;
const unsigned int SPEAKER_TRAIT_SIDES = SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_RIGHT;

const unsigned char SPEAKER_TRAITS[SPEAKER_ID_COUNT] = {
    SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_FRONT,
    SPEAKER_TRAIT_RIGHT | SPEAKER_TRAIT_FRONT,
    SPEAKER_TRAIT_FRONT | SPEAKER_TRAIT_CENTER,
    SPEAKER_TRAIT_LOW_FREQUENCY,
    SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_BACK,
    SPEAKER_TRAIT_RIGHT | SPEAKER_TRAIT_BACK,
    SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_FRONT | SPEAKER_TRAIT_CENTER,
    SPEAKER_TRAIT_RIGHT | SPEAKER_TRAIT_FRONT | SPEAKER_TRAIT_CENTER,
    SPEAKER_TRAIT_BACK | SPEAKER_TRAIT_CENTER,
    SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_SIDE,
    SPEAKER_TRAIT_RIGHT | SPEAKER_TRAIT_SIDE,
    SPEAKER_TRAIT_TOP | SPEAKER_TRAIT_CENTER,
    SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_FRONT | SPEAKER_TRAIT_TOP,
    SPEAKER_TRAIT_FRONT | SPEAKER_TRAIT_CENTER | SPEAKER_TRAIT_TOP,
    SPEAKER_TRAIT_RIGHT | SPEAKER_TRAIT_FRONT | SPEAKER_TRAIT_TOP,
    SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_BACK | SPEAKER_TRAIT_TOP,
    SPEAKER_TRAIT_BACK | SPEAKER_TRAIT_CENTER | SPEAKER_TRAIT_TOP,
    SPEAKER_TRAIT_RIGHT | SPEAKER_TRAIT_BACK | SPEAKER_TRAIT_TOP};

const unsigned int SAMPLE_FORMAT_16_BITS = 1;
const unsigned int SAMPLE_FORMAT_STEREO = 2;
const unsigned int SAMPLE_FORMAT_ADPCM = 4;
const unsigned int SAMPLE_FORMAT_MULTICHANNEL = 0x10;
const unsigned int SAMPLE_FORMAT_MASK = 0xffff;
const int SAMPLE_FORMAT_CHANNEL_SHIFT = 16;
const int SAMPLE_WIDTH_16 = 16;
const int SAMPLE_WIDTH_8 = 8;

const int SAMPLE_STATUS_PLAYING = 4;
const int SAMPLE_NO_REVERB_BUS = 4;
const float REVERB_SEND_THRESHOLD = 0.0001f;

const unsigned int CHANNEL_BUFFER_ALIGNMENT = 16;
const unsigned int CHANNEL_BUFFER_SLACK = 0x1e;
const unsigned int CHANNEL_ARRAY_ALIGNMENT = 4;
const int CHANNEL_HEADER_UNITY = 0x10000;
const int CHANNEL_LIMIT_DEFAULT = 0x7fff;
const float CHANNEL_BUS_LEVEL_DEFAULT = -1.0f;
const float CHANNEL_LOW_PASS_DEFAULT = 1.0f;
const int CHANNEL_MAP_LENGTH = 0x11;
const int CHANNEL_MASK_BITS = 32;
const int CHANNEL_SCRATCH_ROWS = 9;

const float CONE_ANGLE_DIVISOR = 114.59156f;
const float CONE_FULL_ANGLE = 360.0f;
const float CONE_FULL_VOLUME = 1.0f;
const float OCCLUSION_LOW_PASS_BASE = 1.01f;
const float DIRECTION_EPSILON = 0.0001f;

const float VOLUME_CURVE_EXPONENT = 1.6666666f;
const float PAN_CURVE_EXPONENT = 0.3f;
const float PAN_CENTER_GAIN = 0.8122522f;
const float PAN_CENTER = 0.5f;
const float PAN_UNIT = 1.0f;

const unsigned int FILTER_FLAG_MATRIX = 4;
const unsigned int BUS_COMPRESSOR_LINK_FLAGS = 3;
const int BUS_COMPRESSOR_STAGE_SHIFT = 16;
const unsigned int DRIVER_FLAG_16_BITS = 1;
const unsigned int DRIVER_FLAG_STEREO_OR_MULTICHANNEL = 0x12;
const int ASI_DEFAULT_CHANNELS = 2;
const int FILTER_STATE_LINE = 0xc07;
const int FILTER_DRIVER_STATE_LINE = 0x196;
const int FILTER_INSTANCE_LINE = 0xeb;
const int CHANNEL_BUFFER_LINE = 0x336;
const int CHANNEL_MAP_LINE = 0x377;
const unsigned int ASI_PROFILE_BASE = 0x1000;
const unsigned int ASI_PROFILE_LIMIT = 0x11;
const unsigned int ASI_CHANNEL_LIMIT = 3;
const int LOOP_MODE_ASI = 1;
const int LOOP_MODE_STREAMING = 2;
}

extern "C" void AIL_API_set_sample_speaker_scale_factors(_SAMPLE* pSample, const int* pSpeakers,
                                                         const float* pScales, int iCount)
{
    if (pSample == NULL || pSpeakers == NULL || pScales == NULL || iCount == 0)
    {
        return;
    }

    int iConfig = pSample->driver->speakerConfig;
    for (int iIndex = 0; iIndex < iCount; ++iIndex)
    {
        int iChannel = SPEAKER_CHANNEL_MAP[iConfig][pSpeakers[iIndex]];
        if (iChannel != SPEAKER_UNMAPPED)
        {
            pSample->speakerScales[iChannel] = pScales[iIndex];
        }
    }
}

extern "C" void AIL_API_sample_speaker_scale_factors(_SAMPLE* pSample, int* pSpeakers,
                                                     float* pScales, int iCount)
{
    if (pSample == NULL || pSpeakers == NULL || pScales == NULL || iCount == 0)
    {
        return;
    }

    int iConfig = pSample->driver->speakerConfig;
    for (int iIndex = 0; iIndex < iCount; ++iIndex)
    {
        int iChannel = SPEAKER_CHANNEL_MAP[iConfig][pSpeakers[iIndex]];
        if (iChannel != SPEAKER_UNMAPPED)
        {
            pScales[iIndex] = pSample->speakerScales[iChannel];
        }
    }
}

extern "C" void AIL_API_sample_channel_levels(_SAMPLE* pSample, int* pSources, int* pDestinations,
                                              float* pLevels, int iCount)
{
    if (pSample == NULL || pSources == NULL || pDestinations == NULL || pLevels == NULL)
    {
        return;
    }

    int iConfig = pSample->driver->speakerConfig;
    if (iConfig == 0)
    {
        return;
    }

    for (int iIndex = 0; iIndex < iCount; ++iIndex)
    {
        int iSource = pSources[iIndex];
        if (pSample->channelMap != NULL)
        {
            iSource = pSample->channelMap[iSource];
        }
        else if (iSource >= pSample->channelCount)
        {
            iSource = -1;
        }

        int iSpeaker = SPEAKER_UNMAPPED;
        if (iSource != -1)
        {
            iSpeaker = SPEAKER_CHANNEL_MAP[iConfig][pDestinations[iIndex]];
        }

        if (iSpeaker == SPEAKER_UNMAPPED)
        {
            pLevels[iIndex] = 0.0f;
        }
        else
        {
            pLevels[iIndex] = pSample->levels[iSpeaker][iSource];
        }
    }
}

extern "C" void AIL_API_set_sample_channel_levels(_SAMPLE* pSample, const int* pSources,
                                                  const int* pDestinations, const float* pLevels,
                                                  int iCount)
{
    if (pSample == NULL)
    {
        return;
    }

    _DIG_DRIVER* pDriver = pSample->driver;
    int iConfig = pDriver->speakerConfig;
    if (iConfig == 0)
    {
        return;
    }

    if (pSources == NULL || pDestinations == NULL || pLevels == NULL)
    {
        set_user_channel_defaults(iConfig, pSample->channelCount, pSample->levels,
                                  pSample->channelMap);
        return;
    }

    for (int iIndex = 0; iIndex < iCount; ++iIndex)
    {
        int iSource = pSources[iIndex];
        if (pSample->channelMap != NULL)
        {
            iSource = pSample->channelMap[iSource];
        }
        else if (iSource >= pSample->channelCount)
        {
            continue;
        }

        if (iSource != -1)
        {
            int iSpeaker = SPEAKER_CHANNEL_MAP[iConfig][pDestinations[iIndex]];
            if (iSpeaker != SPEAKER_UNMAPPED)
            {
                pSample->levels[iSpeaker][iSource] = pLevels[iIndex];
            }
        }
    }

    pSample->userLevels = 0;
    if ((pSample->channelFlags & SAMPLE_FORMAT_STEREO) != 0)
    {
        for (int iOutput = 0; iOutput < pDriver->outputCount; ++iOutput)
        {
            if (pDriver->outputs[iOutput].mode > 1 &&
                (pSample->levels[iOutput * 2 + 1][0] > 0.0f ||
                 pSample->levels[iOutput * 2][1] > 0.0f))
            {
                pSample->userLevels = 1;
            }
        }
    }
}

extern "C" void SS_update_sample_reverb_state(_SAMPLE* pSample)
{
    if (pSample->reverbBus == SAMPLE_NO_REVERB_BUS)
    {
        return;
    }

    if (pSample->reverbSecond <= REVERB_SEND_THRESHOLD)
    {
        return;
    }

    DriverBus* pBus = &pSample->driver->buses[pSample->reverbBus];
    if (pBus->sendLevel <= REVERB_SEND_THRESHOLD)
    {
        return;
    }

    if (pBus->returnLevel <= REVERB_SEND_THRESHOLD)
    {
        return;
    }

    pBus->updateValue = pSample->driver->sharedState;
    pSample->driver->buses[pSample->reverbBus].update = 1;
}

extern "C" void AIL_API_set_sample_reverb_levels(_SAMPLE* pSample, float fFirst, float fSecond)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->reverbFirst = fFirst;
    pSample->reverbDirty = 1;
    pSample->reverbSecond = fSecond;

    if (pSample->driver->sampleStatus[pSample->index] != SAMPLE_STATUS_PLAYING)
    {
        return;
    }

    SS_update_sample_reverb_state(pSample);
}

extern "C" void AIL_API_set_sample_low_pass_cut_off(_SAMPLE* pSample, float fCutOff, int iChannel)
{
    if (pSample == NULL)
    {
        return;
    }

    int iFirst;
    int iEnd;
    if (iChannel == ALL_CHANNELS)
    {
        iFirst = 0;
        iEnd = pSample->channelCount;
        if (iEnd < 1)
        {
            return;
        }
    }
    else
    {
        if (pSample->channelMap == NULL)
        {
            if (iChannel >= pSample->channelCount)
            {
                return;
            }
        }
        else
        {
            iChannel = pSample->channelMap[iChannel];
            if (iChannel == -1)
            {
                return;
            }
        }

        iFirst = iChannel;
        iEnd = iChannel + 1;
    }

    for (int iIndex = iFirst; iIndex < iEnd; ++iIndex)
    {
        pSample->channelStates[iIndex].lowPass = fCutOff;
        pSample->cacheState = pSample->cacheState | (1 << iIndex);
    }
}

extern "C" void AIL_API_set_sample_51_volume_pan(_SAMPLE* pSample, float fVolume, float fPan,
                                                 float fFade, float fCenter, float fLowFrequency)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->volumeCurve = static_cast<float>(pow(fVolume, VOLUME_CURVE_EXPONENT));
    pSample->mixInput0 = fFade;
    pSample->pan = fPan;
    pSample->mixInput2 = fLowFrequency;
    pSample->mixInput1 = fCenter;

    float fFront = PAN_CENTER_GAIN;
    float fBack = PAN_CENTER_GAIN;
    if (fFade != PAN_CENTER)
    {
        fFront = static_cast<float>(pow(PAN_UNIT - fFade, PAN_CURVE_EXPONENT));
        fBack = static_cast<float>(pow(fFade, PAN_CURVE_EXPONENT));
    }

    if (fPan == fFade)
    {
        float fCurve = pSample->volumeCurve;
        pSample->mixLevel2 = fCurve * fCenter;
        pSample->panLevelB = (fCurve * fBack) * fFront;
        pSample->panLevelA = (fCurve * fFront) * fFront;
        pSample->mixLevel0 = (fCurve * fFront) * fBack;
        pSample->mixLevel1 = (fCurve * fBack) * fBack;
        pSample->mixLevel3 = fCurve * fLowFrequency;
    }
    else if (fPan == PAN_CENTER)
    {
        float fCurve = pSample->volumeCurve;
        float fFrontLevel = (fCurve * PAN_CENTER_GAIN) * fFront;
        float fBackLevel = (fCurve * PAN_CENTER_GAIN) * fBack;
        pSample->panLevelA = fFrontLevel;
        pSample->panLevelB = fFrontLevel;
        pSample->mixLevel0 = fBackLevel;
        pSample->mixLevel2 = fCurve * fCenter;
        pSample->mixLevel1 = fBackLevel;
        pSample->mixLevel3 = fCurve * fLowFrequency;
    }
    else
    {
        float fLeft = pSample->volumeCurve * static_cast<float>(pow(PAN_UNIT - fPan, PAN_CURVE_EXPONENT));
        float fRight = static_cast<float>(pow(fPan, PAN_CURVE_EXPONENT));
        float fCurve = pSample->volumeCurve;
        pSample->panLevelA = fLeft * fFront;
        pSample->mixLevel0 = fLeft * fBack;
        pSample->panLevelB = (fCurve * fRight) * fFront;
        pSample->mixLevel2 = fCurve * fCenter;
        pSample->mixLevel1 = (fCurve * fRight) * fBack;
        pSample->mixLevel3 = fCurve * fLowFrequency;
    }
}

extern "C" int AIL_API_sample_processor(_SAMPLE* pSample, int iStage)
{
    if (iStage == SAMPLE_STAGE_ALL)
    {
        iStage = SAMPLE_STAGE_DECODER;
    }

    const SampleStage* pStage = &pSample->stages[iStage];
    if (pStage->filter.active == 0)
    {
        return 0;
    }

    return pStage->filter.provider;
}

extern "C" int AIL_API_install_bus_compressor(_DIG_DRIVER* pDriver, int iBus, int iStage,
                                              int iInputBus)
{
    _SAMPLE** ppBuses = pDriver->busSamples;
    if (ppBuses[iBus] != NULL && ppBuses[iInputBus] != NULL)
    {
        int iProvider = 0;
        if (AIL_find_filter("Compressor Filter", &iProvider) == 0)
        {
            AIL_set_error(COMPRESSOR_NOT_FOUND_ERROR);
            return 0;
        }

        AIL_set_sample_processor(ppBuses[iBus], iStage, iProvider);
        ppBuses[iBus]->compressorLink = (iStage << BUS_COMPRESSOR_STAGE_SHIFT) |
                                        BUS_COMPRESSOR_LINK_FLAGS;
        ppBuses[iInputBus]->compressorSources = ppBuses[iInputBus]->compressorSources | (1 << iBus);
        return 1;
    }

    AIL_set_error(BUS_INDEX_ERROR);
    return 0;
}

extern "C" int AIL_API_sample_channel_count(_SAMPLE* pSample, unsigned int* puMask)
{
    if (pSample == NULL)
    {
        return 0;
    }

    if (puMask != NULL)
    {
        *puMask = pSample->channelMask;
    }

    return pSample->channelCount;
}

extern "C" int AIL_API_sample_playback_rate(_SAMPLE* pSample)
{
    if (pSample == NULL)
    {
        return 0;
    }

    return pSample->playbackRate;
}

extern "C" void AIL_API_set_sample_playback_rate(_SAMPLE* pSample, int iRate)
{
    if (pSample == NULL)
    {
        return;
    }

    if (iRate < 1)
    {
        return;
    }

    if (iRate != pSample->playbackRate)
    {
        pSample->cacheState = -1;
    }

    pSample->playbackRate = iRate;
}

extern "C" void AIL_API_set_sample_3D_cone(_SAMPLE* pSample, float fInnerAngle, float fOuterAngle,
                                           float fOuterVolume)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->coneInner = fInnerAngle / CONE_ANGLE_DIVISOR;
    pSample->coneOuterVolume = fOuterVolume;
    pSample->coneOuter = fOuterAngle / CONE_ANGLE_DIVISOR;
    pSample->coneEnabled = 0;
    if (fInnerAngle < CONE_FULL_ANGLE && fOuterVolume < CONE_FULL_VOLUME)
    {
        pSample->coneEnabled = 1;
    }
}

extern "C" void AIL_API_set_sample_3D_orientation(_SAMPLE* pSample, float fForwardX,
                                                  float fForwardY, float fForwardZ, float fUpX,
                                                  float fUpY, float fUpZ)
{
    if (pSample == NULL)
    {
        return;
    }

    float fLength = static_cast<float>(
        sqrt(fForwardZ * fForwardZ + (fForwardY * fForwardY + fForwardX * fForwardX)));
    if (fLength > DIRECTION_EPSILON)
    {
        fForwardY = fForwardY / fLength;
        fForwardX = fForwardX / fLength;
        fForwardZ = fForwardZ / fLength;
    }

    fLength = static_cast<float>(sqrt(fUpZ * fUpZ + (fUpY * fUpY + fUpX * fUpX)));
    if (fLength > DIRECTION_EPSILON)
    {
        fUpY = fUpY / fLength;
        fUpX = fUpX / fLength;
        fUpZ = fUpZ / fLength;
    }

    pSample->forwardX = fForwardX;
    pSample->upY = fUpY;
    pSample->upZ = fUpZ;
    pSample->upX = fUpX;
    pSample->forwardZ = fForwardZ;
    pSample->forwardY = fForwardY;
}

extern "C" void AIL_API_set_sample_3D_velocity_vector(_SAMPLE* pSample, float fX, float fY,
                                                      float fZ)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->velocityY = fY;
    pSample->velocityX = fX;
    pSample->velocityZ = fZ;
}

extern "C" void AIL_API_set_sample_3D_velocity(_SAMPLE* pSample, float fDirectionX,
                                               float fDirectionY, float fDirectionZ,
                                               float fMagnitude)
{
    AIL_API_set_sample_3D_velocity_vector(pSample, fDirectionX * fMagnitude,
                                          fDirectionY * fMagnitude, fDirectionZ * fMagnitude);
}

extern "C" void AIL_API_set_sample_obstruction(_SAMPLE* pSample, float fObstruction)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->obstruction = fObstruction;
}

extern "C" void AIL_API_set_sample_exclusion(_SAMPLE* pSample, float fExclusion)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->exclusion = fExclusion;
}

extern "C" void AIL_API_set_sample_occlusion(_SAMPLE* pSample, float fOcclusion)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->occlusion = fOcclusion;
    AIL_set_sample_low_pass_cut_off(pSample, OCCLUSION_LOW_PASS_BASE - fOcclusion, ALL_CHANNELS);
}

extern "C" FLT_INSTANCE* FLT_find_provider_instance(unsigned int uProvider, _DIG_DRIVER* pDriver)
{
    FLT_INSTANCE* pInstance = FLT_list;
    while (pInstance != NULL)
    {
        if (pInstance->driver == pDriver && pInstance->provider == uProvider)
        {
            break;
        }

        pInstance = pInstance->next;
    }

    return pInstance;
}

extern "C" int AIL_find_filter(const char* pName, int* piProvider)
{
    unsigned int uIndex = RIB_ENUMERATE_FIRST;
    unsigned int uProvider;
    const char* pProviderName;
    unsigned int uFlags;
    RIB_PROVIDER_PROPERTY_CALLBACK pProperty = NULL;
    RIB_INTERFACE_ENTRY entries[2] = {{RIB_PROPERTY, "Flags", &uFlags, 0},
                                      {RIB_FUNCTION, "PROVIDER_property", &pProperty, 0}};

    while (RIB_enumerate_providers(PIPELINE_FILTER_INTERFACE, &uIndex, &uProvider) != 0)
    {
        RIB_request_interface(uProvider, PIPELINE_FILTER_INTERFACE, 2, entries);
        if (pProperty(RIB_PROVIDER_NAME, &pProviderName, 0, 0) != 0 &&
            strcmp(pProviderName, pName) == 0)
        {
            if (piProvider != NULL)
            {
                *piProvider = uProvider;
            }
            return 1;
        }
    }

    uIndex = RIB_ENUMERATE_FIRST;
    while (RIB_enumerate_providers(VOICE_FILTER_INTERFACE, &uIndex, &uProvider) != 0)
    {
        RIB_request_interface(uProvider, VOICE_FILTER_INTERFACE, 2, entries);
        if (pProperty(RIB_PROVIDER_NAME, &pProviderName, 0, 0) != 0 &&
            strcmp(pProviderName, pName) == 0)
        {
            if (piProvider != NULL)
            {
                *piProvider = uProvider;
            }
            return 1;
        }
    }

    return 0;
}

extern "C" int AIL_API_open_filter(unsigned int uProvider, _DIG_DRIVER* pDriver)
{
    FLT_INSTANCE* pExisting = FLT_list;
    while (pExisting != NULL)
    {
        if (pExisting->driver == pDriver && pExisting->provider == uProvider)
        {
            return -1;
        }

        pExisting = pExisting->next;
    }

    FLT_INSTANCE* pFilter = static_cast<FLT_INSTANCE*>(
        AIL_mem_alloc_lock_info(sizeof(FLT_INSTANCE), MILES_FILTER_FILE, FILTER_INSTANCE_LINE));
    if (pFilter == NULL)
    {
        AIL_set_error(OUT_OF_MEMORY_ERROR);
        return 0;
    }

    memset(pFilter, 0, sizeof(FLT_INSTANCE));

    unsigned int uFlagsToken = 0;
    unsigned int uSampleStateSizeToken = RIB_ENTRY_NOT_FOUND;
    unsigned int uDriverStateSizeToken = RIB_ENTRY_NOT_FOUND;

    RIB_INTERFACE_ENTRY startupEntries[4] = {
        {RIB_FUNCTION, "PROVIDER_property", &pFilter->property, 0},
        {RIB_PROPERTY, "Flags", &uFlagsToken, 0},
        {RIB_FUNCTION, "FLT_startup", &pFilter->startup, 0},
        {RIB_FUNCTION, "FLT_error", &pFilter->error, 0}};
    RIB_INTERFACE_ENTRY driverEntries[3] = {
        {RIB_FUNCTION, "FLT_shutdown", &pFilter->shutdown, 0},
        {RIB_FUNCTION, "FLT_open_driver", &pFilter->openDriver, 0},
        {RIB_FUNCTION, "FLT_close_driver", &pFilter->closeDriver, 0}};
    RIB_INTERFACE_ENTRY mixEntries[2] = {
        {RIB_FUNCTION, "FLT_premix_process", &pFilter->premixProcess, 0},
        {RIB_FUNCTION, "FLT_postmix_process", &pFilter->postmixProcess, 0}};
    RIB_INTERFACE_ENTRY sizeEntries[2] = {
        {RIB_PROPERTY, "Sample_state_size", &uSampleStateSizeToken, 0},
        {RIB_PROPERTY, "Driver_state_size", &uDriverStateSizeToken, 0}};
    RIB_INTERFACE_ENTRY sampleEntries[4] = {
        {RIB_FUNCTION, "FLTSMP_open_sample", &pFilter->openSample, 0},
        {RIB_FUNCTION, "FLTSMP_close_sample", &pFilter->closeSample, 0},
        {RIB_FUNCTION, "FLTSMP_sample_process", &pFilter->sampleProcess, 0},
        {RIB_FUNCTION, "FLTSMP_sample_property", &pFilter->sampleProperty, 0}};

    int iStartup = RIB_request_interface(uProvider, PIPELINE_FILTER_INTERFACE, 4, startupEntries);
    int iDriver = RIB_request_interface(uProvider, PIPELINE_FILTER_INTERFACE, 3, driverEntries);
    int iMix = RIB_request_interface(uProvider, PIPELINE_FILTER_INTERFACE, 2, mixEntries);
    if (iStartup != 0 || iDriver != 0 || iMix != 0)
    {
        AIL_set_error(NOT_A_FILTER_ERROR);
        AIL_mem_free_lock(pFilter);
        return 0;
    }

    RIB_request_interface(uProvider, PIPELINE_FILTER_INTERFACE, 2, sizeEntries);
    RIB_request_interface(uProvider, FILTER_SAMPLE_INTERFACE, 4, sampleEntries);

    pFilter->flags = 0;
    pFilter->property(uFlagsToken, &pFilter->flags, 0, 0);
    pFilter->sampleStateSize = 0;
    if (uSampleStateSizeToken != RIB_ENTRY_NOT_FOUND)
    {
        pFilter->property(uSampleStateSizeToken, &pFilter->sampleStateSize, 0, 0);
    }

    pFilter->driverStateSize = 0;
    if (uDriverStateSizeToken != RIB_ENTRY_NOT_FOUND)
    {
        pFilter->property(uDriverStateSizeToken, &pFilter->driverStateSize, 0, 0);
    }

    if ((pFilter->flags & FILTER_FLAG_MATRIX) != 0)
    {
        if (pDriver->matrixFilter != NULL)
        {
            AIL_set_error(MATRIX_FILTER_ERROR);
            AIL_mem_free_lock(pFilter);
            return 0;
        }

        pDriver->matrixFilter = pFilter;
    }

    OutMilesMutex();
    void* pDriverState = NULL;
    if (pFilter->driverStateSize != 0)
    {
        pDriverState = AIL_mem_alloc_lock_info(pFilter->driverStateSize, MILES_FILTER_FILE,
                                               FILTER_DRIVER_STATE_LINE);
    }

    pFilter->driverState = pFilter->openDriver(MSS_alloc_info, MSS_free_info, 0, pDriver,
                                               pDriverState);
    InMilesMutex();

    if (pFilter->driverState != NULL)
    {
        pFilter->driver = pDriver;
        pFilter->provider = uProvider;
        pFilter->next = FLT_list;
        FLT_list = pFilter;
        return reinterpret_cast<int>(pFilter);
    }

    const char* pError = pFilter->error();
    if (pError == NULL)
    {
        pError = FILTER_LOAD_ERROR;
    }

    AIL_set_error(pError);
    AIL_mem_free_lock(pFilter);
    return 0;
}

unsigned char* assign_ptr(unsigned char** ppCursor, unsigned int uSize)
{
    unsigned char* pResult = *ppCursor;
    *ppCursor = pResult + ((uSize + CHANNEL_ARRAY_ALIGNMENT - 1) & ~(CHANNEL_ARRAY_ALIGNMENT - 1));
    return pResult;
}

unsigned int assign_chan_ptrs(_SAMPLE* pSample, unsigned char* pBuffer, int iKeepLevelArrays)
{
    int iChannels = pSample->channelCount;
    unsigned char* pCursor = reinterpret_cast<unsigned char*>(
        (reinterpret_cast<unsigned int>(pBuffer) + CHANNEL_BUFFER_ALIGNMENT - 1) &
        ~(CHANNEL_BUFFER_ALIGNMENT - 1));
    unsigned int uRows = (pSample->driver->speakerConfig + 1) & ~1u;
    float* pScratch[CHANNEL_SCRATCH_ROWS];

    pSample->channelStates =
        reinterpret_cast<ChannelState*>(assign_ptr(&pCursor, iChannels * sizeof(ChannelState)));

    float** ppPreviousLevels = pSample->levels;
    float** ppPreviousC = pSample->levelsC;
    float** ppPreviousB = pSample->levelsB;
    float** ppPreviousD = pSample->levelsD;

    pSample->levels = reinterpret_cast<float**>(assign_ptr(&pCursor, (uRows + 1) * sizeof(float*)));
    pSample->levelsB = reinterpret_cast<float**>(assign_ptr(&pCursor, (uRows + 1) * sizeof(float*)));
    pSample->levelsC = reinterpret_cast<float**>(assign_ptr(&pCursor, (uRows + 1) * sizeof(float*)));
    pSample->levelsD = reinterpret_cast<float**>(assign_ptr(&pCursor, (uRows + 1) * sizeof(float*)));

    float** ppLevels = ppPreviousLevels;
    float** ppLevelsB = ppPreviousB;
    float** ppLevelsC = ppPreviousC;
    float** ppLevelsD = ppPreviousD;
    if (iKeepLevelArrays == 0)
    {
        ppLevelsC = pSample->levelsC;
        ppLevelsB = pSample->levelsB;
        ppLevels = pSample->levels;
        ppLevelsD = pSample->levelsD;
    }

    if (pBuffer == NULL)
    {
        ppLevelsD = pScratch;
        ppLevels = pScratch;
        ppLevelsB = pScratch;
        ppLevelsC = pScratch;
    }

    for (unsigned int uRow = 0; uRow < uRows; ++uRow)
    {
        ppLevels[uRow] = reinterpret_cast<float*>(assign_ptr(&pCursor, iChannels * sizeof(float)));
        ppLevelsB[uRow] = reinterpret_cast<float*>(assign_ptr(&pCursor, iChannels * sizeof(float)));
        ppLevelsC[uRow] = reinterpret_cast<float*>(assign_ptr(&pCursor, iChannels * sizeof(float)));
        ppLevelsD[uRow] = reinterpret_cast<float*>(assign_ptr(&pCursor, iChannels * sizeof(float)));
    }

    pSample->channelPositionA = reinterpret_cast<int*>(assign_ptr(&pCursor, iChannels * sizeof(int)));
    pSample->channelPositionB = reinterpret_cast<int*>(assign_ptr(&pCursor, iChannels * sizeof(int)));
    pSample->channelHeader = reinterpret_cast<int*>(assign_ptr(&pCursor, iChannels * sizeof(int)));
    pSample->speakerLevelsA = reinterpret_cast<float*>(assign_ptr(&pCursor, uRows * sizeof(float)));
    pSample->speakerScales = reinterpret_cast<float*>(assign_ptr(&pCursor, uRows * sizeof(float)));
    return reinterpret_cast<unsigned int>(pCursor);
}

void free_channel_buffers(_SAMPLE* pSample)
{
    if (pSample->channelBuffer != NULL)
    {
        AIL_mem_free_lock(pSample->channelBuffer);
        pSample->channelBuffer = NULL;
    }

    pSample->channelBufferCapacity = 0;
    pSample->channelBufferSize = 0;

    if (pSample->channelMap != NULL)
    {
        AIL_mem_free_lock(pSample->channelMap);
    }
}

void clear_last_sample(_SAMPLE* pSample)
{
    for (int iChannel = 0; iChannel < pSample->channelCount; ++iChannel)
    {
        pSample->channelPositionA[iChannel] = 0;
        pSample->channelPositionB[iChannel] = 0;
    }

    *pSample->channelHeader = CHANNEL_HEADER_UNITY;
    pSample->bufferReset = 0;
}

void set_user_channel_defaults(int iSpeakerConfig, int iChannelCount, float** ppLevels,
                               signed char* pChannelMap)
{
    if (iSpeakerConfig == 0)
    {
        return;
    }

    const signed char* pSpeakers = SPEAKER_CHANNEL_MAP[iSpeakerConfig];

    for (int iSpeaker = 0; iSpeaker < iSpeakerConfig; ++iSpeaker)
    {
        for (int iChannel = 0; iChannel < iChannelCount; ++iChannel)
        {
            ppLevels[iSpeaker][iChannel] = 0.0f;
        }
    }

    if (iChannelCount == 2)
    {
        ppLevels[0][0] = 1.0f;
        ppLevels[1][1] = 1.0f;
    }
    else if (iChannelCount == 1)
    {
        ppLevels[0][0] = 1.0f;
        ppLevels[1][0] = 1.0f;
    }
    else
    {
        if (iSpeakerConfig == 1)
        {
            for (int iId = 0; iId < SPEAKER_ID_COUNT; ++iId)
            {
                if ((SPEAKER_TRAITS[iId] & SPEAKER_TRAIT_LOW_FREQUENCY) != 0)
                {
                    continue;
                }

                int iSource;
                if (pChannelMap == NULL)
                {
                    if (iId >= iChannelCount)
                    {
                        continue;
                    }
                    iSource = iId;
                }
                else
                {
                    iSource = pChannelMap[iId];
                }

                if (iSource != -1)
                {
                    ppLevels[0][iSource] = 1.0f;
                }
            }
        }
        else
        {
            for (int iId = 0; iId < SPEAKER_ID_COUNT; ++iId)
            {
                int iSource;
                if (pChannelMap == NULL)
                {
                    if (iId >= iChannelCount)
                    {
                        continue;
                    }
                    iSource = iId;
                }
                else
                {
                    iSource = pChannelMap[iId];
                    if (iSource == -1)
                    {
                        continue;
                    }
                }

                int iSpeaker = pSpeakers[iId];
                if (iSpeaker != SPEAKER_UNMAPPED)
                {
                    ppLevels[iSpeaker][iSource] = 1.0f;
                    continue;
                }

                unsigned int uShared = SPEAKER_TRAITS[iId] & SPEAKER_TRAIT_SHARED;
                int iAssigned = 0;
                for (int iOther = 0; iOther < SPEAKER_ID_COUNT; ++iOther)
                {
                    if ((SPEAKER_TRAITS[iOther] & uShared) != 0 &&
                        pSpeakers[iOther] != SPEAKER_UNMAPPED)
                    {
                        ++iAssigned;
                        ppLevels[pSpeakers[iOther]][iSource] = 1.0f;
                    }
                }

                if (iAssigned == 0)
                {
                    unsigned int uTraits = SPEAKER_TRAITS[iId];
                    int iOther = 0;
                    while (true)
                    {
                        if ((uTraits & SPEAKER_TRAIT_LOW_FREQUENCY) == 0 &&
                            pSpeakers[iOther] != SPEAKER_UNMAPPED)
                        {
                            ppLevels[pSpeakers[iOther]][iSource] = 1.0f;
                        }

                        if (iOther + 1 > SPEAKER_ID_LAST)
                        {
                            break;
                        }

                        int iNext = iOther + 1;
                        while ((SPEAKER_TRAITS[iNext] & SPEAKER_TRAIT_SIDES) == 0)
                        {
                            ++iNext;
                            if (iNext > SPEAKER_ID_LAST)
                            {
                                break;
                            }
                        }

                        if (iNext > SPEAKER_ID_LAST)
                        {
                            break;
                        }

                        iOther = iNext;
                    }
                }
            }
        }

        for (int iId = 0; iId < SPEAKER_ID_COUNT; ++iId)
        {
            int iSpeaker = pSpeakers[iId];
            if (iSpeaker == SPEAKER_UNMAPPED)
            {
                continue;
            }

            int iContributors = 0;
            for (int iChannel = 0; iChannel < iChannelCount; ++iChannel)
            {
                if (ppLevels[iSpeaker][iChannel] > 0.0f)
                {
                    ++iContributors;
                }
            }

            if (iContributors > 1)
            {
                float fScale = 1.0f / static_cast<float>(iContributors);
                for (int iChannel = 0; iChannel < iChannelCount; ++iChannel)
                {
                    ppLevels[iSpeaker][iChannel] = ppLevels[iSpeaker][iChannel] * fScale;
                }
            }
        }
    }
}

extern "C" int SS_set_sample_type(_SAMPLE* pSample, unsigned int uFlags, int iChannelMask)
{
    if (pSample == NULL)
    {
        return 0;
    }

    unsigned int uFormat = uFlags & SAMPLE_FORMAT_MASK;
    int iChannels = static_cast<int>(uFlags) >> SAMPLE_FORMAT_CHANNEL_SHIFT;
    if ((uFormat & SAMPLE_FORMAT_STEREO) != 0)
    {
        iChannels = 2;
    }
    else if ((uFormat & SAMPLE_FORMAT_MULTICHANNEL) == 0)
    {
        iChannels = 1;
    }

    if (uFormat != pSample->channelFlags ||
        iChannelMask != pSample->channelMask || iChannels != pSample->channelCount)
    {
        pSample->channelFlags = uFormat;
        pSample->channelCount = iChannels;
        pSample->channelMask = iChannelMask;

        int iConfig = pSample->driver->speakerConfig;
        unsigned int uRequired = assign_chan_ptrs(pSample, NULL, 0);
        unsigned int uSize = (uRequired + CHANNEL_BUFFER_SLACK) & ~(CHANNEL_BUFFER_ALIGNMENT - 1);
        if (pSample->channelBufferCapacity < uSize || uSize < (pSample->channelBufferCapacity >> 1))
        {
            free_channel_buffers(pSample);
        }

        if (pSample->channelBuffer == NULL)
        {
            pSample->channelBuffer = static_cast<unsigned char*>(
                AIL_mem_alloc_lock_info(uSize, MILES_WAVEFILE_FILE, CHANNEL_BUFFER_LINE));
            if (pSample->channelBuffer == NULL)
            {
                AIL_set_error(CHANNEL_BUFFER_ERROR);
                return 0;
            }

            pSample->channelBufferCapacity = uSize;
        }

        pSample->channelBufferSize = uSize;
        memset(pSample->channelBuffer, 0, uSize);
        assign_chan_ptrs(pSample, pSample->channelBuffer, 0);

        for (int iChannel = 0; iChannel < pSample->channelCount; ++iChannel)
        {
            pSample->channelStates[iChannel].lowPass = CHANNEL_LOW_PASS_DEFAULT;
            pSample->channelStates[iChannel].busLevel = CHANNEL_BUS_LEVEL_DEFAULT;
            pSample->channelStates[iChannel].limitB = CHANNEL_LIMIT_DEFAULT;
            pSample->channelStates[iChannel].limitA = CHANNEL_LIMIT_DEFAULT;
        }

        for (int iSpeaker = 0; iSpeaker < pSample->driver->speakerConfig; ++iSpeaker)
        {
            pSample->speakerLevelsA[iSpeaker] = CHANNEL_LOW_PASS_DEFAULT;
            pSample->speakerScales[iSpeaker] = CHANNEL_LOW_PASS_DEFAULT;
        }

        clear_last_sample(pSample);

        if (iConfig != 0)
        {
            if (pSample->channelMap != NULL)
            {
                AIL_mem_free_lock(pSample->channelMap);
            }

            signed char* pMap = NULL;
            unsigned int uMask = pSample->channelMask;
            pSample->channelMap = NULL;
            if (uMask != 0)
            {
                bool bHasGap = false;
                while (uMask != 0)
                {
                    if ((uMask & 1) == 0)
                    {
                        bHasGap = true;
                    }
                    uMask = uMask >> 1;
                }

                if (bHasGap)
                {
                    pMap = static_cast<signed char*>(AIL_mem_alloc_lock_info(
                        CHANNEL_MAP_LENGTH, MILES_WAVEFILE_FILE, CHANNEL_MAP_LINE));
                    memset(pMap, -1, CHANNEL_MAP_LENGTH);

                    int iNext = 0;
                    unsigned int uBit = 1;
                    for (int iId = 0; iId < CHANNEL_MASK_BITS; ++iId)
                    {
                        if (iNext >= iChannels || iId >= CHANNEL_MAP_LENGTH)
                        {
                            break;
                        }

                        if ((pSample->channelMask & uBit) != 0)
                        {
                            pMap[iId] = static_cast<signed char>(iNext);
                            iNext = static_cast<signed char>(iNext + 1);
                        }

                        uBit = uBit << 1;
                    }

                    pSample->channelMap = pMap;
                }
            }

            set_user_channel_defaults(pSample->driver->speakerConfig, pSample->channelCount,
                                      pSample->levels, pMap);
        }

        if ((uFormat & SAMPLE_FORMAT_ADPCM) != 0)
        {
            pSample->resetA = 0;
            pSample->resetB = 0;
        }
    }

    return 1;
}

int SP_ASI_DECODER_callback(unsigned int uUser, void* pDestination, int iCount, int iOffset)
{
    _SAMPLE* pSample = reinterpret_cast<_SAMPLE*>(uUser);
    int iCurrent = pSample->currentBuffer;
    SampleBuffer* pBuffer = &pSample->buffers[iCurrent];

    if (pBuffer->size < iOffset)
    {
        iOffset = pBuffer->size;
    }

    if (iOffset != -1)
    {
        pBuffer->position = iOffset;
    }

    unsigned int uPosition;
    unsigned int uEnd;
    if (pSample->bufferRewind == 0)
    {
        uPosition = pBuffer->position;
        uEnd = pBuffer->size;
    }
    else
    {
        pSample->bufferRewind = 0;
        uPosition = pBuffer->size;
        pBuffer->position = uPosition;
        uEnd = uPosition;
    }

    if (pSample->loopMode == LOOP_MODE_ASI && pSample->loopState != 1)
    {
        unsigned int uLoopEnd = pSample->loopEnd;
        if (uLoopEnd != 0xffffffff && uLoopEnd < static_cast<unsigned int>(pSample->buffers[pSample->currentBuffer].size))
        {
            uEnd = uLoopEnd;
        }

        if (uEnd <= uPosition)
        {
            return 0;
        }
    }

    int iCopied = iCount;
    if (uEnd < uPosition + iCount)
    {
        iCopied = uEnd - uPosition;
    }

    memcpy(pDestination, pBuffer->data + uPosition, iCopied);
    pDestination = static_cast<unsigned char*>(pDestination) + iCopied;
    iCount = iCount - iCopied;
    pBuffer->position = pBuffer->position + iCopied;

    while (iCount > 0)
    {
        if (pSample->endOfBuffer != NULL)
        {
            pSample->endCounter = pSample->endCounter + 1;
        }

        if (pSample->loopMode != LOOP_MODE_STREAMING)
        {
            return iCopied;
        }

        if (pBuffer->reset != 0)
        {
            return iCopied;
        }

        iCurrent = (iCurrent + 1) % pSample->bufferCount;
        pBuffer = &pSample->buffers[iCurrent];
        if (pBuffer->done != 0)
        {
            break;
        }

        if (iCurrent == pSample->lastBuffer || pBuffer->size == 0 || pBuffer->position != 0)
        {
            pSample->starved = 1;
            return iCopied;
        }

        pSample->currentBuffer = iCurrent;
        int iPosition = pBuffer->position;
        int iChunk = iCount;
        if (static_cast<unsigned int>(pBuffer->size) < static_cast<unsigned int>(iPosition + iCount))
        {
            iChunk = pBuffer->size - iPosition;
        }

        memcpy(pDestination, pBuffer->data + iPosition, iChunk);
        pDestination = static_cast<unsigned char*>(pDestination) + iChunk;
        iCount = iCount - iChunk;
        iCopied = iCopied + iChunk;
        pBuffer->position = pBuffer->position + iChunk;
    }

    return iCopied;
}

extern "C" int AIL_API_set_sample_processor(_SAMPLE* pSample, int iStage, int iProvider)
{
    unsigned int uIndex = iStage;
    if (iStage == SAMPLE_STAGE_ALL)
    {
        uIndex = SAMPLE_STAGE_DECODER;
    }

    unsigned int uLast = uIndex;
    if (iStage == SAMPLE_STAGE_ALL)
    {
        uLast = SAMPLE_STAGE_COUNT - 1;
    }

    SampleStage* pStage;
    while (true)
    {
        pStage = &pSample->stages[uIndex];
        if (pStage->filter.active != 0)
        {
            if (uIndex == SAMPLE_STAGE_DECODER)
            {
                pStage->decoder.close(pStage->decoder.stream);
                pStage->decoder.active = 0;
            }
            else if (uIndex < SAMPLE_STAGE_MERGE)
            {
                FilterStage& filter = pStage->filter;
                for (int iHandle = 0; iHandle < SAMPLE_FILTER_HANDLE_COUNT; ++iHandle)
                {
                    if (filter.handles[iHandle] != NULL)
                    {
                        filter.instance->closeSample(filter.handles[iHandle]);
                        if (filter.handles[iHandle] != NULL)
                        {
                            AIL_mem_free_lock(filter.handles[iHandle]);
                            filter.handles[iHandle] = NULL;
                        }
                    }
                }

                pSample->filterCount = pSample->filterCount - 1;
                filter.active = 0;
            }
            else
            {
                pStage->merge.active = 0;
            }
        }

        if (uIndex == uLast)
        {
            break;
        }

        ++uIndex;
    }

    int iPrevious = pStage->filter.provider;
    pStage->filter.provider = iProvider;
    if (iProvider == 0)
    {
        return iPrevious;
    }

    if (uIndex != SAMPLE_STAGE_DECODER)
    {
        if (uIndex > SAMPLE_FILTER_STAGE_COUNT)
        {
            if (uIndex != SAMPLE_STAGE_MERGE)
            {
                return iPrevious;
            }

            RIB_INTERFACE_ENTRY mergeEntry = {RIB_FUNCTION, "MIXER_merge", &pStage->merge.merge, 0};
            RIB_request_interface(pStage->merge.provider, MIXER_SERVICES_INTERFACE, 1, &mergeEntry);
            pStage->merge.active = 1;
            return iPrevious;
        }

        if (AIL_open_filter(iProvider, pSample->driver) != 0)
        {
            FLT_INSTANCE* pInstance = FLT_find_provider_instance(iProvider, pSample->driver);
            FilterStage& filter = pStage->filter;
            for (int iHandle = 0; iHandle < SAMPLE_FILTER_HANDLE_COUNT; ++iHandle)
            {
                filter.handles[iHandle] = NULL;
            }
            filter.instance = pInstance;

            if (pInstance != NULL && pInstance->openSample != NULL)
            {
                for (int iChannel = 0; iChannel < pSample->channelCount; ++iChannel)
                {
                    void* pState = NULL;
                    if (pInstance->sampleStateSize != 0)
                    {
                        pState = AIL_mem_alloc_lock_info(pInstance->sampleStateSize,
                                                         MILES_WAVEFILE_FILE, FILTER_STATE_LINE);
                    }

                    filter.handles[iChannel] =
                        pInstance->openSample(pInstance->driverState, pSample, pState);
                    if (filter.handles[iChannel] == NULL)
                    {
                        pStage->filter.provider = 0;
                        return iPrevious;
                    }
                }

                filter.active = 1;
                pSample->filterCount = pSample->filterCount + 1;
                return iPrevious;
            }
        }

        pStage->filter.provider = 0;
        return iPrevious;
    }

    DecoderStage& decoder = pStage->decoder;
    RIB_INTERFACE_ENTRY streamEntries[4] = {
        {RIB_FUNCTION, "ASI_stream_property", &decoder.property, 0},
        {RIB_FUNCTION, "ASI_stream_open", &decoder.open, 0},
        {RIB_FUNCTION, "ASI_stream_seek", &decoder.seek, 0},
        {RIB_FUNCTION, "ASI_stream_close", &decoder.close, 0}};
    RIB_INTERFACE_ENTRY inputEntries[3] = {
        {RIB_FUNCTION, "ASI_stream_process", &decoder.process, 0},
        {RIB_PROPERTY, "Input bit rate", &decoder.inputBitRate, 0},
        {RIB_PROPERTY, "Input sample rate", &decoder.inputSampleRate, 0}};
    RIB_INTERFACE_ENTRY inputFormatEntries[4] = {
        {RIB_PROPERTY, "Input sample width", &decoder.inputSampleWidth, 0},
        {RIB_PROPERTY, "Input channels", &decoder.inputChannels, 0},
        {RIB_PROPERTY, "Output bit rate", &decoder.outputBitRate, 0},
        {RIB_PROPERTY, "Output sample rate", &decoder.outputSampleRate, 0}};
    RIB_INTERFACE_ENTRY outputEntries[4] = {
        {RIB_PROPERTY, "Output sample width", &decoder.outputSampleWidth, 0},
        {RIB_PROPERTY, "Output channels", &decoder.outputChannels, 0},
        {RIB_PROPERTY, "Output reservoir", &decoder.outputReservoir, 0},
        {RIB_PROPERTY, "Position", &decoder.position, 0}};
    RIB_INTERFACE_ENTRY progressEntries[4] = {
        {RIB_PROPERTY, "Percent done", &decoder.percentDone, 0},
        {RIB_PROPERTY, "Minimum input block size", &decoder.minimumInputBlockSize, 0},
        {RIB_PROPERTY, "Raw source sample rate", &decoder.rawSourceSampleRate, 0},
        {RIB_PROPERTY, "Raw source sample width", &decoder.rawSourceSampleWidth, 0}};
    RIB_INTERFACE_ENTRY requestEntries[4] = {
        {RIB_PROPERTY, "Raw source channels", &decoder.rawSourceChannels, 0},
        {RIB_PROPERTY, "Requested sample rate", &decoder.requestedSampleRate, 0},
        {RIB_PROPERTY, "Requested sample width", &decoder.requestedSampleWidth, 0},
        {RIB_PROPERTY, "Requested channels", &decoder.requestedChannels, 0}};
    RIB_INTERFACE_ENTRY layoutEntries[4] = {
        {RIB_PROPERTY, "Output channel mask", &decoder.outputChannelMask, 0},
        {RIB_PROPERTY, "Stream seek position", &decoder.streamSeekPosition, 0},
        {RIB_PROPERTY, "Data offset", &decoder.dataOffset, 0},
        {RIB_PROPERTY, "Data length", &decoder.dataLength, 0}};
    RIB_INTERFACE_ENTRY seekEntries[5] = {
        {RIB_PROPERTY, "Exact seek", &decoder.exactSeek, 0},
        {RIB_PROPERTY, "Exact get position", &decoder.exactGetPosition, 0},
        {RIB_PROPERTY, "Seek table lookup", &decoder.seekTableLookup, 0},
        {RIB_PROPERTY, "Set Looping Samples", &decoder.setLoopingSamples, 0},
        {RIB_PROPERTY, "Clear Loop Meta", &decoder.clearLoopMeta, 0}};

    unsigned int uProfile = RIB_ENTRY_NOT_FOUND;
    RIB_INTERFACE_ENTRY profileEntry = {RIB_PROPERTY, "Profile index", &uProfile, 0};

    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 4, streamEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 3, inputEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 4, inputFormatEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 4, outputEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 4, progressEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 4, requestEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 4, layoutEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 5, seekEntries);

    decoder.stream = decoder.open(MSS_alloc_info, MSS_free_info, reinterpret_cast<unsigned int>(pSample),
                                  SP_ASI_DECODER_callback, pSample->buffers[0].size);

    RIB_request_interface(decoder.provider, ASI_CODEC_INTERFACE, 1, &profileEntry);
    uProfile = uProfile - ASI_PROFILE_BASE;
    if (uProfile >= ASI_PROFILE_LIMIT)
    {
        uProfile = 0;
    }

    if (decoder.stream == NULL)
    {
        decoder.provider = 0;
        return iPrevious;
    }

    _DIG_DRIVER* pDriver = pSample->driver;
    decoder.property(decoder.stream, decoder.requestedSampleRate, 0, &pDriver->outputRate, 0);

    int iRequestedChannels = ASI_DEFAULT_CHANNELS - ((pDriver->flags & DRIVER_FLAG_STEREO_OR_MULTICHANNEL) == 0);
    decoder.property(decoder.stream, decoder.requestedChannels, 0, &iRequestedChannels, 0);

    int iRequestedWidth = (pDriver->flags & DRIVER_FLAG_16_BITS) * SAMPLE_WIDTH_8 + SAMPLE_WIDTH_8;
    decoder.property(decoder.stream, decoder.requestedSampleWidth, 0, &iRequestedWidth, 0);

    unsigned int uOutputChannels = RIB_ENTRY_NOT_FOUND;
    decoder.property(decoder.stream, decoder.outputChannels, &uOutputChannels, 0, 0);
    int iOutputRate = -1;
    decoder.property(decoder.stream, decoder.outputSampleRate, &iOutputRate, 0, 0);
    int iOutputWidth = -1;
    decoder.property(decoder.stream, decoder.outputSampleWidth, &iOutputWidth, 0, 0);

    unsigned int uMask = 0;
    unsigned int uSampleChannels = AIL_sample_channel_count(pSample, &uMask);
    if (AIL_sample_playback_rate(pSample) != iOutputRate)
    {
        AIL_set_sample_playback_rate(pSample, iOutputRate);
    }

    int iResult;
    if (uOutputChannels < ASI_CHANNEL_LIMIT && pSample->channelMask == -1)
    {
        if (uOutputChannels != uSampleChannels)
        {
            if (uOutputChannels == 1)
            {
                iResult = SS_set_sample_type(pSample, iOutputWidth == SAMPLE_WIDTH_16, -1);
            }
            else
            {
                unsigned int uFormat = SAMPLE_FORMAT_STEREO;
                if (iOutputWidth == SAMPLE_WIDTH_16)
                {
                    uFormat = SAMPLE_FORMAT_STEREO | SAMPLE_FORMAT_16_BITS;
                }

                iResult = SS_set_sample_type(pSample, uFormat, -1);
            }

            if (iResult == 0)
            {
                return -1;
            }
        }
    }
    else
    {
        unsigned int uOutputMask = RIB_ENTRY_NOT_FOUND;
        if (decoder.outputChannelMask != RIB_ENTRY_NOT_FOUND)
        {
            decoder.property(decoder.stream, decoder.outputChannelMask, &uOutputMask, 0, 0);
        }

        if (uOutputMask != uMask || uSampleChannels != uOutputChannels)
        {
            iResult = SS_set_sample_type(pSample,
                                         (uOutputChannels << SAMPLE_FORMAT_CHANNEL_SHIFT) |
                                             SAMPLE_FORMAT_MULTICHANNEL | SAMPLE_FORMAT_16_BITS,
                                         uOutputMask);
            if (iResult == 0)
            {
                return -1;
            }
        }
    }

    if (pSample->loopMode == LOOP_MODE_ASI)
    {
        if (decoder.dataOffset != RIB_ENTRY_NOT_FOUND)
        {
            unsigned int uDataOffset = 0;
            decoder.property(decoder.stream, decoder.dataOffset, &uDataOffset, 0, 0);
            pSample->buffers[0].position = 0;
            pSample->buffers[0].data = pSample->buffers[0].data + uDataOffset;
        }

        if (decoder.dataLength != RIB_ENTRY_NOT_FOUND)
        {
            decoder.property(decoder.stream, decoder.dataLength, &pSample->buffers[0].size, 0, 0);
        }
    }

    decoder.active = uProfile + 1;
    return iPrevious;
}
