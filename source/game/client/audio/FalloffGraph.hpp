#ifndef GAME_CLIENT_AUDIO_FALLOFFGRAPH_HPP
#define GAME_CLIENT_AUDIO_FALLOFFGRAPH_HPP

#include "game/core/Handle.hpp"

class EventSystem;
class SoundInstance;
class SoundSource;
struct QueuedEvent;
struct EVENT_STEP_INFO;
struct SoundLimitEntry;
struct StateVar;

const int QUEUE_BUFFER_COUNT = 8;
const int QUEUE_BUFFER_SIZE = 0x200;

struct QueueBuffer
{
    unsigned char bytes[QUEUE_BUFFER_SIZE];
};

struct EventQueue
{
    Handle storage;
    unsigned int capacity;
    volatile unsigned int tokens;
    QueueBuffer buffers[QUEUE_BUFFER_COUNT];
};

extern EventQueue g_eventQueue;

struct _MSSSTRINGC
{
    const char* text;
    int length;
};

struct _DIG_DRIVER
{
    unsigned char reserved0[0xbf4];
    int* sampleStatus;
};

struct _LISTENER
{
    unsigned char reserved0[0x28];
    float x;
    float y;
    float z;
};

struct _SAMPLE;

typedef void (*AIL_SAMPLE_CALLBACK)(_SAMPLE* sample);

struct _SAMPLE
{
    unsigned char reserved5[4];
    _DIG_DRIVER* driver;
    int index;
    unsigned char reserved0[0xd4 - 0xc];
    float volume;
    unsigned char reserved4[0x130 - 0xd8];
    int starved;
    unsigned char reserved6[0x1c6 - 0x134];
    bool outputToDRC;
    unsigned char reserved7[0x1dc - 0x1c7];
    AIL_SAMPLE_CALLBACK endOfBuffer;
    AIL_SAMPLE_CALLBACK endOfSample;
    unsigned char reserved1[0x208 - 0x1e4];
    Handle falloffHandle;
    unsigned char reserved2[0x804 - 0x210];
    int is3D;
    float x;
    float y;
    float z;
    unsigned char reserved3[0x850 - 0x814];
    int falloffMode;
};

struct FalloffGraphPoint
{
    float values[8];
};

struct FalloffGraph
{
    int reserved;
    int mode;
    int volumePointCount;
    int exclusionPointCount;
    int lowpassPointCount;
    FalloffGraphPoint points[1];
};

void FalloffGraph_Set(_SAMPLE* sample, int flags, FalloffGraph* graph);

int StateVar_KeyLB(StateVar* vars, int count, void* key);
int StateVar_Lookup(EventSystem* eventSystem, void* key);
void StateVar_Set(EventSystem* eventSystem, void* key, int value);
Handle Sound_Find(EventSystem* eventSystem, const char* name);

int DoLabelsMatchQuery(const char* labels, const char* query);
SoundLimitEntry* IsLabelLimited(EventSystem* eventSystem, _MSSSTRINGC label);
int Blend_Exists(EventSystem* eventSystem, const char* name);
int Blend_Add(EventSystem* eventSystem, const EVENT_STEP_INFO* step);
void Blend_Apply(SoundInstance* instance, float* volume, float* pitch);
void DestroySoundInstance(EventSystem* eventSystem, SoundInstance* instance);
int EvictSound(EventSystem* eventSystem, unsigned long long eventId, unsigned char policy,
               int priority, SoundLimitEntry* limit, SoundSource* source, const char* name);

void Queue_SetMem(int capacity, Handle storage);
unsigned long long Queue_EnumCreate();
QueuedEvent* Queue_EnumNext(unsigned long long* cursor, QueuedEvent* event);
void Queue_EnumClose(unsigned long long start, unsigned long long end);
QueuedEvent* Queue_Add(int size);
void Queue_Commit(QueuedEvent* event);
extern "C" unsigned long long MilesEnqueueEventByNameInternal(const char* pEventName);

extern "C" unsigned int AIL_enqueue_event_start();
extern "C" unsigned int AIL_enqueue_event_buffer(unsigned int* puiQueue, void* pData, int iDataSize, int iFlags);
extern "C" void AIL_enqueue_event_end_named(unsigned int uiQueue, const char* pEventName);
unsigned long long AIL_enqueue_event_end(int queue, void* event, int named, int ownsEvent);
unsigned long long AIL_enqueue_event_from_init_block(const char* name, void* block, int size,
                                                     unsigned long long selection);

#endif
