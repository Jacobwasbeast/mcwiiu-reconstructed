#include "game/core/GameType.hpp"
#include "game/core/Abilities.hpp"

const GameType* GameType::NOT_SET = 0;
const GameType* GameType::SURVIVAL = 0;
const GameType* GameType::CREATIVE = 0;
const GameType* GameType::ADVENTURE = 0;
const GameType* GameType::SPECTATOR = 0;
const GameType* GameType::LOBBY = 0;

GameType::GameType(int id, const std::wstring& name, const std::wstring& shortName, int label, int sliderLabel)
    : id(id), name(name), shortName(shortName), label(label), sliderLabel(sliderLabel)
{
}

void GameType::staticCtor()
{
    NOT_SET = new GameType(-1, L"", L"", -1, -1);
    SURVIVAL = new GameType(0, L"survival", L"s", 0x15AA0E94, (int)0x9BA782A6);
    CREATIVE = new GameType(1, L"creative", L"c", (int)0xB7C2FCAF, 0x12D696DD);
    ADVENTURE = new GameType(2, L"adventure", L"a", (int)0xA48E97FA, (int)0xF9915358);
    SPECTATOR = new GameType(3, L"spectator", L"sp", -1, -1);
    LOBBY = new GameType(4, L"lobby", L"l", -1, -1);
}

int GameType::getId() const { return id; }
std::wstring GameType::getName() const { return name; }
int GameType::getLabel() const { return label; }
int GameType::getSliderLabel() const { return sliderLabel; }
bool GameType::isBlockPlacingRestricted() const { return this == ADVENTURE || this == SPECTATOR || this == LOBBY; }

void GameType::updatePlayerAbilities(Abilities* abilities) const
{
    if (this == CREATIVE)
    {
        abilities->mInvulnerable = true;
        abilities->mInstabuild = true;
        abilities->mMayfly = true;
    }
    else
    {
        abilities->mInstabuild = false;
        if (this == SPECTATOR)
        {
            abilities->mMayfly = true;
            abilities->mInvulnerable = true;
            abilities->mFlying = true;
        }
        else
        {
            abilities->mMayfly = false;
            abilities->mFlying = false;
            abilities->mInvulnerable = this == LOBBY;
        }
    }
    abilities->mMayBuild = !isBlockPlacingRestricted();
}

bool GameType::isCreative() const { return this == CREATIVE || this == LOBBY; }
bool GameType::isSurvival() const { return this == SURVIVAL || this == ADVENTURE; }

const GameType* GameType::byId(int id, const GameType* defaultType)
{
    if (id == NOT_SET->id) return NOT_SET;
    if (id == SURVIVAL->id) return SURVIVAL;
    if (id == CREATIVE->id) return CREATIVE;
    if (id == ADVENTURE->id) return ADVENTURE;
    if (id == SPECTATOR->id) return SPECTATOR;
    if (id == LOBBY->id) return LOBBY;
    return defaultType;
}

const GameType* GameType::byId(int id) { return byId(id, SURVIVAL); }

const GameType* GameType::byName(const std::wstring& value, const GameType* defaultType)
{
    if (value == NOT_SET->name) return NOT_SET;
    if (value == SURVIVAL->name) return SURVIVAL;
    if (value == CREATIVE->name) return CREATIVE;
    if (value == ADVENTURE->name) return ADVENTURE;
    if (value == SPECTATOR->name) return SPECTATOR;
    if (value == LOBBY->name) return LOBBY;
    return defaultType;
}

const GameType* GameType::byName(const std::wstring& value) { return byName(value, SURVIVAL); }
