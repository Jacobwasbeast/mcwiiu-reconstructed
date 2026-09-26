#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/PlatformSoundEngine.hpp"
#include "game/client/audio/PlatformSoundEngineImpl.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundInstance.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

extern CMinecraftApp* g_pApp;
extern void SetSpecialThreadIDForFS_MilesStreamThread();

namespace {

struct PlaySoundData
{
    float x;
    float y;
    float z;
    float volume;
    float pitch;
    const SoundEvent* soundEvent;
    SoundInstance* instance;
    unsigned char activeFlag;
    unsigned char reserved;
    unsigned char drcOutput;
    unsigned char padding;
};

float absDelta(float a, float b)
{
    const float delta = a - b;
    return delta < 0.0f ? -delta : delta;
}

} // namespace

SoundEngine::SoundEngine()
    : m_listenerEnabled(0), m_listenerX(0.0f), m_listenerY(0.0f), m_listenerZ(0.0f), m_listenerForwardX(0.0f), m_listenerForwardY(0.0f), m_listenerForwardZ(0.0f),
      m_musicListenerEnabled(0), m_musicX(0.0f), m_musicY(0.0f), m_musicZ(0.0f), m_musicForwardX(0.0f), m_musicForwardY(0.0f), m_musicForwardZ(0.0f),
      m_thirdListenerEnabled(0), m_thirdX(0.0f), m_thirdY(0.0f), m_thirdZ(0.0f), m_thirdForwardX(0.0f), m_thirdForwardY(0.0f), m_thirdForwardZ(0.0f),
      m_streamListenerEnabled(0), m_streamListenerX(0.0f), m_streamListenerY(0.0f), m_streamListenerZ(0.0f), m_streamListenerForwardX(0.0f), m_streamListenerForwardY(0.0f), m_streamListenerForwardZ(0.0f),
      m_activeListenerCount(0), m_random(new Random()), m_musicTrack(-1), m_musicDelayTicks(0), m_musicActive(0), m_musicPlayState(0), m_musicPlayAux(0), m_musicDelayReset(0),
      m_streamingX(0.0f), m_streamingY(0.0f), m_streamingZ(0.0f), m_streamingVolume(1.0f), m_streamingPitch(1.0f), m_streamingField14(0.0f), m_streamingIs3D(0), m_streamingInstance(0),
      m_platformSoundEngine(0), m_openStreamThread(0), m_currentTrack(-1), m_platformState(0), m_trackCount(0), m_trackUsed(0), m_trackData(0), m_streamingTrackIndex(-1)
{
    std::memset(m_streamPathBuffer, 0, sizeof(m_streamPathBuffer));
    std::memset(m_currentTrackPadding, 0, sizeof(m_currentTrackPadding));
    std::memset(m_streamingPayloadPadding, 0, sizeof(m_streamingPayloadPadding));
    std::memset(m_platformPadding2, 0, sizeof(m_platformPadding2));
    std::memset(m_platformStatePadding, 0, sizeof(m_platformStatePadding));
    SetStreamingSounds(0, 11, 12, 15, 16, 17, 18, 23, 24, 27, 28, 31, 32, 41, 42, 42, 62, 68, 87);
    SetMusicID(eMusicType_End);
    m_platformSoundEngine = new PlatformSoundEngineImpl();
}

SoundEngine::~SoundEngine()
{
    if (m_trackCount > 7)
    {
        delete[] m_trackUsed;
    }
    m_trackData = 0;
    m_currentTrack = 0;
    m_trackCount = 7;
}

int SoundEngine::TracksSortFunction(const void* left, const void* right)
{
    return static_cast<const SoundTrack*>(left)->sortKey - static_cast<const SoundTrack*>(right)->sortKey;
}

void SoundEngine::BuildTrackList(int first, int last, bool keepFirstTrack)
{
    if (first <= last)
    {
        for (int track = first; track <= last; ++track)
        {
            m_trackUsed[track] = 0;
            m_trackData[track].sortKey = m_random->nextInt(100000);
            m_trackData[track].trackIndex = track;
        }
    }

    if (first == last)
        return;

    if (keepFirstTrack)
    {
        std::qsort(m_trackData + first + 1, last - first, sizeof(SoundTrack), TracksSortFunction);
        return;
    }

    std::qsort(m_trackData + first, (last - first) + 1, sizeof(SoundTrack), TracksSortFunction);

    if (m_streamingTrackIndex != -1 && m_trackData[first].trackIndex == m_streamingTrackIndex)
    {
        const int displaced = m_trackData[first + 1].trackIndex;
        m_trackData[first + 1].trackIndex = m_trackData[first].trackIndex;
        m_trackData[first].trackIndex = displaced;
    }
}

void SoundEngine::SetStreamingSounds(int first1, int last1, int first2, int last2, int first3, int last3,
                                     int first4, int last4, int first5, int last5, int first6, int last6,
                                     int first7, int last7, int first8, int last8, int first9, int last9,
                                     int trackCount)
{
    m_trackRanges[0] = first1;
    m_trackRanges[1] = last1;
    m_trackRanges[2] = first2;
    m_trackRanges[3] = last2;
    m_trackRanges[4] = first3;
    m_trackRanges[5] = last3;
    m_trackRanges[6] = first4;
    m_trackRanges[7] = last4;
    m_trackRanges[8] = first5;
    m_trackRanges[9] = last5;
    m_trackRanges[10] = first6;
    m_trackRanges[11] = last6;
    m_trackRanges[12] = first7;
    m_trackRanges[13] = last7;
    m_trackRanges[14] = first8;
    m_trackRanges[15] = last8;
    m_trackRanges[16] = first9;
    m_trackRanges[17] = last9;
    m_trackCount = trackCount;

    delete[] m_trackUsed;
    m_trackUsed = new unsigned char[trackCount];
    std::memset(m_trackUsed, 0, trackCount);

    delete[] m_trackData;
    m_trackData = new SoundTrack[trackCount];
    std::memset(m_trackData, 0, (trackCount - 1) * sizeof(SoundTrack) + 1);

    m_streamingTrackIndex = -1;

    BuildTrackList(first1, last1, false);
    BuildTrackList(first2, last2, false);
    BuildTrackList(first3, last3, false);
    BuildTrackList(first4, last4, false);
    BuildTrackList(first5, last5, false);
    BuildTrackList(first6, last6, false);
    BuildTrackList(first7, last7, false);
    BuildTrackList(first8, last8, false);
    BuildTrackList(first9, last9, true);
}

int SoundEngine::GetNextTrack(int first, int last)
{
    const int rangeSize = (last - first) + 1;
    int usedCount = 0;
    bool picked = false;
    int chosen = m_trackData[first].trackIndex;
    bool rebuild = true;

    if (first <= last)
    {
        for (int track = last; track >= first; --track)
        {
            if (m_trackUsed[track] != 0)
            {
                ++usedCount;
            }
            else if (!picked)
            {
                picked = true;
                chosen = m_trackData[track].trackIndex;
                m_trackUsed[track] = 1;
            }
        }

        if (usedCount != 0 && usedCount != rangeSize)
            rebuild = false;
    }

    if (rebuild)
    {
        BuildTrackList(first, last, false);
        chosen = m_trackData[first].trackIndex;
        m_trackUsed[first] = 1;
    }

    m_streamingTrackIndex = m_trackData[chosen].trackIndex;
    return m_streamingTrackIndex;
}

void SoundEngine::SetMusicID(eMusicType type)
{
    switch (type)
    {
    case eMusicType_Overworld:
        m_musicTrack = GetNextTrack(m_trackRanges[2], m_trackRanges[3]);
        return;
    case eMusicType_Nether:
    case eMusicType_Menu:
        break;
    case eMusicType_Boss:
        m_musicTrack = GetNextTrack(m_trackRanges[4], m_trackRanges[5]);
        return;
    case eMusicType_5:
        m_musicTrack = GetNextTrack(m_trackRanges[6], m_trackRanges[7]);
        return;
    case eMusicType_End:
        m_musicTrack = GetNextTrack(m_trackRanges[8], m_trackRanges[9]);
        return;
    case eMusicType_6:
        m_musicTrack = GetNextTrack(m_trackRanges[10], m_trackRanges[11]);
        return;
    case eMusicType_7:
        m_musicTrack = GetNextTrack(m_trackRanges[12], m_trackRanges[13]);
        return;
    case eMusicType_8:
        m_musicTrack = GetNextTrack(m_trackRanges[14], m_trackRanges[15]);
        return;
    case eMusicType_9:
        m_musicTrack = GetNextTrack(m_trackRanges[16], m_trackRanges[17]);
        return;
    default:
        break;
    }

    m_musicTrack = GetNextTrack(m_trackRanges[0], m_trackRanges[1]);
}

void SoundEngine::destroy()
{
}

void SoundEngine::shutdown()
{
    PlatformSoundEngine* platform = m_platformSoundEngine;
    if (platform != 0)
        platform->shutdown();
    m_platformSoundEngine = 0;
}

void SoundEngine::play(const SoundEvent* sound, float x, float y, float z, float volume, float pitch,
                       SoundInstance* instance)
{
    if (sound == 0)
        return;

    char eventPath[272];
    std::strcpy(eventPath, "Minecraft/");
    {
        char* convertedPath = ConvertSoundPathToName(sound->getName(), false);
        std::strcat(eventPath, convertedPath);
        delete[] convertedPath;
    }

    PlaySoundData data;

    data.x = x;
    data.y = y;
    data.z = z;
    data.volume = volume;
    data.pitch = pitch;
    data.soundEvent = sound;
    data.instance = instance;
    data.activeFlag = 1;
    data.reserved = 0;
    data.drcOutput = 0;
    data.padding = 0;

    PlatformSoundEngine* platform = m_platformSoundEngine;

    if (instance == 0)
    {
        data.drcOutput = 1;
        platform->startSound(&data, 0x20, eventPath);
        data.drcOutput = 0;
        platform->startSound(&data, 0x20, eventPath);
    }
    else
    {
        instance->setSIState(SoundInstance::eSoundState_Pending);
        data.drcOutput = instance->drcClone;
        instance->pendingTicks = 0;
        platform->startSound(&data, 0x20, eventPath);
    }
}

SoundEngine::eMusicType SoundEngine::GetMusicToPlay()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    bool noPlayers = true;
    bool anyAlive = false;
    bool anyDead = false;
    int selectedSlot = 0;

    for (int i = 0; i < 4; ++i)
    {
        boost::shared_ptr<MultiplayerLocalPlayer>& slot = minecraft->mLocalPlayers[i];
        if (!slot)
            continue;

        noPlayers = false;
        selectedSlot = i;
        int viewportState = slot->mViewport;
        if (viewportState == 1)
            anyAlive = true;
        else if (viewportState == -1)
            anyDead = true;
    }

    if (anyAlive)
        return eMusicType_Boss;
    if (anyDead)
        return eMusicType_Overworld;
    if (noPlayers)
    {
        if (g_pApp->GetGameHostOption(eGameHostOption_MiniGame) != 0)
            return eMusicType_6;
        return eMusicType_End;
    }

    boost::shared_ptr<MultiplayerLocalPlayer> selected = minecraft->mLocalPlayers[selectedSlot];
    MultiplayerLocalPlayer* player = selected.get();
    if (player == 0 || !player->isAlive() || player->isSleeping())
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        unsigned int id = miniGame->GetId();
        if (id == 0 || id > 4)
            return eMusicType_None;
        static const eMusicType kMiniGameMusicById[] = {
            eMusicType_6, eMusicType_7, eMusicType_8, eMusicType_9};
        return kMiniGameMusicById[id - 1];
    }

    return eMusicType_5;
}

void SoundEngine::SetMusicID(const std::wstring& name)
{
    static const wchar_t* kRecordNames[] = {
        L"11", L"13", L"blocks", L"cat", L"chirp", L"far",
        L"mall", L"mellohi", L"stal", L"strad", L"where_are_we_now", L"ward",
    };

    char* convertedPath = ConvertSoundPathToName(name, true);
    const int baseTrack = m_trackCount;
    m_musicTrack = baseTrack;

    for (int i = 0; i < 12; ++i)
    {
        char* recordPath = ConvertSoundPathToName(kRecordNames[i], true);
        if (std::strcmp(convertedPath, recordPath) == 0)
        {
            m_musicTrack = i + baseTrack;
            delete[] recordPath;
            break;
        }
        delete[] recordPath;
    }

    delete[] convertedPath;
}

void SoundEngine::playStreaming(const SoundEvent* sound, float x, float y, float z, float volume, float pitch,
                                bool checkPosition, bool waitForMusicStartEvent,
                                ConsoleSoundEngine::eAudioDelayType delayType)
{
    if (sound != NULL || !waitForMusicStartEvent || m_streamingField14 == 0.0f ||
        (m_streamingX == x && m_streamingY == y && m_streamingZ == z))
    {
        m_streamingVolume = volume;
        m_streamingPitch = pitch;
        m_streamingY = y;
        m_streamingX = x;
        m_streamingZ = z;

        if (m_musicPlayState == 6)
        {
            m_musicDelayReset = 200;
            m_musicPlayState = 1;
        }
        else if (m_musicPlayState == 3)
        {
            m_musicPlayState = 4;
        }

        if (sound == NULL)
        {
            m_streamingField14 = 0.0f;
            if (delayType == ConsoleSoundEngine::eAudioDelayType_None)
                m_musicDelayTicks = 0;
            else if (delayType == ConsoleSoundEngine::eAudioDelayType_Random)
                m_musicDelayTicks = m_random->nextInt(0xe10) + 600;
            else if (delayType > ConsoleSoundEngine::eAudioDelayType_ShortRound)
            {
                m_musicActive = checkPosition ? 1 : 0;
                SetMusicID(GetMusicToPlay());
                return;
            }
            else
                m_musicDelayTicks = 0x3c;

            m_musicActive = checkPosition ? 1 : 0;
            SetMusicID(GetMusicToPlay());
            return;
        }

        m_streamingField14 = 1.0f;
        SetMusicID(sound->getName());
        m_musicDelayTicks = 0;
    }
}

void SoundEngine::playUI(const SoundEvent* sound, float volume, float pitch)
{
    if (sound == 0)
        return;

    char eventPath[272];
    if (sound->isSFX())
        std::strcpy(eventPath, "Minecraft/UI/");
    else
        std::strcpy(eventPath, "Minecraft/");

    {
        char* convertedPath = ConvertSoundPathToName(sound->getName(), false);
        std::strcat(eventPath, convertedPath);
        delete[] convertedPath;
    }

    PlaySoundData data;
    data.x = 0.0f;
    data.y = 0.0f;
    data.z = 0.0f;
    data.volume = volume;
    data.pitch = pitch;
    data.soundEvent = sound;
    data.instance = 0;
    data.activeFlag = 1;
    data.reserved = 0;
    data.drcOutput = 1;
    data.padding = 0;

    PlatformSoundEngine* platform = m_platformSoundEngine;
    platform->startSound(&data, 0x20, eventPath);
    data.drcOutput = 0;
    platform->startSound(&data, 0x20, eventPath);
}

void SoundEngine::updateMusicVolume(float volume)
{
    setMasterMusicVolume(volume);
}

unsigned int SoundEngine::OpenStreamThreadProc(void* value)
{
    SetSpecialThreadIDForFS_MilesStreamThread();
    SoundEngine* engine = static_cast<SoundEngine*>(value);
    PlatformSoundEngine* platform = engine->m_platformSoundEngine;
    platform->startStream(engine->m_streamPathBuffer, ConfigureSound);
    return 0U;
}

double SoundEngine::getDistanceScaler(AudioInfo* info)
{
    double scalar = info->soundEvent->getDistanceScalar();
    if (info->distance == 10000.0f)
        scalar = 10000.0;
    return scalar;
}

void SoundEngine::UpdateSound(void* userData, void* soundData, bool* is3DOut, int soundType, float* outX,
                              float* outY, float* outZ, float* outVolume)
{
    SoundEngine* engine = static_cast<SoundEngine*>(userData);
    const bool hasInstance = soundData != 0;
    SoundInstance* instance = engine->m_streamingInstance;
    float x = engine->m_streamingX;
    float y = engine->m_streamingY;
    float z = engine->m_streamingZ;
    float distanceValue = engine->m_streamingVolume;
    bool is3D = engine->m_streamingIs3D != 0;

    if (hasInstance)
    {
        AudioInfo* callbackInfo = static_cast<AudioInfo*>(soundData);
        instance = callbackInfo->instance;
        x = instance->getX();
        y = instance->getY();
        z = instance->getZ();
        distanceValue = instance->getVolume();
        is3D = instance->getAttenuation() == SoundInstance::eAttenuation_Linear;
    }

    if (instance != 0)
    {
        if (soundType == 0)
            instance->setSIState(SoundInstance::eSoundState_Starting);
        else if (soundType == 1)
        {
            instance->setSIState(SoundInstance::eSoundState_Playing);
            x = instance->getX();
            y = instance->getY();
            z = instance->getZ();
            distanceValue = instance->getVolume();
        }
        else if (soundType == 2)
            instance->setSIState(SoundInstance::eSoundState_Complete);
    }

    double distance = distanceValue;
    const double minDistance = 1.0;
    if (distance > minDistance)
    {
        distanceValue = static_cast<float>(minDistance);
        distance = minDistance;
    }

    ConsoleSoundEngine* consoleEngine = static_cast<ConsoleSoundEngine*>(engine);
    const double masterVolume = hasInstance ? consoleEngine->getMasterSoundEffectVolume()
                                            : consoleEngine->getMasterMusicVolume();
    *outVolume = static_cast<float>(distance * masterVolume);
    *is3DOut = is3D;

    if (!is3D)
        return;

    if (engine->m_activeListenerCount < 2)
    {
        *outX = x;
        *outY = y;
        *outZ = -z;
        return;
    }

    const float defaultCoord = 0.0f;
    float bestSum = 10000.0f;
    float bestDx = defaultCoord;
    float bestDy = defaultCoord;
    float bestDz = defaultCoord;

    if (engine->m_listenerEnabled != 0)
    {
        const float dx = absDelta(engine->m_listenerX, x);
        const float dy = absDelta(engine->m_listenerY, y);
        const float dz = absDelta(engine->m_listenerZ, z);
        const float sum = dx + dy + dz;
        if (sum < bestSum)
        {
            bestSum = sum;
            bestDx = dx;
            bestDy = dy;
            bestDz = dz;
        }
    }

    if (engine->m_musicListenerEnabled != 0)
    {
        const float dx = absDelta(engine->m_musicX, x);
        const float dy = absDelta(engine->m_musicY, y);
        const float dz = absDelta(engine->m_musicZ, z);
        const float sum = dx + dy + dz;
        if (sum < bestSum)
        {
            bestSum = sum;
            bestDx = dx;
            bestDy = dy;
            bestDz = dz;
        }
    }

    if (engine->m_thirdListenerEnabled != 0)
    {
        const float dx = absDelta(engine->m_thirdX, x);
        const float dy = absDelta(engine->m_thirdY, y);
        const float dz = absDelta(engine->m_thirdZ, z);
        const float sum = dx + dy + dz;
        if (sum < bestSum)
        {
            bestSum = sum;
            bestDx = dx;
            bestDy = dy;
            bestDz = dz;
        }
    }

    if (engine->m_streamListenerEnabled != 0)
    {
        const float dx = absDelta(engine->m_streamListenerX, x);
        const float dy = absDelta(engine->m_streamListenerY, y);
        const float dz = absDelta(engine->m_streamListenerZ, z);
        const float sum = dx + dy + dz;
        if (sum < bestSum)
        {
            bestDx = dx;
            bestDy = dy;
            bestDz = dz;
        }
    }

    *outX = defaultCoord;
    *outY = defaultCoord;
    *outZ = sqrtf(bestDx * bestDx + bestDy * bestDy + bestDz * bestDz);
}

void SoundEngine::ConfigureSound(void* userData, void* soundData, bool* is3D, float* minDistance,
                                  float* playbackRate, float* x, float* y, float* z, float* volume)
{
    SoundEngine* engine = static_cast<SoundEngine*>(userData);

    if (soundData == 0)
    {
        *minDistance = 64.0f;
        *playbackRate = engine->m_streamingPitch;
        UpdateSound(userData, 0, is3D, 0, x, y, z, volume);
        return;
    }

    AudioInfo* info = static_cast<AudioInfo*>(soundData);
    double distanceScalar = engine->getDistanceScaler(info);
    *minDistance = static_cast<float>(distanceScalar);

    if (info->routeToDRC == 0)
    {
        *playbackRate = info->playbackRate;
        UpdateSound(userData, soundData, is3D, 0, x, y, z, volume);
    }
    else
    {
        *playbackRate = 100.0f;
        UpdateSound(userData, soundData, is3D, 0, x, y, z, volume);
    }
}

void SoundEngine::playMusicUpdate()
{
    if (musicVolumeFading)
    {
        unsigned int current = static_cast<unsigned int>(getMasterMusicVolume() * 100.0f);
        unsigned int destination = musicVolumeFadingUp ? unlimitedMusicVolume : targetMusicVolume;
        unsigned int step = musicVolumeFadeStep == 0 ? 1 : musicVolumeFadeStep;
        if (musicVolumeFadingUp)
        {
            current = current + step >= destination ? destination : current + step;
        }
        else
        {
            current = current <= destination + step ? destination : current - step;
        }
        updateMusicVolume(static_cast<float>(current) / 100.0f);
        if (current == destination)
        {
            musicVolumeFading = false;
            if (musicVolumeFadingUp)
                musicVolumeLimited = false;
        }
    }
    if (m_musicDelayReset > 0)
    {
        --m_musicDelayReset;
        if (m_musicDelayReset == 0 && m_musicPlayState == 6)
            m_musicPlayState = 1;
    }

    if (m_musicDelayTicks > 0)
    {
        --m_musicDelayTicks;
        if (m_musicDelayTicks == 0 && m_musicPlayState == 0)
            m_musicPlayState = 1;
    }

    PlatformSoundEngine* platform = m_platformSoundEngine;
    if (platform == 0)
        return;

    if (m_musicPlayState == 1)
    {
        platform->stopStream();
        m_musicPlayState = 2;
    }
    else if (m_musicPlayState == 2)
    {
        if (platform->openStream(m_streamPathBuffer))
        {
            platform->startStream(this, ConfigureSound);
            m_musicPlayState = 3;
        }
        else
        {
            m_musicPlayState = 6;
        }
    }
    else if (m_musicPlayState == 3)
    {
        if (platform->isStreamComplete())
            m_musicPlayState = 4;
    }
    else if (m_musicPlayState == 4)
    {
        if (!platform->isStreamComplete())
        {
            platform->startStream(this, ConfigureSound);
            m_musicPlayState = 6;
        }
        else
        {
            m_musicPlayState = 1;
        }
    }
    else if (m_musicPlayState == 6)
    {
        platform->setStreamVol(getMasterMusicVolume());
        platform->updateStreamPos(this, UpdateSound);
        if (platform->isStreamComplete())
        {
            platform->stopStream();
            m_musicPlayState = 0;
        }
    }

}

void SoundEngine::playMusicTick()
{
    playMusicUpdate();
}

void SoundEngine::init(Options*)
{
    PlatformSoundEngine* platform = m_platformSoundEngine;
    platform->init(0, 0);
    m_openStreamThread = 0;
}

bool SoundEngine::RouteToDRC(void*, void* value)
{
    return value != 0;
}

void SoundEngine::updateMiles()
{
    float x = 0.0f, y = 0.0f, z = 0.0f, forwardX = 0.0f, forwardY = 0.0f, forwardZ = 1.0f;

    if (m_activeListenerCount == 1)
    {
        if (m_listenerEnabled != 0)
        {
            x = m_listenerX; y = m_listenerY; z = -m_listenerZ;
            forwardX = -m_listenerForwardX; forwardY = m_listenerForwardY; forwardZ = m_listenerForwardZ;
        }
        else if (m_musicListenerEnabled != 0)
        {
            x = m_musicX; y = m_musicY; z = -m_musicZ;
            forwardX = -m_musicForwardX; forwardY = m_musicForwardY; forwardZ = m_musicForwardZ;
        }
        else if (m_thirdListenerEnabled != 0)
        {
            x = m_thirdX; y = m_thirdY; z = -m_thirdZ;
            forwardX = -m_thirdForwardX; forwardY = m_thirdForwardY; forwardZ = m_thirdForwardZ;
        }
        else if (m_streamListenerEnabled != 0)
        {
            x = m_streamListenerX; y = m_streamListenerY; z = -m_streamListenerZ;
            forwardX = -m_streamListenerForwardX; forwardY = m_streamListenerForwardY; forwardZ = m_streamListenerForwardZ;
        }
    }

    PlatformSoundEngine* platform = m_platformSoundEngine;
    bool systemMusicPlaying = false;
    platform->tick(this, x, y, z, forwardX, forwardY, forwardZ, ConfigureSound, UpdateSound, RouteToDRC, &systemMusicPlaying);
    updateSystemMusicPlaying(systemMusicPlaying);
}

void SoundEngine::tick(boost::shared_ptr<MultiplayerLocalPlayer>* players, float partialTick)
{
    ConsoleSoundEngine::tick();

    m_activeListenerCount = 0;
    if (players == 0)
    {
        m_listenerEnabled = 1;
        m_musicListenerEnabled = 0;
        m_thirdListenerEnabled = 0;
        m_streamListenerEnabled = 0;
        m_listenerX = 0.0f;
        m_listenerY = 0.0f;
        m_listenerZ = 0.0f;
        m_listenerForwardX = 0.0f;
        m_listenerForwardY = 0.0f;
        m_listenerForwardZ = 1.0f;
        m_activeListenerCount = 1;
        updateMiles();
        return;
    }

    for (int i = 0; i < 4; ++i)
    {
        boost::shared_ptr<MultiplayerLocalPlayer>& player = players[i];
        unsigned char* enabled = 0;
        float* listenerX = 0;
        float* listenerY = 0;
        float* listenerZ = 0;
        float* forwardX = 0;
        float* forwardY = 0;
        float* forwardZ = 0;
        switch (i)
        {
        case 0:
            enabled = &m_listenerEnabled; listenerX = &m_listenerX; listenerY = &m_listenerY; listenerZ = &m_listenerZ;
            forwardX = &m_listenerForwardX; forwardY = &m_listenerForwardY; forwardZ = &m_listenerForwardZ; break;
        case 1:
            enabled = &m_musicListenerEnabled; listenerX = &m_musicX; listenerY = &m_musicY; listenerZ = &m_musicZ;
            forwardX = &m_musicForwardX; forwardY = &m_musicForwardY; forwardZ = &m_musicForwardZ; break;
        case 2:
            enabled = &m_thirdListenerEnabled; listenerX = &m_thirdX; listenerY = &m_thirdY; listenerZ = &m_thirdZ;
            forwardX = &m_thirdForwardX; forwardY = &m_thirdForwardY; forwardZ = &m_thirdForwardZ; break;
        default:
            enabled = &m_streamListenerEnabled; listenerX = &m_streamListenerX; listenerY = &m_streamListenerY; listenerZ = &m_streamListenerZ;
            forwardX = &m_streamListenerForwardX; forwardY = &m_streamListenerForwardY; forwardZ = &m_streamListenerForwardZ; break;
        }
        if (!player)
        {
            *enabled = 0;
            continue;
        }

        *enabled = 1;
        ++m_activeListenerCount;
        const double x = player->mX;
        const double y = player->mY;
        const double z = player->mZ;
        const double oldX = player->mXo;
        const double oldY = player->mYo;
        const double oldZ = player->mZo;
        *listenerX = static_cast<float>((oldX - x) * partialTick + x);
        *listenerY = static_cast<float>((oldY - y) * partialTick + y);
        *listenerZ = static_cast<float>((oldZ - z) * partialTick + z);

        const float angle = (player->mYRotO - player->mYRot) * partialTick + player->mYRot;
        const float radians = -angle * 0.01745329238474369f - 3.1415927410125732f;
        *forwardX = std::sin(radians);
        *forwardY = 0.0f;
        *forwardZ = std::cos(radians);
    }

    if (m_activeListenerCount == 0)
    {
        m_listenerEnabled = 1;
        m_musicListenerEnabled = 0;
        m_thirdListenerEnabled = 0;
        m_streamListenerEnabled = 0;
        m_listenerX = 0.0f;
        m_listenerY = 0.0f;
        m_listenerZ = 0.0f;
        m_listenerForwardX = 0.0f;
        m_listenerForwardY = 0.0f;
        m_listenerForwardZ = 1.0f;
        m_activeListenerCount = 1;
    }
    else
    {
        m_listenerEnabled = 1;
    }

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
    char* result = new char[path.size() + 1];
    std::size_t i;
    for (i = 0; i < path.size(); ++i)
    {
        wchar_t value = path[i];
        if (value == L'.')
            value = L'/';
        if (replaceSpaces && value == L' ')
            value = L'_';
        result[i] = static_cast<char>(value);
    }
    result[path.size()] = 0;
    return result;
}

void SoundEngine::FlagMusicEventStart()
{
    m_musicActive = 0;
}

void SoundEngine::updateSoundEffectVolume(float volume)
{
    setMasterSoundEffectVolume(volume);
}
