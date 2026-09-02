#include "game/minigame/TargetAreaRuleDefinition.hpp"

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/BlockPos.hpp"

std::string formatstr(const char* format, ...);

TargetAreaRuleDefinition::TargetAreaRuleDefinition()
    : mPrimary(false),
      mPointValue(0),
      mLaps(0)
{
    addBoolAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(78), &mPrimary);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(79), &mLaps);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(72), &mPointValue);
}

TargetAreaRuleDefinition::~TargetAreaRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType TargetAreaRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(19);
}

int TargetAreaRuleDefinition::getPointValue()
{
    return mPointValue;
}

int TargetAreaRuleDefinition::getSize()
{
    switch (mPointValue)
    {
    case 3:
        return 2;

    case 5:
        return 1;

    case 7:
        return 0;
    }

    return 2;
}

void TargetAreaRuleDefinition::WriteAttributesAsXML(std::string& xml)
{
    if (mPrimary)
    {
        NamedAreaRuleDefinition::WriteAttributesAsXML(xml);

        if (mPrimary)
        {
            WriteAttributeAsXML(static_cast<ConsoleGameRules::EGameRuleAttr>(78), xml);
        }

        if (mLaps != 0)
        {
            WriteAttributeAsXML(static_cast<ConsoleGameRules::EGameRuleAttr>(79), xml);
        }

        return;
    }

    if (!mName.empty())
    {
        WriteAttributeAsXML(static_cast<ConsoleGameRules::EGameRuleAttr>(9), xml);
    }

    BlockPos centre(mArea->getCenter()->x, mArea->getCenter()->y, mArea->getCenter()->z);

    xml.append(formatstr(" x=\"%d\"", centre.x));
    xml.append(formatstr(" y=\"%d\"", centre.y));
    xml.append(formatstr(" z=\"%d\"", centre.z));

    double sizeX = mArea->mMaxX - mArea->mMinX;
    double sizeY = mArea->mMaxY - mArea->mMinY;
    double sizeZ = mArea->mMaxZ - mArea->mMinZ;

    if (sizeX < sizeY && sizeX < sizeZ)
    {
        xml.append(" orientation=\"x\"");
    }
    else if (sizeY < sizeX && sizeY < sizeZ)
    {
        xml.append(" orientation=\"y\"");
    }
    else if (sizeZ < sizeX && sizeZ < sizeY)
    {
        xml.append(" orientation=\"z\"");
    }

    if (getPointValue() == 7)
    {
        xml.append(" size=\"small\"");
    }
    else if (getPointValue() == 5)
    {
        xml.append(" size=\"medium\"");
    }
    else
    {
        xml.append(" size=\"large\"");
    }
}
