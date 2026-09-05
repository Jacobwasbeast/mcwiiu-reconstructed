#include "game/world/block/ChorusFlowerBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

static AbstractProperty<int>* CHORUS_AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 5));

static inline AABB* createChorusShape(double minX, double minY, double minZ, double maxX, double maxY, double maxZ)
{
    AABB* shape = AABB::newPermanent(minX, minY, minZ, maxX, maxY, maxZ);
    shape->divideInternalsBy(16.0);
    return shape;
}

static const AABB* CHORUS_UP_SHAPE = createChorusShape(2.0, 14.0, 2.0, 14.0, 16.0, 14.0);
static const AABB* CHORUS_DOWN_SHAPE = createChorusShape(2.0, 0.0, 2.0, 14.0, 14.0, 14.0);
static const AABB* CHORUS_SOUTH_SHAPE = createChorusShape(2.0, 2.0, 14.0, 14.0, 14.0, 16.0);
static const AABB* CHORUS_NORTH_SHAPE = createChorusShape(2.0, 2.0, 0.0, 14.0, 14.0, 2.0);
static const AABB* CHORUS_WEST_SHAPE = createChorusShape(0.0, 2.0, 2.0, 2.0, 14.0, 14.0);
static const AABB* CHORUS_EAST_SHAPE = createChorusShape(14.0, 2.0, 2.0, 16.0, 14.0, 14.0);

ChorusFlowerBlock::ChorusFlowerBlock()
    : Block(Material::PLANT, MaterialColor::COLOR_PURPLE)
{
    DerivedInit();
}

ChorusFlowerBlock::~ChorusFlowerBlock()
{
}

const AABB* ChorusFlowerBlock::getUpShape() { return CHORUS_UP_SHAPE; }
const AABB* ChorusFlowerBlock::getDownShape() { return CHORUS_DOWN_SHAPE; }
const AABB* ChorusFlowerBlock::getSouthShape() { return CHORUS_SOUTH_SHAPE; }
const AABB* ChorusFlowerBlock::getNorthShape() { return CHORUS_NORTH_SHAPE; }
const AABB* ChorusFlowerBlock::getEastShape() { return CHORUS_EAST_SHAPE; }
const AABB* ChorusFlowerBlock::getWestShape() { return CHORUS_WEST_SHAPE; }

void ChorusFlowerBlock::DerivedInit()
{
    Block::DerivedInit();
    TypedBoxed<int> age(0);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(CHORUS_AGE, &age));
    setTicking(true);
    mCulledFace = NULL;
}

Item* ChorusFlowerBlock::getResource(const BlockState*, Random*, int)
{
    return Items::air;
}

bool ChorusFlowerBlock::isDead(int age)
{
    return age > 4;
}

bool ChorusFlowerBlock::allNeighborsEmpty(Level* level, const BlockPos& pos, const Direction* excluded)
{
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        const Direction* direction = directions.data[i];
        if (direction != excluded && !level->isEmptyBlock(pos.relative(direction)))
        {
            return false;
        }
    }
    return true;
}

void ChorusFlowerBlock::placeGrownFlower(Level* level, const BlockPos& pos, int age)
{
    TypedBoxed<int> value(age);
    level->setBlock(pos, defaultBlockState()->setBoxedValue(CHORUS_AGE, &value), 2, false);
    level->levelEvent(0x409, pos, 0);
}

void ChorusFlowerBlock::placeDeadFlower(Level* level, const BlockPos& pos)
{
    TypedBoxed<int> value(5);
    level->setBlock(pos, defaultBlockState()->setBoxedValue(CHORUS_AGE, &value), 2, false);
    level->levelEvent(0x40A, pos, 0);
}

void ChorusFlowerBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (!canSurvive(level, pos, state))
    {
        level->destroyBlock(pos, true);
        return;
    }

    BlockPos above = pos.above();
    if (!level->isEmptyBlock(above) || above.y > 255)
    {
        return;
    }

    int age = state->getValue(CHORUS_AGE);
    if (isDead(age) || random->nextInt(1) != 0)
    {
        return;
    }

    bool rooted = false;
    const BlockState* belowState = level->getBlockState(pos.below());
    Block* belowBlock = belowState->getBlock();
    bool growUp = true;

    if (belowBlock != Blocks::end_stone)
    {
        if (belowBlock == Blocks::chorus_plant)
        {
            int stemHeight = 1;
            Block* support = Blocks::chorus_plant;
            while (stemHeight < 5)
            {
                support = level->getBlock(pos.below(stemHeight + 1));
                if (support != Blocks::chorus_plant)
                {
                    break;
                }
                ++stemHeight;
            }

            rooted = support == Blocks::end_stone;
            if (stemHeight > 1 && random->nextInt(rooted ? 5 : 4) < stemHeight)
            {
                growUp = false;
            }
        }
        else if (belowState->getMaterial() != Material::AIR)
        {
            growUp = false;
        }
    }

    if (growUp && allNeighborsEmpty(level, above, NULL) && level->isEmptyBlock(pos.above(2)))
    {
        level->setBlock(pos, Blocks::chorus_plant->defaultBlockState(), 2, false);
        placeGrownFlower(level, above, age);
        return;
    }

    if (age < 4)
    {
        int branches = random->nextInt(4) + (rooted ? 1 : 0);
        bool grew = false;
        for (int i = 0; i < branches; ++i)
        {
            const Direction* direction = Direction::Plane::HORIZONTAL->getRandomDirection(*random);
            BlockPos branch = pos.relative(direction);
            if (level->isEmptyBlock(branch) && level->isEmptyBlock(branch.below())
                && allNeighborsEmpty(level, branch, direction->getOpposite()))
            {
                placeGrownFlower(level, branch, age + 1);
                grew = true;
            }
        }
        if (grew)
        {
            level->setBlock(pos, Blocks::chorus_plant->defaultBlockState(), 2, false);
            return;
        }
    }
    placeDeadFlower(level, pos);
}

bool ChorusFlowerBlock::isCubeShaped(const BlockState*) { return false; }
bool ChorusFlowerBlock::isSolidRender(const BlockState*) const { return false; }

bool ChorusFlowerBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return Block::mayPlace(level, pos) && canSurvive(level, pos, NULL);
}

void ChorusFlowerBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!canSurvive(level, pos, NULL))
    {
        level->addToTickNextTick(pos, this, 1);
    }
}

bool ChorusFlowerBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    const BlockState* below = level->getBlockState(pos.below());
    Block* block = below->getBlock();
    if (block == Blocks::chorus_plant || block == Blocks::end_stone)
    {
        return true;
    }
    if (below->getMaterial() != Material::AIR)
    {
        return false;
    }

    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    int neighbors = 0;
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        const BlockState* neighbor = level->getBlockState(pos.relative(directions.data[i]));
        if (neighbor->getBlock() == Blocks::chorus_plant)
        {
            ++neighbors;
        }
        else if (neighbor->getMaterial() != Material::AIR)
        {
            return false;
        }
    }
    return neighbors == 1;
}

void ChorusFlowerBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                                      const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                                      not_null_ptr<ItemInstance> tool)
{
    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
    Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this))));
}

int ChorusFlowerBlock::getRenderLayer() { return 0; }

const BlockState* ChorusFlowerBlock::getBlockState(int data)
{
    TypedBoxed<int> age(data);
    return defaultBlockState()->setBoxedValue(CHORUS_AGE, &age);
}

int ChorusFlowerBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(CHORUS_AGE);
}

void ChorusFlowerBlock::growTreeRecursive(Level* level, const BlockPos& pos, Random* random,
                                          const BlockPos& origin, int radius, int depth)
{
    int height = random->nextInt(4) + 1;
    if (depth == 0)
    {
        ++height;
    }
    for (int i = 1; i <= height; ++i)
    {
        BlockPos branch = pos.above(i);
        if (!allNeighborsEmpty(level, branch, NULL))
        {
            return;
        }
        level->setBlock(branch, Blocks::chorus_plant->defaultBlockState(), 2, false);
    }
    if (depth < 4)
    {
        int branches = random->nextInt(4) + (depth == 0 ? 1 : 0);
        bool grew = false;
        for (int i = 0; i < branches; ++i)
        {
            const Direction* direction = Direction::Plane::HORIZONTAL->getRandomDirection(*random);
            BlockPos branch = pos.above(height).relative(direction);
            if (Mth::abs(branch.x - origin.x) < radius
                && Mth::abs(branch.z - origin.z) < radius
                && level->isEmptyBlock(branch) && level->isEmptyBlock(branch.below())
                && allNeighborsEmpty(level, branch, direction->getOpposite()))
            {
                level->setBlock(branch, Blocks::chorus_plant->defaultBlockState(), 2, false);
                growTreeRecursive(level, branch, random, origin, radius, depth + 1);
                grew = true;
            }
        }
        if (grew)
        {
            return;
        }
    }
    TypedBoxed<int> dead(5);
    level->setBlock(pos.above(height), Blocks::chorus_flower->defaultBlockState()->setBoxedValue(CHORUS_AGE, &dead), 2, false);
}

void ChorusFlowerBlock::generatePlant(Level* level, BlockPos pos, Random* random, int radius)
{
    level->setBlock(pos, Blocks::chorus_plant->defaultBlockState(), 2, false);
    growTreeRecursive(level, pos, random, pos, radius, 0);
}

void ChorusFlowerBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"chorus_flower");
    mDeadIcon = iconRegister->registerIcon(L"chorus_flower_dead");
    mPlantIcon = iconRegister->registerIcon(L"chorus_flower_plant");
}

TextureAtlasSprite* ChorusFlowerBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::DOWN)
    {
        return mPlantIcon;
    }
    return isDead(state->getValue(CHORUS_AGE)) ? mDeadIcon : mIcon;
}

EBlockRenderShape ChorusFlowerBlock::getRenderShape(const BlockState*) { return (EBlockRenderShape)45; }
bool ChorusFlowerBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction* direction) { return direction != mCulledFace; }
void ChorusFlowerBlock::setCulledFace(const Direction* direction) { mCulledFace = direction; }
bool ChorusFlowerBlock::isWaterBlocking() const { return false; }
bool ChorusFlowerBlock::waterSpreadCausesSpawn() { return true; }
bool ChorusFlowerBlock::canContainLiquid(const BlockState*) const { return false; }
bool ChorusFlowerBlock::canProvideSupport(const BlockState*, const Direction*, EBlockSupportType) { return false; }
not_null_ptr<ItemInstance> ChorusFlowerBlock::getSilkTouchItemInstance(const BlockState*) { return ItemInstance::EMPTY; }

BlockStateDefinition* ChorusFlowerBlock::createBlockStateDefinition()
{
    const Property* properties[] = { CHORUS_AGE };
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape ChorusFlowerBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}
