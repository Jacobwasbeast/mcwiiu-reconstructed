#include "game/minigame/GameModeTutorial.hpp"

#include "game/core/GameModeConstraints.hpp"
#include "game/core/GameModeHint.hpp"

GameModeTutorial::GameModeTutorial(int playerId, GameModeConstraints* constraints, int gameMode)
    : Tutorial(playerId, constraints, false, true)
{
    if (gameMode == 1)
    {
        if (!getCompleted(0x141))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[1];
            modes[0] = (eTutorial_GameMode)0;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x141, this,
                                                         (int)0xe18d25cc, modes, 1, true, -1));
        }
        if (!getCompleted(0x142))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[1];
            modes[0] = (eTutorial_GameMode)0;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x142, this,
                                                         (int)0x7a0c9966, modes, 1, true, -1));
        }
        if (!getCompleted(0x143))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[1];
            modes[0] = (eTutorial_GameMode)9;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x143, this,
                                                         (int)0x1015193c, modes, 1, true, -1));
        }
        if (!getCompleted(0x144))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[1];
            modes[0] = (eTutorial_GameMode)2;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x144, this,
                                                         (int)0xea36abc3, modes, 1, true, -1));
        }
        if (!getCompleted(0x145))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[1];
            modes[0] = (eTutorial_GameMode)3;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x145, this,
                                                         (int)0x79e5667b, modes, 1, true, -1));
        }
        if (!getCompleted(0x146))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[1];
            modes[0] = (eTutorial_GameMode)5;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x146, this,
                                                         (int)0x2653f950, modes, 1, true, -1));
        }
        if (!getCompleted(0x147))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[2];
            modes[0] = (eTutorial_GameMode)6;
            modes[1] = (eTutorial_GameMode)7;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x147, this,
                                                         (int)0xb5800810, modes, 2, true, -1));
        }
        if (!getCompleted(0x148))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[2];
            modes[0] = (eTutorial_GameMode)6;
            modes[1] = (eTutorial_GameMode)7;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x148, this,
                                                         (int)0xe28e0a7c, modes, 2, true, -1));
        }
        if (!getCompleted(0x149))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[1];
            modes[0] = (eTutorial_GameMode)8;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x149, this,
                                                         (int)0xec671466, modes, 1, true, -1));
        }
        if (!getCompleted(0x14d))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[3];
            modes[0] = (eTutorial_GameMode)7;
            modes[1] = (eTutorial_GameMode)1;
            modes[2] = (eTutorial_GameMode)8;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x14d, this,
                                                         (int)0x2272922b, modes, 3, true, -1));
        }
        if (!getCompleted(0x14a))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[3];
            modes[0] = (eTutorial_GameMode)7;
            modes[1] = (eTutorial_GameMode)3;
            modes[2] = (eTutorial_GameMode)8;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x14a, this,
                                                         (int)0x6f523bbe, modes, 3, true, -1));
        }
        if (!getCompleted(0x14b))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[3];
            modes[0] = (eTutorial_GameMode)7;
            modes[1] = (eTutorial_GameMode)3;
            modes[2] = (eTutorial_GameMode)8;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x14b, this,
                                                         (int)0xa85a4173, modes, 3, true, -1));
        }
        if (!getCompleted(0x14c))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[3];
            modes[0] = (eTutorial_GameMode)7;
            modes[1] = (eTutorial_GameMode)3;
            modes[2] = (eTutorial_GameMode)8;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x14c, this,
                                                         (int)0x8257c70a, modes, 3, true, -1));
        }
        if (!getCompleted(0x152))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[1];
            modes[0] = (eTutorial_GameMode)4;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x152, this,
                                                         (int)0x92614a33, modes, 1, true, -1));
        }
        if (!getCompleted(0x154))
        {
            addHint((eTutorial_State)0, new TutorialHint((eTutorial_Hint)0x154, this,
                                                         (int)0x06e12057,
                                                         (TutorialHint::eHintType)2,
                                                         true, true, -1));
        }
        if (!getCompleted(0x155))
        {
            addHint((eTutorial_State)0, new TutorialHint((eTutorial_Hint)0x155, this,
                                                         (int)0x06e12057,
                                                         (TutorialHint::eHintType)2,
                                                         true, true, -1));
        }
        if (!getCompleted(0x156))
        {
            addHint((eTutorial_State)0, new TutorialHint((eTutorial_Hint)0x156, this,
                                                         (int)0x06e12057,
                                                         (TutorialHint::eHintType)2,
                                                         true, true, -1));
        }
    }
    else if (gameMode == 2)
    {
        if (!getCompleted(0x158))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[1];
            modes[0] = (eTutorial_GameMode)0;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x158, this,
                                                         (int)0xe18d25cc, modes, 1, true, -1));
        }
        if (!getCompleted(0x159))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[1];
            modes[0] = (eTutorial_GameMode)9;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x159, this,
                                                         (int)0x1015193c, modes, 1, true, -1));
        }
        if (!getCompleted(0x15a))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[2];
            modes[0] = (eTutorial_GameMode)6;
            modes[1] = (eTutorial_GameMode)7;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x15a, this,
                                                         (int)0xb5800810, modes, 2, true, -1));
        }
        if (!getCompleted(0x15b))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[2];
            modes[0] = (eTutorial_GameMode)6;
            modes[1] = (eTutorial_GameMode)7;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x15b, this,
                                                         (int)0xe28e0a7c, modes, 2, true, -1));
        }
        if (!getCompleted(0x15c))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[1];
            modes[0] = (eTutorial_GameMode)8;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x15c, this,
                                                         (int)0xec671466, modes, 1, true, -1));
        }
        if (!getCompleted(0x15d))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[3];
            modes[0] = (eTutorial_GameMode)7;
            modes[1] = (eTutorial_GameMode)3;
            modes[2] = (eTutorial_GameMode)8;
            addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x15d, this,
                                                         (int)0xa086b6b3, modes, 3, true, -1));
        }
        eTutorial_GameMode* modes = new eTutorial_GameMode[1];
        modes[0] = (eTutorial_GameMode)10;
        addHint((eTutorial_State)0, new GameModeHint((eTutorial_Hint)0x2c, this,
                                                     (int)0xd3c3dc8f, modes, 1, false, 15000));
    }
}

GameModeTutorial::~GameModeTutorial()
{
}

unsigned char GameModeTutorial::isStateCompleted(eTutorial_State state)
{
    return stateCompleted[(int)state];
}

void GameModeTutorial::setStateCompleted(eTutorial_State state)
{
    stateCompleted[(int)state] = 1;
    Tutorial::setStateCompleted(state);
}

void GameModeTutorial::changeTutorialState(eTutorial_State state, UIScene* scene)
{
    gameModeConstraints->SetCurrentState(0);
}
