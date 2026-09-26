#include "game/world/block/TripWireBlock.hpp"

#include <vector>
#include <typeinfo>
#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Item.hpp"
#include "game/world/generation/Player.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TripWireHookBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<bool>* TripWireBlock::NORTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"north"));
AbstractProperty<bool>* TripWireBlock::EAST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"east"));
AbstractProperty<bool>* TripWireBlock::SOUTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"south"));
AbstractProperty<bool>* TripWireBlock::WEST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"west"));
AbstractProperty<bool>* TripWireBlock::POWERED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"powered"));
AbstractProperty<bool>* TripWireBlock::ATTACHED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"attached"));
AbstractProperty<bool>* TripWireBlock::DISARMED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"disarmed"));
AbstractProperty<bool>* TripWireBlock::SUSPENDED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"suspended"));

static const AABB* TRIPWIRE_UNPOWERED = AABB::newPermanent(0.0, 0.0625, 0.0, 1.0, 0.09375, 1.0);
static const AABB* TRIPWIRE_POWERED = AABB::newPermanent(0.0, 0.0625, 0.0, 1.0, 0.15625, 1.0);

TripWireBlock::TripWireBlock() : Block(Material::DECORATION)
{
    DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<bool> falseValue(false);
    state = state->setBoxedValue(NORTH, &falseValue);
    state = state->setBoxedValue(EAST, &falseValue);
    state = state->setBoxedValue(SOUTH, &falseValue);
    state = state->setBoxedValue(WEST, &falseValue);
    state = state->setBoxedValue(POWERED, &falseValue);
    state = state->setBoxedValue(ATTACHED, &falseValue);
    registerDefaultState(state->setBoxedValue(DISARMED, &falseValue));
    setTicking(true);
}

TripWireBlock::~TripWireBlock() {}

const AABB* TripWireBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* actual = levelSource == NULL ? state : fillVirtualBlockStateProperties(state, levelSource, pos);
    return actual->getValue(POWERED) ? TRIPWIRE_POWERED : TRIPWIRE_UNPOWERED;
}

bool TripWireBlock::shouldConnectTo(LevelSource* levelSource, const BlockPos& pos, const BlockState*, const Direction* direction)
{
    const BlockState* state = levelSource->getBlockState(pos.relative(direction));
    if (state->getBlock() == Blocks::tripwire_hook)
        return state->getValue(static_cast<AbstractProperty<const Direction*>*>(TripWireHookBlock::FACING)) == direction->getOpposite();
    return state->getBlock() == Blocks::tripwire;
}

const BlockState* TripWireBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    TypedBoxed<bool> north(shouldConnectTo(levelSource, pos, state, Direction::NORTH));
    TypedBoxed<bool> east(shouldConnectTo(levelSource, pos, state, Direction::EAST));
    TypedBoxed<bool> south(shouldConnectTo(levelSource, pos, state, Direction::SOUTH));
    TypedBoxed<bool> west(shouldConnectTo(levelSource, pos, state, Direction::WEST));
    const BlockState* result = state->setBoxedValue(NORTH, &north);
    result = result->setBoxedValue(EAST, &east);
    result = result->setBoxedValue(SOUTH, &south);
    return result->setBoxedValue(WEST, &west);
}

const AABB* TripWireBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&) { return NULL; }
bool TripWireBlock::isSolidRender(const BlockState*) const { return false; }
bool TripWireBlock::isCubeShaped(const BlockState*) { return false; }
int TripWireBlock::getRenderLayer() { return 1; }
Item* TripWireBlock::getResource(const BlockState*, Random*, int) { return Items::string; }
not_null_ptr<ItemInstance> TripWireBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*) { return not_null_ptr<ItemInstance>(new ItemInstance(Items::string)); }

void TripWireBlock::updateSource(Level* level, const BlockPos& pos, const BlockState* state)
{
    static const Direction* directions[2] = { Direction::SOUTH, Direction::WEST };
    for (int d = 0; d < 2; ++d)
    {
        const Direction* direction = directions[d];
        for (int i = 1; i < 42; ++i)
        {
            BlockPos testPos = pos.relative(direction, i);
            const BlockState* testState = level->getBlockState(testPos);
            if (testState->getBlock() == Blocks::tripwire_hook)
            {
                if (testState->getValue(TripWireHookBlock::FACING) == direction->getOpposite())
                {
                    static_cast<TripWireHookBlock*>(Blocks::tripwire_hook)
                        ->calculateState(level, testPos, testState, false, true, i, state);
                }
                break;
            }
            if (testState->getBlock() != Blocks::tripwire)
                break;
        }
    }
}

void TripWireBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    level->setBlock(pos, state, Block::UPDATE_ALL, false);
    updateSource(level, pos, state);
}

void TripWireBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    TypedBoxed<bool> powered(true);
    updateSource(level, pos, state->setBoxedValue(POWERED, &powered));
}

bool TripWireBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player)
{
    if (!level->mIsClientSide)
    {
        bool wasShears = false;
        not_null_ptr<ItemInstance> mainHand = player->getMainHandItem();
        if (!mainHand->isEmpty() && mainHand->getItem() == Items::shears)
        {
            wasShears = true;
        }
        if (wasShears)
        {
            TypedBoxed<bool> disarmed(true);
            level->setBlock(pos, state->setBoxedValue(DISARMED, &disarmed), Block::UPDATE_INVISIBLE, false);
        }
    }
    return false;
}

void TripWireBlock::checkPressed(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != this)
        return;

    bool wasPressed = state->getValue(POWERED);
    bool pressed = false;
    std::vector<boost::shared_ptr<Entity> >* entities =
        level->getEntities(boost::shared_ptr<Entity>(), state->getShape(level, pos)->move(pos), NULL);
    if (!entities->empty())
    {
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<Entity> entity = *it;
            if (!entity->isIgnoringBlockTriggers())
            {
                pressed = true;
                break;
            }
        }
    }

    if (pressed != wasPressed)
    {
        TypedBoxed<bool> poweredValue(pressed);
        state = state->setBoxedValue(POWERED, &poweredValue);
        level->setBlock(pos, state, Block::UPDATE_ALL, false);
        updateSource(level, pos, state);
    }
    if (pressed)
        level->addToTickNextTick(pos, this, getTickDelay(level));
}

void TripWireBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state, const boost::shared_ptr<Entity>&)
{
    if (!level->mIsClientSide && !state->getValue(POWERED)) checkPressed(level, pos);
}

void TripWireBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*) {}
void TripWireBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide && state->getValue(POWERED)) checkPressed(level, pos);
}

const BlockState* TripWireBlock::getBlockState(int data)
{
    TypedBoxed<bool> powered((data & 1) != 0);
    TypedBoxed<bool> attached((data & 4) != 0);
    TypedBoxed<bool> disarmed((data & 8) != 0);
    return defaultBlockState()->setBoxedValue(POWERED, &powered)->setBoxedValue(ATTACHED, &attached)->setBoxedValue(DISARMED, &disarmed);
}

int TripWireBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(POWERED) ? 1 : 0;
    if (state->getValue(ATTACHED)) data |= 4;
    if (state->getValue(DISARMED)) data |= 8;
    return data;
}

const BlockState* TripWireBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
    {
        TypedBoxed<bool> north(state->getValue(SOUTH));
        TypedBoxed<bool> east(state->getValue(WEST));
        TypedBoxed<bool> south(state->getValue(NORTH));
        TypedBoxed<bool> west(state->getValue(EAST));
        return state->setBoxedValue(NORTH, &north)
            ->setBoxedValue(EAST, &east)
            ->setBoxedValue(SOUTH, &south)
            ->setBoxedValue(WEST, &west);
    }
    case Rotation::eTYPE_CounterClockwise90:
    {
        TypedBoxed<bool> north(state->getValue(EAST));
        TypedBoxed<bool> east(state->getValue(SOUTH));
        TypedBoxed<bool> south(state->getValue(WEST));
        TypedBoxed<bool> west(state->getValue(NORTH));
        return state->setBoxedValue(NORTH, &north)
            ->setBoxedValue(EAST, &east)
            ->setBoxedValue(SOUTH, &south)
            ->setBoxedValue(WEST, &west);
    }
    case Rotation::eTYPE_Clockwise90:
    {
        TypedBoxed<bool> north(state->getValue(WEST));
        TypedBoxed<bool> east(state->getValue(NORTH));
        TypedBoxed<bool> south(state->getValue(EAST));
        TypedBoxed<bool> west(state->getValue(SOUTH));
        return state->setBoxedValue(NORTH, &north)
            ->setBoxedValue(EAST, &east)
            ->setBoxedValue(SOUTH, &south)
            ->setBoxedValue(WEST, &west);
    }
    default:
        return state;
    }
}

const BlockState* TripWireBlock::mirror(const BlockState* state, Mirror* mirror)
{
    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
    {
        TypedBoxed<bool> north(state->getValue(SOUTH));
        TypedBoxed<bool> south(state->getValue(NORTH));
        return state->setBoxedValue(NORTH, &north)->setBoxedValue(SOUTH, &south);
    }
    case Mirror::eTYPE_FrontBack:
    {
        TypedBoxed<bool> east(state->getValue(WEST));
        TypedBoxed<bool> west(state->getValue(EAST));
        return state->setBoxedValue(EAST, &east)->setBoxedValue(WEST, &west);
    }
    default:
        return Block::mirror(state, mirror);
    }
}

BlockStateDefinition* TripWireBlock::createBlockStateDefinition()
{
    const Property* properties[] = { NORTH, EAST, SOUTH, WEST, POWERED, ATTACHED, DISARMED, SUSPENDED };
    return new BlockStateDefinition(this, properties, 8);
}

EBlockFaceShape TripWireBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*) { return BLOCK_FACE_UNDEFINED; }
EBlockRenderShape TripWireBlock::getRenderShape(const BlockState*) { return BLOCK_RENDER_TRIPWIRE; }
