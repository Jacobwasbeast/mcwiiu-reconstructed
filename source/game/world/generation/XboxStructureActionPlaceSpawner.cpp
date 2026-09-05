#include "game/world/generation/XboxStructureActionPlaceSpawner.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"
#include "game/world/generation/StructurePiece.hpp"

XboxStructureActionPlaceSpawner::XboxStructureActionPlaceSpawner()
{
    mBlock = Blocks::mob_spawner;
    mEntityId.assign(L"Pig", 3);
    addStringAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(20), &mEntityId);
}

XboxStructureActionPlaceSpawner::~XboxStructureActionPlaceSpawner()
{
}

uint32_t XboxStructureActionPlaceSpawner::placeSpawnerInLevel(StructurePiece* structurePiece,
                                                              Level* level, BoundingBox* box)
{
    BlockPos position(structurePiece->getWorldX(static_cast<int>(getEndX()),
                                                 static_cast<int>(getEndZ())),
                      structurePiece->getWorldY(static_cast<int>(getEndY())),
                      structurePiece->getWorldZ(static_cast<int>(getEndX()),
                                                 static_cast<int>(getEndZ())));
    if (!box->isInside(position))
    {
        return 0;
    }

    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(position);
    if (blockEntity)
    {
        level->removeBlockEntity(position);
        level->setBlock(position, Blocks::AIR->defaultBlockState(), 3, false);
    }

    level->setBlock(position, mBlock->defaultBlockState(), 3, false);
    boost::shared_ptr<BlockEntity> placedEntity = level->getBlockEntity(position);
    boost::shared_ptr<MobSpawnerBlockEntity> spawner =
        boost::dynamic_pointer_cast<MobSpawnerBlockEntity>(placedEntity);
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
