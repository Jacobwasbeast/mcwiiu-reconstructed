#include "game/client/audio/SoundEvent.hpp"

SoundEvent::SoundEvent(std::wstring name, EType type, float distanceScalar)
{
    this->name = name;
    this->index = -1;
    this->distanceScalar = distanceScalar;
    this->type = type;
}

SoundEvent::SoundEvent(std::wstring name, float distanceScalar)
{
    this->name = name;
    this->index = -1;
    this->distanceScalar = distanceScalar;
    this->type = eType_Sound;
}

std::wstring SoundEvent::getName() const
{
    return this->name;
}

void SoundEvent::setIndex(int index)
{
    this->index = index;
}

float SoundEvent::getDistanceScalar() const
{
    return this->distanceScalar;
}

bool SoundEvent::isSFX() const
{
    return this->type == eType_SFX;
}

bool SoundEvent::isMusic() const
{
    return this->type == eType_Music;
}

int SoundEvent::getIndex() const
{
    return this->index;
}
