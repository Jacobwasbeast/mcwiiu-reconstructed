#include "game/world/block/EndPortalFrameBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockInWorld.hpp"
#include "game/world/block/BlockPattern.hpp"
#include "game/world/block/BlockPatternBuilder.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BlockStatePredicate.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/item/Item.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

DirectionProperty* EndPortalFrameBlock::FACING = NULL;
BooleanProperty* EndPortalFrameBlock::EYE = NULL;

static const AABB* END_PORTAL_FRAME_SHAPE = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.8125, 1.0);
static const AABB* END_PORTAL_FRAME_EYE_SHAPE = AABB::newPermanent(0.25, 0.8125, 0.25, 0.75, 1.0, 0.75);
static BlockPattern* END_PORTAL_SHAPE = NULL;

static Predicate<BlockInWorld*>* portalFramePredicate(const Direction* facing)
{
    BlockStatePredicate* statePredicate = static_cast<BlockStatePredicate*>(
        BlockStatePredicate::forBlock(Blocks::end_portal_frame));
    statePredicate->where(EndPortalFrameBlock::EYE,
                          new BlockStatePredicate::PredicateConversion<bool>(EndPortalFrameBlock::EYE, true));
    statePredicate->where(EndPortalFrameBlock::FACING,
                          new BlockStatePredicate::PredicateConversion<const Direction*>(
                              EndPortalFrameBlock::FACING, facing));
    return BlockInWorld::hasState(statePredicate);
}

EndPortalFrameBlock::EndPortalFrameBlock()
    : Block(Material::STONE, MaterialColor::STONE),
      mTopIcon(NULL),
      mEyeIcon(NULL)
{
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    TypedBoxed<bool> eye(false);
    registerDefaultState(defaultBlockState()->setBoxedValue(FACING, &facing)->setBoxedValue(EYE, &eye));
}

EndPortalFrameBlock::~EndPortalFrameBlock()
{
}

void EndPortalFrameBlock::blockStaticCtor()
{
    FACING = DirectionProperty::create(L"facing", Direction::Plane::HORIZONTAL);
    EYE = static_cast<BooleanProperty*>(BooleanProperty::create(L"eye"));
}

bool EndPortalFrameBlock::isSolidRender(const BlockState* state) const
{
    return false;
}

const AABB* EndPortalFrameBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return END_PORTAL_FRAME_SHAPE;
}

void EndPortalFrameBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos,
                                            const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                            boost::shared_ptr<Entity> entity, bool isActualState)
{
    if (state->getBlock() == this)
    {
        addCollisionAABB(pos, entityBox, collidingBoxes, END_PORTAL_FRAME_SHAPE);
        if (state->getValue(EYE))
        {
            addCollisionAABB(pos, entityBox, collidingBoxes, END_PORTAL_FRAME_EYE_SHAPE);
        }
    }
}

Item* EndPortalFrameBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Items::air;
}

const BlockState* EndPortalFrameBlock::getStateForPlacement(Level* level, const BlockPos& pos,
                                                             const Direction* direction, float hitX, float hitY,
                                                             float hitZ, int auxValue,
                                                             boost::shared_ptr<LivingEntity> placer)
{
    TypedBoxed<const Direction*> facing(direction->getOpposite());
    TypedBoxed<bool> eye(false);
    return defaultBlockState()->setBoxedValue(FACING, &facing)->setBoxedValue(EYE, &eye);
}

bool EndPortalFrameBlock::hasAnalogOutputSignal(const BlockState* state)
{
    return true;
}

int EndPortalFrameBlock::getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos)
{
    return state->getValue(EYE) ? 15 : 0;
}

const BlockState* EndPortalFrameBlock::getBlockState(int data)
{
    TypedBoxed<bool> eye((data >> 2) & 1);
    TypedBoxed<const Direction*> facing(Direction::from2DDataValue(data & 3));
    return defaultBlockState()->setBoxedValue(EYE, &eye)->setBoxedValue(FACING, &facing);
}

int EndPortalFrameBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(FACING)->get2DDataValue() | (state->getValue(EYE) ? 4 : 0);
}

const BlockState* EndPortalFrameBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}

const BlockState* EndPortalFrameBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}

BlockStateDefinition* EndPortalFrameBlock::createBlockStateDefinition()
{
    const Property* properties[2];
    properties[0] = FACING;
    properties[1] = EYE;
    return new BlockStateDefinition(this, properties, 2);
}

bool EndPortalFrameBlock::isCubeShaped(const BlockState* state)
{
    return false;
}

BlockPattern* EndPortalFrameBlock::getOrCreatePortalShape()
{
    if (END_PORTAL_SHAPE == NULL)
    {
        std::wstring rows[5] = { L"?vvv?", L">???<", L">???<", L">???<", L"?^^^?" };
        END_PORTAL_SHAPE = BlockPatternBuilder::start()->aisle(rows)
            ->where(L'?', Predicates<BlockInWorld*>::alwaysTrue)
            ->where(L'^', portalFramePredicate(Direction::SOUTH))
            ->where(L'>', portalFramePredicate(Direction::WEST))
            ->where(L'v', portalFramePredicate(Direction::NORTH))
            ->where(L'<', portalFramePredicate(Direction::EAST))
            ->build(Direction::SOUTH, Direction::DOWN);
    }
    return END_PORTAL_SHAPE;
}

EBlockFaceShape EndPortalFrameBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state,
                                                        const BlockPos& pos, const Direction* direction)
{
    return direction == Direction::DOWN ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* EndPortalFrameBlock::getEye()
{
    return mEyeIcon;
}

TextureAtlasSprite* EndPortalFrameBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP)
    {
        return mTopIcon;
    }
    if (direction == Direction::DOWN)
    {
        return mIcon;
    }
    return mIcon;
}

void EndPortalFrameBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"endframe_side");
    mTopIcon = iconRegister->registerIcon(L"endframe_top");
    mEyeIcon = iconRegister->registerIcon(L"endframe_eye");
}

EBlockRenderShape EndPortalFrameBlock::getRenderShape(const BlockState* state)
{
    return static_cast<EBlockRenderShape>(26);
}
