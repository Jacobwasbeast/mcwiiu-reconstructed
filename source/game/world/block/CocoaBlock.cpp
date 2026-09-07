#include "game/world/block/CocoaBlock.hpp"

#include <string>

#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

namespace
{
    std::wstring gCocoaIconNames[3] =
    {
        L"cocoa_stage_0", L"cocoa_stage_1", L"cocoa_stage_2"
    };
}

void __T7()
{
    for (int i = 0; i < 3; ++i)
        gCocoaIconNames[i].~basic_string();
}

static AbstractProperty<int>* AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 2));
static const AABB* SHAPES[3][4];

static const AABB* cocoaShape(double x0, double y0, double z0, double x1, double y1, double z1)
{
    return AABB::newPermanent(x0, y0, z0, x1, y1, z1);
}

static void initCocoaShapes()
{
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;
        const double sizes[3] = { 0.375, 0.5, 0.625 };
        int age;
        for (age = 0; age < 3; ++age)
        {
            double half = sizes[age] / 2.0;
            double low = 0.5 - half;
            double high = 0.5 + half;
            SHAPES[age][0] = cocoaShape(low, low, 0.0, high, high, sizes[age]);
            SHAPES[age][1] = cocoaShape(low, low, 1.0 - sizes[age], high, high, 1.0);
            SHAPES[age][2] = cocoaShape(0.0, low, low, sizes[age], high, high);
            SHAPES[age][3] = cocoaShape(1.0 - sizes[age], low, low, 1.0, high, high);
        }
    }
}

CocoaBlock::CocoaBlock()
    : HorizontalDirectionalBlock(Material::PLANT)
{
    DerivedInit();
    setTicking(true);
    mIcons[0] = NULL;
    mIcons[1] = NULL;
    mIcons[2] = NULL;
}

CocoaBlock::~CocoaBlock()
{
}

bool CocoaBlock::isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool)
{
    int age = state->getValue(AGE);
    return age < 2;
}

bool CocoaBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

void CocoaBlock::performBonemeal(Level* level, Random*, const BlockPos& pos, const BlockState* state)
{
    TypedBoxed<int> age(state->getValue(AGE) + 1);
    level->setBlock(pos, state->setBoxedValue(AGE, &age), 2, false);
}

void CocoaBlock::removeAndSpawn(Level* level, const BlockPos& pos, const BlockState* state)
{
    level->setBlock(pos, defaultBlockState(), 3, false);
    spawnResources(level, pos, state, 0.0f, 0);
}

void CocoaBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (!canSurvive(level, pos, state))
    {
        removeAndSpawn(level, pos, state);
        return;
    }
    if (random->nextInt(5) == 0)
    {
        int age = state->getValue(AGE);
        if (age < 2)
        {
            TypedBoxed<int> nextAge(age + 1);
            level->setBlock(pos, state->setBoxedValue(AGE, &nextAge), 2, false);
        }
    }
}

bool CocoaBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING);
    const BlockState* support = level->getBlockState(pos.relative(facing));
    return support->isSolid();
}

bool CocoaBlock::isCubeShaped(const BlockState*) { return false; }
bool CocoaBlock::isSolidRender(const BlockState*) const { return false; }

const AABB* CocoaBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    initCocoaShapes();
    int age = state->getValue(AGE);
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING);
    int index = facing == Direction::SOUTH ? 0 : facing == Direction::NORTH ? 1 : facing == Direction::WEST ? 2 : 3;
    return SHAPES[age][index];
}

const BlockState* CocoaBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(HorizontalDirectionalBlock::FACING)));
    return state->setBoxedValue(HorizontalDirectionalBlock::FACING, &facing);
}

const BlockState* CocoaBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(HorizontalDirectionalBlock::FACING)));
}

void CocoaBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance>)
{
    const Direction* facing = Direction::fromYRot(placer->mYRot);
    TypedBoxed<const Direction*> boxed(facing);
    level->setBlock(pos, state->setBoxedValue(HorizontalDirectionalBlock::FACING, &boxed), 2, false);
}

const BlockState* CocoaBlock::getStateForPlacement(Level*, const BlockPos&, const Direction* direction,
                                                   float, float, float, int auxValue,
                                                   boost::shared_ptr<LivingEntity>)
{
    if (!direction->getAxis()->isHorizontal())
        return NULL;
    TypedBoxed<const Direction*> facing(direction->getOpposite());
    TypedBoxed<int> age((auxValue & 0xf) >> 2);
    return defaultBlockState()->setBoxedValue(HorizontalDirectionalBlock::FACING, &facing)->setBoxedValue(AGE, &age);
}

void CocoaBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!canSurvive(level, pos, state))
        removeAndSpawn(level, pos, state);
}

void CocoaBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float, int)
{
    int count = state->getValue(AGE) > 1 ? 3 : 1;
    while (count-- > 0)
        Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, DyeColor::BROWN->getItemData())));
}

not_null_ptr<ItemInstance> CocoaBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, DyeColor::BROWN->getItemData()));
}

int CocoaBlock::getRenderLayer() { return 0; }

const BlockState* CocoaBlock::getBlockState(int data)
{
    TypedBoxed<const Direction*> facing(Direction::from2DDataValue(data));
    TypedBoxed<int> age((data & 0xf) >> 2);
    return defaultBlockState()->setBoxedValue(HorizontalDirectionalBlock::FACING, &facing)->setBoxedValue(AGE, &age);
}

int CocoaBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(HorizontalDirectionalBlock::FACING)->get2DDataValue() | (state->getValue(AGE) << 2);
}

unsigned int CocoaBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_pItemInstance->getItem() == Items::dye
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
        && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
        return 0x9f5fdc40u;
    return Block::GetInteractTooltip(tooltipData);
}

BlockStateDefinition* CocoaBlock::createBlockStateDefinition()
{
    const Property* properties[] = { HorizontalDirectionalBlock::FACING, AGE };
    return new BlockStateDefinition(this, properties, 2);
}

EBlockFaceShape CocoaBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* CocoaBlock::getTexture(const Direction*, const BlockState*)
{
    return mIcons[2];
}

TextureAtlasSprite* CocoaBlock::getTextureForAge(int age)
{
    if (age > 2)
        age = 2;
    return mIcons[age];
}

void CocoaBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 3; ++i)
        mIcons[i] = iconRegister->registerIcon(gCocoaIconNames[i]);
}

EBlockRenderShape CocoaBlock::getRenderShape(const BlockState*)
{
    return (EBlockRenderShape)28;
}
