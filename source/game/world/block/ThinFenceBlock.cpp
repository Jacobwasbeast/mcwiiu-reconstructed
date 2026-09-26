#include "game/world/block/ThinFenceBlock.hpp"
#include "game/item/Items.hpp"

#include "game/core/Boxed.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<bool>* THIN_FENCE_NORTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"north"));
AbstractProperty<bool>* THIN_FENCE_EAST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"east"));
AbstractProperty<bool>* THIN_FENCE_SOUTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"south"));
AbstractProperty<bool>* THIN_FENCE_WEST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"west"));

static const AABB* thinFenceShape(int index)
{
    double minX = (index & 2) ? 0.0 : 0.375;
    double maxX = (index & 8) ? 1.0 : 0.625;
    double minZ = (index & 4) ? 0.0 : 0.375;
    double maxZ = (index & 1) ? 1.0 : 0.625;
    return AABB::newPermanent(minX, 0.0, minZ, maxX, 1.0, maxZ);
}

static const AABB* thinFenceShapes[16];

static const AABB* getThinFenceShape(int index)
{
    if (thinFenceShapes[index] == NULL)
        thinFenceShapes[index] = thinFenceShape(index);
    return thinFenceShapes[index];
}

ThinFenceBlock::ThinFenceBlock(const std::wstring& name, const std::wstring& edgeTexture, Material* material, bool extraBlock)
    : Block(material), mDropsOnBreak(extraBlock), mEdgeTexture(edgeTexture), mName(name), mEdgeTextureId(0)
{
    DerivedInit();
    TypedBoxed<bool> north(false);
    TypedBoxed<bool> east(false);
    TypedBoxed<bool> south(false);
    TypedBoxed<bool> west(false);
    const BlockState* state = mStateDefinition->any();
    state = state->setBoxedValue(THIN_FENCE_NORTH, &north);
    state = state->setBoxedValue(THIN_FENCE_EAST, &east);
    state = state->setBoxedValue(THIN_FENCE_SOUTH, &south);
    registerDefaultState(state->setBoxedValue(THIN_FENCE_WEST, &west));
}

ThinFenceBlock::~ThinFenceBlock()
{
}

int ThinFenceBlock::indexFor(const Direction* direction)
{
    return 1 << (direction->get2DDataValue() & 0x3f);
}

void ThinFenceBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                       std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity, bool isActualState)
{
    if (!isActualState)
        state = fillVirtualBlockStateProperties(state, level, pos);
    addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.375, 0.0, 0.375, 0.625, 1.0, 0.625));
    if (state->getValue(THIN_FENCE_NORTH)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.375, 0.0, 0.0, 0.625, 1.0, 0.625));
    if (state->getValue(THIN_FENCE_EAST)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.375, 0.0, 0.375, 1.0, 1.0, 0.625));
    if (state->getValue(THIN_FENCE_SOUTH)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.375, 0.0, 0.375, 0.625, 1.0, 1.0));
    if (state->getValue(THIN_FENCE_WEST)) addCollisionAABB(pos, entityBox, collidingBoxes, AABB::newTemp(0.0, 0.0, 0.375, 0.625, 1.0, 0.625));
}

int ThinFenceBlock::getAABBIndex(const BlockState* state)
{
    int index = 0;
    if (state->getValue(THIN_FENCE_NORTH)) index |= indexFor(Direction::NORTH);
    if (state->getValue(THIN_FENCE_EAST)) index |= indexFor(Direction::EAST);
    if (state->getValue(THIN_FENCE_SOUTH)) index |= indexFor(Direction::SOUTH);
    if (state->getValue(THIN_FENCE_WEST)) index |= indexFor(Direction::WEST);
    return index;
}

const AABB* ThinFenceBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* actual = levelSource == NULL ? state : fillVirtualBlockStateProperties(state, levelSource, pos);
    return getThinFenceShape(getAABBIndex(actual));
}

bool ThinFenceBlock::isExceptionBlockForAttachingPlusPiston(Block* block)
{
    return block == NULL || dynamic_cast<ShulkerBoxBlock*>(block) != NULL
        || dynamic_cast<LeafBlock*>(block) != NULL
        || block == Blocks::beacon || block == Blocks::cauldron
        || block == Blocks::glowstone || block == Blocks::ice
        || block == Blocks::sea_lantern || block == Blocks::piston
        || block == Blocks::sticky_piston || block == Blocks::piston_head
        || block == Blocks::melon_block || block == Blocks::pumpkin
        || block == Blocks::carved_pumpkin || block == Blocks::lit_pumpkin
        || block == Blocks::barrier || block == Blocks::turtle_egg;
}

bool ThinFenceBlock::attachsTo(LevelSource* levelSource, const BlockState* state, const BlockPos& pos, const Direction* direction)
{
    Block* block = state->getBlock();
    int faceShape = state->getBlockFaceShape(levelSource, pos, direction);
    return !isExceptionBlockForAttachingPlusPiston(block) && (faceShape == 0 || faceShape == 3);
}

const BlockState* ThinFenceBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    TypedBoxed<bool> north(attachsTo(levelSource, levelSource->getBlockState(pos.north()), pos.north(), Direction::SOUTH));
    TypedBoxed<bool> south(attachsTo(levelSource, levelSource->getBlockState(pos.south()), pos.south(), Direction::NORTH));
    TypedBoxed<bool> west(attachsTo(levelSource, levelSource->getBlockState(pos.west()), pos.west(), Direction::EAST));
    TypedBoxed<bool> east(attachsTo(levelSource, levelSource->getBlockState(pos.east()), pos.east(), Direction::WEST));
    const BlockState* result = state->setBoxedValue(THIN_FENCE_NORTH, &north);
    result = result->setBoxedValue(THIN_FENCE_SOUTH, &south);
    result = result->setBoxedValue(THIN_FENCE_WEST, &west);
    return result->setBoxedValue(THIN_FENCE_EAST, &east);
}

Item* ThinFenceBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return mDropsOnBreak ? Block::getResource(state, random, lootBonus) : Items::air;
}

bool ThinFenceBlock::isSolidRender(const BlockState*) const { return false; }
bool ThinFenceBlock::isCubeShaped(const BlockState*) { return false; }

bool ThinFenceBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    return levelSource->getBlockState(pos.relative(direction))->getBlock() != this
        && Block::shouldRenderFace(state, levelSource, pos, direction);
}

bool ThinFenceBlock::canContainLiquid(const BlockState*) const { return true; }
bool ThinFenceBlock::isSilkTouchable() { return true; }
int ThinFenceBlock::getRenderLayer() { return 0; }
int ThinFenceBlock::convertBlockStateToLegacyData(const BlockState*) { return 0; }

const BlockState* ThinFenceBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
    {
        TypedBoxed<bool> north(state->getValue(THIN_FENCE_SOUTH));
        TypedBoxed<bool> east(state->getValue(THIN_FENCE_WEST));
        TypedBoxed<bool> south(state->getValue(THIN_FENCE_NORTH));
        TypedBoxed<bool> west(state->getValue(THIN_FENCE_EAST));
        return state->setBoxedValue(THIN_FENCE_NORTH, &north)
            ->setBoxedValue(THIN_FENCE_EAST, &east)
            ->setBoxedValue(THIN_FENCE_SOUTH, &south)
            ->setBoxedValue(THIN_FENCE_WEST, &west);
    }
    case Rotation::eTYPE_CounterClockwise90:
    {
        TypedBoxed<bool> north(state->getValue(THIN_FENCE_EAST));
        TypedBoxed<bool> east(state->getValue(THIN_FENCE_SOUTH));
        TypedBoxed<bool> south(state->getValue(THIN_FENCE_WEST));
        TypedBoxed<bool> west(state->getValue(THIN_FENCE_NORTH));
        return state->setBoxedValue(THIN_FENCE_NORTH, &north)
            ->setBoxedValue(THIN_FENCE_EAST, &east)
            ->setBoxedValue(THIN_FENCE_SOUTH, &south)
            ->setBoxedValue(THIN_FENCE_WEST, &west);
    }
    case Rotation::eTYPE_Clockwise90:
    {
        TypedBoxed<bool> north(state->getValue(THIN_FENCE_WEST));
        TypedBoxed<bool> east(state->getValue(THIN_FENCE_NORTH));
        TypedBoxed<bool> south(state->getValue(THIN_FENCE_EAST));
        TypedBoxed<bool> west(state->getValue(THIN_FENCE_SOUTH));
        return state->setBoxedValue(THIN_FENCE_NORTH, &north)
            ->setBoxedValue(THIN_FENCE_EAST, &east)
            ->setBoxedValue(THIN_FENCE_SOUTH, &south)
            ->setBoxedValue(THIN_FENCE_WEST, &west);
    }
    default:
        return state;
    }
}

const BlockState* ThinFenceBlock::mirror(const BlockState* state, Mirror* mirror)
{
    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
    {
        TypedBoxed<bool> north(state->getValue(THIN_FENCE_SOUTH));
        TypedBoxed<bool> south(state->getValue(THIN_FENCE_NORTH));
        return state->setBoxedValue(THIN_FENCE_NORTH, &north)->setBoxedValue(THIN_FENCE_SOUTH, &south);
    }
    case Mirror::eTYPE_FrontBack:
    {
        TypedBoxed<bool> east(state->getValue(THIN_FENCE_WEST));
        TypedBoxed<bool> west(state->getValue(THIN_FENCE_EAST));
        return state->setBoxedValue(THIN_FENCE_EAST, &east)->setBoxedValue(THIN_FENCE_WEST, &west);
    }
    default:
        return Block::mirror(state, mirror);
    }
}

BlockStateDefinition* ThinFenceBlock::createBlockStateDefinition()
{
    const Property* properties[4] = { THIN_FENCE_NORTH, THIN_FENCE_EAST, THIN_FENCE_SOUTH, THIN_FENCE_WEST };
    return new BlockStateDefinition(this, properties, 4);
}

EBlockFaceShape ThinFenceBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction* direction)
{
    return direction == Direction::DOWN || direction == Direction::UP ? BLOCK_FACE_CENTER_SMALL : BLOCK_FACE_MIDDLE_POLE_THIN;
}

void ThinFenceBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mName);
    mEdgeTextureId = iconRegister->registerIcon(mEdgeTexture);
}

TextureAtlasSprite* ThinFenceBlock::getEdgeTexture() { return mEdgeTextureId; }

EBlockRenderShape ThinFenceBlock::getRenderShape(const BlockState* state)
{
    return mMaterial == Material::GLASS ? BLOCK_RENDER_GLASS_PANE : BLOCK_RENDER_THIN_FENCE;
}
