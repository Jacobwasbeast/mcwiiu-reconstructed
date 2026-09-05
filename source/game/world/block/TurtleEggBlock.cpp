#include "game/world/block/TurtleEggBlock.hpp"

#include <string>

#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

static AbstractProperty<int>* TURTLE_EGG_HATCH = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"hatch", 0, 2));
static AbstractProperty<int>* TURTLE_EGG_COUNT = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"eggs", 1, 4));
static TextureAtlasSprite* turtleEggTexture[4];

TurtleEggBlock::TurtleEggBlock()
    : Block(Material::EGG)
{
    DerivedInit();
    TypedBoxed<int> eggs(1);
    TypedBoxed<int> hatch(0);
    const BlockState* state = defaultBlockState()->setBoxedValue(TURTLE_EGG_COUNT, &eggs);
    state = state->setBoxedValue(TURTLE_EGG_HATCH, &hatch);
    registerDefaultState(state);
    setTicking(true);
}

TurtleEggBlock::~TurtleEggBlock()
{
}

bool TurtleEggBlock::shouldUpdateHatchLevel(Level* level)
{
    float time = level->getTimeOfDay(1.0f);
    return (time < 0.7f && time > 0.65f) || level->mRandom->nextInt(500) == 0;
}

void TurtleEggBlock::_decreaseEggs(Level* level, const BlockPos& pos, int eggs, bool, bool) const
{
    const BlockState* state = level->getBlockState(pos);
    level->levelEvent(2001, pos, Block::getId(state->getBlock()));
    if (eggs <= 0)
        level->setBlock(pos, NULL, 3, false);
    else
    {
        TypedBoxed<int> count(eggs - 1);
        level->setBlock(pos, state->setBoxedValue(TURTLE_EGG_COUNT, &count), 3, false);
    }
}

void TurtleEggBlock::_destroyEgg(Level* level, const BlockPos& pos, boost::shared_ptr<Entity>, int chance, bool playSound, bool hatch) const
{
    if (level->mIsClientSide || level->mRandom->nextInt(chance) != 0)
        return;
    const BlockState* state = level->getBlockState(pos);
    int eggs = state->getValue(TURTLE_EGG_COUNT);
    _decreaseEggs(level, pos, eggs, playSound, hatch);
}

void TurtleEggBlock::stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity)
{
    _destroyEgg(level, pos, entity, 100, false, true);
    Block::stepOn(level, pos, entity);
}

void TurtleEggBlock::fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance)
{
    _destroyEgg(level, pos, entity, 3, true, false);
    Block::fallOn(level, pos, entity, fallDistance);
}

void TurtleEggBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide && shouldUpdateHatchLevel(level))
    {
        int hatch = state->getValue(TURTLE_EGG_HATCH);
        if (hatch < 2)
        {
            TypedBoxed<int> value(hatch + 1);
            level->setBlock(pos, state->setBoxedValue(TURTLE_EGG_HATCH, &value), 3, false);
            level->levelEvent(2002, pos, 0);
        }
    }
}

bool TurtleEggBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player)
{
    _decreaseEggs(level, pos, state->getValue(TURTLE_EGG_COUNT), false, false);
    return true;
}

Item* TurtleEggBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(this);
}

bool TurtleEggBlock::mayPlace(Level* level, const BlockPos& pos)
{
    const BlockState* below = level->getBlockState(pos.below());
    return below->getBlock() != NULL && below->getBlock()->isSolidRender(below);
}

bool TurtleEggBlock::use(Level*, const BlockPos&, const BlockState*, boost::shared_ptr<Player>, InteractionHand::EInteractionHand, const Direction*, float, float, float, bool)
{
    return false;
}

bool TurtleEggBlock::TestUse(Level*, const BlockPos&, const BlockState* state, boost::shared_ptr<Player>)
{
    return state->getValue(TURTLE_EGG_COUNT) != 3;
}

unsigned int TurtleEggBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    return Block::GetInteractTooltip(tooltipData);
}

bool TurtleEggBlock::isSolidRender(const BlockState*) const { return false; }
bool TurtleEggBlock::isCubeShaped(const BlockState*) { return false; }

void TurtleEggBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox, std::vector<AABB*>* boxes, boost::shared_ptr<Entity> entity, bool actual)
{
    int count = state->getValue(TURTLE_EGG_COUNT);
    if (count > 0) Block::addCollisionAABBs(state, level, pos, entityBox, boxes, entity, actual);
    if (count > 1) Block::addCollisionAABBs(state, level, pos, entityBox, boxes, entity, actual);
    if (count > 2) Block::addCollisionAABBs(state, level, pos, entityBox, boxes, entity, actual);
    if (count > 3) Block::addCollisionAABBs(state, level, pos, entityBox, boxes, entity, actual);
}

const AABB* TurtleEggBlock::getShape(const BlockState*, LevelSource*, const BlockPos&) { return NULL; }
EBlockRenderShape TurtleEggBlock::getRenderShape(const BlockState*) { return static_cast<EBlockRenderShape>(50); }
int TurtleEggBlock::getRenderLayer() { return 0; }
bool TurtleEggBlock::canInstantlyTick() { return false; }

void TurtleEggBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (!level->mIsClientSide)
        level->levelEvent(2000, pos, 15);
    level->setBlockAndUpdate(pos, level->getBlockState(pos));
}

bool TurtleEggBlock::canContainLiquid(const BlockState*) const { return true; }
bool TurtleEggBlock::waterSpreadCausesSpawn() { return false; }
EPistonPushReaction TurtleEggBlock::getPistonPushReaction(const BlockState*) { return PISTON_PUSH_DESTROY; }

int TurtleEggBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(TURTLE_EGG_COUNT) | (state->getValue(TURTLE_EGG_HATCH) << 2);
}

const BlockState* TurtleEggBlock::getBlockState(int data)
{
    TypedBoxed<int> count(data & 3);
    TypedBoxed<int> hatch((data >> 2) & 3);
    const BlockState* state = defaultBlockState()->setBoxedValue(TURTLE_EGG_COUNT, &count);
    return state->setBoxedValue(TURTLE_EGG_HATCH, &hatch);
}

TextureAtlasSprite* TurtleEggBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return turtleEggTexture[state->getValue(TURTLE_EGG_HATCH)];
}

void TurtleEggBlock::registerIcons(IconRegister* iconRegister)
{
    turtleEggTexture[0] = iconRegister->registerIcon(L"turtle_egg_not_cracked");
    turtleEggTexture[1] = iconRegister->registerIcon(L"turtle_egg_slightly_cracked");
    turtleEggTexture[2] = iconRegister->registerIcon(L"turtle_egg_very_cracked");
    turtleEggTexture[3] = turtleEggTexture[2];
}

BlockStateDefinition* TurtleEggBlock::createBlockStateDefinition()
{
    const Property* properties[2] = { TURTLE_EGG_COUNT, TURTLE_EGG_HATCH };
    return new BlockStateDefinition(this, properties, 2);
}

bool TurtleEggBlock::isSilkTouchable() { return true; }
