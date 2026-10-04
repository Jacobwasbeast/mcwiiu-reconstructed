#ifndef GAME_CLIENT_AUDIO_EVENTSYSTEM_HPP
#define GAME_CLIENT_AUDIO_EVENTSYSTEM_HPP

#include "game/core/Handle.hpp"
#include "game/core/PODHandleArray.hpp"

struct _SAMPLE;
struct _LISTENER;

struct StateVar
{
    void* key;
    int value;
};

class EventSystem
{
public:
    unsigned char m_reserved0[8];
    PODHandleArray<Handle> m_sounds;
    PODHandleArray<Handle> m_instances;
    PODHandleArray<StateVar> m_stateVars;
    Handle m_ramps;
    Handle m_variables;
    Handle m_blends;
    Handle m_persists;
    Handle m_limits;
    Handle m_lfos;
    unsigned char m_reserved1[0x90 - 0x80];
    _LISTENER* m_listener;
    unsigned char m_reserved2[0xa0 - 0x94];
    int m_soundBytes;
};

class SoundInstance;

struct SoundInstanceData
{
    unsigned char m_reserved0[0x10];
    unsigned long long m_eventId;
    _SAMPLE* m_sample;
    unsigned char m_reserved1[4];
    EventSystem* m_eventSystem;
    Handle m_self;
    Handle m_source;
    Handle m_name;
    Handle m_labels;
    Handle m_ramps;
    Handle m_context;
    Handle m_blend;
    unsigned char m_reserved3[0x78 - 0x5c];
    int m_state;
    int m_startTime;
    unsigned char m_reserved4[0x9c - 0x80];
    float m_volume;
    unsigned char m_reserved5[0xaa - 0xa0];
    unsigned char m_priority;
    unsigned char m_flags;
    signed char m_blendIndex;
};

const int MAX_BLEND_POINTS = 10;

struct BlendStep
{
    const char* m_name;
    int m_nameLength;
    float m_rangeStart[MAX_BLEND_POINTS];
    float m_fadeInEnd[MAX_BLEND_POINTS];
    float m_fadeOutStart[MAX_BLEND_POINTS];
    float m_rangeEnd[MAX_BLEND_POINTS];
    float m_pitchAtStart[MAX_BLEND_POINTS];
    float m_pitchAtEnd[MAX_BLEND_POINTS];
    unsigned char m_pointCount;
};

struct RampStep
{
    const char* m_name;
    int m_nameLength;
    const char* m_labels;
    int m_labelsLength;
    const char* m_value;
    int m_valueLength;
    float m_duration;
    unsigned char m_target;
    unsigned char m_reserved0;
    unsigned char m_curve;
};

struct LfoStep
{
    const char* m_name;
    int m_nameLength;
    const char* m_center;
    int m_centerLength;
    const char* m_amplitude;
    int m_amplitudeLength;
    const char* m_phase;
    int m_phaseLength;
    int m_inverted;
    int m_flip;
    int m_mode;
    int m_offset;
};

struct PersistStep
{
    const char* m_name;
    int m_nameLength;
    const char* m_preset;
    int m_presetLength;
    const char* m_labels;
    int m_labelsLength;
    unsigned char m_active;
};

struct LimitStep
{
    const char* m_limits;
    int m_limitsLength;
    const char* m_name;
    int m_nameLength;
};

struct EVENT_STEP_INFO
{
    unsigned int m_reserved0;
    union
    {
        BlendStep m_blend;
        RampStep m_ramp;
        LfoStep m_lfo;
        PersistStep m_persist;
        LimitStep m_limit;
    } m_data;
};

struct Ramp;

int Var_LookupWithContext(EventSystem* eventSystem, Handle handle, const char* name, int flags,
                          void* value);

int FindMarker(EventSystem* eventSystem, SoundInstance* soundInstance, int markerList, const char* markerName,
               int* markerIndex, int* markerType);

Handle Sound_Load(EventSystem* eventSystem, unsigned long long eventId, const char* name);
void Sound_Free(EventSystem* eventSystem, Handle sound);
int Persist_Create(EventSystem* eventSystem, unsigned long long eventId, const EVENT_STEP_INFO* step);
void Persist_Free(EventSystem* eventSystem, unsigned long long eventId, const char* name);
void Persist_Apply(EventSystem* eventSystem, SoundInstance* soundInstance, _SAMPLE* sample,
                   const char* labels, int unusedArgument, int onlyActive);
int Limit_Add(EventSystem* eventSystem, const EVENT_STEP_INFO* step, long long eventId);
int LFO_Add(EventSystem* eventSystem, const EVENT_STEP_INFO* step);
int Ramp_Add(Handle* ramps, EventSystem* eventSystem, const EVENT_STEP_INFO* step);
float Ramp_GetCurrentValue(EventSystem* eventSystem, Ramp* ramp, int applyCurve);
void Ramp_ApplyAll(EventSystem* eventSystem, SoundInstance* soundInstance, float elapsed);
void Var_SetInstance(Handle* variables, const char* name, void* value, int isFloat);

#endif
