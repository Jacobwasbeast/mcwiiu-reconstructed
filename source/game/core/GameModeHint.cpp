#include "game/core/GameModeHint.hpp"

GameModeHint::GameModeHint(eTutorial_Hint hint, Tutorial* tutorial,
                           int descriptionId, eTutorial_GameMode* gameModesValue,
                           unsigned int gameModeCountValue, bool allowFade,
                           int minimumDisplayTime)
    : TutorialHint(hint, tutorial, descriptionId, (eHintType)9, true, allowFade,
                   minimumDisplayTime),
      gameModes(gameModesValue), gameModeCount(gameModeCountValue)
{
}

GameModeHint::~GameModeHint()
{
    delete[] gameModes;
    gameModes = NULL;
}

unsigned int GameModeHint::onGameModeChange(eTutorial_GameMode gameMode)
{
    for (unsigned int i = 0; i < gameModeCount; ++i)
    {
        if (gameModes[i] == gameMode)
        {
            return (unsigned int)descriptionId;
        }
    }
    return 0xffffffffU;
}
