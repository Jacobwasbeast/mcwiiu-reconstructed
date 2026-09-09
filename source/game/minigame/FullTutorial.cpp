#include "game/minigame/FullTutorial.hpp"

#include "game/core/GameModeConstraints.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/item/PopulateContainerRuleDefinition.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/util/Vec3.hpp"

extern MinecraftServer* gMinecraftServer;

FullTutorial::FullTutorial(int playerId, GameModeConstraints* constraints, bool enabled)
    : Tutorial(playerId, constraints, enabled, true), mEnabledForPlayer(enabled),
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
    LevelRuleset* ruleset = g_pApp != NULL ? g_pApp->getGameRuleDefinitions() : NULL;
    if (ruleset == NULL || gMinecraftServer == NULL)
    {
        return;
    }

    GameRuleDefinition* definition = ruleset->getRuleByType(
        static_cast<ConsoleGameRules::EGameRuleType>(30));
    if (definition == NULL)
    {
        return;
    }

    Level* level = gMinecraftServer->getLevel(0);
    MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(
        static_cast<EMiniGameId>(0xf), true);
    if (level == NULL || miniGame == NULL)
    {
        return;
    }

    std::vector<GameRuleDefinition*> children;
    definition->getChildren(&children);
    for (std::vector<GameRuleDefinition*>::iterator it = children.begin();
         it != children.end(); ++it)
    {
        PopulateContainerRuleDefinition* container =
            dynamic_cast<PopulateContainerRuleDefinition*>(*it);
        if (container == NULL)
        {
            continue;
        }

        int x;
        int y;
        int z;
        container->GetPos(x, y, z);
        if (state == static_cast<eTutorial_State>(-2) ||
            (gameModeConstraints != NULL &&
             gameModeConstraints->InConstraintArea(Vec3::newTemp(x, y, z),
                                                   static_cast<int>(state))))
        {
            container->evaluateRule(level, ruleset, *miniGame);
        }
    }
}

void FullTutorial::tick()
{
    std::vector<eTutorial_State>::iterator state = completedStates.begin();
    while (state != completedStates.end())
    {
        if (gameModeConstraints != NULL &&
            gameModeConstraints->NotInConstraintArea(playerId, (int)*state))
        {
            mStateCompleted[(int)*state] = 0;
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
    return mStateCompleted[(int)state];
}

void FullTutorial::setStateCompleted(eTutorial_State state)
{
    mStateCompleted[(int)state] = 1;
    Tutorial::setStateCompleted(state);
}
