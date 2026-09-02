#include "game/core/GameModeOptions.hpp"

static const unsigned int NO_SMALL_SKINS_MASK = 0x00000001;
static const unsigned int NO_SMALL_SKINS_SHIFT = 0;
static const unsigned int CENTRE_SPAWN_MASK = 0x00000004;
static const unsigned int CENTRE_SPAWN_SHIFT = 2;
static const unsigned int ENABLE_VOTING_MASK = 0x00000008;
static const unsigned int ENABLE_VOTING_SHIFT = 3;
static const unsigned int ROUND_LENGTH_MASK = 0x000000F0;
static const unsigned int ROUND_LENGTH_SHIFT = 4;
static const unsigned int MAX_PLAYERS_MASK = 0x00000F00;
static const unsigned int MAX_PLAYERS_SHIFT = 8;
static const unsigned int GAME_VARIANT_MASK = 0x0000F000;
static const unsigned int GAME_VARIANT_SHIFT = 12;
static const unsigned int IS_TEAMS_MASK = 0x00010000;
static const unsigned int IS_TEAMS_SHIFT = 16;
static const unsigned int POINTS_ACTIVE_MASK = 0x00020000;
static const unsigned int POINTS_ACTIVE_SHIFT = 17;
static const unsigned int POWERUPS_ACTIVE_MASK = 0x00040000;
static const unsigned int POWERUPS_ACTIVE_SHIFT = 18;
static const unsigned int TIME_TRIAL_MASK = 0x00080000;
static const unsigned int TIME_TRIAL_SHIFT = 19;
static const unsigned int SPECTATE_MODE_MASK = 0x00F00000;
static const unsigned int SPECTATE_MODE_SHIFT = 20;

static const int HOST_OPTION_NAME_COUNT = 10;

static const std::wstring HOST_OPTION_NAMES[HOST_OPTION_NAME_COUNT] =
{
    L"NoSmallSkins",
    L"SpectateMode",
    L"RoundLength",
    L"MaxPlayers",
    L"CentreSpawn",
    L"PointsActive",
    L"PowerupsActive",
    L"EnableVoting",
    L"IsTeams",
    L"Nothing"
};

GameModeOptions::GameModeOptions()
    : m_uOptions(0)
{
}

GameModeOptions::GameModeOptions(unsigned int uOptions)
    : m_uOptions(uOptions)
{
}

unsigned int GameModeOptions::Get(eCustomGameModeHostOption eOption) const
{
    switch (eOption)
    {
    case eCustomGameModeHostOption_NoSmallSkins:
        return (m_uOptions & NO_SMALL_SKINS_MASK) >> NO_SMALL_SKINS_SHIFT;

    case eCustomGameModeHostOption_SpectateMode:
        return (m_uOptions & SPECTATE_MODE_MASK) >> SPECTATE_MODE_SHIFT;

    case eCustomGameModeHostOption_RoundLength:
        return (m_uOptions & ROUND_LENGTH_MASK) >> ROUND_LENGTH_SHIFT;

    case eCustomGameModeHostOption_MaxPlayers:
        return (m_uOptions & MAX_PLAYERS_MASK) >> MAX_PLAYERS_SHIFT;

    case eCustomGameModeHostOption_CentreSpawn:
        return (m_uOptions & CENTRE_SPAWN_MASK) >> CENTRE_SPAWN_SHIFT;

    case eCustomGameModeHostOption_PointsActive:
        return (m_uOptions & POINTS_ACTIVE_MASK) >> POINTS_ACTIVE_SHIFT;

    case eCustomGameModeHostOption_PowerupsActive:
        return (m_uOptions & POWERUPS_ACTIVE_MASK) >> POWERUPS_ACTIVE_SHIFT;

    case eCustomGameModeHostOption_EnableVoting:
        return (m_uOptions & ENABLE_VOTING_MASK) >> ENABLE_VOTING_SHIFT;

    case eCustomGameModeHostOption_IsTeams:
        return (m_uOptions & IS_TEAMS_MASK) >> IS_TEAMS_SHIFT;

    case eCustomGameModeHostOption_All:
        return m_uOptions;

    case eCustomGameModeHostOption_GameVariant:
        return (m_uOptions & GAME_VARIANT_MASK) >> GAME_VARIANT_SHIFT;

    case eCustomGameModeHostOption_TimeTrial:
        return (m_uOptions & TIME_TRIAL_MASK) >> TIME_TRIAL_SHIFT;

    default:
        break;
    }

    return 0;
}

void GameModeOptions::Set(eCustomGameModeHostOption eOption, unsigned int uValue)
{
    switch (eOption)
    {
    case eCustomGameModeHostOption_NoSmallSkins:
        if (uValue != 0)
        {
            m_uOptions |= NO_SMALL_SKINS_MASK;
        }
        else
        {
            m_uOptions &= ~NO_SMALL_SKINS_MASK;
        }
        break;

    case eCustomGameModeHostOption_SpectateMode:
        m_uOptions = ((uValue << SPECTATE_MODE_SHIFT) & SPECTATE_MODE_MASK) | (m_uOptions & ~SPECTATE_MODE_MASK);
        break;

    case eCustomGameModeHostOption_RoundLength:
        m_uOptions = ((uValue << ROUND_LENGTH_SHIFT) & ROUND_LENGTH_MASK) | (m_uOptions & ~ROUND_LENGTH_MASK);
        break;

    case eCustomGameModeHostOption_MaxPlayers:
        m_uOptions = ((uValue << MAX_PLAYERS_SHIFT) & MAX_PLAYERS_MASK) | (m_uOptions & ~MAX_PLAYERS_MASK);
        break;

    case eCustomGameModeHostOption_CentreSpawn:
        if (uValue != 0)
        {
            m_uOptions |= CENTRE_SPAWN_MASK;
        }
        else
        {
            m_uOptions &= ~CENTRE_SPAWN_MASK;
        }
        break;

    case eCustomGameModeHostOption_PointsActive:
        if (uValue != 0)
        {
            m_uOptions |= POINTS_ACTIVE_MASK;
        }
        else
        {
            m_uOptions &= ~POINTS_ACTIVE_MASK;
        }
        break;

    case eCustomGameModeHostOption_PowerupsActive:
        if (uValue != 0)
        {
            m_uOptions |= POWERUPS_ACTIVE_MASK;
        }
        else
        {
            m_uOptions &= ~POWERUPS_ACTIVE_MASK;
        }
        break;

    case eCustomGameModeHostOption_EnableVoting:
        if (uValue != 0)
        {
            m_uOptions |= ENABLE_VOTING_MASK;
        }
        else
        {
            m_uOptions &= ~ENABLE_VOTING_MASK;
        }
        break;

    case eCustomGameModeHostOption_IsTeams:
        if (uValue != 0)
        {
            m_uOptions |= IS_TEAMS_MASK;
        }
        else
        {
            m_uOptions &= ~IS_TEAMS_MASK;
        }
        break;

    case eCustomGameModeHostOption_All:
        m_uOptions = uValue;
        break;

    case eCustomGameModeHostOption_GameVariant:
        m_uOptions = ((uValue << GAME_VARIANT_SHIFT) & GAME_VARIANT_MASK) | (m_uOptions & ~GAME_VARIANT_MASK);
        break;

    case eCustomGameModeHostOption_TimeTrial:
        if (uValue != 0)
        {
            m_uOptions |= TIME_TRIAL_MASK;
        }
        else
        {
            m_uOptions &= ~TIME_TRIAL_MASK;
        }
        break;

    default:
        break;
    }
}

std::pair<bool, eCustomGameModeHostOption> GameModeOptions::parse(std::wstring sName)
{
    if (sName.at(0) == L'$')
    {
        sName.erase(0, 1);
    }

    for (int nOption = 0; nOption < HOST_OPTION_NAME_COUNT; ++nOption)
    {
        if (sName == HOST_OPTION_NAMES[nOption])
        {
            return std::make_pair(true, static_cast<eCustomGameModeHostOption>(nOption));
        }
    }

    return std::make_pair(false, eCustomGameModeHostOption_NoSmallSkins);
}
