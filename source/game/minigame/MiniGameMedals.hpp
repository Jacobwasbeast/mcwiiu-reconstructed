#ifndef GAME_MINIGAME_MINIGAMEMEDALS_HPP
#define GAME_MINIGAME_MINIGAMEMEDALS_HPP

class GameMedal;

class MiniGameMedals
{
public:
    static void staticCtor();
    static const GameMedal* getMedalById(int medalId);
};

#endif
