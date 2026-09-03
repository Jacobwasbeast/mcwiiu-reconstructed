#include "game/core/GameModeConstraints.hpp"

#include <algorithm>

#include <boost/shared_ptr.hpp>

#include "game/core/ChangeStateConstraint.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

GameModeConstraints::GameModeConstraints(int playerId)
    : playerId(playerId), currentState(0), failedConstraintChanged(false)
{
}

GameModeConstraints::~GameModeConstraints()
{
    for (ConstraintList::iterator it = globalConstraints.begin(); it != globalConstraints.end();
         ++it)
    {
        delete *it;
    }

    for (ConstraintList::iterator it = ownedConstraints.begin(); it != ownedConstraints.end(); ++it)
    {
        delete *it;
    }
}

void GameModeConstraints::tick()
{
    failedConstraintChanged = false;

    for (DelayedConstraintMap::iterator entry = delayedConstraints.begin();
         entry != delayedConstraints.end(); ++entry)
    {
        DelayedConstraintList::iterator delayed = entry->second.begin();
        while (delayed < entry->second.end())
        {
            delayed->second++;

            if (delayed->second < 16)
            {
                ++delayed;
                continue;
            }

            TutorialConstraint* constraint = delayed->first;

            ConstraintList& stateConstraints = constraints[entry->first];
            stateConstraints.erase(
                std::find(stateConstraints.begin(), stateConstraints.end(), constraint));

            constraint->removalPending = false;
            delayed = entry->second.erase(delayed);

            if (constraint->deleteWhenRemoved)
            {
                delete constraint;
            }
        }
    }

    for (ConstraintList::iterator it = globalConstraints.begin(); it < globalConstraints.end(); ++it)
    {
        (*it)->tick(playerId);
    }

    if (failedConstraints.find(currentState) != failedConstraints.end())
    {
        if (failedConstraints[currentState] != NULL
            && !failedConstraints[currentState]->isConstraintSatisfied(playerId))
        {
            return;
        }

        if (failedConstraints[currentState] != NULL
            && failedConstraints[currentState]->isConstraintSatisfied(playerId))
        {
            failedConstraintChanged = true;
            failedConstraints[currentState] = NULL;
        }
    }

    for (ConstraintList::iterator it = constraints[currentState].begin();
         it < constraints[currentState].end(); ++it)
    {
        TutorialConstraint* constraint = *it;

        if (!constraint->isConstraintSatisfied(playerId)
            && constraint->isConstraintRestrictive(playerId))
        {
            failedConstraintChanged = true;
            failedConstraints[currentState] = constraint;
        }
    }
}

void GameModeConstraints::SetCurrentState(int state)
{
    currentState = state;
}

bool GameModeConstraints::HasFailedConstraint(int state)
{
    FailedConstraintMap::iterator it = failedConstraints.find(state);

    return it != failedConstraints.end() && it->second != NULL;
}

TutorialConstraint* GameModeConstraints::GetFailedConstraint(int state)
{
    FailedConstraintMap::iterator it = failedConstraints.find(state);

    if (it != failedConstraints.end())
    {
        return it->second;
    }

    return NULL;
}

void GameModeConstraints::MoveDelayedConstraints(int oldState, int newState)
{
    DelayedConstraintList::iterator delayed = delayedConstraints[oldState].begin();
    while (delayed != delayedConstraints[oldState].end())
    {
        constraints[newState].push_back(delayed->first);
        delayedConstraints[newState].push_back(std::make_pair(delayed->first, delayed->second));

        constraints[oldState].erase(std::find(constraints[oldState].begin(),
                                              constraints[oldState].end(), delayed->first));

        delayed = delayedConstraints[oldState].erase(delayed);
    }
}

void GameModeConstraints::AddGlobalConstraint(TutorialConstraint* constraint)
{
    globalConstraints.push_back(constraint);
}

void GameModeConstraints::AddConstraint(TutorialConstraint* constraint, bool takeOwnership)
{
    constraints[currentState].push_back(constraint);

    if (takeOwnership)
    {
        ownedConstraints.push_back(constraint);
    }
}

void GameModeConstraints::RemoveConstraint(TutorialConstraint* constraint, bool delayed)
{
    FailedConstraintMap::iterator failed = failedConstraints.find(currentState);
    if (failed != failedConstraints.end() && failed->second == constraint)
    {
        failedConstraints[currentState] = NULL;
    }

    if (constraint->removalPending)
    {
        return;
    }

    if (delayed)
    {
        constraint->removalPending = true;
        delayedConstraints[currentState].push_back(
            std::make_pair(constraint, (unsigned char)0));
        return;
    }

    for (DelayedConstraintList::iterator it = delayedConstraints[currentState].begin();
         it < delayedConstraints[currentState].end(); ++it)
    {
        if (it->first == constraint)
        {
            delayedConstraints[currentState].erase(it);
            break;
        }
    }

    if (std::find(constraints[currentState].begin(), constraints[currentState].end(), constraint)
        != constraints[currentState].end())
    {
        constraints[currentState].erase(std::find(constraints[currentState].begin(),
                                                  constraints[currentState].end(), constraint));
    }

    if (std::find(constraints[0].begin(), constraints[0].end(), constraint)
        != constraints[0].end())
    {
        constraints[0].erase(
            std::find(constraints[0].begin(), constraints[0].end(), constraint));
    }
}

void GameModeConstraints::ClearAllAreaConstraints()
{
    for (unsigned int state = 0; state < constraints.size(); state++)
    {
        ConstraintList::iterator it = constraints[state].begin();
        while (it < constraints[state].end())
        {
            TutorialConstraint* constraint = *it;

            if (constraint->getType() != 1)
            {
                ++it;
                continue;
            }

            if (failedConstraints.find(state) != failedConstraints.end()
                && failedConstraints[state] == constraint)
            {
                failedConstraints[state] = NULL;
            }

            delete constraint;
            it = constraints[state].erase(it);
        }
    }

    ownedConstraints.clear();
}

bool GameModeConstraints::InConstraintArea(Vec3* pos, int state)
{
    for (ConstraintList::iterator it = globalConstraints.begin(); it < globalConstraints.end(); ++it)
    {
        ChangeStateConstraint* constraint = (ChangeStateConstraint*)*it;

        if (constraint->isTargetState(state) && constraint->withinTargetStateArea(pos))
        {
            return true;
        }
    }

    return false;
}

bool GameModeConstraints::NotInConstraintArea(int playerId, int state)
{
    bool inArea = false;

    for (ConstraintList::iterator it = globalConstraints.begin(); it < globalConstraints.end(); ++it)
    {
        ChangeStateConstraint* constraint = (ChangeStateConstraint*)*it;

        if (constraint->isTargetState(state) && !inArea)
        {
            inArea = constraint->withinTargetStateArea(playerId);
        }
    }

    return !inArea;
}

bool GameModeConstraints::HasGlobalConstraint(int state)
{
    for (ConstraintList::iterator it = globalConstraints.begin(); it < globalConstraints.end(); ++it)
    {
        ChangeStateConstraint* constraint = (ChangeStateConstraint*)*it;

        if (constraint->isTargetState(state))
        {
            return true;
        }
    }

    return false;
}

bool GameModeConstraints::canMoveToPosition(double oldX, double oldY, double oldZ, double newX,
                                            double newY, double newZ, Vec3* clampedPosition)
{
    bool canMove = false;

    for (ConstraintList::iterator it = constraints[currentState].begin();
         it < constraints[currentState].end(); ++it)
    {
        TutorialConstraint* constraint = *it;

        if (constraint->getType() == 1 && constraint->isConstraintSatisfied(playerId))
        {
            return false;
        }
    }

    for (ConstraintList::iterator it = constraints[currentState].begin();
         it < constraints[currentState].end(); ++it)
    {
        TutorialConstraint* constraint = *it;

        if (constraint->getType() == 1 && !constraint->isConstraintSatisfied(playerId))
        {
            canMove = constraint->canMoveToPosition(oldX, oldY, oldZ, oldX, oldY, oldZ, NULL, true);

            if (!canMove)
            {
                canMove = constraint->canMoveToPosition(oldX, oldY, oldZ, newX, newY, newZ,
                                                        clampedPosition, false);

                if (canMove)
                {
                    break;
                }
            }
        }
    }

    return canMove;
}

bool GameModeConstraints::CanUsePortal()
{
    return currentState != 13;
}

bool GameModeConstraints::isInputAllowed(int mapping)
{
    if (!Minecraft::GetInstance()->mLocalPlayers[playerId]->isUnderLiquid(Material::WATER)
        && failedConstraints[currentState] == NULL)
    {
        for (ConstraintList::iterator it = constraints[currentState].begin();
             it < constraints[currentState].end(); ++it)
        {
            if ((*it)->isMappingConstrained(playerId, mapping))
            {
                return false;
            }
        }
    }

    return true;
}
