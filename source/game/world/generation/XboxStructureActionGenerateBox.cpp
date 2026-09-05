#include "game/world/generation/XboxStructureActionGenerateBox.hpp"

#include "game/core/BoundingBox.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/StructurePiece.hpp"

XboxStructureActionGenerateBox::XboxStructureActionGenerateBox()
{
    mEndX = 0;
    mEndY = 0;
    mOuterBlock = NULL;
    mStartZ = 0;
    mEndZ = 0;
    mStartX = 0;
    mInnerBlock = NULL;
    mExistingOnly = false;
    mStartY = 0;

    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(26), &mStartX);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(29), &mStartY);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(32), &mStartZ);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(27), &mEndX);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(30), &mEndY);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(33), &mEndZ);
    addBlockAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(22), &mOuterBlock);
    addBlockAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(23), &mInnerBlock);
    addBoolAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(24), &mExistingOnly);
}

XboxStructureActionGenerateBox::~XboxStructureActionGenerateBox()
{
}

uint32_t XboxStructureActionGenerateBox::generateBoxInLevel(StructurePiece* structurePiece, Level* level, BoundingBox* box)
{
    const BlockState* outerState = mOuterBlock->defaultBlockState();
    const BlockState* innerState = mInnerBlock->defaultBlockState();
    structurePiece->generateBox(level, box, mStartX, mStartY, mStartZ, mEndX, mEndY, mEndZ,
                                outerState, innerState, mExistingOnly);
    return 1;
}

ConsoleGameRules::EGameRuleType XboxStructureActionGenerateBox::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(4);
}

uint32_t XboxStructureActionGenerateBox::getEndX()
{
    return mEndX;
}

uint32_t XboxStructureActionGenerateBox::getEndY()
{
    return mEndY;
}

uint32_t XboxStructureActionGenerateBox::getEndZ()
{
    return mEndZ;
}
