#include "game/core/BossEvent.hpp"

BossEvent::BossEvent(const std::wstring& id, int nameId, const std::wstring& name,
                     BossBarColor color, BossBarOverlay overlay)
    : id(id),
      name(name),
      nameId(nameId),
      percent(1.0f),
      color(color),
      overlay(overlay)
{
}

BossEvent::~BossEvent()
{
}

std::wstring BossEvent::getId()
{
    return id;
}

std::wstring BossEvent::getName()
{
    return name;
}

int BossEvent::getNameId()
{
    return nameId;
}

void BossEvent::setName(const std::wstring& name)
{
    this->name = name;
}

float BossEvent::getPercent()
{
    return percent;
}

void BossEvent::setPercent(float percent)
{
    this->percent = percent;
}

BossEvent::BossBarColor BossEvent::getColor()
{
    return color;
}

void BossEvent::setColor(BossBarColor color)
{
    this->color = color;
}

BossEvent::BossBarOverlay BossEvent::getOverlay()
{
    return overlay;
}

void BossEvent::setOverlay(BossBarOverlay overlay)
{
    this->overlay = overlay;
}

bool BossEvent::shouldDarkenScreen()
{
    return darkenScreen;
}

void BossEvent::setDarkenScreen(bool darkenScreen)
{
    this->darkenScreen = darkenScreen;
}

bool BossEvent::shouldPlayBossMusic()
{
    return playBossMusic;
}

void BossEvent::setPlayBossMusic(bool playBossMusic)
{
    this->playBossMusic = playBossMusic;
}

void BossEvent::setCreateWorldFog(bool createWorldFog)
{
    this->createWorldFog = createWorldFog;
}

bool BossEvent::shouldCreateWorldFog()
{
    return createWorldFog;
}
