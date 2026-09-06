#include "game/world/block/WaterlilyBlock.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

WaterlilyBlock::WaterlilyBlock()
    : Bush()
{
    DerivedInit();
}

WaterlilyBlock::~WaterlilyBlock()
{
}

void WaterlilyBlock::addCollisionAABBs(const BlockState*, Level*, const BlockPos& pos,
                                       const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                       boost::shared_ptr<Entity> entity, bool)
{
    if (entity && entity->GetType() != ENTITY_TYPE_BOAT)
    {
        AABB emptyBox;
        Block::addCollisionAABB(pos, entityBox, collidingBoxes, &emptyBox);
    }
}

void WaterlilyBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                                  const boost::shared_ptr<Entity>& entity)
{
    Block::entityInside(level, pos, state, entity);
    if (entity && entity->GetType() == ENTITY_TYPE_BOAT)
    {
        level->destroyBlock(pos, true);
    }
}

int WaterlilyBlock::getColor() const
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Tile_WaterLily);
}

int WaterlilyBlock::getColor(const BlockState*)
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Tile_WaterLily);
}

int WaterlilyBlock::getColor(LevelSource*, const BlockPos&)
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Tile_WaterLily);
}

int WaterlilyBlock::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return Block::getColor(levelSource, pos, state);
}

const AABB* WaterlilyBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

bool WaterlilyBlock::mayPlaceOn(Level*, const BlockPos&, const BlockState* state)
{
    Block* block = state->getBlock();
    return block == Blocks::WATER || block == Blocks::frosted_ice;
}

bool WaterlilyBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    if (pos.y > 255)
    {
        return false;
    }

    BlockPos below = pos.below();
    const BlockState* belowState = level->getBlockState(below);
    Block* belowBlock = belowState->getBlock();
    if (belowBlock == Blocks::WATER)
    {
        return true;
    }
    return belowBlock == Blocks::frosted_ice;
}

int WaterlilyBlock::convertBlockStateToLegacyData(const BlockState*)
{
    return 0;
}

bool WaterlilyBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

EBlockRenderShape WaterlilyBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(0x17);
}
