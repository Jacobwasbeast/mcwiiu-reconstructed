#include "game/client/audio/EventSystem.hpp"

#include "game/client/audio/SoundInstance.hpp"
#include "game/util/LegacyUtilityFunctions.hpp"

struct SoundInstanceHandleStorage
{
    unsigned char m_reserved[0x4c];
    EventSystem::Handle m_handle;
};

extern int AIL_stricmp(const char* first, const char* second);
extern int AIL_find_marker_in_list(int markerList, const char* markerName, int* markerType);
extern int strtoi__FPCcPPCc(const char* value, const char** end);
extern int Var_LookupWithContext__FP11EventSystem6HandlePCciPv(EventSystem* eventSystem,
                                                                EventSystem::Handle handle,
                                                                const char* name,
                                                                int flags,
                                                                void* value);

int FindMarker__FP11EventSystemP13SoundInstanceiPCcPiT5(EventSystem* eventSystem,
                                                          SoundInstance* soundInstance,
                                                          int markerList,
                                                          const char* markerName,
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

        return Var_LookupWithContext__FP11EventSystem6HandlePCciPv(eventSystem, handle,
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
