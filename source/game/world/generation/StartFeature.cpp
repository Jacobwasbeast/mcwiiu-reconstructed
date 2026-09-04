#include "game/world/generation/StartFeature.hpp"

#include <string>

#include "game/core/Direction.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

template<typename T>
std::wstring _toString(T value);

template<typename T>
T _fromString(const std::wstring& text);

StartFeature::StartFeature()
{
    mChunkX = 0;
    mChunkZ = 0;
    mOrientation = NULL;
    mFeatureType = 2;
}

StartFeature::~StartFeature()
{
}

ConsoleGameRules::EGameRuleType StartFeature::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(9);
}

void StartFeature::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 4);

    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(34), _toString(mChunkX));
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(35), _toString(mChunkZ));
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(45), _toString(mFeatureType));
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(41),
                                 _toString(mOrientation != NULL ? mOrientation : Direction::NORTH));
}

void StartFeature::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"chunkX") == 0)
    {
        mChunkX = _fromString<int>(value);
        return;
    }

    if (name.compare(L"chunkZ") == 0)
    {
        mChunkZ = _fromString<int>(value);
        return;
    }

    if (name.compare(L"orientation") == 0)
    {
        mOrientation = Direction::from2DDataValue(_fromString<int>(value));
        return;
    }

    if (name.compare(L"feature") == 0)
    {
        mFeatureType = _fromString<int>(value);
        return;
    }

    GameRuleDefinition::addAttribute(name, value);
}

bool StartFeature::isFeatureChunk(int chunkX, int chunkZ, StructureFeature::EFeatureTypes featureType,
                                  const Direction** orientation)
{
    if (orientation != NULL)
    {
        *orientation = mOrientation;
    }

    return chunkX == mChunkX && chunkZ == mChunkZ && featureType == mFeatureType;
}

int StartFeature::getFeatureType()
{
    return mFeatureType;
}

int StartFeature::getChunkX()
{
    return mChunkX;
}

int StartFeature::getChunkZ()
{
    return mChunkZ;
}
