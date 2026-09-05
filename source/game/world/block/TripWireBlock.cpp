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
Item* TripWireBlock::getResource(const BlockState*, Random*, int) { return Item::byBlock(this); }
not_null_ptr<ItemInstance> TripWireBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*) { return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this))); }

void TripWireBlock::updateSource(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* directions[2] = { Direction::EAST, Direction::SOUTH };
    for (int d = 0; d < 2; ++d)
    {
        const Direction* direction = directions[d];
        for (int distance = 1; distance <= 42; ++distance)
        {
            BlockPos check = pos.relative(direction, distance);
            const BlockState* next = level->getBlockState(check);
            if (next->getBlock() == Blocks::tripwire_hook)
            {
                if (next->getValue(static_cast<AbstractProperty<const Direction*>*>(TripWireHookBlock::FACING)) == direction->getOpposite())
                    static_cast<TripWireHookBlock*>(next->getBlock())->calculateState(level, check, next, false, true, distance, state);
                break;
            }
            if (next->getBlock() != Blocks::tripwire)
                break;
        }
    }
}

void TripWireBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    level->setBlock(pos, state, 3, false);
    updateSource(level, pos, state);
}

void TripWireBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    TypedBoxed<bool> powered(true);
    updateSource(level, pos, state->setBoxedValue(POWERED, &powered));
}

bool TripWireBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player)
{
    if (!level->mIsClientSide && player)
    {
        TypedBoxed<bool> disarmed(true);
        level->setBlock(pos, state->setBoxedValue(DISARMED, &disarmed), 4, false);
    }
    return false;
}

void TripWireBlock::checkPressed(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    bool pressed = false;
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(Entity), getShape(state, level, pos)->move(pos));
    for (std::vector<boost::shared_ptr<Entity> >::const_iterator it = entities->begin(); it != entities->end(); ++it)
        if ((*it)->isPickable()) { pressed = true; break; }
    if (pressed != state->getValue(POWERED))
    {
        TypedBoxed<bool> value(pressed);
        level->setBlock(pos, state->setBoxedValue(POWERED, &value), 3, false);
        updateSource(level, pos, state);
    }
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

const BlockState* TripWireBlock::rotate(const BlockState* state, Rotation*) { return state; }
const BlockState* TripWireBlock::mirror(const BlockState* state, Mirror*) { return state; }

BlockStateDefinition* TripWireBlock::createBlockStateDefinition()
{
    const Property* properties[] = { NORTH, EAST, SOUTH, WEST, POWERED, ATTACHED, DISARMED, SUSPENDED };
    return new BlockStateDefinition(this, properties, 8);
}

EBlockFaceShape TripWireBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*) { return BLOCK_FACE_UNDEFINED; }
EBlockRenderShape TripWireBlock::getRenderShape(const BlockState*) { return static_cast<EBlockRenderShape>(30); }
