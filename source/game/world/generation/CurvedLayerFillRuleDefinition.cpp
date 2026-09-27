#include "game/world/generation/CurvedLayerFillRuleDefinition.hpp"

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/generation/BowlLayerFill.hpp"

template<typename T>
std::wstring toString(T value);

template<typename T>
T fromString(const std::wstring& text);

CurvedLayerFillRuleDefinition::CurvedLayerFillRuleDefinition()
    : mCurve(0)
    , mOffset(4)
    , mThickness(1)
{
}

CurvedLayerFillRuleDefinition::~CurvedLayerFillRuleDefinition()
{
}

void CurvedLayerFillRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface,
                                                     unsigned int extraAttributeCount)
{
    static const std::wstring curveNames[3] =
    {
        L"Dome",
        L"Bowl",
        L"Dome"
    };

    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 3);
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_type, curveNames[mCurve + 1]);
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_dY, toString(mOffset));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_thickness, toString(mThickness));
}

void CurvedLayerFillRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"type") == 0)
    {
        mCurve = -1;
        if (value.compare(L"Dome") != 0)
        {
            mCurve = 0;
        }

        if (value.compare(L"Bowl") != 0)
        {
            mCurve = 1;
        }

        return;
    }

    if (name.compare(L"dY") == 0)
    {
        mOffset = fromString<int>(value);
        return;
    }

    if (name.compare(L"thickness") == 0)
    {
        mThickness = fromString<int>(value);
        return;
    }

    GameRuleDefinition::addAttribute(name, value);
}

LayerFill* CurvedLayerFillRuleDefinition::Generate() const
{
    float (*curve)(float) = BowlLayerFill::DOME;
    if (mCurve == 0)
    {
        curve = BowlLayerFill::BOWL;
    }

    return new BowlLayerFill(mOffset, curve, mThickness);
}

ConsoleGameRules::EGameRuleType CurvedLayerFillRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_CurvedLayerFill;
}
