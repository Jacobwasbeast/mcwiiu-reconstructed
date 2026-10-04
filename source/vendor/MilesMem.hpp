#ifndef SOURCE_VENDOR_MILES_MEM_HPP
#define SOURCE_VENDOR_MILES_MEM_HPP

#include "game/client/audio/SoundBank.hpp"

extern "C" void AIL_debug_log(const char* pFormat, ...);
extern "C" void AIL_set_error(const char* pError);
extern "C" void* AIL_mem_alloc_lock_info(int iSize, const char* pOwnerName, int iOwnerLine);
extern "C" void AIL_mem_free_lock(void* pBlock);
extern "C" void outresuhex(unsigned int uResult);
extern "C" void outresint(int iResult);
extern "C" int AIL_ms_count();

struct _SAMPLE;
struct _DIG_DRIVER;
struct _LISTENER;
struct FLT_INSTANCE;
struct _MILESBANKSOUNDINFO;

struct GraphPoint
{
    float x;
    float y;
    float inTangentX;
    float inTangentY;
    float outTangentX;
    float outTangentY;
    int inMode;
    int outMode;
};

const int SOUND_BANK_MAGIC = 0x42414e4b;
const int AUDIO_BANK_MAGIC = 0x41554442;

struct SoundBankHeader
{
    int magic;
    int reserved0[2];
    int pathLength;
    const char* path;
    int reserved1[2];
    SoundBank::AssetEntry* presets;
    SoundBank::AssetEntry* sounds;
    int reserved2[3];
    int presetCount;
    int soundCount;
    char name[1];
};

typedef void (*PresetEvalFunc)(void* instance, const char* name, int index, void* value, int flags);

extern "C" float evaluate_graph(float position, const GraphPoint* points, unsigned int count);
extern "C" int AIL_resolve_raw_sound_preset(void* preset, void* instance, PresetEvalFunc evaluate);
extern "C" int AIL_apply_raw_sound_preset(_SAMPLE* sample, void* preset);
extern "C" int AIL_unapply_raw_sound_preset(_SAMPLE* sample, void* preset);
extern "C" void* AIL_find_sound_preset(SoundBank* bank, const char* name);
extern "C" int AIL_sound_asset_info(SoundBank* bank, const char* name, char* path, _MILESBANKSOUNDINFO* info);
extern "C" const char* AIL_get_soundbank_name(SoundBank* bank);


extern "C" void AIL_set_sample_is_3D(_SAMPLE* pSample, int iEnabled);
extern "C" void AIL_set_sample_3D_position(_SAMPLE* pSample, float fX, float fY, float fZ);
extern "C" void AIL_set_sample_3D_distances(_SAMPLE* pSample, float fMaximum, float fMinimum,
                                            int iAutomaticWetAttenuation);
extern "C" void AIL_set_sample_playback_rate_factor(_SAMPLE* pSample, float fFactor);
extern "C" int AIL_find_filter(const char* pName, int* piProvider);

extern "C" void AIL_API_set_sample_speaker_scale_factors(_SAMPLE* pSample, const int* pSpeakers,
                                                         const float* pScales, int iCount);
extern "C" void AIL_API_sample_speaker_scale_factors(_SAMPLE* pSample, int* pSpeakers,
                                                     float* pScales, int iCount);
extern "C" void AIL_API_sample_channel_levels(_SAMPLE* pSample, int* pSources, int* pDestinations,
                                              float* pLevels, int iCount);
extern "C" void AIL_API_set_sample_channel_levels(_SAMPLE* pSample, const int* pSources,
                                                  const int* pDestinations, const float* pLevels,
                                                  int iCount);
extern "C" void AIL_API_set_sample_reverb_levels(_SAMPLE* pSample, float fFirst, float fSecond);
extern "C" void AIL_API_set_sample_low_pass_cut_off(_SAMPLE* pSample, float fCutOff, int iChannel);
extern "C" void AIL_API_set_sample_51_volume_pan(_SAMPLE* pSample, float fVolume, float fPan,
                                                 float fFade, float fCenter, float fLowFrequency);
extern "C" int AIL_API_sample_processor(_SAMPLE* pSample, int iStage);
extern "C" int AIL_API_set_sample_processor(_SAMPLE* pSample, int iStage, int iProvider);
extern "C" int AIL_API_install_bus_compressor(_DIG_DRIVER* pDriver, int iBus, int iStage,
                                              int iInputBus);
extern "C" int AIL_API_open_filter(unsigned int uProvider, _DIG_DRIVER* pDriver);
extern "C" int AIL_API_sample_channel_count(_SAMPLE* pSample, unsigned int* puMask);
extern "C" int AIL_API_sample_playback_rate(_SAMPLE* pSample);
extern "C" void AIL_API_set_sample_playback_rate(_SAMPLE* pSample, int iRate);
extern "C" void AIL_API_set_sample_3D_cone(_SAMPLE* pSample, float fInnerAngle, float fOuterAngle,
                                           float fOuterVolume);
extern "C" void AIL_API_set_sample_3D_orientation(_SAMPLE* pSample, float fForwardX,
                                                  float fForwardY, float fForwardZ, float fUpX,
                                                  float fUpY, float fUpZ);
extern "C" void AIL_API_set_sample_3D_velocity(_SAMPLE* pSample, float fDirectionX,
                                               float fDirectionY, float fDirectionZ,
                                               float fMagnitude);
extern "C" void AIL_API_set_sample_3D_velocity_vector(_SAMPLE* pSample, float fX, float fY,
                                                      float fZ);
extern "C" void AIL_API_set_sample_occlusion(_SAMPLE* pSample, float fOcclusion);
extern "C" void AIL_API_set_sample_exclusion(_SAMPLE* pSample, float fExclusion);
extern "C" void AIL_API_set_sample_obstruction(_SAMPLE* pSample, float fObstruction);

extern "C" FLT_INSTANCE* FLT_find_provider_instance(unsigned int uProvider, _DIG_DRIVER* pDriver);
extern "C" int SS_set_sample_type(_SAMPLE* pSample, unsigned int uFlags, int iChannelMask);
extern "C" void SS_update_sample_reverb_state(_SAMPLE* pSample);
unsigned char* assign_ptr(unsigned char** ppCursor, unsigned int uSize);
unsigned int assign_chan_ptrs(_SAMPLE* pSample, unsigned char* pBuffer, int iKeepLevelArrays);
void clear_last_sample(_SAMPLE* pSample);
void free_channel_buffers(_SAMPLE* pSample);
void set_user_channel_defaults(int iSpeakerConfig, int iChannelCount, float** ppLevels,
                               signed char* pChannelMap);
int SP_ASI_DECODER_callback(unsigned int uUser, void* pDestination, int iCount, int iOffset);

extern int AIL_debug_enabled;
extern int AIL_debug_nested;
extern int AIL_debug_depth;

#endif
