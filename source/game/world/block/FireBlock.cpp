#include "game/world/block/FireBlock.hpp"

#include <algorithm>

#include "game/core/IconRegister.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

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
        static BooleanProperty* property = static_cast<BooleanProperty*>(BooleanProperty::create(L"upper"));
        return property;
    }
}

FireBlock::FireBlock()
    : Block(Material::FIRE), mFlameOdds(), mBurnOdds(), mFireIcon(NULL), mFireIconAlt(NULL)
{
    mLightEmission = 15;
    DerivedInit();

    TypedBoxed<int> age(0);
    TypedBoxed<bool> north(false);
    TypedBoxed<bool> east(false);
    TypedBoxed<bool> south(false);
    TypedBoxed<bool> west(false);
    TypedBoxed<bool> upper(false);

    registerDefaultState(mStateDefinition->any()
                             ->setBoxedValue(fireAge(), &age)
                             ->setBoxedValue(fireNorth(), &north)
                             ->setBoxedValue(fireEast(), &east)
                             ->setBoxedValue(fireSouth(), &south)
                             ->setBoxedValue(fireWest(), &west)
                             ->setBoxedValue(fireUp(), &upper));

    setTicking(true);
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
    FireBlock* fire = static_cast<FireBlock*>(Blocks::FIRE);
    fire->setFlammable(Blocks::planks, 5, 0x14);
    fire->setFlammable(Blocks::double_wooden_slab, 5, 0x14);
    fire->setFlammable(Blocks::wooden_slab, 5, 0x14);
    fire->setFlammable(Blocks::fence_gate, 5, 0x14);
    fire->setFlammable(Blocks::spruce_fence_gate, 5, 0x14);
    fire->setFlammable(Blocks::birch_fence_gate, 5, 0x14);
    fire->setFlammable(Blocks::jungle_fence_gate, 5, 0x14);
    fire->setFlammable(Blocks::dark_oak_fence_gate, 5, 0x14);
    fire->setFlammable(Blocks::acacia_fence_gate, 5, 0x14);
    fire->setFlammable(Blocks::fence, 5, 0x14);
    fire->setFlammable(Blocks::spruce_fence, 5, 0x14);
    fire->setFlammable(Blocks::birch_fence, 5, 0x14);
    fire->setFlammable(Blocks::jungle_fence, 5, 0x14);
    fire->setFlammable(Blocks::dark_oak_fence, 5, 0x14);
    fire->setFlammable(Blocks::acacia_fence, 5, 0x14);
    fire->setFlammable(Blocks::log, 5, 0x14);
    fire->setFlammable(Blocks::birch_stairs, 5, 0x14);
    fire->setFlammable(Blocks::spruce_stairs, 5, 0x14);
    fire->setFlammable(Blocks::jungle_stairs, 5, 0x14);
    fire->setFlammable(Blocks::acacia_stairs, 5, 0x14);
    fire->setFlammable(Blocks::dark_oak_stairs, 5, 0x14);
    fire->setFlammable(Blocks::leaves, 5, 5);
    fire->setFlammable(Blocks::leaves2, 5, 5);
    fire->setFlammable(Blocks::tallgrass, 0x1e, 0x3c);
    fire->setFlammable(Blocks::deadbush, 0x1e, 0x3c);
    fire->setFlammable(Blocks::bookshelf, 0x1e, 0x14);
    fire->setFlammable(Blocks::tnt, 0xf, 100);
    fire->setFlammable(Blocks::torch, 0x3c, 100);
    fire->setFlammable(Blocks::double_plant, 0x3c, 100);
    fire->setFlammable(Blocks::yellow_flower, 0x3c, 100);
    fire->setFlammable(Blocks::red_flower, 0x3c, 100);
    fire->setFlammable(Blocks::wool, 0x3c, 100);
    fire->setFlammable(Blocks::piston_head, 0x1e, 0x3c);
    fire->setFlammable(Blocks::vine, 0xf, 100);
    fire->setFlammable(Blocks::coal_block, 5, 5);
    fire->setFlammable(Blocks::dried_kelp_block, 0x1e, 5);
    fire->setFlammable(Blocks::hay_block, 0x3c, 0x14);
    fire->setFlammable(Blocks::carpet, 0x3c, 0x14);
    fire->setFlammable(Blocks::sea_pickle, 0xf, 100);
    fire->setFlammable(Blocks::stripped_log_oak, 5, 5);
    fire->setFlammable(Blocks::stripped_log_spruce, 5, 5);
    fire->setFlammable(Blocks::stripped_log_birch, 5, 5);
    fire->setFlammable(Blocks::stripped_log_jungle, 5, 5);
    fire->setFlammable(Blocks::stripped_log_acacia, 5, 5);
    fire->setFlammable(Blocks::stripped_log_dark_oak, 5, 5);
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
        Random* random = level->getRandomFor(pos.x, pos.y, pos.z);
        level->isBlockToBeTickedAt(pos, this);
        random->nextInt(getTickDelay(level) + 10);
    }
}

void FireBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    if (random->nextInt(24) == 0)
    {
        level->addParticle(PARTICLE_FLAME, pos.x + 0.5, pos.y + 0.5, pos.z + 0.5,
                           random->nextFloat() + 1.0, random->nextFloat() * 0.7 + 0.3,
                           16.0, arrayWithLength<int>(4, true));
    }

    if (!level->getBlockState(pos.below())->isSolidBlockingCube())
    {
        if (!canBurn(level, pos.below()))
        {
            if (canBurn(level, pos.west()))
            {
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble() * 0.1,
                                   pos.y + random->nextDouble(), pos.z + random->nextDouble(),
                                   0.0, 0.0, 0.0, arrayWithLength<int>(4, true));
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble() * 0.1,
                                   pos.y + random->nextDouble(), pos.z + random->nextDouble(),
                                   0.0, 0.0, 0.0, arrayWithLength<int>(4, true));
            }
            if (canBurn(level, pos.east()))
            {
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + 1.0 - random->nextDouble() * 0.1,
                                   pos.y + random->nextDouble(), pos.z + random->nextDouble(),
                                   0.0, 0.0, 0.0, arrayWithLength<int>(4, true));
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + 1.0 - random->nextDouble() * 0.1,
                                   pos.y + random->nextDouble(), pos.z + random->nextDouble(),
                                   0.0, 0.0, 0.0, arrayWithLength<int>(4, true));
            }
            if (canBurn(level, pos.north()))
            {
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(),
                                   pos.y + random->nextDouble(), pos.z + random->nextDouble() * 0.1,
                                   0.0, 0.0, 0.0, arrayWithLength<int>(4, true));
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(),
                                   pos.y + random->nextDouble(), pos.z + random->nextDouble() * 0.1,
                                   0.0, 0.0, 0.0, arrayWithLength<int>(4, true));
            }
            if (canBurn(level, pos.south()))
            {
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(),
                                   pos.y + random->nextDouble(), pos.z + 1.0 - random->nextDouble() * 0.1,
                                   0.0, 0.0, 0.0, arrayWithLength<int>(4, true));
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(),
                                   pos.y + random->nextDouble(), pos.z + 1.0 - random->nextDouble() * 0.1,
                                   0.0, 0.0, 0.0, arrayWithLength<int>(4, true));
            }
            if (canBurn(level, pos.above()))
            {
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(),
                                   pos.y + 1.0 - random->nextDouble() * 0.1, pos.z + random->nextDouble(),
                                   0.0, 0.0, 0.0, arrayWithLength<int>(4, true));
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(),
                                   pos.y + 1.0 - random->nextDouble() * 0.1, pos.z + random->nextDouble(),
                                   0.0, 0.0, 0.0, arrayWithLength<int>(4, true));
            }
            return;
        }
    }

    level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(), pos.y + random->nextDouble() * 0.5 + 0.5,
                       pos.z + random->nextDouble(), 0.0, 0.0, 0.0, arrayWithLength<int>(4, true));
    level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(), pos.y + random->nextDouble() * 0.5 + 0.5,
                       pos.z + random->nextDouble(), 0.0, 0.0, 0.0, arrayWithLength<int>(4, true));
    level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(), pos.y + random->nextDouble() * 0.5 + 0.5,
                       pos.z + random->nextDouble(), 0.0, 0.0, 0.0, arrayWithLength<int>(4, true));
}

const MaterialColor* FireBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&) { return MaterialColor::FIRE; }
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

EBlockRenderShape FireBlock::getRenderShape(const BlockState*) { return static_cast<EBlockRenderShape>(3); }
