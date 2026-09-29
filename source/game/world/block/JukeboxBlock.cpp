#include "game/world/block/JukeboxBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/core/IconRegister.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/core/DataWalker.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/util/Random.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
static AbstractProperty<bool>* gJukeboxHasRecord = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"has_record"));
JukeboxBlock::JukeboxBlock()
    : BaseEntityBlock(Material::WOOD, MaterialColor::DIRT), mTopIcon(NULL)
{
    DerivedInit();
    TypedBoxed<bool> value(false);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(gJukeboxHasRecord, &value));
}

JukeboxBlock::~JukeboxBlock()
{
}

boost::shared_ptr<BlockEntity> JukeboxBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<Entity> result(new Entity());
    result->setLevel(level);
    return result;
}

void JukeboxBlock::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new ItemDataWalker(BlockEntityType::JUKEBOX, L"RecordItem"));
}

bool JukeboxBlock::TestUse(Level*, const BlockPos&, const BlockState* state, boost::shared_ptr<Player>)
{
    return state->getValue(gJukeboxHasRecord);
}
void JukeboxBlock::dropRecording(Level* level, const BlockPos& pos, const BlockState*)
{
    if (level->mIsClientSide) return;
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    boost::shared_ptr<Entity> entity = boost::dynamic_pointer_cast<Entity>(blockEntity);
    if (entity.get() == NULL) return;
    not_null_ptr<ItemInstance> record = entity->getRecord();
    if (record->isEmpty()) return;
    level->levelEvent(LevelEvent::SOUND_PLAY_RECORDING, pos, 0);
    entity->setRecord(ItemInstance::EMPTY);
    Random* random = level->mRandom;
    const float spread = 0.7f;
    double xOffset = (double)(random->nextFloat() * spread) + (double)(1.0f - spread) * 0.5;
    double yOffset = (double)(random->nextFloat() * spread) + 0.06f + 0.6;
    double zOffset = (double)(random->nextFloat() * spread) + (double)(1.0f - spread) * 0.5;
    boost::shared_ptr<ItemEntity> item(
        new ItemEntity(level, pos.x + xOffset, pos.y + yOffset, pos.z + zOffset, record->copy()));
    item->setDefaultPickUpDelay();
    level->addEntity(item);
}
bool JukeboxBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (!clientSide && TestUse(level, pos, state, player)) { dropRecording(level, pos, state); TypedBoxed<bool> value(false); level->setBlock(pos, state->setBoxedValue(gJukeboxHasRecord, &value), Block::UPDATE_CLIENTS, false); return true; } return false;
}
void JukeboxBlock::setRecord(Level* level, const BlockPos& pos, const BlockState* state,
                             not_null_ptr<ItemInstance> record)
{
    boost::shared_ptr<Entity> entity =
        boost::dynamic_pointer_cast<Entity>(level->getBlockEntity(pos));
    if (entity.get() == NULL)
        return;

    not_null_ptr<ItemInstance> stored(
        new ItemInstance(record->getItem(), 1, record->getAuxValue()));
    if (record->hasTag())
        stored->setTag(record->getTag()->copy());
    entity->setRecord(stored);
    TypedBoxed<bool> value(true);
    level->setBlock(pos, state->setBoxedValue(gJukeboxHasRecord, &value), Block::UPDATE_CLIENTS, false);
}

void JukeboxBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    dropRecording(level, pos, state);
    BaseEntityBlock::onRemove(level, pos, state);
}

void JukeboxBlock::spawnResources(Level* level, const BlockPos& pos,
                                  const BlockState* state, float chance, int lootBonus)
{
    if (!level->mIsClientSide)
        Block::spawnResources(level, pos, state, chance, lootBonus);
}
bool JukeboxBlock::hasAnalogOutputSignal(const BlockState*) { return true; }
int JukeboxBlock::getAnalogOutputSignal(const BlockState*, Level* level, const BlockPos& pos)
{
    boost::shared_ptr<Entity> entity =
        boost::dynamic_pointer_cast<Entity>(level->getBlockEntity(pos));
    if (entity.get() == NULL)
        return 0;

    not_null_ptr<ItemInstance> record = entity->getRecord();
    if (record->isEmpty())
        return 0;

    return Item::getId(record->getItem()) + 1 - Item::getId(Items::record13);
}
EBlockRenderShape JukeboxBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_MODEL;
}

const BlockState* JukeboxBlock::getBlockState(int data)
{
    TypedBoxed<bool> value((data & 1) != 0);
    return defaultBlockState()->setBoxedValue(gJukeboxHasRecord, &value);
}

int JukeboxBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gJukeboxHasRecord) ? 1 : 0;
}
unsigned int JukeboxBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success &&
        tooltipData.m_pItemInstance.get() != NULL &&
        !tooltipData.m_pItemInstance->isEmpty())
    {
        int id = Item::VALIDgetId(tooltipData.m_pItemInstance->getItem());
        if (id >= 2256 && id < 2268 && tooltipData.m_bUseBlock && tooltipData.m_bCanPlace)
            return IDS_TOOLTIPS_PLAY;
    }

    if (tooltipData.m_pBlockState != NULL &&
        tooltipData.m_pBlockState->getValue(gJukeboxHasRecord) && tooltipData.m_bCanPlace)
        return IDS_TOOLTIPS_EJECT;

    return Block::GetInteractTooltip(tooltipData);
}
BlockStateDefinition* JukeboxBlock::createBlockStateDefinition() { const Property* properties[] = { gJukeboxHasRecord }; return new BlockStateDefinition(this, properties, 1); }
TextureAtlasSprite* JukeboxBlock::getTexture(const Direction* direction, const BlockState*) { return direction == Direction::UP ? mTopIcon : mIcon; }
void JukeboxBlock::registerIcons(IconRegister* iconRegister) { mIcon = iconRegister->registerIcon(getIconName() + L"_side"); mTopIcon = iconRegister->registerIcon(getIconName() + L"_top"); }
JukeboxBlock::Entity::Entity() : BlockEntity(), mRecord() { mRecord = ItemInstance::EMPTY; }
JukeboxBlock::Entity::~Entity() {}
BlockEntity* JukeboxBlock::Entity::create() { return new Entity(); }
void JukeboxBlock::Entity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    if (tag->contains(L"RecordItem", Tag::TAG_COMPOUND))
    {
        setRecord(not_null_ptr<ItemInstance>(
            new ItemInstance(tag->getCompound(L"RecordItem"))));
    }
    else if (tag->getInt(L"Record") > 0)
    {
        setRecord(not_null_ptr<ItemInstance>(
            new ItemInstance(Item::byId(tag->getInt(L"Record")), 1, 0)));
    }
}
CompoundTag* JukeboxBlock::Entity::save(CompoundTag* tag) { BlockEntity::save(tag); if (!getRecord()->isEmpty()) tag->put(L"RecordItem", getRecord()->save(new CompoundTag())); return tag; }
not_null_ptr<ItemInstance> JukeboxBlock::Entity::getRecord() { return mRecord; }
void JukeboxBlock::Entity::setRecord(not_null_ptr<ItemInstance> record) { mRecord = record; setChanged(); }
boost::shared_ptr<BlockEntity> JukeboxBlock::Entity::clone() { boost::shared_ptr<Entity> result(new Entity()); BlockEntity::clone(result); result->mRecord = mRecord; return result; }
unsigned long long JukeboxBlock::Entity::GetType() { return BlockEntityType::JUKEBOX; }
