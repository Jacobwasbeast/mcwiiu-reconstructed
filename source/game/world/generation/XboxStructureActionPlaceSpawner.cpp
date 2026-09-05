#include "game/world/generation/XboxStructureActionPlaceSpawner.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"
#include "game/world/generation/StructurePiece.hpp"

XboxStructureActionPlaceSpawner::XboxStructureActionPlaceSpawner()
{
    mEntityId.assign(L"Pig");
    addStringAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(20), &mEntityId);
}

XboxStructureActionPlaceSpawner::~XboxStructureActionPlaceSpawner()
{
}

uint32_t XboxStructureActionPlaceSpawner::placeSpawnerInLevel(StructurePiece* structurePiece, Level* level,
    BoundingBox* box)
{
    BlockPos pos(structurePiece->getWorldX(mEndX, mEndZ), structurePiece->getWorldY(mEndY),
        structurePiece->getWorldZ(mEndX, mEndZ));
    if (!box->isInside(pos))
    {
        return 0;
    }

    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity)
    {
        level->removeBlockEntity(pos);
        level->setBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
    }

    level->setBlock(pos, mBlock->defaultBlockState(), 3, false);
    boost::shared_ptr<BlockEntity> newBlockEntity = level->getBlockEntity(pos);
    boost::shared_ptr<MobSpawnerBlockEntity> spawner =
        boost::dynamic_pointer_cast<MobSpawnerBlockEntity>(newBlockEntity);
    if (spawner)
    {
        spawner->setEntityId(mEntityId);
    }

    return 1;
}

ConsoleGameRules::EGameRuleType XboxStructureActionPlaceSpawner::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(7);
}
