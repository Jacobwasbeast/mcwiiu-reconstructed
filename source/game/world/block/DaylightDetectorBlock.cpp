#include "game/world/block/DaylightDetectorBlock.hpp"

#include "game/core/Boxed.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
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
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/util/Math.hpp"

static AbstractProperty<int>* gDaylightDetectorPower =
    static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"power", 0, 15));
static const AABB* gDaylightDetectorShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.375, 1.0);
static const unsigned int DAYLIGHT_DETECTOR_ADVENTURE_TOOLTIP = 0xfe985704u;

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

boost::shared_ptr<BlockEntity> DaylightDetectorBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<DaylightDetectorBlockEntity> blockEntity(new DaylightDetectorBlockEntity());
    blockEntity->setLevel(level);
    return blockEntity;
}

const AABB* DaylightDetectorBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return gDaylightDetectorShape;
}

int DaylightDetectorBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction*)
{
    return state->getValue(gDaylightDetectorPower);
}

void DaylightDetectorBlock::updateSignalStrength(Level* level, const BlockPos& pos)
{
    if (level->mDimension->isHasSkyLight())
    {
        const BlockState* oldState = level->getBlockState(pos);
        int brightness = level->getBrightness(LightLayer::SKY, pos);
        int darken = level->getSkyDarken();
        int strength = brightness - darken;
        float sunAngle = level->getSunAngle(1.0f);
        if (mInverted)
            strength = 15 - strength;
        if (strength > 0 && !mInverted)
        {
            float target = 6.2831855f;
            if (sunAngle < 3.1415927f)
                target = 0.0f;
            strength = Math::round((float)strength * Mth::cos((target - sunAngle) * 0.2f + sunAngle));
        }
        int value = Mth::clamp(strength, 0, 15);
        if (oldState->getValue(gDaylightDetectorPower) != value)
        {
            TypedBoxed<int> boxed(value);
            const BlockState* state = oldState->setBoxedValue(gDaylightDetectorPower, &boxed);
            level->setBlock(pos, state, 19, false);
        }
    }
}

bool DaylightDetectorBlock::use(Level* level, const BlockPos& pos, const BlockState* state,
                                boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand,
                                const Direction* direction, float hitX, float hitY, float hitZ, bool clientSide)
{
    if (!player->mayBuild())
    {
        return Block::use(level, pos, state, player, hand, direction, hitX, hitY, hitZ, clientSide);
    }

    if (!level->mIsClientSide)
    {
        if (mInverted)
        {
            TypedBoxed<int> boxed(state->getValue(gDaylightDetectorPower));
            Block* block = Blocks::daylight_detector;
            level->setBlock(pos, block->defaultBlockState()->setBoxedValue(gDaylightDetectorPower, &boxed), 4, false);
            static_cast<DaylightDetectorBlock*>(block)->updateSignalStrength(level, pos);
            return true;
        }

        TypedBoxed<int> boxed(state->getValue(gDaylightDetectorPower));
        Block* block = Blocks::daylight_detector_inverted;
        level->setBlock(pos, block->defaultBlockState()->setBoxedValue(gDaylightDetectorPower, &boxed), 4, false);
        static_cast<DaylightDetectorBlock*>(block)->updateSignalStrength(level, pos);
    }
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
    return state->getValue(gDaylightDetectorPower);
}

unsigned int DaylightDetectorBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_bUseItem || tooltipData.m_bCanPlace)
    {
        if (!tooltipData.m_pPlayer->Player::GetGameMode()->isAdventure())
        {
            return DAYLIGHT_DETECTOR_ADVENTURE_TOOLTIP;
        }
    }
    return Block::GetInteractTooltip(tooltipData);
}

unsigned int DaylightDetectorBlock::GetHitTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_bCanPlace && tooltipData.m_bIsInside)
        return 0x5fb7a25au;
    return 0xffffffffu;
}

BlockStateDefinition* DaylightDetectorBlock::createBlockStateDefinition()
{
    const Property* properties[1] = { gDaylightDetectorPower };
    return new BlockStateDefinition(this, properties, 1);
}

EPistonPushReaction DaylightDetectorBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_NORMAL;
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

