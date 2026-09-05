#include "game/world/block/XboxStructureActionPlaceBlock.hpp"

#include "game/core/BoundingBox.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/generation/StructurePiece.hpp"

XboxStructureActionPlaceBlock::XboxStructureActionPlaceBlock()
{
    mEndX = 0;
    mEndY = 0;
    mEndZ = 0;
    mBlock = NULL;
    mAuxValue = 0;

    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(25), &mEndX);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(28), &mEndY);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(31), &mEndZ);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(18), &mAuxValue);
    addBlockAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(19), &mBlock);
}

XboxStructureActionPlaceBlock::~XboxStructureActionPlaceBlock()
{
}

uint32_t XboxStructureActionPlaceBlock::placeBlockInLevel(StructurePiece* structurePiece, Level* level, BoundingBox* box)
{
    structurePiece->placeBlock(level, mBlock->getBlockState(mAuxValue), mEndX, mEndY, mEndZ, box);
    return 1;
}

ConsoleGameRules::EGameRuleType XboxStructureActionPlaceBlock::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(5);
}

uint32_t XboxStructureActionPlaceBlock::getEndX()
{
    return mEndX;
}

uint32_t XboxStructureActionPlaceBlock::getEndY()
{
    return mEndY;
}

uint32_t XboxStructureActionPlaceBlock::getEndZ()
{
    return mEndZ;
}
