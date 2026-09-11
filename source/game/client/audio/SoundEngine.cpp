#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/PlatformSoundEngine.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundInstance.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

extern CMinecraftApp* g_pApp;
extern void SetSpecialThreadIDForFS_MilesStreamThread__Fv();

namespace {

unsigned char playerByteAt(const MultiplayerLocalPlayer* player, unsigned int offset)
{
    return *reinterpret_cast<const unsigned char*>(reinterpret_cast<const char*>(player) + offset);
}

boost::shared_ptr<MultiplayerLocalPlayer>& musicPlayerSlot(Minecraft* minecraft, int index)
{
    return reinterpret_cast<boost::shared_ptr<MultiplayerLocalPlayer>*>(
        reinterpret_cast<char*>(minecraft) + 0x44)[index];
}

struct MilesSoundBuffer
{
    float x;
    float y;
    float z;
    float distance;
    float pitch;
    float field14;
    unsigned char is3D;
    unsigned char pad15[3];
    SoundInstance* instance;
};

float absDelta(float a, float b)
{
    const float delta = a - b;
    return delta < 0.0f ? -delta : delta;
}

unsigned char soundEngineByteAt(const SoundEngine* engine, unsigned int offset)
{
    return *reinterpret_cast<const unsigned char*>(reinterpret_cast<const char*>(engine) + offset);
}

float soundEngineFloatAt(const SoundEngine* engine, unsigned int offset)
{
    return *reinterpret_cast<const float*>(reinterpret_cast<const char*>(engine) + offset);
}

} // namespace

SoundEngine::SoundEngine()
    : m_listenerEnabled(0), m_musicListenerEnabled(0), m_thirdListenerEnabled(0), m_random(new Random()), m_musicTrack(-1), m_musicDelayTicks(0), m_musicActive(0), m_musicPlayState(0), m_musicPlayAux(0), m_musicDelayReset(0), m_streamingX(0.0f), m_streamingY(0.0f), m_streamingZ(0.0f), m_streamingVolume(0.0f), m_streamingPitch(0.0f), m_musicEventStarted(0), m_musicField15c(0), m_musicField160(0), m_musicState(0), m_musicNameDataA(0), m_musicNameDataB(0), m_musicNameCapacity(7), m_currentTrack(-1), m_platformSoundEngine(0), m_platformState(0), m_trackUsed(0), m_trackData(0), m_streamingTrackIndex(-1)
{
    std::memset(m_statePadding, 0, sizeof(m_statePadding));
    std::memset(m_currentTrackPadding, 0, sizeof(m_currentTrackPadding));
    std::memset(m_platformPadding, 0, sizeof(m_platformPadding));
    std::memset(m_platformStatePadding, 0, sizeof(m_platformStatePadding));
    std::memset(m_streamingSoundIds, 0, sizeof(m_streamingSoundIds));
    SetStreamingSounds(0, 11, 12, 15, 16, 17, 18, 23, 24, 27, 28, 31, 32, 41, 42, 42, 62, 68, 87);
    SetMusicID(eMusicType_End);
}

SoundEngine::~SoundEngine()
{
    delete m_random;
    delete[] m_trackUsed;
    delete[] static_cast<int*>(m_trackData);
}

int SoundEngine::TracksSortFunction(const void* left, const void* right)
{
    return *static_cast<const int*>(left) - *static_cast<const int*>(right);
}

void SoundEngine::BuildTrackList(int first, int last, bool preserve)
{
    if (first > last || m_trackUsed == 0)
        return;
    int i;
    for (i = first; i <= last; ++i)
        m_trackUsed[i] = 0;
    if (!preserve)
        std::sort(m_streamingSoundIds + first, m_streamingSoundIds + last + 1);
}

void SoundEngine::SetStreamingSounds(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n, int o, int p, int q, int r, int count)
{
    const int values[19] = { a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, count };
    std::memcpy(m_streamingSoundIds, values, sizeof(values));
    delete[] m_trackUsed;
    delete[] static_cast<int*>(m_trackData);
    m_trackUsed = new unsigned char[count];
    std::memset(m_trackUsed, 0, count);
    m_trackData = new int[count * 2];
    std::memset(m_trackData, 0, (count - 1) * 8 + 1);
    m_streamingTrackIndex = -1;
    BuildTrackList(a, b, false);
    BuildTrackList(c, d, false);
    BuildTrackList(e, f, false);
    BuildTrackList(g, h, false);
    BuildTrackList(i, j, false);
    BuildTrackList(k, l, false);
    BuildTrackList(m, n, false);
    BuildTrackList(o, p, false);
    BuildTrackList(q, r, true);
}

int SoundEngine::GetNextTrack(int first, int last)
{
    if (first > last || first < 0 || last >= m_streamingSoundIds[18] || m_trackUsed == 0)
        return -1;
    int selected = first;
    while (selected <= last && m_trackUsed[selected] != 0)
        ++selected;
    if (selected > last)
        selected = first;
    m_trackUsed[selected] = 1;
    return m_streamingSoundIds[selected];
}

void SoundEngine::SetMusicID(eMusicType type)
{
    (void)type;
    m_musicTrack = GetNextTrack(0, m_streamingSoundIds[18] - 1);
}

void SoundEngine::destroy()
{
}

void SoundEngine::shutdown()
{
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
    } data;

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

    PlatformSoundEngine* platform = static_cast<PlatformSoundEngine*>(m_platformSoundEngine);

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

unsigned char SoundEngine::GetMusicToPlay()
{
    static const unsigned char kMiniGameMusicById[] = {0, 6, 7, 8, 9};

    Minecraft* minecraft = Minecraft::GetInstance();
    bool noPlayers = true;
    bool anyAlive = false;
    bool anyDead = false;
    int selectedSlot = 0;

    for (int i = 0; i < 4; ++i)
    {
        boost::shared_ptr<MultiplayerLocalPlayer>& slot = musicPlayerSlot(minecraft, i);
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
        return 4;
    if (anyDead)
        return 0;
    if (noPlayers)
    {
        if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(31)) != 0)
            return 6;
        return 2;
    }

    boost::shared_ptr<MultiplayerLocalPlayer> selected = musicPlayerSlot(minecraft, selectedSlot);
    MultiplayerLocalPlayer* player = selected.get();
    if (player == 0 || playerByteAt(player, 0x70f) == 0 || playerByteAt(player, 0x70e) == 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        unsigned int id = miniGame->GetId();
        if (id == 0 || id > 4)
            return 10;
        return kMiniGameMusicById[id];
    }

    return 5;
}

void SoundEngine::SetMusicID(const std::wstring& name)
{
    static const wchar_t* kRecordNames[] = {
        L"11", L"13", L"blocks", L"cat", L"chirp", L"far",
        L"mall", L"mellohi", L"stal", L"strad", L"where_are_we_now", L"ward",
    };

    char* convertedPath = ConvertSoundPathToName(name, true);
    const int baseTrack = m_streamingSoundIds[18];
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
    if (sound != NULL || !waitForMusicStartEvent || m_musicEventStarted == 0 ||
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
            m_musicEventStarted = 0;
            if (delayType == ConsoleSoundEngine::eAudioDelayType_None)
                m_musicDelayTicks = 0;
            else if (delayType == ConsoleSoundEngine::eAudioDelayType_Random)
                m_musicDelayTicks = m_random->nextInt(0xe10) + 600;
            else if (delayType > ConsoleSoundEngine::eAudioDelayType_ShortRound)
            {
                m_musicActive = checkPosition ? 1 : 0;
                SetMusicID(static_cast<eMusicType>(GetMusicToPlay()));
                return;
            }
            else
                m_musicDelayTicks = 0x3c;

            m_musicActive = checkPosition ? 1 : 0;
            SetMusicID(static_cast<eMusicType>(GetMusicToPlay()));
            return;
        }

        m_musicEventStarted = 1;
        SetMusicID(sound->getName());
        m_musicDelayTicks = 0;
    }
}

void SoundEngine::playUI(const SoundEvent* sound, float volume, float pitch)
{
    if (sound != 0)
        play(sound, 0.0f, 0.0f, 0.0f, volume, pitch, 0);
}

void SoundEngine::updateMusicVolume(float volume)
{
    setMasterMusicVolume(volume);
}

unsigned int SoundEngine::OpenStreamThreadProc(void* value)
{
    SetSpecialThreadIDForFS_MilesStreamThread__Fv();
    SoundEngine* engine = static_cast<SoundEngine*>(value);
    PlatformSoundEngine* platform = static_cast<PlatformSoundEngine*>(engine->m_platformSoundEngine);
    const bool started = platform->startStream(reinterpret_cast<char*>(engine) + 0x188, ConfigureSound);
    return started ? 0U : static_cast<unsigned int>(-1);
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
    MilesSoundBuffer* buffer = static_cast<MilesSoundBuffer*>(
        hasInstance ? soundData : reinterpret_cast<void*>(&engine->m_streamingX));

    SoundInstance* instance = buffer->instance;
    if (instance != 0)
    {
        if (soundType == 0)
            instance->setSIState(SoundInstance::eSoundState_Starting);
        else if (soundType == 1)
        {
            instance->setSIState(SoundInstance::eSoundState_Playing);
            buffer->x = instance->getX();
            buffer->y = instance->getY();
            buffer->z = instance->getZ();
            buffer->distance = instance->getVolume();
            buffer->pitch = instance->getPitch();
        }
        else if (soundType == 2)
            instance->setSIState(SoundInstance::eSoundState_Complete);
    }

    double distance = buffer->distance;
    const double minDistance = 1.0;
    if (distance > minDistance)
    {
        buffer->distance = static_cast<float>(minDistance);
        distance = minDistance;
    }

    ConsoleSoundEngine* consoleEngine = static_cast<ConsoleSoundEngine*>(engine);
    const double masterVolume = hasInstance ? consoleEngine->getMasterSoundEffectVolume()
                                            : consoleEngine->getMasterMusicVolume();
    *outVolume = static_cast<float>(distance * masterVolume);
    *is3DOut = buffer->is3D != 0;

    if (buffer->is3D == 0)
        return;

    const int listenerMode = *reinterpret_cast<const int*>(&engine->m_listenerDistanceScale);
    if (listenerMode < 2)
    {
        *outX = buffer->x;
        *outY = buffer->y;
        *outZ = -buffer->z;
        return;
    }

    const float defaultCoord = 0.0f;
    float bestSum = 10000.0f;
    float bestDx = defaultCoord;
    float bestDy = defaultCoord;
    float bestDz = defaultCoord;

    if (engine->m_listenerEnabled != 0)
    {
        const float dx = absDelta(engine->m_listenerX, buffer->x);
        const float dy = absDelta(engine->m_listenerY, buffer->y);
        const float dz = absDelta(engine->m_listenerZ, buffer->z);
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
        const float dx = absDelta(engine->m_musicX, buffer->x);
        const float dy = absDelta(engine->m_musicY, buffer->y);
        const float dz = absDelta(engine->m_musicZ, buffer->z);
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
        const float dx = absDelta(engine->m_thirdX, buffer->x);
        const float dy = absDelta(engine->m_thirdY, buffer->y);
        const float dz = absDelta(engine->m_thirdZ, buffer->z);
        const float sum = dx + dy + dz;
        if (sum < bestSum)
        {
            bestSum = sum;
            bestDx = dx;
            bestDy = dy;
            bestDz = dz;
        }
    }

    if (soundEngineByteAt(engine, 0x104) != 0)
    {
        const float dx = absDelta(soundEngineFloatAt(engine, 0x108), buffer->x);
        const float dy = absDelta(soundEngineFloatAt(engine, 0x10c), buffer->y);
        const float dz = absDelta(soundEngineFloatAt(engine, 0x110), buffer->z);
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

void SoundEngine::ConfigureSound(void*, void*, bool*, float* distance, float*, float*, float*, float*, float*)
{
    if (distance != 0)
        *distance = 1.0f;
}

void SoundEngine::playMusicUpdate()
{
    updateMusicVolume(getMasterMusicVolume());
}

void SoundEngine::playMusicTick()
{
    playMusicUpdate();
}

void SoundEngine::init(Options*)
{
    PlatformSoundEngine* platform = static_cast<PlatformSoundEngine*>(m_platformSoundEngine);
    platform->init(0, 0);
    *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x184) = 0;
}

bool SoundEngine::RouteToDRC(void*, void* value)
{
    return value != 0;
}

void SoundEngine::updateMiles()
{
    updateSystemMusicPlaying(GetMusicToPlay());
}

void SoundEngine::tick(boost::shared_ptr<MultiplayerLocalPlayer>*, float)
{
    ConsoleSoundEngine::tick();
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

bool SoundEngine::isStreamingWavebankReady()
{
    return m_platformSoundEngine != 0;
}
