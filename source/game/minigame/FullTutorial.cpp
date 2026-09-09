#include "game/minigame/FullTutorial.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/item/PopulateContainerRuleDefinition.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/util/Vec3.hpp"

FullTutorial::FullTutorial(int playerId, GameModeConstraints* constraints, bool enabled)
    : Tutorial(playerId, constraints, true, false), mEnabledForPlayer(enabled),
      mTutorialStarted(false)
{
    int i;
    for (i = 0; i < 44; ++i)
    {
        mStateCompleted[i] = 0;
    }
    addMessage(0x857669aa, false, 3);
}

FullTutorial::~FullTutorial()
{
}

void FullTutorial::AddTutorialArea(eTutorial_State state)
{
    if (gameModeConstraints == NULL || state == eTutorial_State_None)
    {
        return;
    }
    if (gameModeConstraints->HasGlobalConstraint((int)state))
    {
        return;
    }
}

void FullTutorial::PopulateChest(eTutorial_State state)
{
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (ruleset == NULL)
    {
        return;
    }

    GameRuleDefinition* definition = ruleset->getRuleByType(
        static_cast<ConsoleGameRules::EGameRuleType>(30));
    if (definition == NULL)
    {
        return;
    }

    ServerLevel* level = gMinecraftServer->getLevel(0);

    std::vector<GameRuleDefinition*> children;
    definition->getChildren(&children, static_cast<ConsoleGameRules::EGameRuleType>(32));

    for (std::vector<GameRuleDefinition*>::iterator it = children.begin();
         it != children.end(); ++it)
    {
        PopulateContainerRuleDefinition* container =
            static_cast<PopulateContainerRuleDefinition*>(*it);

        int x;
        int y;
        int z;
        container->GetPos(x, y, z);

        if (state == static_cast<eTutorial_State>(-2) ||
            gameModeConstraints->InConstraintArea(Vec3::newTemp(x, y, z),
                                                  static_cast<int>(state)))
        {
            container->evaluateRule(level, ruleset,
                                    *MiniGameDef::GetCustomGameModeById(eMiniGameId_Any, true));
        }
    }
}

void FullTutorial::tick()
{
    std::vector<eTutorial_State>::iterator state = completedStates.begin();
    while (state != completedStates.end())
    {
        if (gameModeConstraints->NotInConstraintArea(playerId, static_cast<int>(*state)))
        {
            mStateCompleted[static_cast<int>(*state)] = 0;
            AddTutorialArea(*state);
            PopulateChest(*state);
            state = completedStates.erase(state);
        }
        else
        {
            ++state;
        }
    }
    Tutorial::tick();
}

unsigned char FullTutorial::isStateCompleted(eTutorial_State state)
{
    return mStateCompleted[static_cast<int>(state)];
}

void FullTutorial::setStateCompleted(eTutorial_State state)
{
    mStateCompleted[static_cast<int>(state)] = 1;
    Tutorial::setStateCompleted(state);
}
