#include "game/minigame/TutorialConstraint.hpp"

#include "game/util/Vec3.hpp"

TutorialConstraint::~TutorialConstraint()
{
}

void TutorialConstraint::tick(int playerId)
{
}

bool TutorialConstraint::isConstraintSatisfied(int playerId)
{
    return true;
}

bool TutorialConstraint::isConstraintRestrictive(int playerId)
{
    return true;
}

bool TutorialConstraint::isMappingConstrained(int playerId, int mapping)
{
    return false;
}

bool TutorialConstraint::isXuiInputConstrained(int xuiInput)
{
    return false;
}

bool TutorialConstraint::canMoveToPosition(double oldX, double oldY, double oldZ, double newX,
                                           double newY, double newZ, Vec3* clampedPosition,
                                           bool positionOnly)
{
    return false;
}
