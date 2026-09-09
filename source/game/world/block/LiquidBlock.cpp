#include "game/world/block/LiquidBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/util/Random.hpp"
#include "game/util/Math.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"

#include <cmath>

AbstractProperty<int>* LiquidBlock::LEVEL =
    static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"level", 0, 15));

LiquidBlock::LiquidBlock(Material* material) : Block(material)
{
    mFlowIcon = NULL;
    mBlockType = EBLOCK_TYPE_MOB_SPAWNER;
    setTicking(true);
}

LiquidBlock::~LiquidBlock()
{
}

bool LiquidBlock::isLiquidBlock()
{
    return true;
}

double LiquidBlock::getHeightFromData(int data)
{
    if (data > 7)
        data = 0;
    return (double)(data + 1) / 9.0;
}

short LiquidBlock::getLiquidElement(LevelSource* levelSource, const BlockPos& pos)
{
    short element = 0;
    short alternate = 0;
    levelSource->getElementAndExtraElement(pos.x, pos.y, pos.z, (unsigned short*)&element, (unsigned short*)&alternate);
    return alternate == 0 ? element : alternate;
}

void LiquidBlock::DerivedInit()
{
    Block::DerivedInit();
    BlockStateDefinition* definition = mStateDefinition;
    if (definition != NULL)
    {
        TypedBoxed<int> level(0);
        registerDefaultState(definition->any()->setBoxedValue(LEVEL, &level));
    }
}

const AABB* LiquidBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return Block::getShape(state, levelSource, pos);
}

const AABB* LiquidBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

bool LiquidBlock::isPathfindable(LevelSource*, const BlockPos&)
{
    return mMaterial != Material::LAVA;
}

int LiquidBlock::getColor(LevelSource* levelSource, const BlockPos& pos, int)
{
    return getColor(levelSource, pos, levelSource->getBlockState(pos));
}

int LiquidBlock::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState*)
{
    if (mMaterial != Material::WATER)
        return 0xffffff;
    return BiomeColors::getAverageWaterColor(levelSource, pos);
}

bool LiquidBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

int LiquidBlock::getDepth(LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* state = Block::getStateByElementId((unsigned short)getLiquidElement(levelSource, pos));
    if (state == NULL || state->getMaterial() != mMaterial)
        return -1;
    return state->getValue(LEVEL);
}

int LiquidBlock::getRenderedDepth(LevelSource* levelSource, const BlockPos& pos)
{
    int depth = getDepth(levelSource, pos);
    return depth > 7 ? 0 : depth;
}

bool LiquidBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool LiquidBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool LiquidBlock::mayPick(const BlockState* state, bool includeLiquid)
{
    return includeLiquid && state->getValue(LEVEL) == 0;
}

bool LiquidBlock::isSolidFace(LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = Block::getStateByElementId((unsigned short)getLiquidElement(levelSource, pos));
    if (state == NULL || state->getMaterial() == mMaterial)
        return false;
    if (direction == Direction::UP)
        return true;
    return !Block::isExceptionBlockForAttachingPlusPiston(state->getBlock()) &&
           !state->getBlock()->isSolidFace(levelSource, pos, direction);
}

bool LiquidBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    BlockPos adjacent = pos.relative(direction);
    const BlockState* adjacentState = levelSource->getBlockState(adjacent);
    if (adjacentState != NULL && adjacentState->getMaterial() == mMaterial)
        return false;
    if (direction == Direction::UP)
        return true;
    return Block::shouldRenderFace(state, levelSource, pos, direction);
}

bool LiquidBlock::shouldRenderBackwardUpFace(LevelSource* levelSource, const BlockPos& pos)
{
    for (int x = -1; x <= 1; ++x)
    {
        const BlockState* state = levelSource->getBlockState(pos.offset(x, 0, -1));
        if (state != NULL && state->getMaterial() != mMaterial && !state->isSolid())
            return true;
        state = levelSource->getBlockState(pos.offset(x, 0, 0));
        if (state != NULL && state->getMaterial() != mMaterial && !state->isSolid())
            return true;
        state = levelSource->getBlockState(pos.offset(x, 0, 1));
        if (state != NULL && state->getMaterial() != mMaterial && !state->isSolid())
            return true;
    }
    return false;
}

EBlockRenderShape LiquidBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(4);
}

Item* LiquidBlock::getResource(const BlockState*, Random*, int)
{
    return Items::air;
}

int LiquidBlock::getResourceCount(Random*)
{
    return 0;
}

Vec3* LiquidBlock::getFlow(LevelSource* levelSource, const BlockPos& pos, const BlockState*)
{
    int depth = getRenderedDepth(levelSource, pos);
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    const Direction* directions[4] = { Direction::WEST, Direction::EAST, Direction::NORTH, Direction::SOUTH };
    int i;
    for (i = 0; i < 4; ++i)
    {
        const BlockPos adjacent = pos.relative(directions[i]);
        int adjacentDepth = getRenderedDepth(levelSource, adjacent);
        if (adjacentDepth < 0)
        {
            const BlockState* adjacentState = levelSource->getBlockState(adjacent);
            if (adjacentState != NULL && !adjacentState->isSolid())
            {
                int belowDepth = getRenderedDepth(levelSource, adjacent.below());
                if (belowDepth >= 0)
                    adjacentDepth = belowDepth - (depth - 8);
            }
        }
        if (adjacentDepth >= 0)
        {
            int difference = adjacentDepth - depth;
            x += directions[i] == Direction::WEST ? -difference : directions[i] == Direction::EAST ? difference : 0;
            z += directions[i] == Direction::NORTH ? -difference : directions[i] == Direction::SOUTH ? difference : 0;
        }
    }
    if (depth >= 0 && depth < 8)
        y = 0.0;
    return Vec3::newTemp(x, y, z)->normalize();
}

Vec3* LiquidBlock::handleEntityInside(Level*, const BlockPos&, boost::shared_ptr<Entity>, Vec3* motion)
{
    return motion;
}

void LiquidBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state, const boost::shared_ptr<Entity>& entity)
{
    Block::entityInside(level, pos, state, entity);
}

int LiquidBlock::getTickDelay(Level* level)
{
    if (mMaterial == Material::WATER)
        return 5;
    if (mMaterial == Material::LAVA)
        return 30;
    return 0;
}

int LiquidBlock::getLightColor(const BlockState*, LevelSource* levelSource, const BlockPos& pos, int lightValue)
{
    int color = levelSource->getLightColor(pos, 0, lightValue);
    int above = levelSource->getLightColor(pos.above(), 0, lightValue);
    int red = color & 255;
    int blue = above & 255;
    if (red < blue)
        red = blue;
    int green = (color >> 16) & 255;
    int aboveGreen = (above >> 16) & 255;
    if (aboveGreen < green)
        aboveGreen = green;
    return red | (aboveGreen << 16);
}

int LiquidBlock::getRenderLayer()
{
    return mMaterial == Material::WATER ? 2 : 0;
}

void LiquidBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (mMaterial == Material::WATER)
    {
        if (random->nextInt(64) == 0)
            level->addParticle(PARTICLE_SUSPENDED, pos.x + random->nextDouble(), pos.y + random->nextDouble(),
                               pos.z + random->nextDouble(), 0.0, 0.0, 0.0, arrayWithLength<int>(2, true));

        const BlockState* above = level->getBlockState(pos.above());
        if (above != NULL && above->getMaterial() == Material::AIR && random->nextInt(10) == 0)
            level->addParticle(PARTICLE_DRIP_WATER, pos.x + random->nextFloat(), pos.y + 1.0,
                               pos.z + random->nextFloat(), 0.0, 0.0, 0.0,
                               arrayWithLength<int>(2, true));
    }
    else if (mMaterial == Material::LAVA)
    {
        if (random->nextInt(100) == 0)
            level->addParticle(PARTICLE_LAVA, pos.x + random->nextDouble(), pos.y + 1.0,
                               pos.z + random->nextDouble(), 0.0, 0.0, 0.0,
                               arrayWithLength<int>(2, true));

        const BlockState* above = level->getBlockState(pos.above());
        if (above != NULL && above->getMaterial() == Material::AIR && random->nextInt(200) == 0)
            level->addParticle(PARTICLE_LARGE_SMOKE, pos.x + random->nextFloat(), pos.y + 1.0,
                               pos.z + random->nextFloat(), 0.0, 0.0, 0.0,
                               arrayWithLength<int>(2, true));
    }
}

LiquidBlock* LiquidBlock::getDynamicLiquid(Material* material)
{
    return dynamic_cast<LiquidBlock*>(Block::byId(material == Material::WATER ? 8 : 10));
}

double LiquidBlock::getSlopeAngle(LevelSource* levelSource, const BlockPos& pos, Material* material, const BlockState* state)
{
    LiquidBlock* liquid = getDynamicLiquid(material);
    if (liquid == NULL)
        return -1000.0;
    Vec3* flow = liquid->getFlow(levelSource, pos, state);
    if (flow->x == 0.0 && flow->z == 0.0)
        return -1000.0;
    return std::atan2(flow->z, flow->x) - 1.5707963705062866;
}

void LiquidBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    updateLiquid(level, pos, state);
}

void LiquidBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    updateLiquid(level, pos, state);
}

void LiquidBlock::updateLiquid(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level != NULL && state != NULL)
        level->getBlockState(pos);
}

bool LiquidBlock::mayReplaceWithPlace(LevelSource*, const BlockPos&)
{
    return true;
}

void LiquidBlock::fizz(Level* level, const BlockPos& pos)
{
    level->playLocalSound(pos.x + 0.5, pos.y + 0.5, pos.z + 0.5,
                          SoundEvents::LAVA_EXTINGUISH, SoundSource::eSoundSource_Blocks,
                          0.5f, Math::random() * 0.8f + 1.2f, false, 16.0f);
    for (int i = 0; i < 8; ++i)
    {
        level->addParticle(PARTICLE_LARGE_SMOKE,
                           pos.x + Math::random(), pos.y + 1.2, pos.z + Math::random(),
                           0.0, 0.0, 0.0, arrayWithLength<int>(2, true));
    }
}

const BlockState* LiquidBlock::getBlockState(int data)
{
    TypedBoxed<int> level(data);
    return defaultBlockState()->setBoxedValue(LEVEL, &level);
}

int LiquidBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(LEVEL);
}

BlockStateDefinition* LiquidBlock::createBlockStateDefinition()
{
    const Property* properties[] = { LEVEL };
    return new BlockStateDefinition(this, properties, 1);
}

unsigned int LiquidBlock::getDynamicLiquidElementId(unsigned short elementId)
{
    unsigned int type = elementId >> 4;
    unsigned int data = elementId & 15;
    if (type == 9)
        return data | 0x80;
    if (type == 11)
        return data | 0xa0;
    return elementId;
}

unsigned int LiquidBlock::getStaticLiquidElementId(unsigned short elementId)
{
    unsigned int type = elementId >> 4;
    unsigned int data = elementId & 15;
    if (type == 8)
        return data | 0x90;
    if (type == 10)
        return data | 0xb0;
    return elementId;
}

void LiquidBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mMaterial == Material::WATER ? L"water_still" : L"lava_still");
    mFlowIcon = iconRegister->registerIcon(mMaterial == Material::WATER ? L"water_flow" : L"lava_flow");
    mUseSharedIcon = false;
}

TextureAtlasSprite* LiquidBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::UP || direction == Direction::DOWN)
        return mIcon;
    return mFlowIcon;
}

double LiquidBlock::getBlockLiquidHeight(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    int data = state->getValue(LEVEL);
    if (data == 0 && levelSource->getBlockState(pos.above())->getMaterial() == Material::WATER)
        return 1.0;
    return 1.0 - getHeightFromData(data);
}

double LiquidBlock::getLiquidHeight(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return (double)pos.y + getBlockLiquidHeight(state, levelSource, pos);
}

EBlockFaceShape LiquidBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return (EBlockFaceShape)8;
}

void LiquidBlock::setLiquidElement(Level* level, const BlockPos& pos, unsigned short elementId, bool remove, int flags)
{
    const BlockState* state = Block::getStateByElementId(elementId);
    if (remove)
        level->removeBlock(pos);
    else
        level->setBlock(pos, state, flags, false);
}

void LiquidBlock::trySpreadFire(Level* level, const BlockPos& pos, Random* random)
{
    if (!level->getGameRules()->getBoolean(0))
        return;

    const BlockState* above = level->getBlockState(pos.above());
    if (above != NULL && above->getMaterial() != Material::AIR &&
        !above->getMaterial()->isFlammable())
        return;

    for (int i = 0; i < 5; ++i)
    {
        BlockPos candidate = pos.offset(random->nextInt(3) - 1,
                                        random->nextInt(3) - 1,
                                        random->nextInt(3) - 1);
        if (candidate.y < 256 && !level->hasChunkAt(candidate))
            return;

        const BlockState* state = level->getBlockState(candidate);
        if (state != NULL && state->getMaterial()->isFlammable())
        {
            const Direction* directions[] = { Direction::WEST, Direction::EAST,
                                               Direction::NORTH, Direction::SOUTH,
                                               Direction::UP, Direction::DOWN };
            for (unsigned int direction = 0; direction < 6; ++direction)
            {
                BlockPos adjacent = candidate.relative(directions[direction]);
                if (level->getBlockState(adjacent)->getMaterial() == Material::AIR)
                {
                    level->setBlock(adjacent, Blocks::FIRE->defaultBlockState(), 11, false);
                    return;
                }
            }
        }

        candidate = pos.offset(random->nextInt(3) - 1,
                               random->nextInt(3) - 1,
                               random->nextInt(3) - 1);
        if (candidate.y < 256 && !level->hasChunkAt(candidate))
            return;

        state = level->getBlockState(candidate);
        if (state != NULL && state->getMaterial()->isFlammable())
        {
            const Direction* directions[] = { Direction::WEST, Direction::EAST,
                                               Direction::NORTH, Direction::SOUTH,
                                               Direction::UP, Direction::DOWN };
            for (unsigned int direction = 0; direction < 6; ++direction)
            {
                BlockPos adjacent = candidate.relative(directions[direction]);
                if (level->getBlockState(adjacent)->getMaterial() == Material::AIR)
                {
                    level->setBlock(adjacent, Blocks::FIRE->defaultBlockState(), 11, false);
                    return;
                }
            }
        }
    }
}
