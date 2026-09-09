#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include <algorithm>
#include <cstring>
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundInstance.hpp"
#include "game/util/Random.hpp"

SoundEngine::SoundEngine()
    : m_listenerEnabled(0), m_musicListenerEnabled(0), m_thirdListenerEnabled(0), m_random(new Random()), m_musicDistance(0.0f), m_trackCount(0), m_musicEventStarted(0), m_musicVolume(0.0f), m_effectVolume(0.0f), m_musicType(0), m_musicFade(0.0f), m_musicTrack(-1), m_musicState(0), m_currentTrack(-1), m_platformSoundEngine(0), m_platformState(0), m_trackUsed(0), m_trackData(0), m_streamingCount(0)
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
    m_streamingCount = count;
    delete[] m_trackUsed;
    delete[] static_cast<int*>(m_trackData);
    m_trackUsed = count > 0 ? new unsigned char[count] : 0;
    m_trackData = count > 0 ? new int[count * 2] : 0;
    BuildTrackList(0, count - 1, false);
}

int SoundEngine::GetNextTrack(int first, int last)
{
    if (first > last || first < 0 || last >= m_streamingCount || m_trackUsed == 0)
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
    m_musicType = static_cast<int>(type);
    m_musicTrack = GetNextTrack(0, m_streamingCount - 1);
}

void SoundEngine::destroy()
{
}

void SoundEngine::shutdown()
{
    m_platformSoundEngine = 0;
}

void SoundEngine::play(const SoundEvent*, float, float, float, float, float, SoundInstance* instance)
{
    if (instance != 0)
        ConsoleSoundEngine::play(instance);
}

bool SoundEngine::GetMusicToPlay()
{
    return m_musicEventStarted != 0;
}

void SoundEngine::SetMusicID(const std::wstring& name)
{
    m_musicTrack = static_cast<int>(name.size());
}

void SoundEngine::playStreaming(const SoundEvent* sound, float, float, float, float, float, bool, bool, ConsoleSoundEngine::eAudioDelayType)
{
    if (sound != 0)
        m_musicEventStarted = 1;
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
    return value == 0 ? static_cast<unsigned int>(-1) : 0U;
}

double SoundEngine::getDistanceScaler(AudioInfo* info)
{
    double scalar = info->soundEvent->getDistanceScalar();
    if (info->distance == 10000.0f)
        scalar = 10000.0;
    return scalar;
}

void SoundEngine::UpdateSound(void*, void*, bool* active, int, float*, float*, float*, float*)
{
    if (active != 0)
        *active = true;
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
    m_platformSoundEngine = this;
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
    m_musicEventStarted = 0;
}

void SoundEngine::updateSoundEffectVolume(float volume)
{
    setMasterSoundEffectVolume(volume);
}

bool SoundEngine::isStreamingWavebankReady()
{
    return m_platformSoundEngine != 0;
}
