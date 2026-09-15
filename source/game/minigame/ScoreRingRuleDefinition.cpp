#include "game/minigame/ScoreRingRuleDefinition.hpp"

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/util/AABB.hpp"

ScoreRingRuleDefinition::ScoreRingRuleDefinition()
    : mX(0),
      mY(0),
      mZ(0),
      mOrientation(GlideRingGenerator::eGlideRingOrientation_XY),
      mSize(GlideRingGenerator::eGlideRingSize_Large)
{
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(25), &mX);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(28), &mY);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(31), &mZ);
}

ScoreRingRuleDefinition::~ScoreRingRuleDefinition() = default;

ConsoleGameRules::EGameRuleType ScoreRingRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(20);
}

void ScoreRingRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"orientation") == 0)
    {
        if (value.compare(L"y") == 0)
        {
            mOrientation = GlideRingGenerator::eGlideRingOrientation_XZ;
        }
        else if (value.compare(L"z") == 0)
        {
            mOrientation = GlideRingGenerator::eGlideRingOrientation_XY;
        }
        else if (value.compare(L"x") == 0)
        {
            mOrientation = GlideRingGenerator::eGlideRingOrientation_YZ;
        }
    }
    else if (name.compare(L"size") == 0)
    {
        if (value.compare(L"small") == 0)
        {
            mSize = GlideRingGenerator::eGlideRingSize_Small;
        }
        else if (value.compare(L"medium") == 0)
        {
            mSize = GlideRingGenerator::eGlideRingSize_Medium;
        }
        else if (value.compare(L"large") == 0)
        {
            mSize = GlideRingGenerator::eGlideRingSize_Large;
        }
    }
    else
    {
        GameRuleDefinition::addAttribute(name, value);
    }
}

void ScoreRingRuleDefinition::onAttributesAdded()
{
    int radius;

    if (mSize == GlideRingGenerator::eGlideRingSize_Small)
    {
        radius = 1;
    }
    else if (mSize == GlideRingGenerator::eGlideRingSize_Medium)
    {
        radius = 2;
    }
    else
    {
        radius = 3;
    }

    if (mOrientation == GlideRingGenerator::eGlideRingOrientation_XY)
    {
        mArea->mMinX = mX - radius;
        mArea->mMinY = mY - radius;
        mArea->mMinZ = mZ;
        mArea->mMaxX = mX + radius + 1;
        mArea->mMaxY = mY + radius + 1;
        mArea->mMaxZ = mZ + 1;
    }
    else if (mOrientation == GlideRingGenerator::eGlideRingOrientation_XZ)
    {
        mArea->mMinX = mX - radius;
        mArea->mMinY = mY;
        mArea->mMinZ = mZ - radius;
        mArea->mMaxX = mX + radius + 1;
        mArea->mMaxY = mY + 1;
        mArea->mMaxZ = mZ + radius + 1;
    }
    else
    {
        mArea->mMinX = mX;
        mArea->mMinY = mY - radius;
        mArea->mMinZ = mZ - radius;
        mArea->mMaxX = mX + 1;
        mArea->mMaxY = mY + radius + 1;
        mArea->mMaxZ = mZ + radius + 1;
    }
}

int ScoreRingRuleDefinition::getPointValue()
{
    if (mSize == GlideRingGenerator::eGlideRingSize_Small)
    {
        return 7;
    }

    if (mSize == GlideRingGenerator::eGlideRingSize_Medium)
    {
        return 5;
    }

    return 3;
}

int ScoreRingRuleDefinition::getSize()
{
    return mSize;
}
