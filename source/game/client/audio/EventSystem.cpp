#include "game/client/audio/EventSystem.hpp"
#include "vendor/MilesEventError.hpp"

#include "game/client/audio/SoundInstance.hpp"
#include "game/core/Handle.hpp"
#include "game/util/LegacyUtilityFunctions.hpp"
#include "game/util/StringUtils.hpp"
#include "game/core/PODHandleArray.hpp"
#include "game/client/audio/FalloffGraph.hpp"
#include "vendor/MilesAsync.hpp"
#include "vendor/MilesIO.hpp"
#include "vendor/MilesMem.hpp"

#include <math.h>
#include <stddef.h>
#include <string.h>

namespace
{
const int SOUND_LOAD_SLOT_COUNT = 4;
const int SOUND_ENTRY_SIZE = 0x3c;
const int SOUND_EXTENSION_LIMIT = 8;
const int RAMP_NAME_SIZE = 0x40;
const int RAMP_LABELS_SIZE = 0x100;
const int RAMP_VALUE_SIZE = 0x40;
const int RAMP_SIZE = 0x1ac;
const int PERSIST_HEADER_SIZE = 0x28;
const int LIMIT_LIST_HEADER_SIZE = 0x1d;
const int LIMIT_ENTRY_HEADER_SIZE = 0x19;
const int VARIABLE_HEADER_SIZE = 0x24;
const int LFO_HEADER_SIZE = 0x41;
const int FIRST_AUDITION_ID = 1;
const int NO_LOAD_SLOT = 0;
const int ASYNC_READ_WHOLE_FILE = -1;
const int SAMPLE_UNCHANGED = -1;
const int LOAD_REQUEST_BUSY = 1;
const int LOAD_REQUEST_CANCELLED = 1;
const int LOAD_REQUEST_IDLE = 0;
const int CURVE_CLAMP = 0;
const int CURVE_WRAP = 1;
const int CURVE_MIRROR = 2;

const int RAMP_CURVE_SQUARE = 1;
const int RAMP_CURVE_SMOOTH = 2;

const int LFO_MODE_FLAT = 0;
const int LFO_MODE_SQUARE_FIRST = 3;

const float RAMP_SILENCE_DECIBELS = -96.0f;
const float RAMP_DECIBELS_PER_DECADE = 20.0f;
const double RAMP_DECADE_BASE = 10.0;
const float RAMP_DECIBEL_EXPONENT = 0.6f;
const float RAMP_DURATION_EPSILON = 0.001f;
const float RAMP_VALUE_EPSILON = 0.001f;
const float RAMP_SEMITONE_DIVISOR = 12.0f;
const double RAMP_SEMITONE_BASE = 2.0;
const float RAMP_SMOOTH_CENTER = 0.5f;
const float RAMP_SMOOTH_SCALE = 4.0f;
const float RAMP_UNIT_VALUE = 1.0f;
const float RAMP_ZERO_VALUE = 0.0f;
const float RAMP_TARGET_RATE_DEFAULT = 24000.0f;
const float RAMP_RATE_INITIAL = 96000.0f;
const float SOUND_VOLUME_CURVE_EXPONENT = 1.6666666f;
const float SAMPLE_CHANGE_EPSILON = 0.00001f;
const double NO_SOURCE_VOLUME = 1.0;
const float LFO_HALF = 0.5f;
const float LFO_ONE = 1.0f;
const float LFO_ZERO = 0.0f;
const char LIMIT_FIELD_SEPARATOR[] = " ";
const char SOUND_EXTENSION_WAV[] = ".wav";
const char INPUT_FILE_TYPES[] = "Input file types";
const char NO_PROVIDER_ERROR[] =
    "Couldn't find provider for this file type -- check your AIL_set_redist_directory() call.";
const char MILES_EVENT_SUPPORT_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\mileseventsupport.cpp";

const ErrorType ERROR_TYPE_NO_PROVIDER = static_cast<ErrorType>(2);
const ErrorType ERROR_TYPE_SOUND_NOT_FOUND = static_cast<ErrorType>(6);

enum RampTarget
{
    RAMP_TARGET_LEVEL_A = 0,
    RAMP_TARGET_LEVEL_B = 1,
    RAMP_TARGET_RATE = 2,
    RAMP_TARGET_LEVEL_C = 3
};

struct SoundSourceData
{
    unsigned char m_reserved0[0x30];
    float m_volume;
};

struct EventSystemVariable
{
    Handle m_parent;
    Handle m_self;
    union
    {
        int m_intValue;
        float m_floatValue;
    } m_value;
    int m_isFloat;
    Handle m_curve;
    char m_name[1];
};

struct VariableCurve
{
    float m_start;
    float m_end;
    int m_lowMode;
    int m_highMode;
    unsigned int m_pointCount;
    GraphPoint m_points[1];
};

struct SoundEntry
{
    Handle m_name;
    unsigned int m_provider;
    void* m_data;
    unsigned int m_nameLength;
    int m_size;
    unsigned char m_reserved0[4];
    int m_refCount;
    int m_loadSlot;
    int m_auditionId;
    int m_param;
    unsigned char m_reserved1[4];
    int m_extraParam;
    char m_extension[SOUND_EXTENSION_LIMIT];
};

struct PersistNode
{
    Handle m_next;
    Handle m_self;
    Handle m_preset;
    Handle m_labels;
    int m_active;
    char m_name[1];
};

struct LimitList
{
    Handle m_next;
    Handle m_self;
    Handle m_entries;
    char m_name[1];
};

struct LimitEntry
{
    Handle m_next;
    Handle m_self;
    short m_limit;
    short m_count;
    char m_query[1];
};

struct LfoNode
{
    Handle m_next;
    Handle m_self;
    float m_center;
    float m_amplitude;
    float m_phase;
    Handle m_centerText;
    Handle m_amplitudeText;
    Handle m_phaseText;
    unsigned char m_reserved0[4];
    unsigned char m_inverted;
    unsigned char m_mode;
    unsigned char m_flip;
    unsigned char m_offset;
    char m_name[1];
};

}

struct Ramp
{
    Handle m_next;
    Handle m_self;
    float m_elapsed;
    float m_duration;
    float m_startValue;
    float m_cachedValue;
    int m_cacheValid;
    int m_curve;
    int m_target;
    char m_value[RAMP_VALUE_SIZE];
    char m_name[RAMP_NAME_SIZE];
    char m_labels[RAMP_LABELS_SIZE];
};

int DoLabelsMatchQuery(const char* labels, const char* query);

extern "C" int AIL_find_marker_in_list(int markerList, const char* markerName, int* markerType);
extern int strtoi(const char* value, const char** end);

MilesAsyncRead g_soundLoadRequests[SOUND_LOAD_SLOT_COUNT];
int g_soundLoadBusy[SOUND_LOAD_SLOT_COUNT];
int g_soundLoadCancelled[SOUND_LOAD_SLOT_COUNT];
EventSystem* g_soundLoadOwners[SOUND_LOAD_SLOT_COUNT];
static int sNextAuditionId = FIRST_AUDITION_ID;

int strtoi(const char* value, const char** end)
{
    int result = 0;

    while (*value != '\0' && (unsigned int)(*value - '0') < 10)
    {
        result = result * 10 + *value - '0';
        ++value;
    }

    if (end != 0)
    {
        *end = value;
    }

    return result;
}

int Var_LookupWithContext(EventSystem* eventSystem, Handle handle, const char* name, int flags,
                          void* value)
{
    Handle context = handle;


    for (;;)
    {
        EventSystemVariable* variable = static_cast<EventSystemVariable*>(Handle_Get(context));

        while (variable != 0)
        {
            if (AIL_stricmp(name, variable->m_name) == 0)
            {
                if (flags == 0)
                {
                    int* result = static_cast<int*>(value);
                    if (variable->m_isFloat != 0)
                    {
                        *result = (int)(variable->m_value.m_floatValue + 0.001f);
                    }
                    else
                    {
                        *result = variable->m_value.m_intValue;
                    }
                }
                else
                {
                    float* result = static_cast<float*>(value);
                    if (variable->m_isFloat != 0)
                    {
                        *result = variable->m_value.m_floatValue;
                    }
                    else
                    {
                        *result = (float)variable->m_value.m_intValue;
                    }
                }
                return 1;
            }

            variable = static_cast<EventSystemVariable*>(Handle_Get(variable->m_parent));
        }

        if (context.index == eventSystem->m_variables.index &&
            context.generation == eventSystem->m_variables.generation)
        {
            ErrorHandler(ERROR_TYPE_MISSING_VARIABLE, 0, name);
            return 0;
        }

        context = eventSystem->m_variables;
    }
}

int FindMarker(EventSystem* eventSystem, SoundInstance* soundInstance, int markerList, const char* markerName,
               int* markerIndex, int* markerType)
{
    if (AIL_stricmp(markerName, "SampleStart") == 0)
    {
        *markerType = 1;
        *markerIndex = 0;
        return 1;
    }

    if (AIL_stricmp(markerName, "SampleEnd") == 0)
    {
        *markerType = 1;
        *markerIndex = -1;
        return 1;
    }

    int lookupType = 2;
    int variableType;
    if (*markerName == '@')
    {
        ++markerName;
        lookupType = 1;
        variableType = 1;
    }
    else
    {
        variableType = 2;
    }

    if (*markerName != '=')
    {
        *markerType = lookupType;

        Handle handle;
        if (soundInstance != 0)
        {
            handle = reinterpret_cast<const SoundInstanceData*>(soundInstance)->m_context;
        }
        else
        {
            handle = eventSystem->m_variables;
        }

        return Var_LookupWithContext(eventSystem, handle, markerName, 0, markerIndex);
    }

    ++markerName;
    if (OnlyNum(markerName) != 0)
    {
        *markerIndex = strtoi(markerName, 0);
        *markerType = variableType;
        return 1;
    }

    int foundIndex = AIL_find_marker_in_list(markerList, markerName, &variableType);
    if (foundIndex == -1)
    {
        return 0;
    }

    *markerIndex = foundIndex;
    *markerType = variableType != 0 ? 4 : 1;
    return 1;
}

Handle Sound_Load(EventSystem* eventSystem, unsigned long long eventId, const char* name)
{
    char path[512];
    _MILESBANKSOUNDINFO info;

    if (Container_GetSound(name, path, &info) == 0)
    {
        ErrorHandler(ERROR_TYPE_SOUND_NOT_FOUND, eventId, name);
        return Handle();
    }

    if (!eventSystem->m_sounds.PushEmpty())
    {
        ErrorHandler(ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY, eventId, name);
        return Handle();
    }

    Handle sound = Handle_AllocateInfo(SOUND_ENTRY_SIZE, MILES_EVENT_SUPPORT_FILE, 0x2a3);
    SoundEntry* entry = static_cast<SoundEntry*>(Handle_Get(sound));
    if (entry == NULL)
    {
        ErrorHandler(ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY, eventId, name);
        eventSystem->m_sounds.Pop();
        return Handle();
    }

    memset(entry, 0, SOUND_ENTRY_SIZE);

    char* extension = strrchr(path, '.');
    if (extension != NULL)
    {
        if (strlen(extension) < SOUND_EXTENSION_LIMIT)
        {
            strcpy(entry->m_extension, extension);
        }
    }

    entry->m_provider = find_ASI_provider(INPUT_FILE_TYPES, extension);
    if (AIL_stricmp(extension, SOUND_EXTENSION_WAV) != 0 && entry->m_provider == 0)
    {
        AIL_set_error(NO_PROVIDER_ERROR);
        ErrorHandler(ERROR_TYPE_NO_PROVIDER, eventId, name);
        Handle_Free(sound);
        eventSystem->m_sounds.Pop();
        return Handle();
    }

    entry->m_param = info.m_param;
    entry->m_extraParam = info.m_extraParam;
    entry->m_nameLength = strlen(name);
    entry->m_name = Handle_AllocateInfo(entry->m_nameLength + 1, MILES_EVENT_SUPPORT_FILE, 0x2c6);
    char* nameCopy = static_cast<char*>(Handle_Get(entry->m_name));
    if (nameCopy == NULL)
    {
        ErrorHandler(ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY, eventId, name);
        Handle_Free(sound);
        eventSystem->m_sounds.Pop();
        return Handle();
    }

    entry->m_auditionId = sNextAuditionId++;
    Audition_AsyncQueued(eventId, entry->m_auditionId, name);
    strcpy(nameCopy, name);
    entry->m_refCount = 1;
    eventSystem->m_sounds.Top() = sound;

    for (int slot = 0; slot < SOUND_LOAD_SLOT_COUNT; ++slot)
    {
        if (g_soundLoadBusy[slot] != LOAD_REQUEST_IDLE)
        {
            continue;
        }

        MilesAsyncRead* request = &g_soundLoadRequests[slot];
        memset(request, 0, sizeof(MilesAsyncRead));
        strcpy(request->szFileName, path);
        Audition_AsyncLoad(entry->m_auditionId, info.m_dataSize);
        if (info.m_unbuffered == 0)
        {
            entry->m_data = AIL_mem_alloc_lock_info(info.m_dataSize, MILES_EVENT_SUPPORT_FILE, 0x2e8);
            if (entry->m_data == NULL)
            {
                ErrorHandler(ERROR_TYPE_OUT_OF_MEMORY, eventId, name);
                Handle_Free(entry->m_name);
                Handle_Free(sound);
                eventSystem->m_sounds.Pop();
                return Handle();
            }

            entry->m_size = info.m_dataSize;
            request->iLength = info.m_dataSize;
            request->iLengthHigh = info.m_dataSize >> 31;
            request->pBuffer = entry->m_data;
            request->pPriorityUserData = reinterpret_cast<void*>(entry->m_auditionId);
        }
        else
        {
            request->iLength = ASYNC_READ_WHOLE_FILE;
            request->iLengthHigh = ASYNC_READ_WHOLE_FILE;
            request->pPriorityUserData = reinterpret_cast<void*>(entry->m_auditionId);
        }

        MilesAsyncFileRead(request);
        g_soundLoadOwners[slot] = eventSystem;
        g_soundLoadBusy[slot] = LOAD_REQUEST_BUSY;
        entry->m_loadSlot = slot + 1;
        break;
    }

    int count = eventSystem->m_sounds.Count();
    for (int i = 0; i < count - 1; ++i)
    {
        SoundEntry* other = static_cast<SoundEntry*>(Handle_Get(eventSystem->m_sounds[i]));
        if (other->m_loadSlot == NO_LOAD_SLOT)
        {
            Handle swapped = eventSystem->m_sounds[i];
            eventSystem->m_sounds[i] = eventSystem->m_sounds.Top();
            eventSystem->m_sounds.Top() = swapped;
            break;
        }
    }

    return sound;
}

void Sound_Free(EventSystem* eventSystem, Handle sound)
{
    SoundEntry* entry = static_cast<SoundEntry*>(Handle_Get(sound));
    if (entry == NULL)
    {
        return;
    }

    entry->m_refCount--;
    if (entry->m_refCount != 0)
    {
        return;
    }

    if (entry->m_loadSlot == NO_LOAD_SLOT)
    {
        eventSystem->m_soundBytes -= entry->m_size;
        AIL_mem_free_lock(entry->m_data);
        Handle_Free(entry->m_name);
        Handle_Free(sound);
    }
    else
    {
        int slot = entry->m_loadSlot - 1;
        if (MilesAsyncFileCancel(&g_soundLoadRequests[slot]) == 0)
        {
            g_soundLoadCancelled[slot] = LOAD_REQUEST_CANCELLED;
            entry->m_data = NULL;
        }
        else
        {
            Audition_AsyncCancel(reinterpret_cast<int>(g_soundLoadRequests[slot].pPriorityUserData));
            g_soundLoadBusy[slot] = LOAD_REQUEST_IDLE;
            AIL_mem_free_lock(entry->m_data);
        }
        Handle_Free(entry->m_name);
        Handle_Free(sound);
    }

    Handle* handles = eventSystem->m_sounds.Data();
    Handle* end = handles + eventSystem->m_sounds.Count();
    for (Handle* current = handles; current != end; ++current)
    {
        if (current->index == sound.index && current->generation == sound.generation)
        {
            eventSystem->m_sounds.RemoveSortedCount(static_cast<int>(current - handles), 1);
            return;
        }
    }
}

void Persist_Free(EventSystem* eventSystem, unsigned long long eventId, const char* name)
{
    PersistNode* previous = NULL;
    PersistNode* node = static_cast<PersistNode*>(Handle_Get(eventSystem->m_persists));
    while (node != NULL)
    {
        if (AIL_stricmp(name, node->m_name) == 0)
        {
            void* preset = Container_GetPreset(static_cast<const char*>(Handle_Get(node->m_preset)));
            const char* labels = static_cast<const char*>(Handle_Get(node->m_labels));
            int count = eventSystem->m_instances.Count();
            for (int i = 0; i < count; ++i, --count)
            {
                SoundInstanceData* instance =
                    static_cast<SoundInstanceData*>(Handle_Get(eventSystem->m_instances[i]));
                const char* instanceLabels = static_cast<const char*>(Handle_Get(instance->m_labels));
                if (DoLabelsMatchQuery(instanceLabels, labels))
                {
                    AIL_unapply_raw_sound_preset(instance->m_sample, preset);
                }
            }

            if (previous == NULL)
            {
                eventSystem->m_persists = node->m_next;
            }
            else
            {
                previous->m_next = node->m_next;
            }

            Audition_SetPersist(eventId, node->m_name, NULL);
            Handle_Free(node->m_preset);
            Handle_Free(node->m_labels);
            Handle_Free(node->m_self);
            return;
        }

        previous = node;
        node = static_cast<PersistNode*>(Handle_Get(node->m_next));
    }
}

int Persist_Create(EventSystem* eventSystem, unsigned long long eventId, const EVENT_STEP_INFO* step)
{
    const PersistStep& persist = step->m_data.m_persist;
    PersistNode* node = static_cast<PersistNode*>(Handle_Get(eventSystem->m_persists));
    while (node != NULL)
    {
        if (AIL_stricmp(node->m_name, persist.m_name) == 0)
        {
            break;
        }
        node = static_cast<PersistNode*>(Handle_Get(node->m_next));
    }

    bool existing = false;
    if (node == NULL)
    {
        Handle self = Handle_AllocateInfo(persist.m_nameLength + PERSIST_HEADER_SIZE,
                                          MILES_EVENT_SUPPORT_FILE, 0x4a5);
        node = static_cast<PersistNode*>(Handle_Get(self));
        if (node == NULL)
        {
            ErrorHandler(ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY, eventId, persist.m_name);
            return 0;
        }

        node->m_self = self;
        strcpy(node->m_name, persist.m_name);
        node->m_active = persist.m_active;
        node->m_preset = Handle_AllocateInfo(persist.m_presetLength + 1, MILES_EVENT_SUPPORT_FILE, 0x4b2);
        node->m_labels = Handle_AllocateInfo(persist.m_labelsLength + 1, MILES_EVENT_SUPPORT_FILE, 0x4b3);
    }
    else
    {
        Handle_Free(node->m_preset);
        Handle_Free(node->m_labels);
        node->m_active = persist.m_active;
        existing = true;
        node->m_preset = Handle_AllocateInfo(persist.m_presetLength + 1, MILES_EVENT_SUPPORT_FILE, 0x4b2);
        node->m_labels = Handle_AllocateInfo(persist.m_labelsLength + 1, MILES_EVENT_SUPPORT_FILE, 0x4b3);
    }

    char* presetText = static_cast<char*>(Handle_Get(node->m_preset));
    char* labelsText = static_cast<char*>(Handle_Get(node->m_labels));
    if (presetText != NULL && labelsText != NULL)
    {
        strcpy(presetText, persist.m_preset);
        strcpy(labelsText, persist.m_labels);
        if (!existing)
        {
            node->m_next = eventSystem->m_persists;
            eventSystem->m_persists = node->m_self;
        }

        void* preset = Container_GetPreset(persist.m_preset);
        if (preset != NULL)
        {
            int count = eventSystem->m_instances.Count();
            for (int i = 0; i < count; ++i, --count)
            {
                SoundInstanceData* instance =
                    static_cast<SoundInstanceData*>(Handle_Get(eventSystem->m_instances[i]));
                const char* instanceLabels = static_cast<const char*>(Handle_Get(instance->m_labels));
                if (DoLabelsMatchQuery(instanceLabels, persist.m_labels))
                {
                    AIL_resolve_raw_sound_preset(preset, instance, EvalFunc);
                    AIL_apply_raw_sound_preset(instance->m_sample, preset);
                }
            }

            Audition_SetPersist(eventId, node->m_name, presetText);
            return 1;
        }

        ErrorHandler(ERROR_TYPE_SOUND_NOT_FOUND, eventId, persist.m_preset);
        return 1;
    }

    ErrorHandler(ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY, eventId, persist.m_name);
    if (!existing)
    {
        Handle_Free(node->m_preset);
        Handle_Free(node->m_labels);
        Handle_Free(node->m_self);
        return 0;
    }

    Persist_Free(eventSystem, eventId, node->m_name);
    return 0;
}

void Persist_Apply(EventSystem* eventSystem, SoundInstance* soundInstance, _SAMPLE* sample,
                   const char* labels, int unusedArgument, int onlyActive)
{
    PersistNode* node = static_cast<PersistNode*>(Handle_Get(eventSystem->m_persists));
    while (node != NULL)
    {
        if (onlyActive == 0 || node->m_active != 0)
        {
            const char* nodeLabels = static_cast<const char*>(Handle_Get(node->m_labels));
            if (DoLabelsMatchQuery(labels, nodeLabels))
            {
                void* preset = Container_GetPreset(static_cast<const char*>(Handle_Get(node->m_preset)));
                AIL_resolve_raw_sound_preset(preset, soundInstance, EvalFunc);
                AIL_apply_raw_sound_preset(sample, preset);
            }
        }

        node = static_cast<PersistNode*>(Handle_Get(node->m_next));
    }
}

int Limit_Add(EventSystem* eventSystem, const EVENT_STEP_INFO* step, long long eventId)
{
    const LimitStep& limit = step->m_data.m_limit;
    Handle self = Handle_AllocateInfo(limit.m_nameLength + LIMIT_LIST_HEADER_SIZE,
                                      MILES_EVENT_SUPPORT_FILE, 0x547);
    LimitList* list = static_cast<LimitList*>(Handle_Get(self));
    if (list == NULL)
    {
        ErrorHandler(ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY, eventId, NULL);
        return 0;
    }

    strcpy(list->m_name, limit.m_name);
    list->m_self = self;
    list->m_next = eventSystem->m_limits;
    eventSystem->m_limits = self;

    const char* cursor = limit.m_limits;
    while (*cursor != '\0')
    {
        const char* separator = strstr(cursor, LIMIT_FIELD_SEPARATOR);
        if (separator != NULL && separator[1] != '\0' && (unsigned int)(separator[1] - '0') < 10)
        {
            const char* digits = separator + 1;
            short count = static_cast<short>(strtoi(digits, &digits));
            int length = static_cast<int>(separator - cursor);
            Handle entryHandle = Handle_AllocateInfo(length + LIMIT_ENTRY_HEADER_SIZE,
                                                     MILES_EVENT_SUPPORT_FILE, 0x576);
            LimitEntry* entry = static_cast<LimitEntry*>(Handle_Get(entryHandle));
            if (entry == NULL)
            {
                ErrorHandler(ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY, eventId, NULL);
                return 0;
            }

            entry->m_self = entryHandle;
            entry->m_limit = count;
            entry->m_count = 0;
            entry->m_next = list->m_entries;
            memcpy(entry->m_query, cursor, length);
            entry->m_query[length] = '\0';
            list->m_entries = entryHandle;
            if (*digits != '\0')
            {
                ++digits;
            }
            cursor = digits;
        }
        else
        {
            ErrorHandler(ERROR_TYPE_BAD_STRING_PASSED_TO_SET_LIMITS, eventId, NULL);
            return 0;
        }
    }

    return 1;
}

int LFO_Add(EventSystem* eventSystem, const EVENT_STEP_INFO* step)
{
    const LfoStep& source = step->m_data.m_lfo;
    Handle self = Handle_AllocateInfo(source.m_nameLength + LFO_HEADER_SIZE, MILES_EVENT_SUPPORT_FILE, 0x1cd);
    LfoNode* lfo = static_cast<LfoNode*>(Handle_Get(self));
    if (lfo == NULL)
    {
        return 0;
    }

    memcpy(lfo->m_name, source.m_name, source.m_nameLength + 1);
    lfo->m_self = self;
    lfo->m_next = eventSystem->m_lfos;
    eventSystem->m_lfos = self;

    if (OnlyNum(source.m_center) == 0)
    {
        lfo->m_centerText = Handle_AllocateInfo(source.m_centerLength + 1, MILES_EVENT_SUPPORT_FILE, 0x1de);
        char* text = static_cast<char*>(Handle_Get(lfo->m_centerText));
        if (text != NULL)
        {
            memcpy(text, source.m_center, source.m_centerLength + 1);
        }
    }
    else
    {
        lfo->m_center = atof_with_len(source.m_center, 0);
    }

    if (OnlyNum(source.m_amplitude) == 0)
    {
        lfo->m_amplitudeText = Handle_AllocateInfo(source.m_amplitudeLength + 1, MILES_EVENT_SUPPORT_FILE, 0x1ea);
        char* text = static_cast<char*>(Handle_Get(lfo->m_amplitudeText));
        if (text != NULL)
        {
            memcpy(text, source.m_amplitude, source.m_amplitudeLength + 1);
        }
    }
    else
    {
        lfo->m_amplitude = atof_with_len(source.m_amplitude, 0);
    }

    if (OnlyNum(source.m_phase) == 0)
    {
        lfo->m_phaseText = Handle_AllocateInfo(source.m_phaseLength + 1, MILES_EVENT_SUPPORT_FILE, 0x1f6);
        char* text = static_cast<char*>(Handle_Get(lfo->m_phaseText));
        if (text != NULL)
        {
            memcpy(text, source.m_phase, source.m_phaseLength + 1);
        }
    }
    else
    {
        lfo->m_phase = atof_with_len(source.m_phase, 0);
    }

    lfo->m_inverted = source.m_inverted;
    lfo->m_flip = source.m_flip;
    lfo->m_offset = source.m_offset;
    lfo->m_mode = source.m_mode;

    float value;
    if (lfo->m_mode == LFO_MODE_FLAT)
    {
        float level = LFO_ZERO;
        if (lfo->m_flip != 0)
        {
            level = LFO_ONE;
        }
        value = (level + LFO_ONE) * lfo->m_amplitude + lfo->m_center;
    }
    else
    {
        float level;
        if (lfo->m_mode < LFO_MODE_SQUARE_FIRST)
        {
            level = LFO_ZERO;
            if (lfo->m_inverted != 0)
            {
                level = LFO_ONE;
            }
        }
        else
        {
            if (lfo->m_mode != LFO_MODE_SQUARE_FIRST)
            {
                value = LFO_ZERO * lfo->m_amplitude + lfo->m_center;
                Var_SetInstance(&eventSystem->m_variables, lfo->m_name, &value, 1);
                return 1;
            }

            float phase = LFO_ZERO;
            if (lfo->m_offset != 0)
            {
                phase = LFO_ONE;
            }
            level = phase;
            if (lfo->m_inverted != 0)
            {
                level = LFO_ONE;
                if (phase > LFO_ZERO)
                {
                    level = LFO_ZERO;
                }
            }
        }

        if (lfo->m_flip == 0)
        {
            level = level - LFO_HALF;
        }
        value = level * lfo->m_amplitude + lfo->m_center;
    }

    Var_SetInstance(&eventSystem->m_variables, lfo->m_name, &value, 1);
    return 1;
}

float Ramp_GetCurrentValue(EventSystem* eventSystem, Ramp* ramp, int applyCurve)
{
    if (ramp->m_cacheValid != 0 && applyCurve != 0)
    {
        return ramp->m_cachedValue;
    }

    float value = RAMP_ZERO_VALUE;
    char first = ramp->m_value[0];
    if ((first >= '0' && first <= '9') || first == '-' || first == '.')
    {
        value = atof_with_len(ramp->m_value, 0);
    }
    else
    {
        Var_LookupWithContext(eventSystem, eventSystem->m_variables, ramp->m_value, 1, &value);
    }

    if (ramp->m_target == RAMP_TARGET_LEVEL_A || ramp->m_target == RAMP_TARGET_LEVEL_B)
    {
        if (value > RAMP_SILENCE_DECIBELS)
        {
            value = static_cast<float>(pow(RAMP_DECADE_BASE, value / RAMP_DECIBELS_PER_DECADE));
            value = static_cast<float>(pow(value, RAMP_DECIBEL_EXPONENT));
        }
        else
        {
            value = RAMP_ZERO_VALUE;
        }
    }

    float duration = ramp->m_duration;
    if (duration > RAMP_DURATION_EPSILON)
    {
        float progress = ramp->m_elapsed / duration;
        if (ramp->m_curve == RAMP_CURVE_SQUARE)
        {
            value = (value - ramp->m_startValue) * (progress * progress) + ramp->m_startValue;
        }
        else if (ramp->m_curve == RAMP_CURVE_SMOOTH)
        {
            float centered = progress - RAMP_SMOOTH_CENTER;
            value = (value - ramp->m_startValue) *
                        (centered * centered * centered * RAMP_SMOOTH_SCALE + RAMP_SMOOTH_CENTER) +
                    ramp->m_startValue;
        }
        else
        {
            value = (value - ramp->m_startValue) * progress + ramp->m_startValue;
        }
    }

    if (applyCurve != 0)
    {
        if (ramp->m_target < RAMP_TARGET_RATE)
        {
            value = static_cast<float>(pow(value, SOUND_VOLUME_CURVE_EXPONENT));
        }
        else if (ramp->m_target == RAMP_TARGET_LEVEL_C)
        {
            value = static_cast<float>(pow(RAMP_SEMITONE_BASE, value / RAMP_SEMITONE_DIVISOR));
        }
    }

    value = (fabs(value) - RAMP_VALUE_EPSILON < 0.0) ? RAMP_ZERO_VALUE : value;

    if (ramp->m_elapsed == duration)
    {
        ramp->m_cachedValue = value;
        ramp->m_cacheValid = 1;
    }

    return value;
}

int Ramp_Add(Handle* ramps, EventSystem* eventSystem, const EVENT_STEP_INFO* step)
{
    const RampStep& source = step->m_data.m_ramp;
    Ramp* ramp = static_cast<Ramp*>(Handle_Get(*ramps));
    while (ramp != NULL && AIL_stricmp(ramp->m_name, source.m_name) != 0)
    {
        ramp = static_cast<Ramp*>(Handle_Get(ramp->m_next));
    }

    if (ramp == NULL)
    {
        Handle self = Handle_AllocateInfo(RAMP_SIZE, MILES_EVENT_SUPPORT_FILE, 0x121);
        ramp = static_cast<Ramp*>(Handle_Get(self));
        if (ramp == NULL)
        {
            return 0;
        }

        if (source.m_nameLength < RAMP_NAME_SIZE && source.m_labelsLength < RAMP_LABELS_SIZE &&
            source.m_valueLength < RAMP_VALUE_SIZE)
        {
            ramp->m_self = self;
            memcpy(ramp->m_name, source.m_name, source.m_nameLength + 1);
            memcpy(ramp->m_labels, source.m_labels, source.m_labelsLength + 1);
            memcpy(ramp->m_value, source.m_value, source.m_valueLength + 1);

            float startValue;
            char first = *source.m_value;
            if ((first >= '0' && first <= '9') || first == '-' || first == '.')
            {
                startValue = atof_with_len(source.m_value, 0);
            }
            else if (source.m_target < RAMP_TARGET_RATE)
            {
                startValue = RAMP_UNIT_VALUE;
            }
            else if (source.m_target == RAMP_TARGET_RATE)
            {
                startValue = RAMP_TARGET_RATE_DEFAULT;
            }
            else
            {
                startValue = RAMP_ZERO_VALUE;
            }

            if (source.m_target == RAMP_TARGET_LEVEL_A || source.m_target == RAMP_TARGET_LEVEL_B)
            {
                if (startValue > RAMP_SILENCE_DECIBELS)
                {
                    startValue = pow(RAMP_DECADE_BASE, startValue / RAMP_DECIBELS_PER_DECADE);
                    startValue = pow(startValue, RAMP_DECIBEL_EXPONENT);
                }
                else
                {
                    startValue = RAMP_ZERO_VALUE;
                }
            }

            ramp->m_startValue = startValue;
            ramp->m_next = *ramps;
            *ramps = self;
            ramp->m_target = source.m_target;
        }
        else
        {
            Handle_Free(self);
            return 0;
        }
    }
    else
    {
        ramp->m_startValue = Ramp_GetCurrentValue(eventSystem, ramp, 0);
    }

    if (source.m_valueLength < RAMP_VALUE_SIZE)
    {
        memcpy(ramp->m_value, source.m_value, source.m_valueLength + 1);
    }

    ramp->m_elapsed = RAMP_ZERO_VALUE;
    ramp->m_cacheValid = 0;
    ramp->m_duration = source.m_duration;
    ramp->m_curve = source.m_curve;
    return 1;
}

void Ramp_ApplyAll(EventSystem* eventSystem, SoundInstance* soundInstance, float elapsed)
{
    SoundInstanceData* instance = reinterpret_cast<SoundInstanceData*>(soundInstance);
    const char* labels = static_cast<const char*>(Handle_Get(instance->m_labels));
    SoundSourceData* source = static_cast<SoundSourceData*>(Handle_Get(instance->m_source));

    float levelA;
    if (source != NULL)
    {
        levelA = pow(instance->m_volume, SOUND_VOLUME_CURVE_EXPONENT) * source->m_volume;
    }
    else
    {
        levelA = pow(instance->m_volume, SOUND_VOLUME_CURVE_EXPONENT) * NO_SOURCE_VOLUME;
    }

    float levelB = RAMP_UNIT_VALUE;
    float rate = RAMP_RATE_INITIAL;
    float levelC = RAMP_UNIT_VALUE;

    Ramp* ramp = static_cast<Ramp*>(Handle_Get(eventSystem->m_ramps));
    while (ramp != NULL)
    {
        if (DoLabelsMatchQuery(labels, ramp->m_labels))
        {
            float value = Ramp_GetCurrentValue(eventSystem, ramp, 1);
            if (ramp->m_target == RAMP_TARGET_LEVEL_A)
            {
                levelA = levelA * value;
            }
            else if (ramp->m_target == RAMP_TARGET_LEVEL_B)
            {
                levelB = levelB * value;
            }
            else if (ramp->m_target == RAMP_TARGET_RATE)
            {
                rate = value;
            }
            else if (ramp->m_target == RAMP_TARGET_LEVEL_C)
            {
                levelC = levelC * value;
            }
        }

        ramp = static_cast<Ramp*>(Handle_Get(ramp->m_next));
    }

    ramp = static_cast<Ramp*>(Handle_Get(instance->m_ramps));
    while (ramp != NULL)
    {
        ramp->m_elapsed = ramp->m_elapsed + elapsed;
        if (ramp->m_duration <= ramp->m_elapsed)
        {
            ramp->m_elapsed = ramp->m_duration;
        }

        float value = Ramp_GetCurrentValue(eventSystem, ramp, 1);
        Audition_RampState(ramp->m_name, instance->m_eventId, ramp->m_target);
        if (ramp->m_target == RAMP_TARGET_LEVEL_A)
        {
            levelA = levelA * value;
        }
        else if (ramp->m_target == RAMP_TARGET_LEVEL_B)
        {
            levelB = levelB * value;
        }
        else if (ramp->m_target == RAMP_TARGET_RATE)
        {
            rate = value;
        }
        else if (ramp->m_target == RAMP_TARGET_LEVEL_C)
        {
            levelC = levelC * value;
        }

        ramp = static_cast<Ramp*>(Handle_Get(ramp->m_next));
    }

    _SAMPLE* sample = instance->m_sample;
    if (fabs(sample->levelC - levelC) > SAMPLE_CHANGE_EPSILON ||
        fabs(sample->rate - rate) > SAMPLE_CHANGE_EPSILON)
    {
        sample->cacheState = SAMPLE_UNCHANGED;
    }

    sample->levelA = levelA;
    sample->levelB = levelB;
    sample->rate = rate;
    sample->levelC = levelC;
}

void Var_SetInstance(Handle* variables, const char* name, void* value, int isFloat)
{
    EventSystemVariable* variable = static_cast<EventSystemVariable*>(Handle_Get(*variables));
    while (variable != NULL && AIL_stricmp(variable->m_name, name) != 0)
    {
        variable = static_cast<EventSystemVariable*>(Handle_Get(variable->m_parent));
    }

    if (variable == NULL)
    {
        Handle self = Handle_AllocateInfo(strlen(name) + VARIABLE_HEADER_SIZE, MILES_EVENT_SUPPORT_FILE, 0x477);
        variable = static_cast<EventSystemVariable*>(Handle_Get(self));
        if (variable != NULL)
        {
            variable->m_parent = *variables;
            *variables = self;
            if (isFloat == 0)
            {
                variable->m_value.m_intValue = *static_cast<int*>(value);
            }
            else
            {
                variable->m_value.m_floatValue = *static_cast<float*>(value);
            }
            variable->m_isFloat = isFloat;
            variable->m_self = self;
            strcpy(variable->m_name, name);
        }
        return;
    }

    variable->m_isFloat = isFloat;
    if (isFloat == 0)
    {
        variable->m_value.m_intValue = *static_cast<int*>(value);
    }
    else
    {
        variable->m_value.m_floatValue = *static_cast<float*>(value);
    }

    VariableCurve* curve = static_cast<VariableCurve*>(Handle_Get(variable->m_curve));
    if (curve == NULL)
    {
        return;
    }

    float position = *static_cast<float*>(value);
    if (isFloat == 0)
    {
        position = static_cast<float>(*static_cast<int*>(value));
    }

    float start = curve->m_start;
    float end = curve->m_end;
    if (position < start)
    {
        if (curve->m_lowMode == CURVE_CLAMP)
        {
            position = start;
        }
        else if (curve->m_lowMode == CURVE_WRAP)
        {
            position = fmodf(position - start, end - start) + end;
        }
        else if (curve->m_lowMode == CURVE_MIRROR)
        {
            int wraps = static_cast<int>((position - start) / (end - start));
            float remainder = fmodf(position - start, end - start);
            if (((wraps - 1) & 1) != 0)
            {
                remainder = (end - start) - (start - remainder);
            }
            position = remainder + end;
        }
    }

    if (position > end)
    {
        if (curve->m_highMode == CURVE_CLAMP)
        {
            position = end;
        }
        else if (curve->m_highMode == CURVE_WRAP)
        {
            position = fmodf(position - start, end - start) + start;
        }
        else if (curve->m_highMode == CURVE_MIRROR)
        {
            float span = end - start;
            float offset = position - start;
            int wraps = static_cast<int>(offset / span);
            float remainder = fmodf(offset, span);
            if ((wraps & 1) != 0)
            {
                remainder = (curve->m_end - curve->m_start) - remainder;
            }
            position = remainder + start;
        }
    }

    variable->m_value.m_floatValue = evaluate_graph(position, curve->m_points, curve->m_pointCount);
}
