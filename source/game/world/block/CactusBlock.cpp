#include "game/world/block/CactusBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/entity/mob/Entity.hpp"
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

static AbstractProperty<int>* AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 15));
static const AABB* CACTUS_SHAPE = AABB::newPermanent(1.0 / 16.0, 0.0, 1.0 / 16.0,
                                                      15.0 / 16.0, 1.0, 15.0 / 16.0);

CactusBlock::CactusBlock()
    : Block(Material::CACTUS)
{
    DerivedInit();
    TypedBoxed<int> age(0);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(AGE, &age));
    setTicking(true);
}

CactusBlock::~CactusBlock()
{
}

void CactusBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (level->isEmptyBlock(pos.above()))
    {
        int height = 1;
        while (level->getBlockState(pos.below(height))->getBlock() == this)
            ++height;
        if (height <= 3)
        {
            int age = state->getValue(AGE);
            if (age == 15)
            {
                level->setBlock(pos.above(), defaultBlockState(), 4, false);
                TypedBoxed<int> reset(0);
                level->setBlock(pos, state->setBoxedValue(AGE, &reset), 4, false);
            }
            else
            {
                TypedBoxed<int> next(age + 1);
                level->setBlock(pos, state->setBoxedValue(AGE, &next), 4, false);
            }
        }
    }
}

const AABB* CactusBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

const AABB* CactusBlock::getOutlineAABB(const BlockState*, Level*, const BlockPos& pos)
{
    return CACTUS_SHAPE->move(pos);
}

bool CactusBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool CactusBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool CactusBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return Block::mayPlace(level, pos) && canSurvive(level, pos, NULL);
}

void CactusBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!canSurvive(level, pos, NULL))
        level->blockUpdated(pos, this, true);
}

bool CactusBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        const BlockState* neighbor = level->getBlockState(pos.relative(directions.data[i]));
        if (neighbor->getMaterial()->isSolid() || neighbor->getMaterial() == Material::CACTUS)
            return false;
    }

    const Material* belowMaterial = level->getBlockState(pos.below())->getMaterial();
    if (belowMaterial != Material::CACTUS && belowMaterial != Material::SAND)
        return false;
    return !level->getBlockState(pos.above())->getMaterial()->blocksMotion();
}

void CactusBlock::entityInside(Level*, const BlockPos&, const BlockState*, const boost::shared_ptr<Entity>& entity)
{
    entity->hurt(NULL, 1.0f);
}

int CactusBlock::getRenderLayer()
{
    return 0;
}

const BlockState* CactusBlock::getBlockState(int data)
{
    TypedBoxed<int> age(data);
    return defaultBlockState()->setBoxedValue(AGE, &age);
}

int CactusBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(AGE);
}

BlockStateDefinition* CactusBlock::createBlockStateDefinition()
{
    const Property* properties[] = { AGE };
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape CactusBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

bool CactusBlock::shouldBlockTick(Level* level, const BlockPos& pos, const BlockState*)
{
    return level->isEmptyBlock(pos.above());
}

TextureAtlasSprite* CactusBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::UP)
        return mTopIcon;
    if (direction == Direction::DOWN)
        return mBottomIcon;
    return mIcon;
}

void CactusBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"cactus_side");
    mTopIcon = iconRegister->registerIcon(L"cactus_top");
    mBottomIcon = iconRegister->registerIcon(L"cactus_bottom");
}

EBlockRenderShape CactusBlock::getRenderShape(const BlockState*)
{
    return (EBlockRenderShape)13;
}
