#include "game/client/audio/AbstractSoundInstance.hpp"

AbstractSoundInstance::AbstractSoundInstance(const SoundEvent* soundEvent, SoundSource::ESoundSource soundSource)
    : source(soundSource)
    , sound(soundEvent)
    , volume(1.0f)
    , pitch(1.0f)
    , looping(false)
    , delay(0)
    , attenuation(SoundInstance::DEFAULT_ATTENUATION)
{
}

AbstractSoundInstance::AbstractSoundInstance(const AbstractSoundInstance& other)
    : SoundInstance(other)
    , source(other.source)
    , sound(other.sound)
    , volume(other.volume)
    , pitch(other.pitch)
    , x(other.x)
    , y(other.y)
    , z(other.z)
    , looping(other.looping)
    , delay(other.delay)
    , attenuation(other.attenuation)
{
}

AbstractSoundInstance::~AbstractSoundInstance()
{
}

const SoundEvent* AbstractSoundInstance::getSound()
{
    return this->sound;
}

SoundSource::ESoundSource AbstractSoundInstance::getSource()
{
    return this->source;
}

bool AbstractSoundInstance::isLooping()
{
    return this->looping;
}

int AbstractSoundInstance::getDelay()
{
    return this->delay;
}

float AbstractSoundInstance::getVolume()
{
    return this->volume;
}

float AbstractSoundInstance::getPitch()
{
    return this->pitch;
}

float AbstractSoundInstance::getX()
{
    return this->x;
}

float AbstractSoundInstance::getY()
{
    return this->y;
}

float AbstractSoundInstance::getZ()
{
    return this->z;
}

SoundInstance::EAttenuation AbstractSoundInstance::getAttenuation()
{
    return this->attenuation;
}
