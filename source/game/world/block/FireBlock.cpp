#include "game/world/block/FireBlock.hpp"

#include <algorithm>

#include "game/core/IconRegister.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"

namespace
{
    IntegerProperty* fireAge()
    {
        static IntegerProperty* property = static_cast<IntegerProperty*>(IntegerProperty::create(L"age", 0, 15));
        return property;
    }

    BooleanProperty* fireNorth()
    {
        static BooleanProperty* property = static_cast<BooleanProperty*>(BooleanProperty::create(L"north"));
        return property;
    }

    BooleanProperty* fireEast()
    {
        static BooleanProperty* property = static_cast<BooleanProperty*>(BooleanProperty::create(L"east"));
        return property;
    }

    BooleanProperty* fireSouth()
    {
        static BooleanProperty* property = static_cast<BooleanProperty*>(BooleanProperty::create(L"south"));
        return property;
    }

    BooleanProperty* fireWest()
    {
        static BooleanProperty* property = static_cast<BooleanProperty*>(BooleanProperty::create(L"west"));
        return property;
    }

    BooleanProperty* fireUp()
    {
        static BooleanProperty* property = static_cast<BooleanProperty*>(BooleanProperty::create(L"up"));
        return property;
    }
}

FireBlock::FireBlock()
    : Block(NULL), mFlameOdds(), mBurnOdds(), mFireIcon(NULL), mFireIconAlt(NULL)
{
    setTicking(true);
    mLightEmission = 15;
    registerDefaultState(Block::defaultBlockState());
}

FireBlock::~FireBlock()
{
}

int FireBlock::getFlameOdds(Block* block)
{
    boost::unordered_map<Block*, int>::const_iterator it = mFlameOdds.find(block);
    return it == mFlameOdds.end() ? 0 : it->second;
}

bool FireBlock::canBurn(LevelSource* levelSource, const BlockPos& pos)
{
    return getFlameOdds(levelSource->getBlock(pos)) > 0;
}

const BlockState* FireBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    if (levelSource->getBlockState(pos.below())->isSolidBlockingCube())
        return defaultBlockState();
    if (canBurn(levelSource, pos.below()))
        return defaultBlockState();
    TypedBoxed<bool> north(canBurn(levelSource, pos.north()));
    TypedBoxed<bool> east(canBurn(levelSource, pos.east()));
    TypedBoxed<bool> south(canBurn(levelSource, pos.south()));
    TypedBoxed<bool> west(canBurn(levelSource, pos.west()));
    TypedBoxed<bool> up(canBurn(levelSource, pos.above()));
    const BlockState* result = state->setBoxedValue(fireNorth(), &north);
    result = result->setBoxedValue(fireEast(), &east);
    result = result->setBoxedValue(fireSouth(), &south);
    result = result->setBoxedValue(fireWest(), &west);
    return result->setBoxedValue(fireUp(), &up);
}

FireBlock* FireBlock::setFlammable(Block* block, int flameOdds, int burnOdds)
{
    mFlameOdds[block] = flameOdds;
    mBurnOdds[block] = burnOdds;
    return this;
}

void FireBlock::staticCtor()
{
}

const AABB* FireBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

bool FireBlock::isSolidRender(const BlockState*) const { return false; }
bool FireBlock::isCubeShaped(const BlockState*) { return false; }
int FireBlock::getResourceCount(Random*) { return 0; }
int FireBlock::getTickDelay(Level*) { return 30; }

int FireBlock::getFireOdds(Level* level, const BlockPos& pos)
{
    if (level->isEmptyBlock(pos))
        return 0;
    int odds = 0;
    static const Direction* directions[6] = { Direction::DOWN, Direction::UP, Direction::NORTH, Direction::SOUTH, Direction::WEST, Direction::EAST };
    int i;
    for (i = 0; i < 6; ++i)
        odds = std::max(odds, getFlameOdds(level->getBlock(pos.relative(directions[i]))));
    return odds;
}

bool FireBlock::isNearRain(Level* level, const BlockPos& pos)
{
    return level->isRainingAt(pos) || level->isRainingAt(pos.west()) || level->isRainingAt(pos.east()) ||
           level->isRainingAt(pos.north()) || level->isRainingAt(pos.south());
}

bool FireBlock::isValidFireLocation(Level* level, const BlockPos& pos)
{
    static const Direction* directions[6] = { Direction::DOWN, Direction::UP, Direction::NORTH, Direction::SOUTH, Direction::WEST, Direction::EAST };
    int i;
    for (i = 0; i < 6; ++i)
        if (canBurn(level, pos.relative(directions[i])))
            return true;
    return false;
}

int FireBlock::getBurnOdd(Block* block)
{
    boost::unordered_map<Block*, int>::const_iterator it = mBurnOdds.find(block);
    return it == mBurnOdds.end() ? 0 : it->second;
}

void FireBlock::checkBurnOut(Level* level, const BlockPos& pos, int odds, Random* random, int age)
{
    Block* block = level->getBlock(pos);
    if (random->nextInt(odds) >= getBurnOdd(block))
        return;
    if (random->nextInt(age + 10) < 5 && !level->isRainingAt(pos))
    {
        int nextAge = std::min(15, age + random->nextInt(5) / 4);
        TypedBoxed<int> boxed(nextAge);
        level->setBlock(pos, defaultBlockState()->setBoxedValue(fireAge(), &boxed), 3, false);
    }
    else
        level->removeBlock(pos);
}

void FireBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    int age = static_cast<const TypedBoxed<int>*>(state->getBoxedValue(fireAge()))->getValue();
    if (level->isRaining() && isNearRain(level, pos))
    {
        level->removeBlock(pos);
        return;
    }
    if (age < 15)
    {
        TypedBoxed<int> boxed(age + random->nextInt(3) / 2);
        level->setBlock(pos, state->setBoxedValue(fireAge(), &boxed), 4, false);
    }
    level->isBlockToBeTickedAt(pos, this);
    int odds = level->isHumidAt(pos) ? 250 : 300;
    checkBurnOut(level, pos.east(), odds, random, age);
    checkBurnOut(level, pos.west(), odds, random, age);
    checkBurnOut(level, pos.below(), 250, random, age);
    checkBurnOut(level, pos.above(), 250, random, age);
    checkBurnOut(level, pos.north(), odds, random, age);
    checkBurnOut(level, pos.south(), odds, random, age);
    if (!isValidFireLocation(level, pos) && !level->getBlockState(pos.below())->isSolidBlockingCube())
        level->removeBlock(pos);
}

bool FireBlock::canInstantlyTick() { return false; }
bool FireBlock::mayPick() { return false; }
bool FireBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos.below())->isSolidBlockingCube() || isValidFireLocation(level, pos);
}

void FireBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!level->getBlockState(pos.below())->isSolidBlockingCube() && !isValidFireLocation(level, pos))
        level->removeBlock(pos);
}

void FireBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (!level->mIsClientSide)
    {
        Random random = level->getRandomFor(pos.x, pos.y, pos.z);
        level->isBlockToBeTickedAt(pos, this);
        random.nextInt(getTickDelay(level) + 10);
    }
}

void FireBlock::animateTick(const BlockState*, Level*, const BlockPos&, Random*)
{
}

const MaterialColor* FireBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&) { return NULL; }
int FireBlock::getRenderLayer() { return 0; }
const BlockState* FireBlock::getBlockState(int data)
{
    TypedBoxed<int> age(data);
    return defaultBlockState()->setBoxedValue(fireAge(), &age);
}
int FireBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    const TypedBoxed<int>* age = static_cast<const TypedBoxed<int>*>(state->getBoxedValue(fireAge()));
    return age == NULL ? 0 : age->getValue();
}
bool FireBlock::waterSpreadCausesSpawn() { return true; }
bool FireBlock::canContainLiquid(const BlockState*) const { return false; }

BlockStateDefinition* FireBlock::createBlockStateDefinition()
{
    const Property* properties[6] = { fireAge(), fireNorth(), fireEast(), fireSouth(), fireWest(), fireUp() };
    return new BlockStateDefinition(this, properties, 6);
}

EBlockFaceShape FireBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* FireBlock::getTexture(const Direction*, const BlockState*) { return mFireIcon; }
TextureAtlasSprite* FireBlock::getTextureLayer(int layer) { return layer == 0 ? mFireIcon : mFireIconAlt; }

void FireBlock::registerIcons(IconRegister* iconRegister)
{
    mFireIcon = iconRegister->registerIcon(L"fire_0");
    mFireIconAlt = iconRegister->registerIcon(L"fire_1");
}

EBlockRenderShape FireBlock::getRenderShape(const BlockState*) { return BLOCK_RENDER_INVISIBLE; }
