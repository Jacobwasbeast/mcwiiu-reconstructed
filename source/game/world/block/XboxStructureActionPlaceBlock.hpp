#ifndef GAME_WORLD_BLOCK_XBOXSTRUCTUREACTIONPLACEBLOCK_HPP
#define GAME_WORLD_BLOCK_XBOXSTRUCTUREACTIONPLACEBLOCK_HPP

#include <stdint.h>

#include "game/minigame/GameRuleDefinition.hpp"

class Block;
class BoundingBox;
class Level;
class StructurePiece;

class XboxStructureActionPlaceBlock : public GameRuleDefinition
{
public:
    XboxStructureActionPlaceBlock();
    virtual ~XboxStructureActionPlaceBlock();

    uint32_t placeBlockInLevel(StructurePiece* structurePiece, Level* level, BoundingBox* box);
    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual uint32_t getEndX();
    virtual uint32_t getEndY();
    virtual uint32_t getEndZ();

private:
    int mEndX;
    int mEndY;
    int mEndZ;
    Block* mBlock;
    int mAuxValue;
};

#endif
