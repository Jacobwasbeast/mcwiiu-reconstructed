#ifndef GAME_CLIENT_AUDIO_SOUNDENGINE_HPP
#define GAME_CLIENT_AUDIO_SOUNDENGINE_HPP
#include <string>
#include <boost/shared_ptr.hpp>
#include "game/client/audio/ConsoleSoundEngine.hpp"
class File; class Options; class Random; class SoundEvent; class SoundInstance; class MultiplayerLocalPlayer;
struct AudioInfo { float distance; SoundEvent* soundEvent; };
class SoundEngine : public ConsoleSoundEngine {
public:
 enum eMusicType { eMusicType_Overworld, eMusicType_Nether, eMusicType_End, eMusicType_Menu };
 SoundEngine(); virtual ~SoundEngine(); static int TracksSortFunction(const void*,const void*);
 void BuildTrackList(int,int,bool);
 void SetStreamingSounds(int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int);
 int GetNextTrack(int,int); void SetMusicID(eMusicType);
 virtual void destroy(); virtual void shutdown();
 virtual void play(const SoundEvent*,float,float,float,float,float,SoundInstance*);
 bool GetMusicToPlay(); void SetMusicID(const std::wstring&);
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
 unsigned char m_thirdListenerEnabled; float m_thirdX,m_thirdY,m_thirdZ,m_thirdForwardX,m_thirdForwardY;
 double m_listenerDistanceX; double m_listenerDistanceY; double m_listenerDistanceZ; double m_listenerDistanceW; float m_listenerDistanceScale;
 Random* m_random; float m_musicDistance; int m_trackCount; unsigned char m_musicEventStarted;
 float m_musicVolume,m_effectVolume; int m_musicType; float m_musicFade; int m_musicTrack; short m_musicState; unsigned char m_statePadding[14];
 unsigned char m_musicActive; unsigned char m_musicActivePadding[3]; int m_musicValueA; int m_musicValueB; int m_musicValueC; int m_musicValueD; int m_musicValueE; int m_musicValueF; int m_musicValueG; int m_musicValueH; int m_musicValueI;
 void* m_platformSoundEngine; unsigned char m_platformPadding[188]; int m_currentTrack; unsigned char m_currentTrackPadding[24];
 void* m_platformState; unsigned char m_platformStatePadding[40]; int m_streamingSoundIds[19]; unsigned char* m_trackUsed; void* m_trackData; int m_streamingCount;
};
#endif
