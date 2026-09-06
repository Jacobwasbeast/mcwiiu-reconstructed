#include "game/world/block/AnvilBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/ui/AnvilMenu.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/world/block/Blocks.hpp"

DirectionProperty* AnvilBlock::FACING = NULL;
AbstractProperty<int>* AnvilBlock::DAMAGE =
    static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"damage", 0, 2));

static const AABB* ANVIL_X_SHAPE = AABB::newPermanent(0.0, 0.0, 0.125, 1.0, 0.75, 0.875);
static const AABB* ANVIL_Z_SHAPE = AABB::newPermanent(0.125, 0.0, 0.0, 0.875, 0.75, 1.0);

AnvilBlock::AnvilBlock()
    : FallingBlock(Material::HEAVY_METAL), mTextureType(0), mTopIcons()
{
    Block::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    state = state->setBoxedValue(FACING, &facing);
    TypedBoxed<int> damage(0);
    registerDefaultState(state->setBoxedValue(DAMAGE, &damage));
    setLightBlock(0);
}

AnvilBlock::~AnvilBlock()
{
}

void AnvilBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING;
}

bool AnvilBlock::isCubeShaped(const BlockState*)
{
    return false;
}

EBlockFaceShape AnvilBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return static_cast<EBlockFaceShape>(8);
}

bool AnvilBlock::isSolidRender(const BlockState*) const
{
    return false;
}

const BlockState* AnvilBlock::getStateForPlacement(Level*, const BlockPos& pos, const Direction*, float, float,
                                                    float, int, boost::shared_ptr<LivingEntity> placer)
{
    const Direction* facing = Direction::getDirectionFromLivingEntity(pos, placer)->getClockWise();
    TypedBoxed<const Direction*> boxedFacing(facing);
    TypedBoxed<int> damage(0);
    const BlockState* state = defaultBlockState()->setBoxedValue(FACING, &boxedFacing);
    return state->setBoxedValue(DAMAGE, &damage);
}

bool AnvilBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (!clientSide)
    {
        Anvil anvil(level, pos);
        player->startBlockInteraction(&anvil);
    }
    return true;
}

int AnvilBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(DAMAGE);
}

const AABB* AnvilBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(FACING)->getAxis() == Direction::Axis::X ? ANVIL_X_SHAPE : ANVIL_Z_SHAPE;
}

bool AnvilBlock::canContainLiquid(const BlockState*) const
{
    return true;
}

void AnvilBlock::falling(boost::shared_ptr<FallingBlockEntity> entity)
{
    entity->setHurtsEntities(true);
}

void AnvilBlock::onLand(Level* level, const BlockPos& pos, const BlockState*, const BlockState*)
{
    level->levelEvent(0x407, pos, 0);
}

void AnvilBlock::onBroken(Level* level, const BlockPos& pos)
{
    level->levelEvent(0x405, pos, 0);
}

bool AnvilBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction*)
{
    return true;
}

const BlockState* AnvilBlock::getBlockState(int data)
{
    const Direction* facing = Direction::from2DDataValue(data & 3);
    TypedBoxed<const Direction*> boxedFacing(facing);
    TypedBoxed<int> damage((data & 0xf) >> 2);
    const BlockState* state = defaultBlockState()->setBoxedValue(FACING, &boxedFacing);
    return state->setBoxedValue(DAMAGE, &damage);
}

int AnvilBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(FACING)->get2DDataValue() | (state->getValue(DAMAGE) << 2);
}

unsigned int AnvilBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseItem || tooltipData.m_bCanDestroy) && tooltipData.m_bIsInside)
        return 0xea4e897e;
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* AnvilBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}

BlockStateDefinition* AnvilBlock::createBlockStateDefinition()
{
    const Property* properties[] = { FACING, DAMAGE };
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* AnvilBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (mTextureType == 3 && direction == Direction::UP)
        return mTopIcons[state->getValue(DAMAGE)];
    return mIcon;
}

void AnvilBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"anvil_base");
    mTopIcons[0] = iconRegister->registerIcon(L"anvil_top");
    mTopIcons[1] = iconRegister->registerIcon(L"anvil_top_damaged");
    mTopIcons[2] = iconRegister->registerIcon(L"anvil_top_damaged_2");
}

EBlockRenderShape AnvilBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(0x23);
}

AnvilBlock::Anvil::Anvil(Level* level, const BlockPos& pos)
    : InteractionObject(), mLevel(level), mBlockPos(pos)
{
}

AbstractContainerMenu* AnvilBlock::Anvil::createMenu(boost::shared_ptr<Inventory> inventory,
                                                      boost::shared_ptr<Player> player)
{
    return new AnvilMenu(inventory, mLevel, mBlockPos, player);
}

int AnvilBlock::Anvil::getTypeId(bool)
{
    return 0x13;
}

std::wstring AnvilBlock::Anvil::getName()
{
    return L"anvil";
}

std::wstring AnvilBlock::Anvil::getCustomName()
{
    return L"anvil";
}

bool AnvilBlock::Anvil::hasCustomName()
{
    return false;
}

std::wstring AnvilBlock::Anvil::getDisplayName()
{
    return std::wstring(L10N::GetString(Blocks::anvil->getDescriptionId(-1)));
}
