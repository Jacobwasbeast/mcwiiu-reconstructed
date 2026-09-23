#include "game/world/block/WallBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

WallBlock::Variant* WallBlock::Variant::COBBLESTONE = new WallBlock::Variant(0, L"cobblestone", 0);
WallBlock::Variant* WallBlock::Variant::MOSSY = new WallBlock::Variant(1, L"mossy_cobblestone", 0);
WallBlock::Variant* gWallVariants[2] = { WallBlock::Variant::COBBLESTONE, WallBlock::Variant::MOSSY };
EnumProperty<WallBlock::Variant*>* WallBlock::VARIANT = NULL;
static Predicate<WallBlock::Variant*>* gWallVariantPredicate = new Predicates<WallBlock::Variant*>::ConstantPredicate(true);
static AABB* gWallShapes[16] = { NULL };
static AABB* gWallClipShapes[16] = { NULL };
static AbstractProperty<bool>* WALL_UP = NULL;
static AbstractProperty<bool>* WALL_NORTH = NULL;
static AbstractProperty<bool>* WALL_EAST = NULL;
static AbstractProperty<bool>* WALL_SOUTH = NULL;
static AbstractProperty<bool>* WALL_WEST = NULL;

WallBlock::Variant::Variant(int data, const std::wstring& name, int translationKey)
    : mData(data), mName(name), mTranslationKey(translationKey)
{
}

int WallBlock::Variant::getData() { return mData; }
WallBlock::Variant* WallBlock::Variant::byData(int data) { return data > 1 || data < 0 ? gWallVariants[0] : gWallVariants[data]; }
std::wstring WallBlock::Variant::toString() const { return mName; }
std::wstring WallBlock::Variant::getSerializedName() const { return mName; }
int WallBlock::Variant::getTranslationKey() { return mTranslationKey; }

void WallBlock::blockStaticCtor()
{
    boost::unordered_set<Variant*> values;
    values.insert(gWallVariants[0]);
    values.insert(gWallVariants[1]);
    VARIANT = EnumProperty<Variant*>::create(L"variant", typeid(Variant), values, gWallVariantPredicate);
}

WallBlock::WallBlock(Block* block)
    : Block(block == NULL ? Material::STONE : block->getCraftingMaterial())
{
    if (WALL_UP == NULL)
    {
        WALL_UP = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"up"));
        WALL_NORTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"north"));
        WALL_EAST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"east"));
        WALL_SOUTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"south"));
        WALL_WEST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"west"));
    }
    mBlockType = EBLOCK_TYPE_FENCE;
    DerivedInit();
    TypedBoxed<Variant*> variant(Variant::COBBLESTONE);
    const BlockState* state = mStateDefinition->any()->setBoxedValue(VARIANT, &variant);
    TypedBoxed<bool> falseValue(false);
    state = state->setBoxedValue(WALL_UP, &falseValue);
    state = state->setBoxedValue(WALL_NORTH, &falseValue);
    state = state->setBoxedValue(WALL_EAST, &falseValue);
    state = state->setBoxedValue(WALL_SOUTH, &falseValue);
    registerDefaultState(state->setBoxedValue(WALL_WEST, &falseValue));
}

WallBlock::~WallBlock() {}

unsigned int WallBlock::getAABBIndex(const BlockState* state)
{
    unsigned int result = 0;
    if (state->getValue(WALL_NORTH)) result |= 1u << Direction::NORTH->get2DDataValue();
    if (state->getValue(WALL_EAST)) result |= 1u << Direction::EAST->get2DDataValue();
    if (state->getValue(WALL_SOUTH)) result |= 1u << Direction::SOUTH->get2DDataValue();
    if (state->getValue(WALL_WEST)) result |= 1u << Direction::WEST->get2DDataValue();
    return result;
}

const AABB* WallBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    if (levelSource != NULL) state = fillVirtualBlockStateProperties(state, levelSource, pos);
    unsigned int index = getAABBIndex(state);
    if (gWallShapes[index] == NULL)
        gWallShapes[index] = AABB::newTemp(0.25, 0.0, 0.25, 0.75, 1.0, 0.75);
    return gWallShapes[index];
}

void WallBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                  std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity, bool isActualState)
{
    if (!isActualState) state = fillVirtualBlockStateProperties(state, (LevelSource*)level, pos);
    unsigned int index = getAABBIndex(state);
    if (gWallClipShapes[index] == NULL)
        gWallClipShapes[index] = AABB::newTemp(0.25, 0.0, 0.25, 0.75, 1.0, 0.75);
    addCollisionAABB(pos, entityBox, collidingBoxes, gWallClipShapes[index]);
}

const AABB* WallBlock::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return getShape(state, levelSource, pos);
}

std::wstring WallBlock::getName() { return L10N::GetString(0x2b4e0150); }
bool WallBlock::isCubeShaped(const BlockState*) { return false; }
bool WallBlock::isPathfindable(LevelSource*, const BlockPos&) { return false; }
bool WallBlock::isSolidRender(const BlockState*) const { return false; }
bool WallBlock::canContainLiquid(const BlockState*) const { return true; }

bool WallBlock::isExceptionBlockForAttachingPlusPiston(Block* block)
{
    return Block::isExceptionBlockForAttachingPlusPiston(block) || block == Blocks::glass || block == Blocks::leaves;
}

bool WallBlock::connectsTo(LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    Block* block = levelSource->getBlock(pos);
    if (block == NULL || isExceptionBlockForAttachingPlusPiston(block)) return false;
    const BlockState* state = levelSource->getBlockState(pos);
    EBlockFaceShape shape = state->getBlockFaceShape(levelSource, pos, direction);
    return shape == BLOCK_FACE_MIDDLE_POLE_THICK || shape == (EBlockFaceShape)7;
}

int WallBlock::getSpawnResourcesAuxValue(const BlockState* state) { return state->getValue(VARIANT)->getData(); }
bool WallBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction*) { return true; }
const BlockState* WallBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> value(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(VARIANT, &value);
}
int WallBlock::convertBlockStateToLegacyData(const BlockState* state) { return state->getValue(VARIANT)->getData(); }

const BlockState* WallBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    bool north = connectsTo(levelSource, pos.north(), Direction::NORTH);
    bool east = connectsTo(levelSource, pos.east(), Direction::EAST);
    bool south = connectsTo(levelSource, pos.south(), Direction::SOUTH);
    bool west = connectsTo(levelSource, pos.west(), Direction::WEST);
    bool up = !(north && south && !east && !west) && !(east && west && !north && !south);
    if (up && (north || east || south || west)) up = levelSource->getBlock(pos.above()) == NULL;
    TypedBoxed<bool> valueUp(up), valueNorth(north), valueEast(east), valueSouth(south), valueWest(west);
    state = state->setBoxedValue(WALL_UP, &valueUp);
    state = state->setBoxedValue(WALL_NORTH, &valueNorth);
    state = state->setBoxedValue(WALL_EAST, &valueEast);
    state = state->setBoxedValue(WALL_SOUTH, &valueSouth);
    return state->setBoxedValue(WALL_WEST, &valueWest);
}

BlockStateDefinition* WallBlock::createBlockStateDefinition()
{
    const Property* properties[] = { WALL_UP, WALL_NORTH, WALL_EAST, WALL_SOUTH, WALL_WEST, VARIANT };
    return new BlockStateDefinition(this, properties, 6);
}

EBlockFaceShape WallBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction* direction)
{
    return direction == Direction::UP || direction == Direction::DOWN ? BLOCK_FACE_CENTER : BLOCK_FACE_MIDDLE_POLE_THICK;
}

TextureAtlasSprite* WallBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Block* textureBlock = state->getValue(VARIANT) == Variant::MOSSY
        ? Blocks::mossy_cobblestone : Blocks::cobblestone;
    return textureBlock->getTexture(direction);
}
void WallBlock::registerIcons(IconRegister*) {}
EBlockRenderShape WallBlock::getRenderShape(const BlockState*) { return (EBlockRenderShape)32; }
