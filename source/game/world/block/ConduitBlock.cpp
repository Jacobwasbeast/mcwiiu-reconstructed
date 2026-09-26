#include "game/world/block/ConduitBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/ConduitBlockEntity.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

static const AABB* CONDUIT_SHAPE = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);

ConduitBlock::ConduitBlock()
    : BaseEntityBlock(Material::CONDUIT, MaterialColor::STONE)
{
    DerivedInit();
    setDestroyTime(3.0f);
}

ConduitBlock::~ConduitBlock()
{
}

boost::shared_ptr<BlockEntity> ConduitBlock::newBlockEntity(Level* level, int data)
{
    boost::shared_ptr<ConduitBlockEntity> entity(new ConduitBlockEntity());
    entity->setData(data);
    return entity;
}

bool ConduitBlock::mayPlace(Level* level, const BlockPos& pos, const Direction* direction)
{
    if (direction == Direction::DOWN && level->getBlock(pos.below())->mMaterial->isSolid())
    {
        return Block::mayPlace(level, pos);
    }
    return false;
}

bool ConduitBlock::isCubeShaped(const BlockState* state)
{
    return false;
}

bool ConduitBlock::isSolidRender(const BlockState* state) const
{
    return false;
}

void ConduitBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                               boost::shared_ptr<LivingEntity> placer,
                               not_null_ptr<ItemInstance> itemInstance)
{
    Block::setPlacedBy(level, pos, state, placer, itemInstance);
}

bool ConduitBlock::hasCustomBreakingProgress(const BlockState* state)
{
    return true;
}

void ConduitBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"conduit_top");
}

EBlockRenderShape ConduitBlock::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_CONDUIT;
}

void ConduitBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos,
                                     const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                     boost::shared_ptr<Entity> entity, bool isActualState)
{
    addCollisionAABB(pos, entityBox, collidingBoxes, CONDUIT_SHAPE);
}

const AABB* ConduitBlock::getShape(const BlockState* state, LevelSource* levelSource,
                                   const BlockPos& pos)
{
    return CONDUIT_SHAPE;
}

bool ConduitBlock::canContainLiquid(const BlockState* state) const
{
    return true;
}
