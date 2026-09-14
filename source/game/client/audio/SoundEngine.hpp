#ifndef GAME_CLIENT_AUDIO_SOUNDENGINE_HPP
#define GAME_CLIENT_AUDIO_SOUNDENGINE_HPP
#include <string>
#include <boost/shared_ptr.hpp>
#include "game/client/audio/ConsoleSoundEngine.hpp"
class File; class Options; class Random; class SoundEvent; class SoundInstance; class MultiplayerLocalPlayer;
struct AudioInfo { float distance; SoundEvent* soundEvent; };
struct SoundTrack { int sortKey; int trackIndex; };
class SoundEngine : public ConsoleSoundEngine {
public:
 enum eMusicType { eMusicType_Overworld, eMusicType_Nether, eMusicType_End, eMusicType_Menu, eMusicType_Boss, eMusicType_5, eMusicType_6, eMusicType_7, eMusicType_8, eMusicType_9, eMusicType_None };
 SoundEngine(); virtual ~SoundEngine(); static int TracksSortFunction(const void*,const void*);
 void BuildTrackList(int,int,bool);
 void SetStreamingSounds(int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int);
 int GetNextTrack(int,int); void SetMusicID(eMusicType);
 virtual void destroy(); virtual void shutdown();
 virtual void play(const SoundEvent*,float,float,float,float,float,SoundInstance*);
 unsigned char GetMusicToPlay(); void SetMusicID(const std::wstring&);
 virtual void playStreaming(const SoundEvent*,float,float,float,float,float,bool,bool,ConsoleSoundEngine::eAudioDelayType);
 virtual void playUI(const SoundEvent*,float,float); void updateMusicVolume(float);
 static unsigned int OpenStreamThreadProc(void*); double getDistanceScaler(AudioInfo*);
 static void UpdateSound(void*,void*,bool*,int,float*,float*,float* ,float*);
 static void ConfigureSound(void*,void*,bool*,float*,float*,float*,float*,float*,float*);
 void playMusicUpdate(); virtual void playMusicTick(); virtual void init(Options*);
 static bool RouteToDRC(void*,void*); void updateMiles(); virtual void tick(boost::shared_ptr<MultiplayerLocalPlayer>*,float);
 virtual void add(const std::wstring&,File*); virtual void addMusic(const std::wstring&,File*); virtual void addStreaming(const std::wstring&,File*);
 virtual char* ConvertSoundPathToName(const std::wstring&,bool); void FlagMusicEventStart(); void updateSoundEffectVolume(float); bool isStreamingWavebankReady();
private:
 unsigned char m_listenerEnabled; float m_listenerX,m_listenerY,m_listenerZ,m_listenerForwardX,m_listenerForwardY,m_listenerForwardZ;
 unsigned char m_musicListenerEnabled; float m_musicX,m_musicY,m_musicZ,m_musicForwardX,m_musicForwardY,m_musicForwardZ;
 unsigned char m_thirdListenerEnabled; float m_thirdX,m_thirdY,m_thirdZ,m_thirdForwardX,m_thirdForwardY,m_thirdForwardZ;
 unsigned char m_streamListenerEnabled; float m_streamListenerX,m_streamListenerY,m_streamListenerZ,m_streamListenerForwardX,m_streamListenerForwardY,m_streamListenerForwardZ;
 int m_activeListenerCount;
 Random* m_random; int m_musicTrack; int m_musicDelayTicks; unsigned char m_musicActive; unsigned char m_musicActivePadding[3];
 int m_musicPlayState; int m_musicPlayAux; int m_musicDelayReset; float m_streamingX,m_streamingY,m_streamingZ; float m_streamingVolume,m_streamingPitch;
 unsigned char m_musicEventStarted; unsigned char m_musicEventStartedPadding[3];
 unsigned char m_platformPadding2[40];
 void* m_platformSoundEngine; void* m_openStreamThread; char m_streamPathBuffer[92];
 int m_currentTrack; unsigned char m_currentTrackPadding[24];
 void* m_platformState; unsigned char m_platformStatePadding[132]; int m_trackRanges[18]; int m_trackCount; unsigned char* m_trackUsed; SoundTrack* m_trackData; int m_streamingTrackIndex;
};
#endif
