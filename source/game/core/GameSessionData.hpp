#ifndef GAME_CORE_GAMESESSIONDATA_HPP
#define GAME_CORE_GAMESESSIONDATA_HPP

#include <string.h>

class GameSessionData
{
public:
    enum
    {
        MAX_SESSION_PLAYERS = 8,
        MAX_GAMERTAG_LENGTH = 21
    };

    class Gamertag
    {
    public:
        char m_name[MAX_GAMERTAG_LENGTH];
    };

    GameSessionData();

    unsigned short m_version;
    Gamertag m_hostGamertag;
    Gamertag m_playerGamertags[MAX_SESSION_PLAYERS];
    unsigned int m_gameHostOptions;
    unsigned int m_texturePackParentId;
    unsigned char m_subTexturePackId;
    bool m_bPublicGame;
    unsigned char m_numPlayers;
    bool m_bGameStarted;
    unsigned int m_gameModeId : 4;
    unsigned int m_miniGameId : 4;
    unsigned int m_customGameModeHostOptions;
};

#endif
