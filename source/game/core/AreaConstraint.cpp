#include "game/core/AreaConstraint.hpp"

#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

AreaConstraint::AreaConstraint(int messageId, double x0, double y0, double z0, double x1, double y1,
                               double z1, bool insideArea, bool constrainHorizontally,
                               bool constrainVertically, int border)
    : TutorialConstraint(messageId)
{
    innerArea = AABB::newPermanent(x0 + border, y0 + border, z0 + border, x1 - border, y1 - border,
                                   z1 - border);
    area = AABB::newPermanent(x0, y0, z0, x1, y1, z1);
    this->insideArea = insideArea;
    this->constrainHorizontally = constrainHorizontally;
    this->constrainVertically = constrainVertically;
}

AreaConstraint::~AreaConstraint()
{
    delete innerArea;
    delete area;
}

int AreaConstraint::getType()
{
    return 1;
}

bool AreaConstraint::isConstraintSatisfied(int playerId)
{
    Vec3* pos = Minecraft::GetInstance()->mLocalPlayers[playerId]->getPos(1.0f);

    bool withinHorizontally = true;
    bool withinVertically = true;

    if (pos->x <= innerArea->mMinX || innerArea->mMaxX <= pos->x)
    {
        withinHorizontally = false;
    }

    if (pos->z <= innerArea->mMinZ || innerArea->mMaxZ <= pos->z)
    {
        withinHorizontally = false;
    }

    if (pos->y <= innerArea->mMinY || innerArea->mMaxY <= pos->y)
    {
        withinVertically = false;
    }

    if (constrainHorizontally)
    {
        if (withinHorizontally != insideArea)
        {
            return false;
        }

        if (!constrainVertically)
        {
            return true;
        }
    }
    else if (!constrainVertically)
    {
        return false;
    }

    return withinVertically == insideArea;
}

bool AreaConstraint::isConstraintRestrictive(int playerId)
{
    return constrainHorizontally || constrainVertically;
}

int AreaConstraint::canMoveToPosition(double oldX, double oldY, double oldZ, double newX,
                                      double newY, double newZ, Vec3* clampedPosition,
                                      bool positionOnly)
{
    if (!constrainHorizontally && !constrainVertically)
    {
        return 0;
    }

    Vec3* oldPos = Vec3::newTemp(oldX, oldY, oldZ);
    Vec3* newPos = Vec3::newTemp(newX, newY, newZ);

    bool withinX = true;
    bool withinY = true;
    bool withinZ = true;

    if (positionOnly)
    {
        if (newPos->x < area->mMinX || area->mMaxX < newPos->x)
        {
            withinX = false;
        }

        if (newPos->y < area->mMinY || area->mMaxY < newPos->y)
        {
            withinY = false;
        }

        if (newPos->z < area->mMinZ || area->mMaxZ < newPos->z)
        {
            withinZ = false;
        }
    }
    else
    {
        if ((newPos->x <= area->mMinX && newPos->x < oldPos->x) ||
            (area->mMaxX <= newPos->x && oldPos->x < newPos->x))
        {
            withinX = false;
        }

        if ((newPos->y <= area->mMinY && newPos->y < oldPos->y) ||
            (area->mMaxY <= newPos->y && oldPos->y < newPos->y))
        {
            withinY = false;
        }

        if ((newPos->z <= area->mMinZ && newPos->z < oldPos->z) ||
            (area->mMaxZ <= newPos->z && oldPos->z < newPos->z))
        {
            withinZ = false;
        }
    }

    bool blockedX = constrainHorizontally && withinX != insideArea;
    bool blockedY = constrainVertically && withinY != insideArea;
    bool blockedZ = constrainHorizontally && withinZ != insideArea;

    if (clampedPosition != NULL)
    {
        if (blockedX && !withinX)
        {
            clampedPosition->x = newPos->x < area->mMaxX ? area->mMinX : area->mMaxX;
        }

        if (blockedY && !withinY)
        {
            clampedPosition->y = newPos->y < area->mMaxY ? area->mMinY : area->mMaxY;
        }

        if (blockedZ && !withinZ)
        {
            clampedPosition->z = newPos->z < area->mMaxZ ? area->mMinZ : area->mMaxZ;
        }
    }

    int blockedAxes = 0;

    if (blockedX)
    {
        blockedAxes |= 1;
    }

    if (blockedY)
    {
        blockedAxes |= 2;
    }

    if (blockedZ)
    {
        blockedAxes |= 4;
    }

    return blockedAxes;
}
