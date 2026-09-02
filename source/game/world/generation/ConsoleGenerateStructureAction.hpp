#ifndef GAME_WORLD_GENERATION_CONSOLEGENERATESTRUCTUREACTION_HPP
#define GAME_WORLD_GENERATION_CONSOLEGENERATESTRUCTUREACTION_HPP

#include "game/minigame/GameRuleDefinition.hpp"

class ConsoleGenerateStructureAction : public GameRuleDefinition
{
public:
    virtual ~ConsoleGenerateStructureAction();

    virtual int getEndX() = 0;
    virtual int getEndY() = 0;
    virtual int getEndZ() = 0;
};

#endif
