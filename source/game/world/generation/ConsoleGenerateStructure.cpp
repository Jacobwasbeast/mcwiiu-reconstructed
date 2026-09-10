#include "game/world/generation/ConsoleGenerateStructure.hpp"

#include <string>

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/item/PlaceContainerRuleDefinition.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/XboxStructureActionPlaceBlock.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/XboxStructureActionGenerateBox.hpp"
#include "game/world/generation/XboxStructureActionPlaceSpawner.hpp"

template <typename T>
std::wstring _toString(T value);

template <typename T>
T _fromString(const std::wstring& value);

ConsoleGenerateStructure::ConsoleGenerateStructure()
    : StructurePiece(0)
{
    mX = 0;
    mY = 0;
    mZ = 0;
    mBoundingBox = NULL;
    setOrientation(Direction::NORTH);
    mDimension = 0;
}

ConsoleGenerateStructure::~ConsoleGenerateStructure()
{
}

ConsoleGameRules::EGameRuleType ConsoleGenerateStructure::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(3);
}

void ConsoleGenerateStructure::writeAttributes(GameRuleSaveInterface* saveInterface,
                                                unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 5);
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(25), _toString(mX));
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(28), _toString(mY));
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(31), _toString(mZ));
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(41), _toString(getOrientation()));
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(42), _toString(mDimension));
}

void ConsoleGenerateStructure::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<GameRuleDefinition*>::iterator it = mActions.begin();
         it != mActions.end(); ++it)
    {
        children->push_back(*it);
    }
}

GameRuleDefinition* ConsoleGenerateStructure::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* action = NULL;
    int value = static_cast<int>(type);
    if (value == 4)
    {
        action = new XboxStructureActionGenerateBox();
        mActions.push_back(action);
    }
    else if (value == 5)
    {
        action = new XboxStructureActionPlaceBlock();
        mActions.push_back(action);
    }
    else if (value == 6)
    {
        action = new PlaceContainerRuleDefinition();
        mActions.push_back(action);
    }
    else if (value == 7)
    {
        action = new XboxStructureActionPlaceSpawner();
        mActions.push_back(action);
    }
    return action;
}

void ConsoleGenerateStructure::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name == L"x")
    {
        mX = _fromString<int>(value);
        return;
    }
    if (name == L"y")
    {
        mY = _fromString<int>(value);
        return;
    }
    if (name == L"z")
    {
        mZ = _fromString<int>(value);
        return;
    }
    if (name == L"orientation")
    {
        setOrientation(Direction::from3DDataValue(_fromString<int>(value)));
        return;
    }
    if (name == L"dim")
    {
        int dimension = _fromString<int>(value);
        mDimension = dimension >= -1 && dimension < 2 ? dimension : 0;
        return;
    }
    GameRuleDefinition::addAttribute(name, value);
}

bool ConsoleGenerateStructure::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (level->mDimension->getType() == DimensionType::getById(mDimension))
    {
        for (std::vector<GameRuleDefinition*>::iterator it = mActions.begin();
             it != mActions.end(); ++it)
        {
            GameRuleDefinition* action = *it;
            int actionType = static_cast<int>(action->getActionType());
            if (actionType == 4)
                static_cast<XboxStructureActionGenerateBox*>(action)->generateBoxInLevel(this, level, box);
            else if (actionType == 5)
                static_cast<XboxStructureActionPlaceBlock*>(action)->placeBlockInLevel(this, level, box);
            else if (actionType == 6)
                static_cast<PlaceContainerRuleDefinition*>(action)->placeContainerInChunk(this, level, box);
            else if (actionType == 7)
                static_cast<XboxStructureActionPlaceSpawner*>(action)->placeSpawnerInLevel(this, level, box);
        }
    }

    return false;
}

BoundingBox* ConsoleGenerateStructure::getBoundingBox()
{
    if (StructurePiece::getBoundingBox() == NULL)
    {
        int maxX = 1;
        int maxY = 1;
        int maxZ = 1;
        for (std::vector<GameRuleDefinition*>::iterator it = mActions.begin();
             it != mActions.end(); ++it)
        {
            GameRuleDefinition* action = *it;
            int actionType = static_cast<int>(action->getActionType());
            int endX = 0;
            int endY = 0;
            int endZ = 0;
            if (actionType == 4)
            {
                XboxStructureActionGenerateBox* generateBox = static_cast<XboxStructureActionGenerateBox*>(action);
                endX = static_cast<int>(generateBox->getEndX());
                endY = static_cast<int>(generateBox->getEndY());
                endZ = static_cast<int>(generateBox->getEndZ());
            }
            else if (actionType == 5 || actionType == 6 || actionType == 7)
            {
                XboxStructureActionPlaceBlock* placeBlock = static_cast<XboxStructureActionPlaceBlock*>(action);
                endX = static_cast<int>(placeBlock->getEndX());
                endY = static_cast<int>(placeBlock->getEndY());
                endZ = static_cast<int>(placeBlock->getEndZ());
            }
            if (maxX < endX) maxX = endX;
            if (maxY < endY) maxY = endY;
            if (maxZ < endZ) maxZ = endZ;
        }
        mBoundingBox = new BoundingBox(mX, mY, mZ, mX + maxX, mY + maxY, mZ + maxZ);
    }
    return mBoundingBox;
}

EStructurePiece ConsoleGenerateStructure::GetType()
{
    return static_cast<EStructurePiece>(0);
}

void ConsoleGenerateStructure::addAdditionalSaveData(DataOutputStream* output)
{
}

void ConsoleGenerateStructure::readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager)
{
}

bool ConsoleGenerateStructure::checkIntersects(const BlockPos& min, const BlockPos& max)
{
    return getBoundingBox()->intersects(min, max);
}

int ConsoleGenerateStructure::getMinY()
{
    return getBoundingBox()->y0;
}
