#include "game/world/block/ShulkerBoxBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/StringUtils.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/Container.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/NonNullList.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/item/Containers.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

ShulkerBoxBlock::ShulkerBoxBlock(const DyeColor* color)
    : BaseEntityBlock(Material::STONE, MaterialColor::NONE), mColor(color)
{
    mBlockEntity = true;
    DerivedInit();
    TypedBoxed<const Direction*> facing(Direction::UP);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &facing));
}

ShulkerBoxBlock::~ShulkerBoxBlock()
{
}

boost::shared_ptr<BlockEntity> ShulkerBoxBlock::newBlockEntity(Level*, int)
{
    return boost::shared_ptr<BlockEntity>(new ShulkerBoxBlockEntity(mColor));
}

bool ShulkerBoxBlock::isSolidRender(const BlockState*) const { return false; }
bool ShulkerBoxBlock::isViewBlocking(const BlockState*) { return true; }
bool ShulkerBoxBlock::isCubeShaped(const BlockState*) { return false; }
bool ShulkerBoxBlock::hasCustomBreakingProgress(const BlockState*) { return true; }
EBlockRenderShape ShulkerBoxBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(0x16);
}

bool ShulkerBoxBlock::isEntityOnChest(Level* level, const AABB* box)
{
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(Entity), box);
    bool result = entities != NULL && !entities->empty();
    delete entities;
    return result;
}

bool ShulkerBoxBlock::use(Level* level, const BlockPos& pos, const BlockState* state,
                          boost::shared_ptr<Player> player, InteractionHand::EInteractionHand,
                          const Direction* direction, float, float, float, bool clientSide)
{
    if (clientSide)
        return true;
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(entity);
    if (box.get() == NULL)
        return false;
    const Direction* facing = state->getValue(DirectionalBlock::FACING_PROPERTY);
    if (box->getAnimationStatus() == 0)
    {
        AABB* bounds = box->getBoundingBox(state);
        BlockPos adjacent = pos.relative(direction);
        AABB* expanded = bounds->expand(0.5, 0.5, 0.5)->contract(0.5, 0.5, 0.5)->move(adjacent);
        if (level->hasBlockCubes(expanded) || isEntityOnChest(level, expanded))
            return true;
    }
    player->openContainer(boost::static_pointer_cast<Container>(box));
    return true;
}

const BlockState* ShulkerBoxBlock::getStateForPlacement(Level*, const BlockPos&, const Direction* direction,
                                                         float, float, float, int,
                                                         boost::shared_ptr<LivingEntity>)
{
    TypedBoxed<const Direction*> facing(direction);
    return defaultBlockState()->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &facing);
}

BlockStateDefinition* ShulkerBoxBlock::createBlockStateDefinition()
{
    const Property* properties[] = { DirectionalBlock::FACING_PROPERTY };
    return new BlockStateDefinition(this, properties, 1);
}

int ShulkerBoxBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(DirectionalBlock::FACING_PROPERTY)->get3DDataValue();
}

const BlockState* ShulkerBoxBlock::getBlockState(int data)
{
    const Direction* facing = Direction::from3DDataValue(data);
    TypedBoxed<const Direction*> boxed(facing);
    return defaultBlockState()->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &boxed);
}

bool ShulkerBoxBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                        boost::shared_ptr<Player> player)
{
    boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(level->getBlockEntity(pos));
    if (box.get() != NULL && player->isCreative())
        box->setDestroyedByCreativePlayer(true);
    return Block::playerWillDestroy(level, pos, state, player);
}

void ShulkerBoxBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state,
                                     float chance, int lootBonus)
{
    Block::spawnResources(level, pos, state, chance, lootBonus);
}

void ShulkerBoxBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                  boost::shared_ptr<LivingEntity> placer,
                                  not_null_ptr<ItemInstance> itemInstance)
{
    Block::setPlacedBy(level, pos, state, placer, itemInstance);
    if (itemInstance->hasCustomHoverName())
    {
        boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(level->getBlockEntity(pos));
        if (box.get() != NULL)
            box->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                               itemInstance->wasCreatedByRestrictedPlayer(), itemInstance->wasCreatedOnHost());
    }
}

void ShulkerBoxBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(level->getBlockEntity(pos));
    if (box.get() != NULL && !box->isCleared() && box->shouldDrop())
    {
        not_null_ptr<ItemInstance> item(new ItemInstance(this));
        CompoundTag tag;
        box->saveToTag(&tag);
        item->addTagElement(L"BlockEntityTag", &tag);
        Block::popResource(level, pos, item);
    }
    level->removeBlockEntity(pos);
    BaseEntityBlock::onRemove(level, pos, state);
}

void ShulkerBoxBlock::appendHoverText(not_null_ptr<ItemInstance> itemInstance,
                                      boost::shared_ptr<Player> player,
                                      std::vector<HtmlString>* lines, bool advanced)
{
    Block::appendHoverText(itemInstance, player, lines, advanced);

    CompoundTag* tag = itemInstance->getTag();
    if (tag == NULL || !tag->contains(L"BlockEntityTag", 10))
        return;

    CompoundTag* blockEntityTag = tag->getCompound(L"BlockEntityTag");
    if (blockEntityTag->contains(L"LootTable", 8))
    {
        lines->push_back(HtmlString(L"???????",
                                    eMinecraftColour_NOTSET, false, false, 0));
    }

    if (!blockEntityTag->contains(L"Items", 9))
        return;

    NonNullList<not_null_ptr<ItemInstance> > items =
        NonNullList<not_null_ptr<ItemInstance> >::withSize(27, ItemInstance::EMPTY);
    ContainerHelper::loadAllItems(blockEntityTag, items);

    int displayedItems = 0;
    int totalItems = 0;
    for (unsigned int index = 0; index < items.size(); ++index)
    {
        if (!items[index]->isEmpty())
        {
            ++totalItems;
            if (displayedItems < 5)
            {
                ++displayedItems;
                std::wstring itemText =
                    formatwstr(L"%ls x%d", items[index]->getHoverName().c_str(),
                               items[index]->getCount());
                lines->push_back(HtmlString(itemText, eMinecraftColour_NOTSET,
                                            false, false, 0));
            }
        }
    }

    if (totalItems - displayedItems > 0)
    {
        std::wstring moreText = L10N::GetString(0xfcb7d502);
        replaceAll(moreText, moreText, L"%s", L"%d");
        moreText = formatwstr(moreText.c_str(), totalItems - displayedItems);
        lines->push_back(HtmlString(moreText, eMinecraftColour_NOTSET, true, false, 0));
    }
}

EPistonPushReaction ShulkerBoxBlock::getPistonPushReaction(const BlockState*) { return PISTON_PUSH_DESTROY; }

const AABB* ShulkerBoxBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    boost::shared_ptr<BlockEntity> entity = levelSource->getBlockEntity(pos);
    boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(entity);
    if (box.get() != NULL)
        return box->getBoundingBox(state);
    return AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
}

bool ShulkerBoxBlock::hasAnalogOutputSignal(const BlockState*) { return true; }

int ShulkerBoxBlock::getAnalogOutputSignal(const BlockState*, Level* level, const BlockPos& pos)
{
    boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(level->getBlockEntity(pos));
    if (box.get() == NULL)
        return 0;
    return AbstractContainerMenu::getRedstoneSignalFromContainer(boost::static_pointer_cast<Container>(box));
}

not_null_ptr<ItemInstance> ShulkerBoxBlock::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state)
{
    not_null_ptr<ItemInstance> item = Block::getCloneItemInstance(level, pos, state);
    boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(level->getBlockEntity(pos));
    if (box.get() != NULL)
    {
        CompoundTag tag;
        box->saveToTag(&tag);
        item->addTagElement(L"BlockEntityTag", &tag);
    }
    return item;
}

const DyeColor* ShulkerBoxBlock::getDyeColor() { return mColor; }

const DyeColor* ShulkerBoxBlock::getColorFromBlock(Block* block)
{
    ShulkerBoxBlock* shulker = dynamic_cast<ShulkerBoxBlock*>(block);
    return shulker == NULL ? NULL : shulker->getDyeColor();
}

Block* ShulkerBoxBlock::getBlockByColor(const DyeColor* color)
{
    static Block* blocks[] = { Blocks::white_shulker_box, Blocks::orange_shulker_box,
        Blocks::magenta_shulker_box, Blocks::light_blue_shulker_box, Blocks::yellow_shulker_box,
        Blocks::lime_shulker_box, Blocks::pink_shulker_box, Blocks::gray_shulker_box,
        Blocks::silver_shulker_box, Blocks::cyan_shulker_box, Blocks::purple_shulker_box,
        Blocks::blue_shulker_box, Blocks::brown_shulker_box, Blocks::green_shulker_box,
        Blocks::red_shulker_box, Blocks::black_shulker_box };
    int data = color == NULL ? 10 : color->getBlockData();
    return data >= 0 && data < 16 ? blocks[data] : blocks[10];
}

not_null_ptr<ItemInstance> ShulkerBoxBlock::getColoredItemInstance(const DyeColor* color)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(getBlockByColor(color)));
}

const BlockState* ShulkerBoxBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(DirectionalBlock::FACING_PROPERTY)));
    return state->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &facing);
}

const BlockState* ShulkerBoxBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(DirectionalBlock::FACING_PROPERTY)));
}

EBlockFaceShape ShulkerBoxBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state,
                                                   const BlockPos& pos, const Direction* direction)
{
    const BlockState* virtualState = fillVirtualBlockStateProperties(state, levelSource, pos);
    const Direction* facing = virtualState->getValue(DirectionalBlock::FACING_PROPERTY);
    boost::shared_ptr<ShulkerBoxBlockEntity> box = boost::dynamic_pointer_cast<ShulkerBoxBlockEntity>(levelSource->getBlockEntity(pos));
    if (box.get() != NULL)
    {
        int animationStatus = box->getAnimationStatus();
        if (animationStatus == 0 ||
            (animationStatus == 2 && (facing == direction->getOpposite() || facing == direction)))
        {
            return BLOCK_FACE_SOLID;
        }
    }
    return BLOCK_FACE_UNDEFINED;
}

unsigned int ShulkerBoxBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseItem || tooltipData.m_bCanDestroy) && tooltipData.m_bIsInside)
        return 0x41590ad1;
    return Block::GetInteractTooltip(tooltipData);
}
