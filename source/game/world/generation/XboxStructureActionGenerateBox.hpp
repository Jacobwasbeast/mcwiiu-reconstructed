#ifndef GAME_WORLD_GENERATION_XBOXSTRUCTUREACTIONGENERATEBOX_HPP
#define GAME_WORLD_GENERATION_XBOXSTRUCTUREACTIONGENERATEBOX_HPP

#include <stdint.h>

#include "game/minigame/GameRuleDefinition.hpp"

class Block;
class BoundingBox;
class Level;
class StructurePiece;

class XboxStructureActionGenerateBox : public GameRuleDefinition
{
public:
    XboxStructureActionGenerateBox();
    virtual ~XboxStructureActionGenerateBox();

    uint32_t generateBoxInLevel(StructurePiece* structurePiece, Level* level, BoundingBox* box);
    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual uint32_t getEndX();
    virtual uint32_t getEndY();
    virtual uint32_t getEndZ();

private:
    int mStartX;
    int mStartY;
    int mStartZ;
    int mEndX;
    int mEndY;
    int mEndZ;
    Block* mOuterBlock;
    Block* mInnerBlock;
    bool mExistingOnly;
};

#endif
