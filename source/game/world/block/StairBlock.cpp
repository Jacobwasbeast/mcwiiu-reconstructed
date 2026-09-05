#include "game/world/block/StairBlock.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/Explosion.hpp"
#include "game/core/IconRegister.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include <typeinfo>
#include <boost/unordered_set.hpp>

EnumProperty<StairBlock::Half*>* StairBlock::HALF = NULL;
EnumProperty<StairBlock::Shape*>* StairBlock::SHAPE = NULL;
static StairBlock::Half* gStairBottom = new StairBlock::Half(L"bottom");
static StairBlock::Half* gStairTop = new StairBlock::Half(L"top");
static StairBlock::Shape* gStairStraight = new StairBlock::Shape(L"straight", 0);
static StairBlock::Shape* gStairInnerLeft = new StairBlock::Shape(L"inner_left", 1);
static StairBlock::Shape* gStairInnerRight = new StairBlock::Shape(L"inner_right", 2);
static StairBlock::Shape* gStairOuterLeft = new StairBlock::Shape(L"outer_left", 3);
static StairBlock::Shape* gStairOuterRight = new StairBlock::Shape(L"outer_right", 4);
static AbstractProperty<const Direction*>* gStairFacing = NULL;
static AABB* gStairAABBs[16] = { NULL };
static AABB* gStairCorners[16] = { NULL };
static arrayWithLength<const AABB*> gStairShapes;

StairBlock::Half::Half(const std::wstring& name) : mName(name) {}
StairBlock::Half::~Half() {}
std::wstring StairBlock::Half::toString() const { return mName; }
std::wstring StairBlock::Half::getSerializedName() const { return mName; }
StairBlock::Shape::Shape(const std::wstring& name, int id) : mName(name), mId(id) {}
StairBlock::Shape::~Shape() {}
int StairBlock::Shape::getId() const { return mId; }
std::wstring StairBlock::Shape::toString() const { return mName; }
std::wstring StairBlock::Shape::getSerializedName() const { return mName; }

void StairBlock::blockStaticCtor()
{
    boost::unordered_set<Half*> halves; halves.insert(gStairBottom); halves.insert(gStairTop);
    boost::unordered_set<Shape*> shapes; shapes.insert(gStairStraight); shapes.insert(gStairInnerLeft); shapes.insert(gStairInnerRight); shapes.insert(gStairOuterLeft); shapes.insert(gStairOuterRight);
    boost::unordered_set<const Direction*> directions; directions.insert(Direction::NORTH); directions.insert(Direction::SOUTH); directions.insert(Direction::WEST); directions.insert(Direction::EAST);
    Predicates<Half*>::ConstantPredicate halfFilter(true);
    Predicates<Shape*>::ConstantPredicate shapeFilter(true);
    Predicates<const Direction*>::ConstantPredicate directionFilter(true);
    HALF = EnumProperty<Half*>::create(L"half", typeid(Half*), halves, &halfFilter);
    SHAPE = EnumProperty<Shape*>::create(L"shape", typeid(Shape*), shapes, &shapeFilter);
    gStairFacing = static_cast<AbstractProperty<const Direction*>*>(EnumProperty<const Direction*>::create(L"facing", typeid(const Direction*), directions, &directionFilter));
}

StairBlock::StairBlock(const BlockState* modelState)
    : Block(modelState == NULL || modelState->getBlock() == NULL ? NULL : modelState->getBlock()->getCraftingMaterial()), mModelBlock(NULL), mModelState(modelState), mIsModelState(false), mShapeState(0)
{
    DerivedInit();
    if (mModelState != NULL) mModelBlock = mModelState->getBlock();
    setLightBlock(255);
}
StairBlock::~StairBlock() {}

static const Direction* stairFacing(const BlockState* state) { return gStairFacing == NULL ? Direction::NORTH : state->getValue(gStairFacing); }
static bool stairTop(const BlockState* state) { return state->getValue(StairBlock::HALF) == gStairTop; }
static int stairIndex(const BlockState* state) { return stairFacing(state)->get2DDataValue() + (stairTop(state) ? 4 : 0); }

void StairBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* box, std::vector<AABB*>* result, boost::shared_ptr<Entity> entity, bool actual)
{
    if (!actual) state = fillVirtualBlockStateProperties(state, level, pos);
    const arrayWithLength<const AABB*>* shapes = getShapes(state, level, pos);
    for (unsigned int i = 0; i < shapes->length; ++i) if (shapes->data[i] != NULL) addCollisionAABB(pos, box, result, shapes->data[i]);
}
const AABB* StairBlock::getStepAABB(const BlockState* state)
{
    int i = stairIndex(state); if (gStairAABBs[i] == NULL) { bool top = stairTop(state); gStairAABBs[i] = AABB::newTemp(0.0, top ? 0.5 : 0.0, 0.0, 1.0, top ? 1.0 : 0.5, 1.0); } return gStairAABBs[i];
}
const AABB* StairBlock::getCornerAABB(const BlockState* state)
{
    int i = stairIndex(state); if (gStairCorners[i] == NULL) { bool top = stairTop(state); gStairCorners[i] = AABB::newTemp(0.0, top ? 0.5 : 0.0, 0.0, 1.0, top ? 1.0 : 0.5, 1.0); } return gStairCorners[i];
}
EBlockFaceShape StairBlock::getBlockFaceShape(LevelSource*, const BlockState* state, const BlockPos&, const Direction* direction)
{
    if (direction == Direction::UP) return stairTop(state) ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
    if (direction == Direction::DOWN) return stairTop(state) ? BLOCK_FACE_UNDEFINED : BLOCK_FACE_SOLID;
    const Shape* shape = state->getValue(SHAPE); return shape == gStairOuterLeft || shape == gStairOuterRight ? BLOCK_FACE_UNDEFINED : BLOCK_FACE_SOLID;
}
int StairBlock::getSpawnResourcesAuxValue(const BlockState*) { return 2; }
const arrayWithLength<const AABB*>* StairBlock::getShapes(const BlockState* state, LevelSource* source, const BlockPos& pos)
{
    if (source != NULL) state = fillVirtualBlockStateProperties(state, source, pos);
    gStairShapes.resize(2); gStairShapes.data[0] = getStepAABB(state); gStairShapes.data[1] = getCornerAABB(state); return &gStairShapes;
}
bool StairBlock::isSolidRender(const BlockState*) const { return false; }
bool StairBlock::isCubeShaped(const BlockState*) { return false; }
void StairBlock::addLights(Level*, const BlockPos&) {}
void StairBlock::animateTick(const BlockState*, Level*, const BlockPos&, Random*) {}
void StairBlock::attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player) { Block::attack(level, pos, player); }
void StairBlock::destroy(Level* level, const BlockPos& pos, const BlockState* state) { Block::destroy(level, pos, state); }
int StairBlock::getLightColor(const BlockState* state, LevelSource* source, const BlockPos& pos, int light) { return Block::getLightColor(state, source, pos, light); }
float StairBlock::getExplosionResistance(boost::shared_ptr<Entity> entity) { return mModelBlock == NULL ? Block::getExplosionResistance(entity) : mModelBlock->getExplosionResistance(entity); }
int StairBlock::getRenderLayer() { return 10; }
int StairBlock::getTickDelay(Level* level) { return mModelBlock == NULL ? Block::getTickDelay(level) : mModelBlock->getTickDelay(level); }
const AABB* StairBlock::getOutlineAABB(const BlockState* state, Level* level, const BlockPos& pos) { return mModelBlock == NULL ? getStepAABB(state) : mModelBlock->getOutlineAABB(state, level, pos); }
Vec3* StairBlock::handleEntityInside(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, Vec3* motion)
{
    if (mModelBlock == NULL)
        return Block::handleEntityInside(level, pos, entity, motion);
    return mModelBlock->handleEntityInside(level, pos, entity, motion);
}
bool StairBlock::mayPick() { return true; }
bool StairBlock::mayPick(const BlockState*, bool) { return true; }
bool StairBlock::mayPlace(Level* level, const BlockPos& pos) { return Block::mayPlace(level, pos); }
void StairBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state) { Block::onPlace(level, pos, state); }
void StairBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state) { Block::onRemove(level, pos, state); }
void StairBlock::prepareRender(Level* level, const BlockPos& pos) { Block::prepareRender(level, pos); }
void StairBlock::stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity) { Block::stepOn(level, pos, entity); }
void StairBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random) { Block::tick(level, pos, state, random); }
bool StairBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand, const Direction* direction, float x, float y, float z, bool client)
{
    return Block::use(level, pos, state, player, hand, direction, x, y, z, client);
}
void StairBlock::wasExploded(Level* level, const BlockPos& pos, Explosion* explosion) { Block::wasExploded(level, pos, explosion); }
bool StairBlock::isTopSolidBlocking(const BlockState* state) { return state->getValue(HALF) == gStairTop; }
const MaterialColor* StairBlock::getMapColor(const BlockState* state, LevelSource* source, const BlockPos& pos) { return mModelBlock == NULL ? Block::getMapColor(state, source, pos) : mModelBlock->getMapColor(state, source, pos); }

const BlockState* StairBlock::getStateForPlacement(Level*, const BlockPos&, const Direction* direction, float, float hitY, float, int, boost::shared_ptr<LivingEntity>)
{
    TypedBoxed<const Direction*> facing(direction == Direction::DOWN ? Direction::NORTH : direction->getOpposite());
    TypedBoxed<Half*> half(hitY > 0.5f ? gStairBottom : gStairTop);
    const BlockState* state = defaultBlockState()->setBoxedValue(gStairFacing, &facing); return state->setBoxedValue(HALF, &half);
}
bool StairBlock::_neighboringBlockCheckForCreatingBarrierInDirection(LevelSource* source, const BlockPos& pos, const BlockState* state, const Direction* direction) const
{
    const BlockState* neighbor = source->getBlockState(pos); return neighbor != NULL && neighbor->getBlock() == state->getBlock() && neighbor->getValue(gStairFacing) == direction;
}
bool StairBlock::liquidCanFlowIntoFromDirection(LevelSource* source, const BlockPos& pos, const Direction* direction) const
{
    const BlockState* state = source->getBlockState(pos); if (state == NULL) return true; if (direction == Direction::UP || direction == Direction::DOWN) return false; return !_neighboringBlockCheckForCreatingBarrierInDirection(source, pos.relative(direction), state, direction);
}
HitResult* StairBlock::clip(const BlockState* state, Level* level, const BlockPos& pos, Vec3* from, Vec3* to) { return Block::clip(state, level, pos, from, to); }
const BlockState* StairBlock::getBlockState(int data)
{
    TypedBoxed<const Direction*> facing(Direction::from2DDataValue(data & 3));
    TypedBoxed<Half*> half((data & 4) != 0 ? gStairTop : gStairBottom);
    const BlockState* state = defaultBlockState()->setBoxedValue(gStairFacing, &facing);
    return state->setBoxedValue(HALF, &half);
}
int StairBlock::convertBlockStateToLegacyData(const BlockState* state) { return state->getValue(gStairFacing)->get2DDataValue() | (state->getValue(HALF) == gStairTop ? 4 : 0); }
bool StairBlock::isStairs(const BlockState* state) { return state != NULL && state->getBlock() != NULL && dynamic_cast<const StairBlock*>(state->getBlock()) != NULL; }
bool StairBlock::canTakeShape(const BlockState* state, LevelSource* source, const BlockPos& pos, const Direction* direction)
{
    return isStairs(state) && getStairsShape(state, source, pos) != NULL && state->getValue(gStairFacing) == direction;
}
const StairBlock::Shape* StairBlock::getStairsShape(const BlockState* state, LevelSource* source, const BlockPos& pos)
{
    const BlockState* neighbor = source->getBlockState(pos);
    return isStairs(neighbor) ? neighbor->getValue(SHAPE) : NULL;
}
const BlockState* StairBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* source, const BlockPos& pos) { return source == NULL ? state : state; }
bool StairBlock::isStairs(int data)
{
    return data == 0x35 || data == 0x43 || (data >= 0x6c && data <= 0x6d)
        || data == 0x72 || data == 0x80 || (data >= 0x86 && data <= 0x88)
        || data == 0x9c || (data >= 0xa3 && data <= 0xa4)
        || data == 0xb4 || data == 0xcb;
}
const BlockState* StairBlock::rotate(const BlockState* state, Rotation* rotation) { TypedBoxed<const Direction*> value(rotation->rotate(state->getValue(gStairFacing))); return state->setBoxedValue(gStairFacing, &value); }
const BlockState* StairBlock::mirror(const BlockState* state, Mirror* mirror) { return rotate(state, mirror->getRotation(state->getValue(gStairFacing))); }
BlockStateDefinition* StairBlock::createBlockStateDefinition() { const Property* properties[] = { gStairFacing, HALF, SHAPE }; return new BlockStateDefinition(this, properties, 3); }
TextureAtlasSprite* StairBlock::getTexture(const Direction* direction, const BlockState* state) { return mModelBlock == NULL ? Block::getTexture(direction, state) : mModelBlock->getTexture(direction, state); }
void StairBlock::registerIcons(IconRegister*) {}
EBlockRenderShape StairBlock::getRenderShape(const BlockState*) { return static_cast<EBlockRenderShape>(10); }
