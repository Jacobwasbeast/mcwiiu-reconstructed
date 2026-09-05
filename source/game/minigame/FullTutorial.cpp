#include "game/minigame/FullTutorial.hpp"

#include "game/core/GameModeConstraints.hpp"
#include "game/item/PopulateContainerRuleDefinition.hpp"
#include "game/util/Vec3.hpp"

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
    if (gameModeConstraints == NULL)
    {
        return;
    }
    if (state == (eTutorial_State)-2)
    {
        return;
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
