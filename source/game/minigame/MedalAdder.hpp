#ifndef GAME_MINIGAME_MEDALADDER_HPP
#define GAME_MINIGAME_MEDALADDER_HPP

#include <vector>

class GameMedal;

class MedalAdder
{
public:
    MedalAdder(int* nextMedalId, std::vector<const GameMedal*>* medals, std::vector<const GameMedal*>* allMedals);

    void addMedal(GameMedal* medal);

private:
    int* mNextMedalId;
    std::vector<const GameMedal*>* mMedals;
    std::vector<const GameMedal*>* mAllMedals;
};

#endif
