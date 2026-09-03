#include "game/client/audio/ConsoleSoundEngine.hpp"

#include <algorithm>

ConsoleSoundEngine::ConsoleSoundEngine()
    : isPlayingStreamingCDMusic(false)
    , isPlayingStreamingGameMusic(false)
    , isPlayingEndMusic(false)
    , isPlayingNetherMusic(false)
    , systemMusicPlaying(0)
    , masterMusicVolume(1.0f)
    , masterSoundEffectVolume(1.0f)
    , musicVolumeLimited(false)
    , musicVolumeFading(false)
    , musicVolumeFadingUp(false)
    , unlimitedMusicVolume(100)
    , targetMusicVolume(100)
    , musicVolumeFadeStep(0)
{
    InitializeCriticalSection(&this->criticalSection);
}

ConsoleSoundEngine::~ConsoleSoundEngine()
{
    DeleteCriticalSection(&this->criticalSection);
}

void ConsoleSoundEngine::updateSystemMusicPlaying(bool playing)
{
    this->systemMusicPlaying = playing;
}

float ConsoleSoundEngine::getMasterMusicVolume()
{
    if (this->systemMusicPlaying != 0)
    {
        return 0.0f;
    }
    return this->masterMusicVolume;
}

void ConsoleSoundEngine::setMasterMusicVolume(float volume)
{
    this->masterMusicVolume = volume;
}

float ConsoleSoundEngine::getMasterSoundEffectVolume()
{
    return this->masterSoundEffectVolume;
}

void ConsoleSoundEngine::setMasterSoundEffectVolume(float volume)
{
    this->masterSoundEffectVolume = volume;
}

void ConsoleSoundEngine::LimitMusicVolume()
{
    unsigned int volume = (unsigned int)(this->getMasterMusicVolume() * 100.0f);
    this->unlimitedMusicVolume = volume;
    if (volume > 20)
    {
        this->targetMusicVolume = 20;

        unsigned int step = (volume - 20) / 20;
        if (step == 0)
        {
            step = 1;
        }
        this->musicVolumeFadeStep = step;
        this->musicVolumeFadingUp = false;
        this->musicVolumeFading = true;
    }
}

void ConsoleSoundEngine::RemoveMusicVolumeLimit()
{
    if (!this->musicVolumeLimited && !this->musicVolumeFading)
    {
        return;
    }

    unsigned int volume = (unsigned int)(this->getMasterMusicVolume() * 100.0f);
    unsigned int unlimited = this->unlimitedMusicVolume;
    this->targetMusicVolume = unlimited;
    if (volume != unlimited)
    {
        unsigned int step = (unlimited - volume) / 20;
        if (step == 0)
        {
            step = 1;
        }
        this->musicVolumeFadeStep = step;
        this->musicVolumeFadingUp = true;
        this->musicVolumeFading = true;
        return;
    }

    this->musicVolumeLimited = false;
    this->musicVolumeFading = false;
}

bool ConsoleSoundEngine::GetIsPlayingStreamingCDMusic()
{
    return this->isPlayingStreamingCDMusic;
}

bool ConsoleSoundEngine::GetIsPlayingStreamingGameMusic()
{
    return this->isPlayingStreamingGameMusic;
}

void ConsoleSoundEngine::SetIsPlayingStreamingCDMusic(bool playing)
{
    this->isPlayingStreamingCDMusic = playing;
}

void ConsoleSoundEngine::SetIsPlayingStreamingGameMusic(bool playing)
{
    this->isPlayingStreamingGameMusic = playing;
}

bool ConsoleSoundEngine::GetIsPlayingEndMusic()
{
    return this->isPlayingEndMusic;
}

bool ConsoleSoundEngine::GetIsPlayingNetherMusic()
{
    return this->isPlayingNetherMusic;
}

void ConsoleSoundEngine::SetIsPlayingEndMusic(bool playing)
{
    this->isPlayingEndMusic = playing;
}

void ConsoleSoundEngine::SetIsPlayingNetherMusic(bool playing)
{
    this->isPlayingNetherMusic = playing;
}

void ConsoleSoundEngine::playInstance(SoundInstance* sound)
{
    EnterCriticalSection(&this->criticalSection);

    this->play(sound->getSound(), sound->getX(), sound->getY(), sound->getZ(), sound->getVolume(), sound->getPitch(),
               sound);

    this->playingSoundsStopTime[sound] = this->playTime + 20;
    this->playingSounds[sound] = sound;

    TickableSoundInstance* tickable = dynamic_cast<TickableSoundInstance*>(sound);
    if (tickable != NULL)
    {
        this->tickableSounds.push_back(tickable);
    }

    LeaveCriticalSection(&this->criticalSection);
}

void ConsoleSoundEngine::play(SoundInstance* sound)
{
    this->playInstance(sound);
    this->playInstance(sound->getDRCClone());
}

bool ConsoleSoundEngine::isPlaying(SoundInstance* sound)
{
    boost::unordered_map<SoundInstance*, SoundInstance*>::const_iterator it = this->playingSounds.find(sound);
    return it != this->playingSounds.end();
}

void ConsoleSoundEngine::stop(SoundInstance* sound)
{
}

void ConsoleSoundEngine::stopAll()
{
    std::vector<TickableSoundInstance*>::iterator it;
    for (it = this->tickableSounds.begin(); it != this->tickableSounds.end(); ++it)
    {
        (*it)->stop();
    }
}

void ConsoleSoundEngine::tick()
{
    EnterCriticalSection(&this->criticalSection);

    this->playTime++;

    std::vector<TickableSoundInstance*>::iterator tickableIt;
    for (tickableIt = this->tickableSounds.begin(); tickableIt != this->tickableSounds.end(); ++tickableIt)
    {
        (*tickableIt)->tick();
    }

    boost::unordered_map<SoundInstance*, SoundInstance*>::iterator playingIt = this->playingSounds.begin();
    while (playingIt != this->playingSounds.end())
    {
        SoundInstance* sound = playingIt->second;

        if (sound->isPending())
        {
            if (sound->pendingTicks == 60)
            {
                TickableSoundInstance* pendingTickable = dynamic_cast<TickableSoundInstance*>(sound);
                playingIt = this->playingSounds.erase(playingIt);
                if (pendingTickable != NULL)
                {
                    this->tickableSounds.erase(
                        std::find(this->tickableSounds.begin(), this->tickableSounds.end(), pendingTickable));
                }
                delete sound;
            }
            else
            {
                sound->pendingTicks++;
                ++playingIt;
            }
            continue;
        }

        if (!sound->isComplete())
        {
            ++playingIt;
            continue;
        }

        if (this->playTime < this->playingSoundsStopTime[playingIt->first])
        {
            ++playingIt;
            continue;
        }

        TickableSoundInstance* tickable = dynamic_cast<TickableSoundInstance*>(sound);
        bool looping = sound->isLooping();
        int delay = sound->getDelay();
        SoundInstance* finished = sound;

        if (looping && tickable != NULL)
        {
            if (delay > 0 && !tickable->isStopped())
            {
                this->delayedSounds[sound] = this->playTime + delay;
                finished = NULL;
                playingIt = this->playingSounds.erase(playingIt);
            }
            else if (!tickable->isStopped())
            {
                sound->pendingTicks = 0;
                this->play(sound->getSound(), sound->getX(), sound->getY(), sound->getZ(), sound->getVolume(),
                           sound->getPitch(), sound);
                this->playingSoundsStopTime[sound] = this->playTime + 20;
                ++playingIt;
                continue;
            }
            else
            {
                playingIt = this->playingSounds.erase(playingIt);
            }
        }
        else
        {
            playingIt = this->playingSounds.erase(playingIt);
        }

        if (tickable != NULL)
        {
            this->tickableSounds.erase(
                std::find(this->tickableSounds.begin(), this->tickableSounds.end(), tickable));
        }

        if (finished != NULL)
        {
            delete finished;
        }
    }

    boost::unordered_map<SoundInstance*, long long>::iterator delayedIt = this->delayedSounds.begin();
    while (delayedIt != this->delayedSounds.end())
    {
        if (this->playTime < delayedIt->second)
        {
            ++delayedIt;
            continue;
        }

        SoundInstance* sound = delayedIt->first;
        TickableSoundInstance* tickable = dynamic_cast<TickableSoundInstance*>(sound);
        if (tickable != NULL)
        {
            tickable->tick();
        }
        this->playInstance(sound);
        delayedIt = this->delayedSounds.erase(delayedIt);
    }

    LeaveCriticalSection(&this->criticalSection);
}

void ConsoleSoundEngine::schedule(const SoundEvent* sound, SoundSource::ESoundSource source, float x, float y, float z,
                                  float volume, float pitch, int delay)
{
    EnterCriticalSection(&this->criticalSection);

    ScheduledSound* scheduled = new ScheduledSound(sound, source, x, y, z, volume, pitch, delay);
    this->delayedSounds[scheduled] = this->playTime + delay;
    this->delayedSounds[scheduled->getDRCClone()] = this->playTime + delay;

    LeaveCriticalSection(&this->criticalSection);
}

ConsoleSoundEngine::ScheduledSound::ScheduledSound(const SoundEvent* sound, SoundSource::ESoundSource source, float x,
                                                   float y, float z, float volume, float pitch, int delay)
    : AbstractSoundInstance(sound, source)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->volume = volume;
    this->pitch = pitch;
    this->delay = delay;
}

ConsoleSoundEngine::ScheduledSound::ScheduledSound(const ScheduledSound& other)
    : SoundInstance(other)
    , AbstractSoundInstance(other)
{
}

ConsoleSoundEngine::ScheduledSound::~ScheduledSound()
{
}

SoundInstance* ConsoleSoundEngine::ScheduledSound::getDRCClone() const
{
    ScheduledSound* clone = new ScheduledSound(*this);
    clone->drcClone = true;
    return clone;
}
