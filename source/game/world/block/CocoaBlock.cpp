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
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
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

AbstractProperty<int>* CocoaBlock::AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 2));

static const AABB* COCOA_EAST_SHAPES[3] =
{
    AABB::newPermanent(0.6875, 0.4375, 0.375, 0.9375, 0.75, 0.625),
    AABB::newPermanent(0.5625, 0.3125, 0.3125, 0.9375, 0.75, 0.6875),
    AABB::newPermanent(0.4375, 0.1875, 0.25, 0.9375, 0.75, 0.75)
};
static const AABB* COCOA_WEST_SHAPES[3] =
{
    AABB::newPermanent(0.0625, 0.4375, 0.375, 0.3125, 0.75, 0.625),
    AABB::newPermanent(0.0625, 0.3125, 0.3125, 0.4375, 0.75, 0.6875),
    AABB::newPermanent(0.0625, 0.1875, 0.25, 0.5625, 0.75, 0.75)
};
static const AABB* COCOA_NORTH_SHAPES[3] =
{
    AABB::newPermanent(0.375, 0.4375, 0.0625, 0.625, 0.75, 0.3125),
    AABB::newPermanent(0.3125, 0.3125, 0.0625, 0.6875, 0.75, 0.4375),
    AABB::newPermanent(0.25, 0.1875, 0.0625, 0.75, 0.75, 0.5625)
};
static const AABB* COCOA_SOUTH_SHAPES[3] =
{
    AABB::newPermanent(0.375, 0.4375, 0.6875, 0.625, 0.75, 0.9375),
    AABB::newPermanent(0.3125, 0.3125, 0.5625, 0.6875, 0.75, 0.9375),
    AABB::newPermanent(0.25, 0.1875, 0.4375, 0.75, 0.75, 0.9375)
};

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
    level->setBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
    spawnResources(level, pos, state, 0.0f, 0);
}

void CocoaBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!canSurvive(level, pos, state))
    {
        removeAndSpawn(level, pos, state);
        return;
    }
    if (level->mRandom->nextInt(5) == 0)
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
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    const BlockState* support = level->getBlockState(pos.relative(facing));
    return support->getBlock() == Blocks::log
        && support->getValue(OldLogBlock::VARIANT) == PlanksBlock::Variant::JUNGLE;
}

bool CocoaBlock::isCubeShaped(const BlockState*) { return false; }
bool CocoaBlock::isSolidRender(const BlockState*) const { return false; }

const AABB* CocoaBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    int age = state->getValue(AGE);
    switch (state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->get3DDataValue())
    {
    case Direction::eDirection_South: return COCOA_SOUTH_SHAPES[age];
    case Direction::eDirection_West: return COCOA_WEST_SHAPES[age];
    case Direction::eDirection_East: return COCOA_EAST_SHAPES[age];
    default: return COCOA_NORTH_SHAPES[age];
    }
}

const BlockState* CocoaBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)));
    return state->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
}

const BlockState* CocoaBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)));
}

void CocoaBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance>)
{
    const Direction* facing = Direction::fromYRot(placer->mYRot);
    TypedBoxed<const Direction*> boxed(facing);
    level->setBlock(pos, state->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &boxed), 2, false);
}

const BlockState* CocoaBlock::getStateForPlacement(Level*, const BlockPos&, const Direction* direction,
                                                   float, float, float, int auxValue,
                                                   boost::shared_ptr<LivingEntity>)
{
    if (!direction->getAxis()->isHorizontal())
        return NULL;
    TypedBoxed<const Direction*> facing(direction->getOpposite());
    TypedBoxed<int> age((auxValue & 0xf) >> 2);
    return defaultBlockState()->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing)->setBoxedValue(AGE, &age);
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
    return defaultBlockState()->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing)->setBoxedValue(AGE, &age);
}

int CocoaBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->get2DDataValue() | (state->getValue(AGE) << 2);
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
    const Property* properties[] = { HorizontalDirectionalBlock::FACING_PROPERTY, AGE };
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
