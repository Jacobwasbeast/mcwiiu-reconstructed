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

struct _SAMPLE;
struct _DIG_DRIVER;
struct FLT_INSTANCE;

const int SAMPLE_BUFFER_COUNT = 8;
const int SAMPLE_STAGE_COUNT = 10;
const int SAMPLE_FILTER_STAGE_COUNT = 8;
const int SAMPLE_FILTER_HANDLE_COUNT = 8;
const int SAMPLE_STAGE_DECODER = 0;
const int SAMPLE_STAGE_MERGE = 9;
const int SAMPLE_STAGE_ALL = 11;
const int DRIVER_BUS_COUNT = 4;
const int DRIVER_OUTPUT_COUNT = 8;

typedef int (*FilterPropertyFunc)(unsigned int property, void* before, const void* newValue,
                                  void* after);
typedef int (*FilterStartupFunc)();
typedef const char* (*FilterErrorFunc)();
typedef void (*FilterShutdownFunc)();
typedef void* (*MssAllocFunc)(unsigned int size, unsigned int user, const char* file, int line);
typedef void (*MssFreeFunc)(void* block);
typedef void* (*FilterOpenDriverFunc)(MssAllocFunc allocator, MssFreeFunc deallocator,
                                      unsigned int reserved, _DIG_DRIVER* driver, void* state);
typedef void (*FilterCloseDriverFunc)(void* driverState);
typedef void (*FilterProcessFunc)();
typedef void* (*FilterOpenSampleFunc)(void* driverState, _SAMPLE* sample, void* state);
typedef void (*FilterCloseSampleFunc)(void* handle);
typedef int (*FilterSamplePropertyFunc)(void* handle, unsigned int property, void* before,
                                        const void* newValue, void* after);
typedef int (*AsiStreamCallback)(unsigned int user, void* destination, int count, int offset);
typedef void* (*AsiStreamOpenFunc)(MssAllocFunc allocator, MssFreeFunc deallocator,
                                   unsigned int user, AsiStreamCallback callback,
                                   unsigned int totalSize);
typedef void (*AsiStreamCloseFunc)(void* stream);
typedef int (*AsiStreamPropertyFunc)(void* stream, unsigned int property, void* before,
                                     const void* newValue, void* after);
typedef void (*MixerMergeFunc)();

struct FLT_INSTANCE
{
    unsigned int flags;
    unsigned int driverStateSize;
    unsigned int sampleStateSize;
    FilterPropertyFunc property;
    FilterStartupFunc startup;
    FilterErrorFunc error;
    FilterShutdownFunc shutdown;
    FilterOpenDriverFunc openDriver;
    FilterCloseDriverFunc closeDriver;
    FilterProcessFunc premixProcess;
    FilterProcessFunc postmixProcess;
    FilterOpenSampleFunc openSample;
    FilterCloseSampleFunc closeSample;
    void* sampleProcess;
    FilterSamplePropertyFunc sampleProperty;
    unsigned char reserved0[0x54 - 0x3c];
    _DIG_DRIVER* driver;
    unsigned int provider;
    void* driverState;
    FLT_INSTANCE* next;
};

struct FilterStage
{
    int active;
    unsigned int provider;
    FLT_INSTANCE* instance;
    void* handles[SAMPLE_FILTER_HANDLE_COUNT];
};

struct DecoderStage
{
    int active;
    unsigned int provider;
    AsiStreamOpenFunc open;
    void* process;
    void* seek;
    AsiStreamCloseFunc close;
    AsiStreamPropertyFunc property;
    unsigned int inputBitRate;
    unsigned int inputSampleRate;
    unsigned int inputSampleWidth;
    unsigned int inputChannels;
    unsigned int outputBitRate;
    unsigned int outputSampleRate;
    unsigned int outputSampleWidth;
    unsigned int outputChannels;
    unsigned int outputChannelMask;
    unsigned int outputReservoir;
    unsigned int position;
    unsigned int percentDone;
    unsigned int minimumInputBlockSize;
    unsigned int rawSourceSampleRate;
    unsigned int rawSourceSampleWidth;
    unsigned int rawSourceChannels;
    unsigned int requestedSampleRate;
    unsigned int requestedSampleWidth;
    unsigned int requestedChannels;
    unsigned int streamSeekPosition;
    unsigned int dataOffset;
    unsigned int dataLength;
    unsigned int exactSeek;
    unsigned int exactGetPosition;
    unsigned int seekTableLookup;
    unsigned int setLoopingSamples;
    unsigned int clearLoopMeta;
    void* stream;
};

struct MergeStage
{
    int active;
    unsigned int provider;
    MixerMergeFunc merge;
};

union SampleStage
{
    FilterStage filter;
    DecoderStage decoder;
    MergeStage merge;
};

struct SampleBuffer
{
    unsigned char* data;
    int size;
    int position;
    int done;
    int reset;
    int reserved0;
};

struct ChannelState
{
    unsigned char reserved0[0x10];
    int limitA;
    unsigned char reserved1[0x20 - 0x14];
    int limitB;
    unsigned char reserved2[4];
    float busLevel;
    float lowPass;
};

struct DriverOutput
{
    int mode;
    int reserved0[3];
};

struct DriverBus
{
    float sendLevel;
    unsigned char reserved0[0x94 - 4];
    int update;
    int updateValue;
    unsigned char reserved1[4];
    float returnLevel;
    unsigned char reserved2[0xb8 - 0xa4];
};

struct _DIG_DRIVER
{
    unsigned char reserved2[8];
    int sharedState;
    unsigned char reserved3[0x14 - 0xc];
    int outputRate;
    unsigned int flags;
    unsigned char reserved4[0xbec - 0x1c];
    int speakerConfig;
    unsigned char reserved0[4];
    int* sampleStatus;
    unsigned char reserved1[0xc1c - 0xbf8];
    _SAMPLE* busSamples[DRIVER_BUS_COUNT];
    unsigned char reserved5[0xc68 - 0xc2c];
    DriverOutput outputs[DRIVER_OUTPUT_COUNT];
    unsigned char reserved6[0xcf0 - 0xce8];
    int outputCount;
    unsigned char reserved7[0xd68 - 0xcf4];
    FLT_INSTANCE* matrixFilter;
    unsigned char reserved8[4];
    DriverBus buses[DRIVER_BUS_COUNT];
};

struct _LISTENER
{
    unsigned char reserved0[0x28];
    float x;
    float y;
    float z;
};

typedef void (*AIL_SAMPLE_CALLBACK)(_SAMPLE* sample);

struct _SAMPLE
{
    unsigned char reserved5[4];
    _DIG_DRIVER* driver;
    int index;
    SampleBuffer buffers[SAMPLE_BUFFER_COUNT];
    int bufferReset;
    unsigned char reserved0[0xd4 - 0xd0];
    float volume;
    unsigned char reserved4[0xe4 - 0xd8];
    unsigned int channelBufferCapacity;
    unsigned int channelBufferSize;
    unsigned char* channelBuffer;
    int* channelPositionA;
    int* channelPositionB;
    int* channelHeader;
    ChannelState* channelStates;
    float** levels;
    float** levelsB;
    float** levelsC;
    float** levelsD;
    float* speakerLevelsA;
    float* speakerScales;
    signed char* channelMap;
    unsigned char reserved11[4];
    int userLevels;
    int bufferCount;
    int lastBuffer;
    int currentBuffer;
    int starved;
    int bufferRewind;
    int loopState;
    unsigned char reserved6[0x140 - 0x13c];
    unsigned int loopEnd;
    unsigned char reserved12[0x150 - 0x144];
    unsigned int channelFlags;
    int channelCount;
    int channelMask;
    int playbackRate;
    unsigned char reserved13[0x164 - 0x160];
    float volumeCurve;
    float pan;
    float panLevelA;
    float panLevelB;
    float reverbSecond;
    float reverbFirst;
    unsigned char reserved8[0x180 - 0x17c];
    float levelA;
    float levelB;
    float rate;
    float levelC;
    int cacheState;
    int reverbBus;
    unsigned int compressorSources;
    unsigned int compressorLink;
    unsigned char reserved14[0x1a4 - 0x1a0];
    int reverbDirty;
    unsigned char reserved9[0x1c6 - 0x1a8];
    bool outputToDRC;
    unsigned char reserved7a[0x1c8 - 0x1c7];
    float obstruction;
    float occlusion;
    float exclusion;
    int loopMode;
    unsigned char reserved7b[0x1dc - 0x1d8];
    AIL_SAMPLE_CALLBACK endOfBuffer;
    AIL_SAMPLE_CALLBACK endOfSample;
    unsigned char reserved1[0x208 - 0x1e4];
    Handle falloffHandle;
    unsigned char reserved2[0x22c - 0x210];
    int resetA;
    int resetB;
    unsigned char reserved15[0x270 - 0x234];
    int endCounter;
    unsigned char reserved16[0x288 - 0x274];
    SampleStage stages[SAMPLE_STAGE_COUNT];
    int filterCount;
    int is3D;
    float x;
    float y;
    float z;
    float forwardX;
    float forwardY;
    float forwardZ;
    float upX;
    float upY;
    float upZ;
    float velocityX;
    float velocityY;
    float velocityZ;
    unsigned char reserved3[0x840 - 0x838];
    float coneInner;
    float coneOuter;
    float coneOuterVolume;
    int coneEnabled;
    int falloffMode;
    unsigned char reserved10[0x864 - 0x854];
    float presetValue;
    unsigned char reserved17[0xb7c - 0x868];
    float mixLevel0;
    float mixLevel1;
    float mixLevel2;
    float mixLevel3;
    float mixInput0;
    float mixInput1;
    float mixInput2;
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
