#include "game/world/block/FireBlock.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundEvents.hpp"

#include <algorithm>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/IconRegister.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/util/Random.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/PortalBlock.hpp"
#include "game/world/block/TntBlock.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<int>* FireBlock::AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 15));
AbstractProperty<bool>* FireBlock::NORTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"north"));
AbstractProperty<bool>* FireBlock::EAST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"east"));
AbstractProperty<bool>* FireBlock::SOUTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"south"));
AbstractProperty<bool>* FireBlock::WEST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"west"));
AbstractProperty<bool>* FireBlock::UP = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"upper"));

FireBlock::FireBlock()
    : Block(Material::FIRE), mFlameOdds(), mBurnOdds()
{
    DerivedInit();
    TypedBoxed<int> age(0);
    TypedBoxed<bool> north(false);
    TypedBoxed<bool> east(false);
    TypedBoxed<bool> south(false);
    TypedBoxed<bool> west(false);
    TypedBoxed<bool> up(false);
    registerDefaultState(mStateDefinition->any()
                             ->setBoxedValue(AGE, &age)
                             ->setBoxedValue(NORTH, &north)
                             ->setBoxedValue(EAST, &east)
                             ->setBoxedValue(SOUTH, &south)
                             ->setBoxedValue(WEST, &west)
                             ->setBoxedValue(UP, &up));
    setTicking(true);
    mFireIcon = NULL;
    mFireIconAlt = NULL;
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
    if (levelSource->getBlockState(pos.below())->isTopSolidBlocking())
        return defaultBlockState();
    if (static_cast<FireBlock*>(Blocks::fire)->canBurn(levelSource, pos.below()))
        return defaultBlockState();
    TypedBoxed<bool> north(canBurn(levelSource, pos.north()));
    TypedBoxed<bool> east(canBurn(levelSource, pos.east()));
    TypedBoxed<bool> south(canBurn(levelSource, pos.south()));
    TypedBoxed<bool> west(canBurn(levelSource, pos.west()));
    TypedBoxed<bool> up(canBurn(levelSource, pos.above()));
    const BlockState* result = state->setBoxedValue(NORTH, &north);
    result = result->setBoxedValue(EAST, &east);
    result = result->setBoxedValue(SOUTH, &south);
    result = result->setBoxedValue(WEST, &west);
    return result->setBoxedValue(UP, &up);
}

FireBlock* FireBlock::setFlammable(Block* block, int flameOdds, int burnOdds)
{
    mFlameOdds[block] = flameOdds;
    mBurnOdds[block] = burnOdds;
    return this;
}

void FireBlock::staticCtor()
{
    FireBlock* fire = static_cast<FireBlock*>(Blocks::fire);
    fire->setFlammable(Blocks::planks, 5, 20);
    fire->setFlammable(Blocks::double_wooden_slab, 5, 20);
    fire->setFlammable(Blocks::wooden_slab, 5, 20);
    fire->setFlammable(Blocks::fence_gate, 5, 20);
    fire->setFlammable(Blocks::spruce_fence_gate, 5, 20);
    fire->setFlammable(Blocks::birch_fence_gate, 5, 20);
    fire->setFlammable(Blocks::jungle_fence_gate, 5, 20);
    fire->setFlammable(Blocks::dark_oak_fence_gate, 5, 20);
    fire->setFlammable(Blocks::acacia_fence_gate, 5, 20);
    fire->setFlammable(Blocks::fence, 5, 20);
    fire->setFlammable(Blocks::spruce_fence, 5, 20);
    fire->setFlammable(Blocks::birch_fence, 5, 20);
    fire->setFlammable(Blocks::jungle_fence, 5, 20);
    fire->setFlammable(Blocks::dark_oak_fence, 5, 20);
    fire->setFlammable(Blocks::acacia_fence, 5, 20);
    fire->setFlammable(Blocks::oak_stairs, 5, 20);
    fire->setFlammable(Blocks::birch_stairs, 5, 20);
    fire->setFlammable(Blocks::spruce_stairs, 5, 20);
    fire->setFlammable(Blocks::jungle_stairs, 5, 20);
    fire->setFlammable(Blocks::acacia_stairs, 5, 20);
    fire->setFlammable(Blocks::dark_oak_stairs, 5, 20);
    fire->setFlammable(Blocks::log, 5, 5);
    fire->setFlammable(Blocks::log2, 5, 5);
    fire->setFlammable(Blocks::leaves, 30, 60);
    fire->setFlammable(Blocks::leaves2, 30, 60);
    fire->setFlammable(Blocks::bookshelf, 30, 20);
    fire->setFlammable(Blocks::tnt, 15, 100);
    fire->setFlammable(Blocks::tallgrass, 60, 100);
    fire->setFlammable(Blocks::double_plant, 60, 100);
    fire->setFlammable(Blocks::yellow_flower, 60, 100);
    fire->setFlammable(Blocks::red_flower, 60, 100);
    fire->setFlammable(Blocks::deadbush, 60, 100);
    fire->setFlammable(Blocks::wool, 30, 60);
    fire->setFlammable(Blocks::vine, 15, 100);
    fire->setFlammable(Blocks::coal_block, 5, 5);
    fire->setFlammable(Blocks::dried_kelp_block, 30, 5);
    fire->setFlammable(Blocks::hay_block, 60, 20);
    fire->setFlammable(Blocks::carpet, 60, 20);
    fire->setFlammable(Blocks::sea_pickle, 15, 100);
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
    int burnOdd = getBurnOdd(level->getBlockState(pos)->getBlock());
    if (random->nextInt(odds) >= burnOdd)
        return;

    const BlockState* burning = level->getBlockState(pos);
    if (random->nextInt(age + 10) < 5 && !level->isRainingAt(pos)
        && g_pApp->GetGameHostOption(eGameHostOption_FireSpreads))
    {
        int newAge = age + random->nextInt(5) / 4;
        if (newAge > 15)
            newAge = 15;
        TypedBoxed<int> ageValue(newAge);
        level->setBlock(pos, defaultBlockState()->setBoxedValue(AGE, &ageValue), Block::UPDATE_ALL, false);
    }
    else
    {
        level->removeBlock(pos);
    }

    if (burning->getBlock() == Blocks::tnt)
    {
        TypedBoxed<bool> explode(true);
        Blocks::tnt->destroy(level, pos, burning->setBoxedValue(TntBlock::EXPLODE, &explode));
    }
}

void FireBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (!level->getGameRules()->getBoolean(GameRules::RULE_DOFIRETICK))
        return;

    if (!level->mIsClientSide && !gMinecraftServer->getPlayers()->isTrackingBlock(pos, level->mDimension))
    {
        level->addToTickNextTick(pos, this, getTickDelay(level) * 5);
        return;
    }

    if (!mayPlace(level, pos))
        level->removeBlock(pos);

    Block* below = level->getBlockState(pos.below())->getBlock();
    bool infiniburn = below == Blocks::netherrack || below == Blocks::magma;
    if (dynamic_cast<TheEndDimension*>(level->mDimension) != NULL && below == Blocks::bedrock)
        infiniburn = true;

    int age = state->getValue(AGE);
    if (!infiniburn && level->isRaining() && isNearRain(level, pos))
    {
        level->removeBlock(pos);
        return;
    }

    if (age < 15)
    {
        TypedBoxed<int> ageValue(age + random->nextInt(3) / 2);
        state = state->setBoxedValue(AGE, &ageValue);
        level->setBlock(pos, state, Block::UPDATE_INVISIBLE, false);
    }
    level->addToTickNextTick(pos, this, getTickDelay(level) + random->nextInt(10));

    if (!infiniburn)
    {
        if (!isValidFireLocation(level, pos)
            || level->getExtraBlockState(pos.below())->getMaterial() == Material::WATER)
        {
            if (!level->getBlockState(pos.below())->isTopSolidBlocking() || age > 3)
                level->removeBlock(pos);
            return;
        }
        if (!canBurn(level, pos.below()) && age == 15 && random->nextInt(4) == 0)
        {
            level->removeBlock(pos);
            return;
        }
    }

    bool humid = level->isHumidAt(pos);
    int humidity = humid ? -50 : 0;
    checkBurnOut(level, pos.east(), humidity + 300, random, age);
    checkBurnOut(level, pos.west(), humidity + 300, random, age);
    checkBurnOut(level, pos.below(), humidity + 250, random, age);
    checkBurnOut(level, pos.above(), humidity + 250, random, age);
    checkBurnOut(level, pos.north(), humidity + 300, random, age);
    checkBurnOut(level, pos.south(), humidity + 300, random, age);

    if (!g_pApp->GetGameHostOption(eGameHostOption_FireSpreads))
        return;

    for (int x = -1; x <= 1; ++x)
    {
        for (int z = -1; z <= 1; ++z)
        {
            for (int y = -1; y <= 4; ++y)
            {
                if (x == 0 && y == 0 && z == 0)
                    continue;
                int chance = 100;
                if (y > 1)
                    chance = y * 100;
                BlockPos target = pos.offset(x, y, z);
                int fireOdds = getFireOdds(level, target);
                if (fireOdds <= 0)
                    continue;
                int odds = (fireOdds + 40 + level->getDifficulty()->getId() * 7) / (age + 30);
                if (humid)
                    odds /= 2;
                if (odds < 1 || random->nextInt(chance) > odds
                    || (level->isRaining() && isNearRain(level, target)))
                    continue;
                int newAge = age + random->nextInt(5) / 4;
                if (newAge > 15)
                    newAge = 15;
                TypedBoxed<int> ageValue(newAge);
                level->setBlock(target, state->setBoxedValue(AGE, &ageValue), Block::UPDATE_ALL, false);
            }
        }
    }
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
    const DimensionType* type = level->mDimension->getType();
    if ((type == DimensionType::OVERWORLD || type == DimensionType::NETHER)
        && static_cast<PortalBlock*>(Blocks::portal)->trySpawnPortal(level, pos, true))
    {
        return;
    }
    if (!level->getBlockState(pos.below())->isTopSolidBlocking() && !isValidFireLocation(level, pos))
    {
        level->removeBlock(pos);
        return;
    }
    level->addToTickNextTick(pos, this, getTickDelay(level) + level->mRandom->nextInt(10));
}

void FireBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    if (random->nextInt(24) == 0)
        level->playLocalSound(pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f, SoundEvents::FIRE_AMBIENT,
                              SoundSource::eSoundSource_Blocks, 1.0f + random->nextFloat(),
                              random->nextFloat() * 0.7f + 0.3f, false, 16.0f);

    if (!level->getBlockState(pos.below())->isTopSolidBlocking())
    {
        if (!static_cast<FireBlock*>(Blocks::fire)->canBurn(level, pos.below()))
        {
            if (static_cast<FireBlock*>(Blocks::fire)->canBurn(level, pos.west()))
            {
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble() * 0.1,
                                   pos.y + random->nextDouble(), pos.z + random->nextDouble(),
                                   0.0, 0.0, 0.0, arrayWithLength<int>());
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble() * 0.1,
                                   pos.y + random->nextDouble(), pos.z + random->nextDouble(),
                                   0.0, 0.0, 0.0, arrayWithLength<int>());
            }
            if (static_cast<FireBlock*>(Blocks::fire)->canBurn(level, pos.east()))
            {
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + 1.0 - random->nextDouble() * 0.1,
                                   pos.y + random->nextDouble(), pos.z + random->nextDouble(),
                                   0.0, 0.0, 0.0, arrayWithLength<int>());
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + 1.0 - random->nextDouble() * 0.1,
                                   pos.y + random->nextDouble(), pos.z + random->nextDouble(),
                                   0.0, 0.0, 0.0, arrayWithLength<int>());
            }
            if (static_cast<FireBlock*>(Blocks::fire)->canBurn(level, pos.north()))
            {
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(),
                                   pos.y + random->nextDouble(), pos.z + random->nextDouble() * 0.1,
                                   0.0, 0.0, 0.0, arrayWithLength<int>());
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(),
                                   pos.y + random->nextDouble(), pos.z + random->nextDouble() * 0.1,
                                   0.0, 0.0, 0.0, arrayWithLength<int>());
            }
            if (static_cast<FireBlock*>(Blocks::fire)->canBurn(level, pos.south()))
            {
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(),
                                   pos.y + random->nextDouble(), pos.z + 1.0 - random->nextDouble() * 0.1,
                                   0.0, 0.0, 0.0, arrayWithLength<int>());
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(),
                                   pos.y + random->nextDouble(), pos.z + 1.0 - random->nextDouble() * 0.1,
                                   0.0, 0.0, 0.0, arrayWithLength<int>());
            }
            if (static_cast<FireBlock*>(Blocks::fire)->canBurn(level, pos.above()))
            {
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(),
                                   pos.y + 1.0 - random->nextDouble() * 0.1, pos.z + random->nextDouble(),
                                   0.0, 0.0, 0.0, arrayWithLength<int>());
                level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(),
                                   pos.y + 1.0 - random->nextDouble() * 0.1, pos.z + random->nextDouble(),
                                   0.0, 0.0, 0.0, arrayWithLength<int>());
            }
            return;
        }
    }

    level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(), pos.y + random->nextDouble() * 0.5 + 0.5,
                       pos.z + random->nextDouble(), 0.0, 0.0, 0.0, arrayWithLength<int>());
    level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(), pos.y + random->nextDouble() * 0.5 + 0.5,
                       pos.z + random->nextDouble(), 0.0, 0.0, 0.0, arrayWithLength<int>());
    level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextDouble(), pos.y + random->nextDouble() * 0.5 + 0.5,
                       pos.z + random->nextDouble(), 0.0, 0.0, 0.0, arrayWithLength<int>());
}

const MaterialColor* FireBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&) { return MaterialColor::FIRE; }
int FireBlock::getRenderLayer() { return 0; }
const BlockState* FireBlock::getBlockState(int data)
{
    TypedBoxed<int> age(data);
    return defaultBlockState()->setBoxedValue(AGE, &age);
}
int FireBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    const TypedBoxed<int>* age = static_cast<const TypedBoxed<int>*>(state->getBoxedValue(AGE));
    return age == NULL ? 0 : age->getValue();
}
bool FireBlock::waterSpreadCausesSpawn() { return true; }
bool FireBlock::canContainLiquid(const BlockState*) const { return false; }

BlockStateDefinition* FireBlock::createBlockStateDefinition()
{
    const Property* properties[6] = { AGE, NORTH, EAST, SOUTH, WEST, UP };
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

EBlockRenderShape FireBlock::getRenderShape(const BlockState*) { return BLOCK_RENDER_FIRE; }
