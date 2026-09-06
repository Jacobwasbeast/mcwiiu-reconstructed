#include "game/world/block/FlowerPotBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/L10N.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/FlowerBlock.hpp"
#include "game/world/block/FlowerPotBlockEntity.hpp"
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
    case Contents::DEAD_BUSH: item = Item::byBlock(Blocks::deadbush); break;
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
    return NULL;
}

bool FlowerPotBlock::isSolidRender(const BlockState*) const { return false; }
bool FlowerPotBlock::isCubeShaped(const BlockState*) { return false; }

boost::shared_ptr<FlowerPotBlockEntity> FlowerPotBlock::getFlowerPotEntity(Level* level, const BlockPos& pos)
{
    return boost::dynamic_pointer_cast<FlowerPotBlockEntity>(level->getBlockEntity(pos));
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

bool FlowerPotBlock::use(Level* level, const BlockPos& pos, const BlockState*,
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
        if (item->isEmpty())
        {
            if (!player->addItem(plant))
            {
                player->drop(plant, false);
            }
            entity->setItemInstance(ItemInstance::EMPTY);
            return true;
        }
        return false;
    }
    if (!canBePotted(item))
    {
        return false;
    }
    entity->setItemInstance(item);
    if (!player->mAbilities.mInstabuild)
    {
        item->shrink(1);
    }
    return true;
}

not_null_ptr<ItemInstance> FlowerPotBlock::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState*)
{
    boost::shared_ptr<FlowerPotBlockEntity> entity = getFlowerPotEntity(level, pos);
    return entity.get() == NULL ? not_null_ptr<ItemInstance>(new ItemInstance(Blocks::flower_pot))
                                : entity->getPlantItemInstance();
}

void FlowerPotBlock::CanSurvive(const BlockState*)
{
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
        Block::popResource(level, pos, entity->getPlantItemInstance());
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

bool FlowerPotBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                       boost::shared_ptr<Player> player)
{
    return Block::playerWillDestroy(level, pos, state, player);
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

const BlockState* FlowerPotBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state;
}

int FlowerPotBlock::getRenderLayer() { return 0; }
EBlockFaceShape FlowerPotBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*) { return BLOCK_FACE_UNDEFINED; }
EBlockRenderShape FlowerPotBlock::getRenderShape(const BlockState*) { return BLOCK_RENDER_ENTITY; }
unsigned int FlowerPotBlock::GetInteractTooltip(const BlockTooltipDataHolder&) { return 0; }

bool FlowerPotBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return Block::mayPlace(level, pos) && level->getBlockState(pos.below())->isSolid();
}

bool FlowerPotBlock::waterSpreadCausesSpawn() { return true; }
