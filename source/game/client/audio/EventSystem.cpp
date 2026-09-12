#include "game/client/audio/EventSystem.hpp"

#include "game/client/audio/SoundInstance.hpp"
#include "game/core/Handle.hpp"
#include "game/util/LegacyUtilityFunctions.hpp"

struct SoundInstanceHandleStorage
{
    unsigned char m_reserved[0x4c];
    EventSystem::Handle m_handle;
};

struct EventSystemVariable
{
    EventSystem::Handle m_parent;
    unsigned char m_reserved0[8];
    union
    {
        int m_intValue;
        float m_floatValue;
    } m_value;
    int m_isFloat;
    unsigned char m_reserved1[8];
    char m_name[1];
};

enum ErrorType
{
    ERROR_TYPE_OUT_OF_MEMORY = 0,
    ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY = 1,
    ERROR_TYPE_BAD_STRING_PASSED_TO_SET_LIMITS = 3,
    ERROR_TYPE_COMMAND_BUFFER_FULL = 4,
    ERROR_TYPE_MISSING_VARIABLE = 5
};

extern int AIL_stricmp(const char* first, const char* second);
extern int AIL_find_marker_in_list(int markerList, const char* markerName, int* markerType);
extern void* Handle_Get(Handle handle);
extern void ErrorHandler__F9ErrorTypeULPCc(ErrorType type, unsigned long long context,
                                            const char* text);
extern int strtoi__FPCcPPCc(const char* value, const char** end);
extern int Var_LookupWithContext__FP11EventSystem6HandlePCciPv(EventSystem* eventSystem,
                                                                EventSystem::Handle* handle,
                                                                const char* name,
                                                                int flags,
                                                                void* value);

static EventSystemVariable* lookupVariable(const EventSystem::Handle& handle)
{
    Handle lookup;
    lookup.index = handle.index;
    lookup.generation = handle.generation;
    return static_cast<EventSystemVariable*>(Handle_Get(lookup));
}

int strtoi__FPCcPPCc(const char* value, const char** end)
{
    int result = 0;

    while (*value != '\0')
    {
        const int digit = *value;
        if ((unsigned int)(digit - '0') > 9)
        {
            break;
        }
        result = result * 10 + digit - '0';
        ++value;
    }

    if (end != 0)
    {
        *end = value;
    }

    return result;
}

int Var_LookupWithContext__FP11EventSystem6HandlePCciPv(EventSystem* eventSystem,
                                                         EventSystem::Handle* handle,
                                                         const char* name,
                                                         int flags,
                                                         void* value)
{
    EventSystem::Handle context = *handle;

    for (;;)
    {
        EventSystemVariable* variable = lookupVariable(context);

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

            variable = lookupVariable(variable->m_parent);
        }

        if (context.index == eventSystem->m_handle.index &&
            context.generation == eventSystem->m_handle.generation)
        {
            ErrorHandler__F9ErrorTypeULPCc(ERROR_TYPE_MISSING_VARIABLE, 0, name);
            return 0;
        }

        context = eventSystem->m_handle;
    }
}

int FindMarker__FP11EventSystemP13SoundInstanceiPCcPiT5(EventSystem* eventSystem,
                                                          SoundInstance* soundInstance,
                                                          int markerList,
                                                          char* markerName,
                                                          int* markerIndex,
                                                          int* markerType)
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

        EventSystem::Handle handle;
        if (soundInstance != 0)
        {
            handle = reinterpret_cast<const SoundInstanceHandleStorage*>(soundInstance)->m_handle;
        }
        else
        {
            handle = eventSystem->m_handle;
        }

        return Var_LookupWithContext__FP11EventSystem6HandlePCciPv(eventSystem, &handle,
                                                                    markerName, 0,
                                                                    markerIndex);
    }

    ++markerName;
    if (OnlyNum__FPCc(markerName) != 0)
    {
        *markerIndex = strtoi__FPCcPPCc(markerName, 0);
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
