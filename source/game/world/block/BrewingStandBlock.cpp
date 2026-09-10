#include "game/world/block/BrewingStandBlock.hpp"

#include <string>

#include "game/core/L10N.hpp"
#include "game/core/IconRegister.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/item/Containers.hpp"
#include "game/item/Items.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/BrewingStandBlockEntity.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"

static AbstractProperty<bool>* gBrewingStandBottle0 =
    static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"has_bottle_0"));
static AbstractProperty<bool>* gBrewingStandBottle1 =
    static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"has_bottle_1"));
static AbstractProperty<bool>* gBrewingStandBottle2 =
    static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"has_bottle_2"));

static const AABB* gBrewingStandShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.875, 1.0);
static const AABB* gBrewingStandBaseShape = AABB::newPermanent(0.125, 0.0, 0.125, 0.875, 0.125, 0.875);

BrewingStandBlock::BrewingStandBlock()
    : BaseEntityBlock(Material::STONE),
      mBaseTexture(0)
{
    DerivedInit();
}

BrewingStandBlock::~BrewingStandBlock()
{
}

boost::shared_ptr<BlockEntity> BrewingStandBlock::newBlockEntity(Level* level, int data)
{
    boost::shared_ptr<BrewingStandBlockEntity> blockEntity(new BrewingStandBlockEntity());
    blockEntity->setLevelForLoading(level);
    blockEntity->setData(data);
    return blockEntity;
}

std::wstring BrewingStandBlock::getName()
{
    return std::wstring(L10N::GetString(0xdf516f15));
}

bool BrewingStandBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool BrewingStandBlock::isCubeShaped(const BlockState*)
{
    return false;
}

void BrewingStandBlock::addCollisionAABBs(const BlockState*, Level*, const BlockPos& pos,
                                          const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                          boost::shared_ptr<Entity>, bool)
{
    addCollisionAABB(pos, entityBox, collidingBoxes, gBrewingStandShape);
    addCollisionAABB(pos, entityBox, collidingBoxes, gBrewingStandBaseShape);
}

const AABB* BrewingStandBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return gBrewingStandBaseShape;
}

bool BrewingStandBlock::use(Level* level, const BlockPos& pos, const BlockState*,
                            boost::shared_ptr<Player> player, InteractionHand::EInteractionHand,
                            const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
        return false;
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    boost::shared_ptr<BrewingStandBlockEntity> brewingStand =
        boost::dynamic_pointer_cast<BrewingStandBlockEntity>(blockEntity);
    if (brewingStand.get() != NULL)
        player->openContainer(brewingStand);
    return true;
}

void BrewingStandBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                    boost::shared_ptr<LivingEntity>, not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance.get() == NULL || !itemInstance->hasCustomHoverName())
        return;
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    boost::shared_ptr<BrewingStandBlockEntity> brewingStand =
        boost::dynamic_pointer_cast<BrewingStandBlockEntity>(blockEntity);
    if (brewingStand.get() != NULL)
    {
        brewingStand->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                                    itemInstance->wasCreatedByRestrictedPlayer(), itemInstance->wasCreatedOnHost());
    }
}

void BrewingStandBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    level->addParticle(PARTICLE_SMOKE, pos.x + 0.4 + random->nextFloat() * 0.2,
                       pos.y + 0.7, pos.z + 0.4 + random->nextFloat() * 0.2,
                       0.0, 0.0, 0.0, arrayWithLength<int>());
}

void BrewingStandBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    boost::shared_ptr<BrewingStandBlockEntity> brewingStand =
        boost::dynamic_pointer_cast<BrewingStandBlockEntity>(blockEntity);
    if (brewingStand.get() != NULL)
        Containers::dropContents(level, pos, brewingStand);
    BaseEntityBlock::onRemove(level, pos, state);
}

Item* BrewingStandBlock::getResource(const BlockState*, Random*, int)
{
    return Items::brewingStand;
}

not_null_ptr<ItemInstance> BrewingStandBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this)));
}

bool BrewingStandBlock::hasAnalogOutputSignal(const BlockState*)
{
    return true;
}

int BrewingStandBlock::getAnalogOutputSignal(const BlockState*, Level* level, const BlockPos& pos)
{
    return AbstractContainerMenu::getRedstoneSignalFromBlockEntity(level->getBlockEntity(pos));
}

int BrewingStandBlock::getRenderLayer()
{
    return 0;
}

const BlockState* BrewingStandBlock::getBlockState(int data)
{
    TypedBoxed<bool> bottle0((data & 1) != 0);
    TypedBoxed<bool> bottle1((data & 2) != 0);
    TypedBoxed<bool> bottle2((data & 4) != 0);
    const BlockState* state = defaultBlockState()->setBoxedValue(gBrewingStandBottle0, &bottle0);
    state = state->setBoxedValue(gBrewingStandBottle1, &bottle1);
    return state->setBoxedValue(gBrewingStandBottle2, &bottle2);
}

int BrewingStandBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(gBrewingStandBottle0) ? 1 : 0;
    if (state->getValue(gBrewingStandBottle1))
        data |= 2;
    if (state->getValue(gBrewingStandBottle2))
        data |= 4;
    return data;
}

unsigned int BrewingStandBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bCanPlace || tooltipData.m_bCanDestroy) && tooltipData.m_bIsInside)
        return 0xea4e897e;
    return Block::GetInteractTooltip(tooltipData);
}

BlockStateDefinition* BrewingStandBlock::createBlockStateDefinition()
{
    const Property* properties[3];
    properties[0] = gBrewingStandBottle0;
    properties[1] = gBrewingStandBottle1;
    properties[2] = gBrewingStandBottle2;
    return new BlockStateDefinition(this, properties, 3);
}

EBlockFaceShape BrewingStandBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&,
                                                     const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

void BrewingStandBlock::registerIcons(IconRegister* iconRegister)
{
    Block::registerIcons(iconRegister);
    mBaseTexture = iconRegister->registerIcon(getIconName() + L"_base");
}

TextureAtlasSprite* BrewingStandBlock::getBaseTexture()
{
    return mBaseTexture;
}

EBlockRenderShape BrewingStandBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(25);
}
