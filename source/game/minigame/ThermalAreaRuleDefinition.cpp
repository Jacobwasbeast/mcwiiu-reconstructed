#include "game/minigame/ThermalAreaRuleDefinition.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/ClientScoreboard.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/StringFormat.hpp"

ThermalAreaRuleDefinition::ThermalAreaRuleDefinition()
{
    mEnabled = true;
    mStaticLift = 0.0;
    mBoostDirection = 4;
    mLiftForceModifier = 1.0;
    mSpeedBoost = 0.0;
    mTargetHeight = 0.0;
    mConditionType = 0;
    mConditionValue1 = 0;
    mConditionValue0 = 0;

    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_liftForceModifier, &mLiftForceModifier);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_staticLift, &mStaticLift);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_targetHeight, &mTargetHeight);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_speedBoost, &mSpeedBoost);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_condition_type, &mConditionType);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_condition_value_0, &mConditionValue0);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_condition_value_1, &mConditionValue1);
    mUseCount = 0;
}

ThermalAreaRuleDefinition::~ThermalAreaRuleDefinition() = default;

bool ThermalAreaRuleDefinition::getConditionsMet(boost::shared_ptr<Entity> entity)
{
    if (mConditionType == 1)
    {
        if (mConditionValue0 <= mUseCount)
        {
            return false;
        }
    }
    else if (mConditionType == 2)
    {
        if ((entity->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
            int points = Minecraft::GetInstance()->mLevel->GetClientScoreboard()->GetPoints(player->getSUID(), -1);
            if (points < mConditionValue0 || mConditionValue1 < points)
            {
                return false;
            }
        }
    }

    return true;
}

void ThermalAreaRuleDefinition::resetConditions()
{
    mUseCount = 0;
}

void ThermalAreaRuleDefinition::updateUseCount()
{
    ++mUseCount;
}

void ThermalAreaRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name == L"boostDirection")
    {
        if (value == L"plus_x")
        {
            mBoostDirection = 0;
            return;
        }
        if (value == L"minus_x")
        {
            mBoostDirection = 1;
            return;
        }
        if (value == L"plus_z")
        {
            mBoostDirection = 2;
            return;
        }
        if (value == L"minus_z")
        {
            mBoostDirection = 3;
            return;
        }
        if (value == L"omni_plus_x")
        {
            mBoostDirection = 4;
            return;
        }
        if (value == L"omni_minus_x")
        {
            mBoostDirection = 5;
            return;
        }
        if (value == L"omni_plus_z")
        {
            mBoostDirection = 6;
            return;
        }
        if (value == L"omni_minus_z")
        {
            mBoostDirection = 7;
            return;
        }
    }
    else
    {
        GameRuleDefinition::addAttribute(name, value);
    }
}

unsigned int ThermalAreaRuleDefinition::setBoostMods(int& boostMod, int& boostDirection)
{
    unsigned int direction = static_cast<unsigned int>(mBoostDirection);
    if (direction == 0)
    {
        boostMod = 1;
        boostDirection = 0;
        return 1;
    }
    if (direction == 1)
    {
        boostMod = -1;
        boostDirection = 0;
        return 1;
    }
    if (direction < 3)
    {
        boostMod = 0;
        boostDirection = 1;
        return 1;
    }
    if (direction != 3)
    {
        return 0;
    }
    boostMod = 0;
    boostDirection = -1;
    return 1;
}

unsigned int ThermalAreaRuleDefinition::getBoostDirection()
{
    return static_cast<unsigned int>(mBoostDirection);
}

void ThermalAreaRuleDefinition::WriteAttributesAsXML(std::string& xml)
{
    if (!mName.empty())
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_name, xml);
    }
    if (mLiftForceModifier != 1.0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_liftForceModifier, xml);
    }
    if (mStaticLift != 0.0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_staticLift, xml);
    }
    if (mTargetHeight != 0.0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_targetHeight, xml);
    }
    if (mSpeedBoost != 0.0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_speedBoost, xml);
        const wchar_t* direction = L"none";
        switch (mBoostDirection)
        {
        case 0:
            direction = L"plus_x";
            break;
        case 1:
            direction = L"minus_x";
            break;
        case 2:
            direction = L"plus_z";
            break;
        case 3:
            direction = L"minus_z";
            break;
        case 4:
            direction = L"omni_plus_x";
            break;
        case 5:
            direction = L"omni_minus_x";
            break;
        case 6:
            direction = L"omni_plus_z";
            break;
        case 7:
            direction = L"omni_minus_z";
            break;
        }
        xml.append(formatstr(" boostDirection=\"%ls\"", direction));
    }
    if (mConditionType != 0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_condition_type, xml);
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_condition_value_0, xml);
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_condition_value_1, xml);
    }
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_x0, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_y0, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_z0, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_x1, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_y1, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_z1, xml);
    if (mDataTag != 0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_dataTag, xml);
    }
}

ConsoleGameRules::EGameRuleType ThermalAreaRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_ThermalArea;
}
