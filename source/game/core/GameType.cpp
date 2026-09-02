#include "game/core/GameType.hpp"

#include "game/core/Abilities.hpp"

const GameType* GameType::NOT_SET;
const GameType* GameType::SURVIVAL;
const GameType* GameType::CREATIVE;
const GameType* GameType::ADVENTURE;
const GameType* GameType::SPECTATOR;
const GameType* GameType::LOBBY;

GameType::GameType(int id, const std::wstring& name, const std::wstring& shortName,
                   int label, int sliderLabel)
{
    this->id = id;
    this->name = name;
    this->shortName = shortName;
    this->label = label;
    this->sliderLabel = sliderLabel;
}

void GameType::staticCtor()
{
    NOT_SET = new GameType(-1, L"", L"", -1, -1);
    SURVIVAL = new GameType(0, L"survival", L"s", 0x15AA0E94, 0x9BA782A6);
    CREATIVE = new GameType(1, L"creative", L"c", 0xB7C2FCAF, 0x12D696DD);
    ADVENTURE = new GameType(2, L"adventure", L"a", 0xA48E97FA, 0xF9915358);
    SPECTATOR = new GameType(3, L"spectator", L"sp", -1, -1);
    LOBBY = new GameType(4, L"lobby", L"l", -1, -1);
}

int GameType::getId() const
{
    return this->id;
}

int GameType::getLabel() const
{
    return this->label;
}

int GameType::getSliderLabel() const
{
    return this->sliderLabel;
}

bool GameType::isBlockPlacingRestricted() const
{
    return this == ADVENTURE || this == SPECTATOR || this == LOBBY;
}

void GameType::updatePlayerAbilities(Abilities* abilities) const
{
    if (this == CREATIVE)
    {
        abilities->mInvulnerable = true;
        abilities->mInstabuild = true;
        abilities->mMayfly = true;
    }
    else if (this == SPECTATOR)
    {
        abilities->mInstabuild = false;
        abilities->mMayfly = true;
        abilities->mInvulnerable = true;
        abilities->mFlying = true;
    }
    else if (this == LOBBY)
    {
        abilities->mInstabuild = false;
        abilities->mMayfly = false;
        abilities->mFlying = false;
        abilities->mInvulnerable = true;
    }
    else
    {
        abilities->mInstabuild = false;
        abilities->mMayfly = false;
        abilities->mFlying = false;
        abilities->mInvulnerable = false;
    }

    abilities->mMayBuild = !isBlockPlacingRestricted();
}

bool GameType::isCreative() const
{
    return this == CREATIVE || this == LOBBY;
}

bool GameType::isSurvival() const
{
    return this == SURVIVAL || this == ADVENTURE;
}

const GameType* GameType::byId(int id, const GameType* defaultType)
{
    if (id == NOT_SET->id)
    {
        return NOT_SET;
    }

    if (id == SURVIVAL->id)
    {
        return SURVIVAL;
    }

    if (id == CREATIVE->id)
    {
        return CREATIVE;
    }

    if (id == ADVENTURE->id)
    {
        return ADVENTURE;
    }

    if (id == SPECTATOR->id)
    {
        return SPECTATOR;
    }

    if (id == LOBBY->id)
    {
        return LOBBY;
    }

    return defaultType;
}

const GameType* GameType::byId(int id)
{
    return byId(id, SURVIVAL);
}
