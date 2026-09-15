#include "game/world/block/FlowerPotBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/L10N.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPlanterItem.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/FlowerBlock.hpp"
#include "game/world/block/FlowerPotBlockEntity.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Predicates.hpp"

EnumProperty<FlowerPotBlock::Contents*>* FlowerPotBlock::CONTENTS = NULL;

FlowerPotBlock::Contents::Contents(EContents contents, const std::wstring& name)
    : mName(name), mContents(contents)
{
}

std::wstring FlowerPotBlock::Contents::toString() const
{
    return mName;
}

std::wstring FlowerPotBlock::Contents::getSerializedName() const
{
    return mName;
}

FlowerPotBlock::FlowerPotBlock()
    : BaseEntityBlock(Material::DECORATION)
{
    blockStaticCtor();
    mBlockEntity = true;
    DerivedInit();
    TypedBoxed<Contents*> contents(CONTENTS == NULL ? NULL : CONTENTS->getUnboxedValue(L"empty"));
    registerDefaultState(mStateDefinition->any()->setBoxedValue(CONTENTS, &contents));
    sendBlockData(15);
}

FlowerPotBlock::~FlowerPotBlock()
{
}

void FlowerPotBlock::blockStaticCtor()
{
    if (CONTENTS != NULL)
    {
        return;
    }
    boost::unordered_set<Contents*> values;
    values.insert(new Contents(Contents::EMPTY, L"empty"));
    values.insert(new Contents(Contents::POPPY, L"poppy"));
    values.insert(new Contents(Contents::DANDELION, L"dandelion"));
    values.insert(new Contents(Contents::OAK_SAPLING, L"oak_sapling"));
    values.insert(new Contents(Contents::SPRUCE_SAPLING, L"spruce_sapling"));
    values.insert(new Contents(Contents::BIRCH_SAPLING, L"birch_sapling"));
    values.insert(new Contents(Contents::JUNGLE_SAPLING, L"jungle_sapling"));
    values.insert(new Contents(Contents::ACACIA_SAPLING, L"acacia_sapling"));
    values.insert(new Contents(Contents::DARK_OAK_SAPLING, L"dark_oak_sapling"));
    values.insert(new Contents(Contents::MUSHROOM_RED, L"red_mushroom"));
    values.insert(new Contents(Contents::MUSHROOM_BROWN, L"brown_mushroom"));
    values.insert(new Contents(Contents::DEAD_BUSH, L"dead_bush"));
    values.insert(new Contents(Contents::FERN, L"fern"));
    values.insert(new Contents(Contents::GRASS, L"grass"));
    values.insert(new Contents(Contents::BLUE_ORCHID, L"blue_orchid"));
    values.insert(new Contents(Contents::ALLIUM, L"allium"));
    values.insert(new Contents(Contents::HOUSTONIA, L"houstonia"));
    values.insert(new Contents(Contents::RED_TULIP, L"red_tulip"));
    values.insert(new Contents(Contents::ORANGE_TULIP, L"orange_tulip"));
    values.insert(new Contents(Contents::WHITE_TULIP, L"white_tulip"));
    values.insert(new Contents(Contents::PINK_TULIP, L"pink_tulip"));
    values.insert(new Contents(Contents::OXEYE_DAISY, L"oxeye_daisy"));
    values.insert(new Contents(Contents::CACTUS, L"cactus"));
    CONTENTS = EnumProperty<Contents*>::create(L"contents", typeid(Contents), values,
                                                Predicates<Contents*>::alwaysTrue);
}

boost::shared_ptr<BlockEntity> FlowerPotBlock::newBlockEntity(Level*, int data)
{
    Item* item = NULL;
    int itemData = 0;
    switch (data)
    {
    case Contents::POPPY: item = Item::byBlock(Blocks::red_flower); break;
    case Contents::DANDELION: item = Item::byBlock(Blocks::yellow_flower); break;
    case Contents::OAK_SAPLING: item = Item::byBlock(Blocks::sapling); break;
    case Contents::SPRUCE_SAPLING: item = Item::byBlock(Blocks::sapling); itemData = PlanksBlock::Variant::SPRUCE->getData(); break;
    case Contents::BIRCH_SAPLING: item = Item::byBlock(Blocks::sapling); itemData = PlanksBlock::Variant::BIRCH->getData(); break;
    case Contents::JUNGLE_SAPLING: item = Item::byBlock(Blocks::sapling); itemData = PlanksBlock::Variant::JUNGLE->getData(); break;
    case Contents::ACACIA_SAPLING: item = Item::byBlock(Blocks::sapling); itemData = PlanksBlock::Variant::ACACIA->getData(); break;
    case Contents::DARK_OAK_SAPLING: item = Item::byBlock(Blocks::sapling); itemData = PlanksBlock::Variant::DARK_OAK->getData(); break;
    case Contents::MUSHROOM_RED: item = Item::byBlock(Blocks::red_mushroom); break;
    case Contents::MUSHROOM_BROWN: item = Item::byBlock(Blocks::brown_mushroom); break;
    case Contents::DEAD_BUSH: item = Item::byBlock(Blocks::tallgrass); itemData = TallGrassBlock::Variant::DEAD_BUSH->getData(); break;
    case Contents::FERN: item = Item::byBlock(Blocks::tallgrass); itemData = gFern->getData(); break;
    case Contents::GRASS: item = Item::byBlock(Blocks::tallgrass); itemData = gTallGrass->getData(); break;
    default: break;
    }
    return boost::shared_ptr<BlockEntity>(new FlowerPotBlockEntity(item, itemData));
}

std::wstring FlowerPotBlock::getName()
{
    return std::wstring(L10N::GetString(0xd6cef3c9));
}

const AABB* FlowerPotBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    static AABB* shape = NULL;
    if (shape == NULL)
        shape = AABB::newPermanent(0.3, 0.0, 0.3, 0.7, 0.6, 0.7);
    return shape;
}

bool FlowerPotBlock::isSolidRender(const BlockState*) const { return false; }
bool FlowerPotBlock::isCubeShaped(const BlockState*) { return false; }

boost::shared_ptr<FlowerPotBlockEntity> FlowerPotBlock::getFlowerPotEntity(Level* level, const BlockPos& pos)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity.get() == NULL || blockEntity->GetType() != 0x100020dULL)
    {
        return boost::shared_ptr<FlowerPotBlockEntity>();
    }
    return boost::dynamic_pointer_cast<FlowerPotBlockEntity>(blockEntity);
}

int FlowerPotBlock::getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex)
{
    boost::shared_ptr<FlowerPotBlockEntity> entity =
        boost::dynamic_pointer_cast<FlowerPotBlockEntity>(levelSource->getBlockEntity(pos));
    if (entity.get() == NULL || entity->getPlantItem() == NULL)
    {
        return 0xFFFFFF;
    }
    Block* block = Block::byItem(entity->getPlantItem());
    return block == NULL ? 0xFFFFFF : block->getColor(levelSource, pos, tintIndex);
}

bool FlowerPotBlock::canBePotted(const not_null_ptr<ItemInstance>& item)
{
    Block* block = Block::byItem(item->getItem());
    if (block == Blocks::yellow_flower || block == Blocks::red_flower
        || block == Blocks::brown_mushroom || block == Blocks::red_mushroom
        || block == Blocks::sapling || block == Blocks::deadbush)
    {
        return true;
    }
    return block == Blocks::tallgrass && item->getAuxValue() == gTallGrass->getData();
}

bool FlowerPotBlock::use(Level* level, const BlockPos& pos, const BlockState* state,
                         boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand,
                         const Direction*, float, float, float, bool)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    boost::shared_ptr<FlowerPotBlockEntity> entity = getFlowerPotEntity(level, pos);
    if (entity.get() == NULL || item.get() == NULL)
    {
        return false;
    }
    not_null_ptr<ItemInstance> plant = entity->getPlantItemInstance();
    if (plant.get() != NULL && !plant->isEmpty())
    {
        if (!item->isEmpty())
        {
            if (!player->addItem(plant))
            {
                player->drop(plant, false);
            }
        }
        else
        {
            player->setItemInHand(hand, plant);
        }
        entity->setItemInstance(ItemInstance::EMPTY);
    }
    else
    {
        if (!canBePotted(item))
        {
            return false;
        }
        entity->setItemInstance(item);
        if (!player->mAbilities.mInstabuild)
        {
            item->shrink(1);
        }
    }
    level->sendBlockUpdated(pos, state, state, 3, true);
    level->updateObservingBlocksAt(pos, this);
    return true;
}

not_null_ptr<ItemInstance> FlowerPotBlock::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState*)
{
    boost::shared_ptr<FlowerPotBlockEntity> entity = getFlowerPotEntity(level, pos);
    if (entity.get() != NULL)
    {
        not_null_ptr<ItemInstance> plant = entity->getPlantItemInstance();
        if (!plant->isEmpty())
        {
            return plant;
        }
    }
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::flowerPot));
}

void FlowerPotBlock::CanSurvive(const BlockState* state)
{
    if (state != NULL)
        (void)state->isTopSolidBlocking();
}

void FlowerPotBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos,
                                     Block*, const BlockPos&)
{
    if (!level->getBlockState(pos.below())->isSolid())
    {
        level->removeBlock(pos);
    }
}

void FlowerPotBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<FlowerPotBlockEntity> entity = getFlowerPotEntity(level, pos);
    if (entity.get() != NULL && entity->getPlantItem() != NULL)
    {
        Block::popResource(level, pos,
            not_null_ptr<ItemInstance>(new ItemInstance(entity->getPlantItem(), 1, entity->getItemData())));
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

bool FlowerPotBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                       boost::shared_ptr<Player> player)
{
    bool result = Block::playerWillDestroy(level, pos, state, player);
    if (player->mAbilities.mInstabuild)
    {
        boost::shared_ptr<FlowerPotBlockEntity> entity = getFlowerPotEntity(level, pos);
        if (entity.get() != NULL)
        {
            entity->setItemInstance(ItemInstance::EMPTY);
        }
    }
    return result;
}

Item* FlowerPotBlock::getResource(const BlockState*, Random*, int) { return Item::byBlock(Blocks::flower_pot); }
bool FlowerPotBlock::canContainLiquid(const BlockState*) const { return true; }

BlockStateDefinition* FlowerPotBlock::createBlockStateDefinition()
{
    const Property* properties[] = { CONTENTS };
    return new BlockStateDefinition(this, properties, 1);
}

int FlowerPotBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    Contents* contents = state->getValue(CONTENTS);
    return contents == NULL ? 0 : contents->mContents;
}

const BlockState* FlowerPotBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    Contents* contents = CONTENTS->getUnboxedValue(L"empty");
    boost::shared_ptr<BlockEntity> blockEntity = levelSource->getBlockEntity(pos);
    if (blockEntity.get() != NULL && blockEntity->GetType() == 0x100020dULL)
    {
        boost::shared_ptr<FlowerPotBlockEntity> pot = boost::static_pointer_cast<FlowerPotBlockEntity>(blockEntity);
        Item* item = pot->getPlantItem();
        if (item != NULL && dynamic_cast<BlockPlanterItem*>(item) != NULL)
        {
            int itemData = pot->getItemData();
            Block* block = Block::byItem(item);
            if (block == Blocks::sapling)
            {
                PlanksBlock::Variant* variant = PlanksBlock::Variant::byData(itemData);
                switch (variant->getData())
                {
                case 0: contents = CONTENTS->getUnboxedValue(L"oak_sapling"); break;
                case 1: contents = CONTENTS->getUnboxedValue(L"spruce_sapling"); break;
                case 2: contents = CONTENTS->getUnboxedValue(L"birch_sapling"); break;
                case 3: contents = CONTENTS->getUnboxedValue(L"jungle_sapling"); break;
                case 4: contents = CONTENTS->getUnboxedValue(L"acacia_sapling"); break;
                case 5: contents = CONTENTS->getUnboxedValue(L"dark_oak_sapling"); break;
                default: break;
                }
            }
            else if (block == Blocks::tallgrass)
            {
                if (itemData == 0)
                {
                    contents = CONTENTS->getUnboxedValue(L"dead_bush");
                }
                else if (itemData == 2)
                {
                    contents = CONTENTS->getUnboxedValue(L"fern");
                }
            }
            else if (block == Blocks::yellow_flower)
            {
                contents = CONTENTS->getUnboxedValue(L"dandelion");
            }
            else if (block == Blocks::red_flower)
            {
                FlowerBlock::Flower* flower = FlowerBlock::Flower::byData(FlowerBlock::FlowerCategory::RED, itemData);
                switch (flower->getId())
                {
                case FlowerBlock::Flower::FLOWER_POPPY: contents = CONTENTS->getUnboxedValue(L"poppy"); break;
                case FlowerBlock::Flower::FLOWER_BLUE_ORCHID: contents = CONTENTS->getUnboxedValue(L"blue_orchid"); break;
                case FlowerBlock::Flower::FLOWER_ALLIUM: contents = CONTENTS->getUnboxedValue(L"allium"); break;
                case FlowerBlock::Flower::FLOWER_HOUSTONIA: contents = CONTENTS->getUnboxedValue(L"houstonia"); break;
                case FlowerBlock::Flower::FLOWER_RED_TULIP: contents = CONTENTS->getUnboxedValue(L"red_tulip"); break;
                case FlowerBlock::Flower::FLOWER_ORANGE_TULIP: contents = CONTENTS->getUnboxedValue(L"orange_tulip"); break;
                case FlowerBlock::Flower::FLOWER_WHITE_TULIP: contents = CONTENTS->getUnboxedValue(L"white_tulip"); break;
                case FlowerBlock::Flower::FLOWER_PINK_TULIP: contents = CONTENTS->getUnboxedValue(L"pink_tulip"); break;
                case FlowerBlock::Flower::FLOWER_OXEYE_DAISY: contents = CONTENTS->getUnboxedValue(L"oxeye_daisy"); break;
                default: break;
                }
            }
            else if (block == Blocks::red_mushroom)
            {
                contents = CONTENTS->getUnboxedValue(L"red_mushroom");
            }
            else if (block == Blocks::brown_mushroom)
            {
                contents = CONTENTS->getUnboxedValue(L"brown_mushroom");
            }
            else if (block == Blocks::deadbush)
            {
                contents = CONTENTS->getUnboxedValue(L"dead_bush");
            }
            else if (block == Blocks::cactus)
            {
                contents = CONTENTS->getUnboxedValue(L"cactus");
            }
        }
    }
    TypedBoxed<Contents*> boxed(contents);
    return state->setBoxedValue(CONTENTS, &boxed);
}

int FlowerPotBlock::getRenderLayer() { return 0; }
EBlockFaceShape FlowerPotBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*) { return BLOCK_FACE_UNDEFINED; }
EBlockRenderShape FlowerPotBlock::getRenderShape(const BlockState*) { return static_cast<EBlockRenderShape>(0x21); }
unsigned int FlowerPotBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    boost::shared_ptr<BlockEntity> blockEntity;
    if (tooltipData.m_pLevel != NULL)
        blockEntity = tooltipData.m_pLevel->getBlockEntity(tooltipData.m_blockPos);

    boost::shared_ptr<FlowerPotBlockEntity> pot =
        boost::dynamic_pointer_cast<FlowerPotBlockEntity>(blockEntity);
    bool occupied = false;
    if (pot)
    {
        not_null_ptr<ItemInstance> plant = pot->getPlantItemInstance();
        occupied = !plant->isEmpty();
    }

    if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail)
    {
        not_null_ptr<ItemInstance> offered = tooltipData.m_pItemInstance;
        if (!offered->isEmpty())
        {
            const int id = Item::getId(offered->getItem());
            bool accepted = id == 0x06 || id == 0x20 || (id >= 0x25 && id <= 0x28) || id == 0x51;
            if (id == 0x1f)
                accepted = gTallGrass != NULL && offered->getAuxValue() == gTallGrass->getData();
            if (accepted)
                return occupied ? 0x512092d2u : 0x3d8502afu;
        }
    }

    if (tooltipData.m_pItemInstance->isEmpty() && !occupied)
        return 0x3d8502afu;
    return Block::GetInteractTooltip(tooltipData);
}

bool FlowerPotBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return Block::mayPlace(level, pos) && level->getBlockState(pos.below())->isSolid();
}

bool FlowerPotBlock::waterSpreadCausesSpawn() { return true; }
