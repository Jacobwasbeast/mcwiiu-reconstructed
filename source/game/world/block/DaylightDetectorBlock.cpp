#include "game/world/block/DaylightDetectorBlock.hpp"

#include "game/core/Boxed.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DaylightDetectorBlockEntity.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/util/Math.hpp"

static Property* gDaylightDetectorPower = IntegerProperty::create(L"power", 0, 15);

DaylightDetectorBlock::DaylightDetectorBlock(bool inverted)
    : BaseEntityBlock(Material::WOOD), mTopIcon(NULL), mInvertedTopIcon(NULL), mSideIcon(NULL), mInverted(inverted)
{
    DerivedInit();
    TypedBoxed<int> boxed(0);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(gDaylightDetectorPower, &boxed));
}

DaylightDetectorBlock::~DaylightDetectorBlock()
{
}

boost::shared_ptr<BlockEntity> DaylightDetectorBlock::newBlockEntity(Level*, int data)
{
    boost::shared_ptr<DaylightDetectorBlockEntity> blockEntity(new DaylightDetectorBlockEntity());
    blockEntity->setData(data);
    return blockEntity;
}

const AABB* DaylightDetectorBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

int DaylightDetectorBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction*)
{
    return state->getValue(static_cast<const AbstractProperty<int>*>(gDaylightDetectorPower));
}

void DaylightDetectorBlock::updateSignalStrength(Level* level, const BlockPos& pos)
{
    if (level->mDimension->isHasSkyLight())
    {
        int sky = level->getRawBrightness(pos);
        int darken = level->getSkyDarken();
        unsigned int strength = sky - darken;
        float sunAngle = level->getSunAngle(1.0f);
        if (mInverted)
            strength = 15 - strength;
        if (strength > 0 && !mInverted)
        {
            float factor = 6.2831855f;
            if (sunAngle < 3.1415927f)
                factor = 0.25f;
            strength = Math::round((float)((15 - strength) * Mth::cos((factor - sunAngle) * 0.2f + sunAngle)));
        }
        int value = Mth::clamp((int)strength, 0, 15);
        const BlockState* oldState = level->getBlockState(pos);
        if (oldState->getValue(static_cast<const AbstractProperty<int>*>(gDaylightDetectorPower)) != value)
        {
            TypedBoxed<int> boxed(value);
            const BlockState* state = oldState->setBoxedValue(gDaylightDetectorPower, &boxed);
            level->setBlock(pos, state, 19, false);
        }
    }
}

bool DaylightDetectorBlock::use(Level* level, const BlockPos& pos, const BlockState* state,
                                boost::shared_ptr<Player>, InteractionHand::EInteractionHand,
                                const Direction*, float, float, float, bool)
{
    Block* block = mInverted ? Blocks::daylight_detector : Blocks::daylight_detector_inverted;
    level->setBlock(pos, block->defaultBlockState(), 4, false);
    level->updateNeighborsAt(pos, this, false);
    return true;
}

Item* DaylightDetectorBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::daylight_detector);
}

not_null_ptr<ItemInstance> DaylightDetectorBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::daylight_detector)));
}

bool DaylightDetectorBlock::isInverted()
{
    return mInverted;
}

bool DaylightDetectorBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool DaylightDetectorBlock::isSolidRender(const BlockState*) const
{
    return false;
}

EBlockRenderShape DaylightDetectorBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_MODEL;
}

bool DaylightDetectorBlock::isSignalSource(const BlockState*)
{
    return true;
}

const BlockState* DaylightDetectorBlock::getBlockState(int data)
{
    TypedBoxed<int> boxed(data);
    return defaultBlockState()->setBoxedValue(gDaylightDetectorPower, &boxed);
}

int DaylightDetectorBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(static_cast<const AbstractProperty<int>*>(gDaylightDetectorPower));
}

unsigned int DaylightDetectorBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    return Block::GetInteractTooltip(tooltipData);
}

unsigned int DaylightDetectorBlock::GetHitTooltip(const BlockTooltipDataHolder& tooltipData)
{
    return 0xffffffffu;
}

BlockStateDefinition* DaylightDetectorBlock::createBlockStateDefinition()
{
    const Property* properties[1] = { gDaylightDetectorPower };
    return new BlockStateDefinition(this, properties, 1);
}

EPistonPushReaction DaylightDetectorBlock::getPistonPushReaction(const BlockState*)
{
    return (EPistonPushReaction)0;
}

EBlockFaceShape DaylightDetectorBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction* direction)
{
    return direction == Direction::DOWN ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* DaylightDetectorBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction != Direction::UP)
        return mSideIcon;
    return mInverted ? mInvertedTopIcon : mTopIcon;
}

void DaylightDetectorBlock::registerIcons(IconRegister* iconRegister)
{
    mTopIcon = iconRegister->registerIcon(mIconName + L"_top");
    mInvertedTopIcon = iconRegister->registerIcon(mIconName + L"_inverted_top");
    mSideIcon = iconRegister->registerIcon(mIconName + L"_side");
}
