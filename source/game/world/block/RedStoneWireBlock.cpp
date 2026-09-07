#include "game/world/block/RedStoneWireBlock.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BlockPattern.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include <boost/unordered_set.hpp>
#include <typeinfo>

AbstractProperty<int>* RedStoneWireBlock::POWER =
    static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"power", 0, 15));

static RedStoneWireBlock::ConnectingSide* gRedStoneWireUp =
    new RedStoneWireBlock::ConnectingSide(L"up");
static RedStoneWireBlock::ConnectingSide* gRedStoneWireSide =
    new RedStoneWireBlock::ConnectingSide(L"side");
static RedStoneWireBlock::ConnectingSide* gRedStoneWireNone =
    new RedStoneWireBlock::ConnectingSide(L"none");
static RedStoneWireBlock::ConnectingSide* gRedStoneWireConnectingSides[] =
    { gRedStoneWireUp, gRedStoneWireSide, gRedStoneWireNone };

static EnumProperty<RedStoneWireBlock::ConnectingSide*>* gNorthProperty = NULL;
static EnumProperty<RedStoneWireBlock::ConnectingSide*>* gEastProperty = NULL;
static EnumProperty<RedStoneWireBlock::ConnectingSide*>* gSouthProperty = NULL;
static EnumProperty<RedStoneWireBlock::ConnectingSide*>* gWestProperty = NULL;

RedStoneWireBlock::ConnectingSide::ConnectingSide(const std::wstring& name)
    : mName(name), mValue(0)
{
}

std::wstring RedStoneWireBlock::ConnectingSide::toString() const
{
    return mName;
}

std::wstring RedStoneWireBlock::ConnectingSide::getSerializedName() const
{
    return mName;
}

RedStoneWireBlock::RedStoneWireBlock()
    : Block(NULL), mNorthIcon(NULL), mEastIcon(NULL), mSouthIcon(NULL), mWestIcon(NULL), mSignalSource(true)
{
    DerivedInit();
    TypedBoxed<ConnectingSide*> none(gRedStoneWireNone);
    TypedBoxed<int> power(0);
    const BlockState* state = mStateDefinition->any()->setBoxedValue(gNorthProperty, &none);
    state = state->setBoxedValue(gEastProperty, &none);
    state = state->setBoxedValue(gSouthProperty, &none);
    state = state->setBoxedValue(gWestProperty, &none);
    registerDefaultState(state->setBoxedValue(POWER, &power));
}

RedStoneWireBlock::~RedStoneWireBlock()
{
}

void RedStoneWireBlock::blockStaticCtor()
{
    boost::unordered_set<ConnectingSide*> values;
    values.insert(gRedStoneWireConnectingSides[0]);
    values.insert(gRedStoneWireConnectingSides[1]);
    values.insert(gRedStoneWireConnectingSides[2]);
    gNorthProperty = EnumProperty<ConnectingSide*>::create(
        L"north", typeid(ConnectingSide), values, Predicates<ConnectingSide*>::alwaysTrue);
    gEastProperty = EnumProperty<ConnectingSide*>::create(
        L"east", typeid(ConnectingSide), values, Predicates<ConnectingSide*>::alwaysTrue);
    gSouthProperty = EnumProperty<ConnectingSide*>::create(
        L"south", typeid(ConnectingSide), values, Predicates<ConnectingSide*>::alwaysTrue);
    gWestProperty = EnumProperty<ConnectingSide*>::create(
        L"west", typeid(ConnectingSide), values, Predicates<ConnectingSide*>::alwaysTrue);
}

int RedStoneWireBlock::getAABBIndex(const BlockState* state)
{
    int index = 0;
    if (state != NULL)
    {
        if (shouldConnectTo(state, Direction::NORTH)) index |= 1;
        if (shouldConnectTo(state, Direction::EAST)) index |= 2;
        if (shouldConnectTo(state, Direction::SOUTH)) index |= 4;
        if (shouldConnectTo(state, Direction::WEST)) index |= 8;
    }
    return index;
}

const AABB* RedStoneWireBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return getClipAABB(state, levelSource, pos);
}

bool RedStoneWireBlock::shouldConnectTo(const BlockState* state, const Direction* direction)
{
    if (state == NULL)
    {
        return false;
    }
    return state->getBlock() != NULL && direction != NULL;
}

bool RedStoneWireBlock::shouldConnectTo(const BlockState* state)
{
    return state != NULL && state->getBlock() != NULL;
}

RedStoneWireBlock::ConnectingSide* RedStoneWireBlock::getConnectingSide(LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    static ConnectingSide north(L"north");
    static ConnectingSide east(L"east");
    static ConnectingSide south(L"south");
    static ConnectingSide west(L"west");
    if (direction == Direction::EAST) return &east;
    if (direction == Direction::SOUTH) return &south;
    if (direction == Direction::WEST) return &west;
    return &north;
}

const BlockState* RedStoneWireBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return state;
}

const AABB* RedStoneWireBlock::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return NULL;
}

bool RedStoneWireBlock::isSolidRender(const BlockState* state) const
{
    return false;
}

bool RedStoneWireBlock::isCubeShaped(const BlockState* state)
{
    return false;
}

bool RedStoneWireBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return true;
}

bool RedStoneWireBlock::waterSpreadCausesSpawn()
{
    return true;
}

bool RedStoneWireBlock::canContainLiquid(const BlockState* state) const
{
    return false;
}

int RedStoneWireBlock::checkTarget(Level* level, const BlockPos& pos, int strength)
{
    return strength;
}

void RedStoneWireBlock::updatePowerStrength(Level* level, const BlockPos& pos, const BlockPos& sourcePos, const BlockState* state)
{
    mUpdatedPositions.insert(pos);
}

void RedStoneWireBlock::updatePowerStrength(Level* level, const BlockPos& pos, const BlockState* state)
{
    updatePowerStrength(level, pos, pos, state);
}

void RedStoneWireBlock::checkCornerChangeAt(Level* level, const BlockPos& pos)
{
    mUpdatedPositions.insert(pos);
}

void RedStoneWireBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    updatePowerStrength(level, pos, state);
}

void RedStoneWireBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    mUpdatedPositions.erase(pos);
}

void RedStoneWireBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    updatePowerStrength(level, pos, state);
}

Item* RedStoneWireBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Block::getResource(state, random, lootBonus);
}

int RedStoneWireBlock::getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    return mSignalSource && state != NULL ? state->getDirectSignal(levelSource, pos, direction) : 0;
}

bool RedStoneWireBlock::shouldConnectTo(LevelSource* levelSource, const BlockPos& pos)
{
    return levelSource != NULL;
}

bool RedStoneWireBlock::isPowerSourceAt(LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    return shouldConnectTo(levelSource, pos);
}

int RedStoneWireBlock::getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    return mSignalSource && state != NULL ? state->getSignal(levelSource, pos, direction) : 0;
}

bool RedStoneWireBlock::isSignalSource(const BlockState* state)
{
    return mSignalSource;
}

int RedStoneWireBlock::getColorForData(int data)
{
    float value = static_cast<float>(data) / 15.0f;
    int red = Mth::clamp(static_cast<int>((value * 0.6f + 0.4f) * 255.0f), 0, 255);
    int green = Mth::clamp(static_cast<int>((value * value * 0.7f - 0.5f) * 255.0f), 0, 255);
    int blue = Mth::clamp(static_cast<int>((value * value * 0.6f - 0.7f) * 255.0f), 0, 255);
    return (red << 16) | (green << 8) | blue | 0xff000000;
}

void RedStoneWireBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    mUpdatedPositions.find(pos);
}

not_null_ptr<ItemInstance> RedStoneWireBlock::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state)
{
    return Block::getCloneItemInstance(level, pos, state);
}

int RedStoneWireBlock::getRenderLayer()
{
    return 0;
}

const BlockState* RedStoneWireBlock::getBlockState(int data)
{
    return defaultBlockState();
}

int RedStoneWireBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return 0;
}

const BlockState* RedStoneWireBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state;
}

const BlockState* RedStoneWireBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return state;
}

BlockStateDefinition* RedStoneWireBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { gNorthProperty, gEastProperty, gSouthProperty, gWestProperty, POWER };
    return new BlockStateDefinition(this, properties, 5);
}

EBlockFaceShape RedStoneWireBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos, const Direction* direction)
{
    return BLOCK_FACE_UNDEFINED;
}

void RedStoneWireBlock::registerIcons(IconRegister* iconRegister)
{
    Block::registerIcons(iconRegister);
}

TextureAtlasSprite* RedStoneWireBlock::getTexture(const std::wstring& name)
{
    return NULL;
}

EBlockRenderShape RedStoneWireBlock::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_INVISIBLE;
}
