#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/DLCAudioFile.hpp"
#include "game/client/audio/PlatformSoundEngine.hpp"
#include "game/client/audio/PlatformSoundEngineImpl.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundInstance.hpp"
#include "game/client/render/DLCTexturePack.hpp"
#include "game/client/render/DefaultTexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/BossHealthOverlay.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/Gui.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CStorage.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern CMinecraftApp* g_pApp;
extern void SetSpecialThreadIDForFS_MilesStreamThread();

namespace {

const int DIMENSION_NETHER = -1;
const int DIMENSION_END = 1;
const int STREAM_TYPE_GAME_MUSIC = 10;
const int STREAM_TYPE_CD = 2;
const int FADE_OUT_TICKS = 200;
const int MINIGAME_MUSIC_DELAY = 60;
const unsigned int SHORT_MUSIC_DELAY_TEXTURE_PACK_ID = 1036;

float absDelta(float a, float b)
{
    const float delta = a - b;
    return delta < 0.0f ? -delta : delta;
}

}

char SoundEngine::m_szMusicPath[8] = "music/";

const char* SoundEngine::m_szStreamFileA[SoundEngine::STREAM_COUNT] = {
    "calm1",
    "calm2",
    "calm3",
    "hal1",
    "hal2",
    "hal3",
    "hal4",
    "nuance1",
    "nuance2",
    "piano1",
    "piano2",
    "piano3",
    "nether1",
    "nether2",
    "nether3",
    "nether4",
    "the_end_dragon_alive",
    "the_end_end",
    "creative1",
    "creative2",
    "creative3",
    "creative4",
    "creative5",
    "creative6",
    "menu1",
    "menu2",
    "menu3",
    "menu4",
    "MG01_01",
    "MG01_02",
    "MG01_03",
    "MG01_04",
    "MG02_01",
    "MG02_02",
    "MG02_03",
    "MG02_04",
    "MG02_05",
    "MG02_06",
    "MG02_07",
    "MG02_08",
    "MG02_09",
    "MG02_10",
    "MG03_Vanilla_Map1Track1",
    "MG03_Vanilla_Map2Track1",
    "MG03_Vanilla_Map3Track1",
    "MG03_Vanilla_Map4Track1",
    "MG03_Vanilla_Map5Track1",
    "MG03_Vanilla_Map6Track1",
    "MG03_Vanilla_Map7Track1",
    "MG03_Vanilla_Pack2TrackBody1",
    "MG03_Vanilla_Pack2TrackBody2",
    "MG03_Vanilla_Pack2TrackBody3",
    "MG03_Vanilla_Canyon1",
    "MG03_Vanilla_Canyon2",
    "MG03_Vanilla_Canyon3",
    "MG03_Plastic_Pack2TrackShrunk1",
    "MG03_Plastic_Pack2TrackShrunk2",
    "MG03_Plastic_Pack2TrackShrunk3",
    "MG03_Vanilla_Pack3_Celts1",
    "MG03_Vanilla_Pack3_Celts2",
    "MG03_Fantasy_Pack3_Excalibur1",
    "MG03_Fantasy_Pack3_Excalibur2",
    "MG04_01",
    "creative1",
    "creative2",
    "creative3",
    "creative4",
    "creative5",
    "creative6",
    "Fantasy_BornToArms",
    "Fantasy_Crusaders",
    "Fantasy_Hastilude",
    "City_8BitsPiecesChunks",
    "City_BeatAroundTheBlock",
    "City_FromDistrictToDistrict",
    "City_IndustrialQuarter",
    "City_PrecinctsAtNight",
    "Steampunk_AirshipAdventurers",
    "Steampunk_ClockworkCrafter",
    "Steampunk_MagnificentMachines",
    "Steampunk_SteamingSuperstructures",
    "MG01_Plastic_Bedroom_ToysOnATear",
    "MG01_Plastic_Bedroom_DanceOTBlocks",
    "MG01_Plastic_Shrunk_MasterBuilders",
    "MG01_Vanilla_Western_DesertDuel",
    "MG01_Vanilla_Western_SunsetRiders",
    "MG01_Vanilla_Western_TheOutlaw",
    "11",
    "13",
    "blocks",
    "cat",
    "chirp",
    "far",
    "mall",
    "mellohi",
    "stal",
    "strad",
    "ward",
    "where_are_we_now",
};

SoundEngine::SoundEngine()
{
    m_random = new Random();
    m_musicDelay = 0;
    m_tracks = NULL;
    m_waitingForMusicStartEvent = false;
    m_trackPlayed = NULL;
    m_streamState = eMusicStreamState_Idle;
    m_validListenerCount = 0;
    SetStreamingSounds(0, 11, 12, 15, 16, 17, 18, 23, 24, 27, 28, 31, 32, 41, 42, 42, 62, 68, STREAM_FIRST_CD);
    SetMusicID(eMusicType_Menu);
    m_streamingAudioInfo.instance = NULL;
    m_streamingAudioInfo.is3D = false;
    m_streamingAudioInfo.volume = 1.0f;
    m_fadeOutTicks = 0;
    m_streamingAudioInfo.pitch = 1.0f;
    m_streamingAudioInfo.y = 0.0f;
    m_streamingAudioInfo.x = 0.0f;
    m_streamingAudioInfo.z = 0.0f;
    std::memset(&m_listeners[0], 0, sizeof(ListenerInfo));
    m_platformSoundEngine = new PlatformSoundEngineImpl();
}

SoundEngine::~SoundEngine()
{
}

int SoundEngine::TracksSortFunction(const void* left, const void* right)
{
    return static_cast<const SoundTrack*>(left)->sortKey - static_cast<const SoundTrack*>(right)->sortKey;
}

void SoundEngine::BuildTrackList(int first, int last, bool keepFirstTrack)
{
    for (int track = first; track <= last; ++track)
    {
        m_trackPlayed[track] = 0;
        m_tracks[track].sortKey = m_random->nextInt(100000);
        m_tracks[track].trackIndex = track;
    }

    if (first == last)
        return;

    if (keepFirstTrack)
    {
        std::qsort(m_tracks + first + 1, last - first, sizeof(SoundTrack), TracksSortFunction);
        return;
    }

    std::qsort(m_tracks + first, (last - first) + 1, sizeof(SoundTrack), TracksSortFunction);

    if (m_lastTrack != -1 && m_tracks[first].trackIndex == m_lastTrack)
    {
        const int displaced = m_tracks[first + 1].trackIndex;
        m_tracks[first + 1].trackIndex = m_tracks[first].trackIndex;
        m_tracks[first].trackIndex = displaced;
    }
}

void SoundEngine::SetStreamingSounds(int overworldFirst, int overworldLast, int netherFirst, int netherLast,
                                     int endFirst, int endLast, int creativeFirst, int creativeLast,
                                     int menuFirst, int menuLast, int battleFirst, int battleLast,
                                     int tumbleFirst, int tumbleLast, int glideFirst, int glideLast,
                                     int buildFirst, int buildLast, int firstCDTrack)
{
    m_overworldFirst = overworldFirst;
    m_overworldLast = overworldLast;
    m_netherFirst = netherFirst;
    m_netherLast = netherLast;
    m_endFirst = endFirst;
    m_endLast = endLast;
    m_creativeFirst = creativeFirst;
    m_creativeLast = creativeLast;
    m_menuFirst = menuFirst;
    m_menuLast = menuLast;
    m_battleFirst = battleFirst;
    m_battleLast = battleLast;
    m_tumbleFirst = tumbleFirst;
    m_tumbleLast = tumbleLast;
    m_glideFirst = glideFirst;
    m_glideLast = glideLast;
    m_buildFirst = buildFirst;
    m_buildLast = buildLast;
    m_firstCDTrack = firstCDTrack;

    delete[] m_trackPlayed;
    m_trackPlayed = new unsigned char[firstCDTrack];
    std::memset(m_trackPlayed, 0, firstCDTrack);

    delete[] m_tracks;
    m_tracks = new SoundTrack[firstCDTrack];
    std::memset(m_tracks, 0, (firstCDTrack - 1) * sizeof(SoundTrack) + 1);

    m_lastTrack = -1;

    BuildTrackList(overworldFirst, overworldLast, false);
    BuildTrackList(netherFirst, netherLast, false);
    BuildTrackList(endFirst, endLast, false);
    BuildTrackList(creativeFirst, creativeLast, false);
    BuildTrackList(menuFirst, menuLast, false);
    BuildTrackList(battleFirst, battleLast, false);
    BuildTrackList(tumbleFirst, tumbleLast, false);
    BuildTrackList(glideFirst, glideLast, false);
    BuildTrackList(buildFirst, buildLast, true);
}

int SoundEngine::GetNextTrack(int first, int last)
{
    const unsigned int rangeSize = (last - first) + 1;
    unsigned int playedCount = 0;
    bool picked = false;
    int chosen = m_tracks[first].trackIndex;

    for (int track = last; track >= first; --track)
    {
        if (m_trackPlayed[track] != 0)
        {
            ++playedCount;
        }
        else if (!picked)
        {
            picked = true;
            chosen = m_tracks[track].trackIndex;
            m_trackPlayed[track] = 1;
        }
    }

    if (playedCount == 0 || playedCount == rangeSize)
    {
        BuildTrackList(first, last, false);
        chosen = m_tracks[first].trackIndex;
        m_trackPlayed[first] = 1;
    }

    m_lastTrack = m_tracks[chosen].trackIndex;
    return m_lastTrack;
}

void SoundEngine::SetMusicID(eMusicType type)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    const bool defaultPack = minecraft == NULL
                             || minecraft->mTexturePackRepository->getSelected() == TexturePackRepository::defaultTexturePack;

    switch (type)
    {
    case eMusicType_Nether:
        m_musicID = GetNextTrack(m_netherFirst, m_netherLast);
        return;
    case eMusicType_Menu:
        m_musicID = GetNextTrack(m_menuFirst, m_menuLast);
        return;
    case eMusicType_End:
        if (defaultPack)
        {
            Minecraft* instance = Minecraft::GetInstance();
            BossHealthOverlay* overlay = instance->mGui->getBossOverlay(instance->mPlayer->mUserIndex);
            if (overlay->shouldPlayMusic())
                m_musicID = m_endFirst;
            else
                m_musicID = m_endLast;
        }
        else
        {
            m_musicID = GetNextTrack(m_endFirst, m_endLast);
        }
        return;
    case eMusicType_Creative:
        m_musicID = GetNextTrack(m_creativeFirst, m_creativeLast);
        return;
    case eMusicType_Battle:
        m_musicID = GetNextTrack(m_battleFirst, m_battleLast);
        return;
    case eMusicType_Tumble:
        m_musicID = GetNextTrack(m_tumbleFirst, m_tumbleLast);
        return;
    case eMusicType_Glide:
        m_musicID = GetNextTrack(m_glideFirst, m_glideLast);
        return;
    case eMusicType_Build:
        m_musicID = GetNextTrack(m_buildFirst, m_buildLast);
        return;
    default:
        break;
    }

    m_musicID = GetNextTrack(m_overworldFirst, m_overworldLast);
}

void SoundEngine::destroy()
{
}

void SoundEngine::shutdown()
{
    m_platformSoundEngine->shutdown();
}

void SoundEngine::play(const SoundEvent* sound, float x, float y, float z, float volume, float pitch,
                       SoundInstance* instance)
{
    if (sound == NULL)
        return;

    char eventName[272];
    std::strcpy(eventName, "Minecraft/");
    std::strcat(eventName, ConvertSoundPathToName(sound->getName(), false));

    AudioInfo info;
    info.is3D = true;
    info.volume = volume;
    info.pitch = pitch;
    info.y = y;
    info.x = x;
    info.z = z;
    info.soundEvent = sound;
    info.instance = instance;

    if (instance != NULL)
    {
        instance->setSIState(SoundInstance::eSoundState_Pending);
        info.routeToDRC = instance->drcClone;
        instance->pendingTicks = 0;
        m_platformSoundEngine->startSound(&info, sizeof(AudioInfo), eventName);
    }
    else
    {
        info.routeToDRC = true;
        m_platformSoundEngine->startSound(&info, sizeof(AudioInfo), eventName);
        info.routeToDRC = false;
        m_platformSoundEngine->startSound(&info, sizeof(AudioInfo), eventName);
    }
}

SoundEngine::eMusicType SoundEngine::GetMusicToPlay()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    bool playerInEnd = false;
    bool playerInNether = false;
    bool noPlayers = true;
    int lastPlayer = 0;

    for (int i = 0; i < MAX_LISTENERS; ++i)
    {
        if (minecraft->mLocalPlayers[i] != NULL)
        {
            noPlayers = false;
            lastPlayer = i;
            if (minecraft->mLocalPlayers[i]->mDimension == DIMENSION_END)
                playerInEnd = true;
            else if (minecraft->mLocalPlayers[i]->mDimension == DIMENSION_NETHER)
                playerInNether = true;
        }
    }

    if (playerInEnd)
        return eMusicType_End;
    if (playerInNether)
        return eMusicType_Nether;
    if (noPlayers)
    {
        if (g_pApp->GetGameHostOption(eGameHostOption_MiniGame) != 0)
            return eMusicType_Battle;
        return eMusicType_Menu;
    }

    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[lastPlayer];
    if (player != NULL && player->mAbilities.mInstabuild && player->mAbilities.mMayfly)
        return eMusicType_Creative;

    switch (minecraft->GetMiniGame()->GetId())
    {
    case eMiniGameId_Battle:
        return eMusicType_Battle;
    case eMiniGameId_Tumble:
        return eMusicType_Tumble;
    case eMiniGameId_Glide:
        return eMusicType_Glide;
    case eMiniGameId_Build:
        return eMusicType_Build;
    default:
        return eMusicType_Overworld;
    }
}

void SoundEngine::SetMusicID(const std::wstring& name)
{
    char* soundName = ConvertSoundPathToName(name, true);

    for (int i = 0; i < STREAM_COUNT - STREAM_FIRST_CD; ++i)
    {
        if (std::strcmp(soundName, m_szStreamFileA[STREAM_FIRST_CD + i]) == 0)
        {
            m_musicID = i + m_firstCDTrack;
            return;
        }
    }

    m_musicID = m_firstCDTrack;
}

void SoundEngine::playStreaming(const SoundEvent* sound, float x, float y, float z, float volume, float pitch,
                                bool checkPosition, bool waitForMusicStartEvent,
                                ConsoleSoundEngine::eAudioDelayType delayType)
{
    if (sound == NULL && checkPosition && m_streamingAudioInfo.is3D
        && (m_streamingAudioInfo.x != x || m_streamingAudioInfo.y != y || m_streamingAudioInfo.z != z))
        return;

    m_streamingAudioInfo.volume = volume;
    m_streamingAudioInfo.pitch = pitch;
    m_streamingAudioInfo.y = y;
    m_streamingAudioInfo.x = x;
    m_streamingAudioInfo.z = z;

    if (m_streamState == eMusicStreamState_Playing)
    {
        m_fadeOutTicks = FADE_OUT_TICKS;
        m_streamState = eMusicStreamState_Stop;
    }
    else if (m_streamState == eMusicStreamState_Opening)
    {
        m_streamState = eMusicStreamState_OpeningCancel;
    }

    if (sound != NULL)
    {
        m_streamingAudioInfo.is3D = true;
        SetMusicID(sound->getName());
        m_musicDelay = 0;
        return;
    }

    m_streamingAudioInfo.is3D = false;
    switch (delayType)
    {
    case ConsoleSoundEngine::eAudioDelayType_None:
        m_musicDelay = 0;
        break;
    case ConsoleSoundEngine::eAudioDelayType_Random:
        m_musicDelay = m_random->nextInt(3600) + 600;
        break;
    case ConsoleSoundEngine::eAudioDelayType_Short:
    case ConsoleSoundEngine::eAudioDelayType_ShortMenu:
    case ConsoleSoundEngine::eAudioDelayType_ShortLobby:
    case ConsoleSoundEngine::eAudioDelayType_ShortRound:
        m_musicDelay = 60;
        break;
    default:
        break;
    }

    m_waitingForMusicStartEvent = waitForMusicStartEvent;
    SetMusicID(GetMusicToPlay());
}

void SoundEngine::playUI(const SoundEvent* sound, float volume, float pitch)
{
    char eventName[272];
    std::wstring soundName;
    if (sound->isSFX())
    {
        std::strcpy(eventName, "Minecraft/UI/");
        soundName = sound->getName();
    }
    else
    {
        std::strcpy(eventName, "Minecraft/");
        soundName = sound->getName();
    }
    std::strcat(eventName, ConvertSoundPathToName(soundName, false));

    AudioInfo info;
    std::memset(&info, 0, sizeof(AudioInfo));
    info.volume = volume;
    info.routeToDRC = true;
    info.pitch = pitch;
    info.isUI = true;
    info.soundEvent = sound;
    m_platformSoundEngine->startSound(&info, sizeof(AudioInfo), eventName);
    info.routeToDRC = false;
    m_platformSoundEngine->startSound(&info, sizeof(AudioInfo), eventName);
}

void SoundEngine::updateMusicVolume(float volume)
{
    setMasterMusicVolume(volume);
}

int SoundEngine::OpenStreamThreadProc(void* param)
{
    SetSpecialThreadIDForFS_MilesStreamThread();
    SoundEngine* engine = static_cast<SoundEngine*>(param);
    if (engine->m_platformSoundEngine->openStream(engine->m_streamName))
        return 0;
    return -1;
}

double SoundEngine::getDistanceScaler(AudioInfo* info)
{
    double scaler = info->soundEvent->getDistanceScalar();
    if (info->volume == 10000.0f)
        scaler = 10000.0f;
    return scaler;
}

void SoundEngine::UpdateSound(void* userData, void* soundData, bool* is3D, int soundState, float* x, float* y,
                              float* z, float* volume)
{
    SoundEngine* engine = static_cast<SoundEngine*>(userData);
    const bool isSound = soundData != NULL;
    AudioInfo* info = isSound ? static_cast<AudioInfo*>(soundData) : &engine->m_streamingAudioInfo;

    SoundInstance* instance = info->instance;
    if (instance != NULL)
    {
        if (soundState == 0)
        {
            instance->setSIState(SoundInstance::eSoundState_Starting);
        }
        else if (soundState == 1)
        {
            instance->setSIState(SoundInstance::eSoundState_Playing);
            info->x = info->instance->getX();
            info->y = info->instance->getY();
            info->z = info->instance->getZ();
            info->volume = info->instance->getVolume();
            info->pitch = info->instance->getPitch();
        }
        else if (soundState == 2)
        {
            instance->setSIState(SoundInstance::eSoundState_Complete);
        }
    }

    if (info->volume > 1.0f)
        info->volume = 1.0f;

    if (isSound)
        *volume = info->volume * engine->getMasterSoundEffectVolume();
    else
        *volume = info->volume * engine->getMasterMusicVolume();

    *is3D = info->is3D;
    if (!info->is3D)
        return;

    if (engine->m_validListenerCount < 2)
    {
        *x = info->x;
        *y = info->y;
        *z = -info->z;
        return;
    }

    float nearest = 10000.0f;
    float nearestX = 0.0f;
    float nearestY = 0.0f;
    float nearestZ = 0.0f;
    for (int i = 0; i < MAX_LISTENERS; ++i)
    {
        const ListenerInfo& listener = engine->m_listeners[i];
        if (!listener.valid)
            continue;

        const float dx = absDelta(listener.x, info->x);
        const float dy = absDelta(listener.y, info->y);
        const float dz = absDelta(listener.z, info->z);
        const float distance = dx + dy + dz;
        if (distance < nearest)
        {
            nearest = distance;
            nearestX = dx;
            nearestY = dy;
            nearestZ = dz;
        }
    }

    *x = 0.0f;
    *y = 0.0f;
    *z = sqrtf(nearestZ * nearestZ + nearestX * nearestX + nearestY * nearestY);
}

void SoundEngine::ConfigureSound(void* userData, void* soundData, bool* is3D, float* minDistance, float* pitch,
                                 float* x, float* y, float* z, float* volume)
{
    SoundEngine* engine = static_cast<SoundEngine*>(userData);

    if (soundData == NULL)
    {
        *minDistance = 64.0f;
        *pitch = engine->m_streamingAudioInfo.pitch;
        UpdateSound(userData, NULL, is3D, 0, x, y, z, volume);
        return;
    }

    AudioInfo* info = static_cast<AudioInfo*>(soundData);
    *minDistance = static_cast<float>(engine->getDistanceScaler(info));
    if (!info->isUI)
    {
        *pitch = info->pitch;
        UpdateSound(userData, soundData, is3D, 0, x, y, z, volume);
    }
    else
    {
        *pitch = 0.0f;
        UpdateSound(userData, soundData, is3D, 0, x, y, z, volume);
    }
}

void SoundEngine::playMusicUpdate()
{
    static float s_musicVolume;
    static bool s_firstUpdate = true;

    if (s_firstUpdate)
    {
        s_musicVolume = getMasterMusicVolume();
        s_firstUpdate = false;
    }

    if (musicVolumeFading)
    {
        const unsigned int current = static_cast<unsigned int>(getMasterMusicVolume() * 100.0f);
        const unsigned int target = targetMusicVolume;
        if (musicVolumeFadingUp)
        {
            if (current < target)
            {
                updateMusicVolume(static_cast<float>(current + musicVolumeFadeStep) / 100.0f);
            }
            else
            {
                musicVolumeFading = false;
                if (current > target)
                    updateMusicVolume(static_cast<float>(target) / 100.0f);
                musicVolumeLimited = !musicVolumeLimited;
            }
        }
        else if (current > target)
        {
            updateMusicVolume(static_cast<float>(current - musicVolumeFadeStep) / 100.0f);
        }
        else
        {
            musicVolumeFading = false;
            if (current < target)
                updateMusicVolume(static_cast<float>(target) / 100.0f);
            musicVolumeLimited = !musicVolumeLimited;
        }
    }

    switch (m_streamState)
    {
    case eMusicStreamState_Idle:
        if (m_waitingForMusicStartEvent)
            return;

        if (m_musicDelay > 0)
        {
            --m_musicDelay;
            return;
        }

        if (m_musicID != -1)
        {
            std::sprintf(m_streamName, "%s", m_szMusicPath);

            if (Minecraft::GetInstance()->mTexturePackRepository->getSelected()->hasAudio())
            {
                DLCTexturePack* pack =
                    static_cast<DLCTexturePack*>(Minecraft::GetInstance()->mTexturePackRepository->getSelected());
                DLCAudioFile* audioFile = static_cast<DLCAudioFile*>(
                    pack->getDLCInfoParentPack()->getFile(DLCManager::eDLCType_Audio, 0, true, true));

                if (m_musicID < m_firstCDTrack)
                {
                    SetIsPlayingStreamingGameMusic(true);
                    SetIsPlayingStreamingCDMusic(false);
                    m_streamType = STREAM_TYPE_GAME_MUSIC;
                    m_streamingAudioInfo.is3D = false;

                    char fileName[255];
                    std::wstring& soundName = audioFile->GetSoundName(m_musicID);
                    std::wcstombs(fileName, soundName.c_str(), 255);
                    std::string file(fileName);
                    std::string path = std::string("TPACK:\\Data\\").append(file);
                    std::string streamPath = path + ".binka";
                    std::string mountedPath = gStorage->GetMountedPath(streamPath);
                    std::strcpy(m_streamName, mountedPath.c_str());
                }
                else
                {
                    SetIsPlayingStreamingGameMusic(false);
                    SetIsPlayingStreamingCDMusic(true);
                    const int stream = m_musicID - m_firstCDTrack + STREAM_FIRST_CD;
                    m_streamType = STREAM_TYPE_CD;
                    m_streamingAudioInfo.is3D = true;
                    if (stream >= STREAM_COUNT)
                        return;

                    std::strcat(m_streamName, "cds/");
                    std::strcat(m_streamName, m_szStreamFileA[stream]);
                    std::strcat(m_streamName, ".binka");
                }
            }
            else
            {
                if (m_musicID < m_firstCDTrack)
                {
                    SetIsPlayingStreamingGameMusic(true);
                    SetIsPlayingStreamingCDMusic(false);
                    m_streamingAudioInfo.is3D = false;
                    m_streamType = STREAM_TYPE_GAME_MUSIC;
                    std::strcat(m_streamName, "music/");
                }
                else
                {
                    SetIsPlayingStreamingGameMusic(false);
                    SetIsPlayingStreamingCDMusic(true);
                    m_streamType = STREAM_TYPE_CD;
                    m_streamingAudioInfo.is3D = true;
                    std::strcat(m_streamName, "cds/");
                }
                std::strcat(m_streamName, m_szStreamFileA[m_musicID]);
                std::strcat(m_streamName, ".binka");
            }

            m_openStreamThread = new C4JThreadImpl(OpenStreamThreadProc, this, "OpenStreamThreadProc", 0);
            m_openStreamThread->SetProcessor(2);
            m_openStreamThread->Run();
            m_streamState = eMusicStreamState_Opening;
        }
        break;

    case eMusicStreamState_Stop:
        if (m_fadeOutTicks != 0)
        {
            --m_fadeOutTicks;
            const float fade = static_cast<float>(m_fadeOutTicks) / 200.0f;
            m_platformSoundEngine->setStreamVol(m_streamingAudioInfo.volume * getMasterMusicVolume() * fade);
        }
        else
        {
            m_platformSoundEngine->stopStream();
            SetIsPlayingStreamingCDMusic(false);
            SetIsPlayingStreamingGameMusic(false);
            m_streamState = eMusicStreamState_Idle;
        }
        break;

    case eMusicStreamState_Opening:
        if (!m_openStreamThread->isRunning())
        {
            const int result = m_openStreamThread->GetExitCode();
            delete m_openStreamThread;
            if (result != 0)
            {
                m_openStreamThread = new C4JThreadImpl(OpenStreamThreadProc, this, "RetryOpenStreamThreadProc", 0);
                m_openStreamThread->SetProcessor(2);
                m_openStreamThread->Run();
            }
            else
            {
                m_openStreamThread = NULL;
                m_platformSoundEngine->startStream(this, ConfigureSound);
                m_streamState = eMusicStreamState_Playing;
            }
        }
        break;

    case eMusicStreamState_OpeningCancel:
        if (!m_openStreamThread->isRunning())
        {
            delete m_openStreamThread;
            m_streamState = eMusicStreamState_Stop;
            m_openStreamThread = NULL;
        }
        break;

    case eMusicStreamState_Playing:
        if (GetIsPlayingStreamingGameMusic())
        {
            Minecraft* minecraft = Minecraft::GetInstance();
            bool playerInEnd = false;
            bool playerInNether = false;
            bool noPlayers = true;
            int lastPlayer = 0;

            for (int i = 0; i < MAX_LISTENERS; ++i)
            {
                if (minecraft->mLocalPlayers[i] != NULL)
                {
                    noPlayers = false;
                    lastPlayer = i;
                    if (minecraft->mLocalPlayers[i]->mDimension == DIMENSION_END)
                        playerInEnd = true;
                    else if (minecraft->mLocalPlayers[i]->mDimension == DIMENSION_NETHER)
                        playerInNether = true;
                }
            }

            if (playerInEnd && !GetIsPlayingEndMusic())
            {
                m_fadeOutTicks = FADE_OUT_TICKS;
                m_streamState = eMusicStreamState_Stop;
                SetMusicID(eMusicType_End);
                SetIsPlayingEndMusic(true);
                SetIsPlayingNetherMusic(false);
            }
            else if (!playerInEnd && GetIsPlayingEndMusic())
            {
                m_fadeOutTicks = FADE_OUT_TICKS;
                m_streamState = eMusicStreamState_Stop;
                if (playerInNether)
                {
                    SetMusicID(eMusicType_Nether);
                    SetIsPlayingEndMusic(false);
                    SetIsPlayingNetherMusic(true);
                }
                else if (noPlayers)
                {
                    SetMusicID(eMusicType_Menu);
                    SetIsPlayingEndMusic(false);
                    SetIsPlayingNetherMusic(false);
                }
                else
                {
                    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[lastPlayer];
                    if (player != NULL && player->mAbilities.mInstabuild && player->mAbilities.mMayfly)
                    {
                        SetMusicID(eMusicType_Creative);
                    }
                    else
                    {
                        switch (minecraft->GetMiniGame()->GetId())
                        {
                        case eMiniGameId_Battle:
                            SetMusicID(eMusicType_Battle);
                            break;
                        case eMiniGameId_Tumble:
                            SetMusicID(eMusicType_Tumble);
                            break;
                        case eMiniGameId_Glide:
                            SetMusicID(eMusicType_Glide);
                            break;
                        case eMiniGameId_Build:
                            SetMusicID(eMusicType_Build);
                            break;
                        default:
                            SetMusicID(eMusicType_Overworld);
                            break;
                        }
                    }
                    SetIsPlayingEndMusic(false);
                    SetIsPlayingNetherMusic(false);
                }
            }
            else if (playerInNether && !GetIsPlayingNetherMusic() && !GetIsPlayingEndMusic())
            {
                m_fadeOutTicks = FADE_OUT_TICKS;
                m_streamState = eMusicStreamState_Stop;
                SetMusicID(eMusicType_Nether);
                SetIsPlayingNetherMusic(true);
                SetIsPlayingEndMusic(false);
            }
            else if (!playerInNether && GetIsPlayingNetherMusic())
            {
                m_fadeOutTicks = FADE_OUT_TICKS;
                m_streamState = eMusicStreamState_Stop;
                if (playerInEnd)
                {
                    SetMusicID(eMusicType_End);
                    SetIsPlayingNetherMusic(false);
                    SetIsPlayingEndMusic(true);
                }
                else if (noPlayers)
                {
                    SetMusicID(eMusicType_Menu);
                    SetIsPlayingNetherMusic(false);
                    SetIsPlayingEndMusic(false);
                }
                else
                {
                    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[lastPlayer];
                    if (player != NULL && player->mAbilities.mInstabuild && player->mAbilities.mMayfly)
                    {
                        SetMusicID(eMusicType_Creative);
                    }
                    else
                    {
                        switch (minecraft->GetMiniGame()->GetId())
                        {
                        case eMiniGameId_Battle:
                            SetMusicID(eMusicType_Battle);
                            break;
                        case eMiniGameId_Tumble:
                            SetMusicID(eMusicType_Tumble);
                            break;
                        case eMiniGameId_Glide:
                            SetMusicID(eMusicType_Glide);
                            break;
                        case eMiniGameId_Build:
                            SetMusicID(eMusicType_Build);
                            break;
                        default:
                            SetMusicID(eMusicType_Overworld);
                            break;
                        }
                    }
                    SetIsPlayingNetherMusic(false);
                    SetIsPlayingEndMusic(false);
                }
            }
        }
        else if (m_streamingAudioInfo.is3D)
        {
            m_platformSoundEngine->updateStreamPos(this, UpdateSound);
        }

        if (s_musicVolume != getMasterMusicVolume())
        {
            s_musicVolume = getMasterMusicVolume();
            m_platformSoundEngine->setStreamVol(s_musicVolume);
        }
        break;

    case eMusicStreamState_Completed:
    {
        m_musicDelay = m_random->nextInt(3600) + 600;

        Minecraft* minecraft = Minecraft::GetInstance();
        bool playerInEnd = false;
        bool playerInNether = false;
        bool noPlayers = true;
        int lastPlayer = 0;

        for (int i = 0; i < MAX_LISTENERS; ++i)
        {
            if (minecraft->mLocalPlayers[i] != NULL)
            {
                noPlayers = false;
                lastPlayer = i;
                if (minecraft->mLocalPlayers[i]->mDimension == DIMENSION_END)
                    playerInEnd = true;
                else if (minecraft->mLocalPlayers[i]->mDimension == DIMENSION_NETHER)
                    playerInNether = true;
            }
        }

        if (playerInEnd)
        {
            SetMusicID(eMusicType_End);
            SetIsPlayingEndMusic(true);
            SetIsPlayingNetherMusic(false);
        }
        else if (playerInNether)
        {
            SetMusicID(eMusicType_Nether);
            SetIsPlayingNetherMusic(true);
            SetIsPlayingEndMusic(false);
        }
        else if (noPlayers)
        {
            if (Minecraft::GetInstance()->mTexturePackRepository->getSelected()->getId()
                == SHORT_MUSIC_DELAY_TEXTURE_PACK_ID)
                SetMusicID(eMusicType_Overworld);
            else
                SetMusicID(eMusicType_Menu);
            SetIsPlayingNetherMusic(false);
            SetIsPlayingEndMusic(false);
        }
        else
        {
            boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[lastPlayer];
            if (player != NULL && player->mAbilities.mInstabuild && player->mAbilities.mMayfly)
            {
                SetMusicID(eMusicType_Creative);
            }
            else
            {
                switch (minecraft->GetMiniGame()->GetId())
                {
                case eMiniGameId_Lobby:
                    SetMusicID(eMusicType_Overworld);
                    break;
                case eMiniGameId_Battle:
                    SetMusicID(eMusicType_Battle);
                    m_musicDelay = MINIGAME_MUSIC_DELAY;
                    break;
                case eMiniGameId_Tumble:
                    SetMusicID(eMusicType_Tumble);
                    m_musicDelay = MINIGAME_MUSIC_DELAY;
                    break;
                case eMiniGameId_Glide:
                    SetMusicID(eMusicType_Glide);
                    m_musicDelay = MINIGAME_MUSIC_DELAY;
                    break;
                case eMiniGameId_Build:
                    SetMusicID(eMusicType_Build);
                    m_musicDelay = MINIGAME_MUSIC_DELAY;
                    break;
                default:
                    SetMusicID(eMusicType_Overworld);
                    break;
                }
            }
            SetIsPlayingNetherMusic(false);
            SetIsPlayingEndMusic(false);
        }

        if (Minecraft::GetInstance()->mTexturePackRepository->getSelected()->getId()
            == SHORT_MUSIC_DELAY_TEXTURE_PACK_ID)
            m_musicDelay = m_random->nextInt(200) + 100;

        m_streamState = eMusicStreamState_Idle;
        break;
    }

    default:
        break;
    }

    if (m_platformSoundEngine->isStreamComplete())
    {
        SetIsPlayingStreamingCDMusic(false);
        SetIsPlayingStreamingGameMusic(false);
        m_streamState = eMusicStreamState_Completed;
    }
}

void SoundEngine::playMusicTick()
{
    playMusicUpdate();
}

void SoundEngine::init(Options*)
{
    m_platformSoundEngine->init(NULL, NULL);
    m_openStreamThread = NULL;
}

bool SoundEngine::RouteToDRC(void*, void* soundData)
{
    return static_cast<AudioInfo*>(soundData)->routeToDRC;
}

void SoundEngine::updateMiles()
{
    bool systemMusicPlaying;

    if (m_validListenerCount == 1)
    {
        for (int i = 0; i < MAX_LISTENERS; ++i)
        {
            const ListenerInfo& listener = m_listeners[i];
            if (listener.valid)
            {
                m_platformSoundEngine->tick(this, listener.x, listener.y, -listener.z, -listener.forwardX,
                                            listener.forwardY, listener.forwardZ, ConfigureSound, UpdateSound,
                                            RouteToDRC, &systemMusicPlaying);
                updateSystemMusicPlaying(systemMusicPlaying);
                return;
            }
        }
    }

    m_platformSoundEngine->tick(this, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, ConfigureSound, UpdateSound, RouteToDRC,
                                &systemMusicPlaying);
    updateSystemMusicPlaying(systemMusicPlaying);
}

void SoundEngine::tick(boost::shared_ptr<Mob>* mobs, float partialTicks)
{
    ConsoleSoundEngine::tick();

    int validListeners = 0;
    if (mobs == NULL)
    {
        m_listeners[0].z = 0.0f;
        validListeners = 1;
        m_listeners[0].y = 0.0f;
        m_listeners[0].forwardX = 0.0f;
        m_listeners[0].forwardY = 0.0f;
        m_listeners[0].x = 0.0f;
        m_listeners[0].forwardZ = 1.0f;
    }
    else
    {
        for (int i = 0; i < MAX_LISTENERS; ++i)
        {
            ListenerInfo& listener = m_listeners[i];
            if (mobs[i] == NULL)
            {
                listener.valid = false;
                continue;
            }

            listener.valid = true;
            listener.x = static_cast<float>((mobs[i]->mX - mobs[i]->mXo) * partialTicks + mobs[i]->mXo);
            listener.y = static_cast<float>((mobs[i]->mY - mobs[i]->mYo) * partialTicks + mobs[i]->mYo);
            listener.z = static_cast<float>((mobs[i]->mZ - mobs[i]->mZo) * partialTicks + mobs[i]->mZo);

            const float yRot = (mobs[i]->mYRot - mobs[i]->mYRotO) * partialTicks + mobs[i]->mYRotO;
            const float angle = -yRot * 0.017453292f - 3.1415927f;
            listener.forwardX = sinf(angle);
            listener.forwardY = 0.0f;
            listener.forwardZ = cosf(angle);
            ++validListeners;
        }

        if (validListeners == 0)
        {
            m_listeners[0].forwardX = 0.0f;
            m_listeners[0].forwardY = 0.0f;
            validListeners = 1;
            m_listeners[0].z = 0.0f;
            m_listeners[0].forwardZ = 1.0f;
            m_listeners[0].x = 0.0f;
            m_listeners[0].y = 0.0f;
        }
    }

    m_validListenerCount = validListeners;
    updateMiles();
}

void SoundEngine::add(const std::wstring&, File*)
{
}

void SoundEngine::addMusic(const std::wstring&, File*)
{
}

void SoundEngine::addStreaming(const std::wstring&, File*)
{
}

char* SoundEngine::ConvertSoundPathToName(const std::wstring& path, bool replaceSpaces)
{
    static char name[260];
    const std::size_t length = path.size();
    for (std::size_t i = 0; i < length; ++i)
    {
        wchar_t value = path[i];
        if (value == L'.')
            value = L'/';
        else if (replaceSpaces && value == L' ')
            value = L'_';
        name[i] = static_cast<char>(value);
    }
    name[length] = 0;
    return name;
}

void SoundEngine::FlagMusicEventStart()
{
    m_waitingForMusicStartEvent = false;
}

void SoundEngine::updateSoundEffectVolume(float volume)
{
    setMasterSoundEffectVolume(volume);
}
