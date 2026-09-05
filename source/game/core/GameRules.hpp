#ifndef GAME_CORE_GAMERULES_HPP
#define GAME_CORE_GAMERULES_HPP

class GameRules
{
public:
    GameRules();
    ~GameRules();
    bool getBoolean(int rule);
    unsigned int getInt(int rule);
    void set(int rule, bool value);
};

#endif
