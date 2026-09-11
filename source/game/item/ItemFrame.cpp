#include "game/item/ItemFrame.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/core/DataWalker.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/core/GenericStats.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Minecraft.hpp"

const EntityDataAccessor<not_null_ptr<ItemInstance> >* ItemFrame::DATA_ITEM = NULL;
const EntityDataAccessor<int>* ItemFrame::DATA_ROTATION = NULL;

ItemFrame::ItemFrame(Level* level) : HangingEntity(level)
{
    Entity::fjDerivedCtorCalls();
    _init();
}

ItemFrame::ItemFrame(Level* level, const BlockPos& pos, const Direction* direction)
    : HangingEntity(level, pos)
{
    Entity::fjDerivedCtorCalls();
    _init();
    setDirection(direction);
}

ItemFrame::~ItemFrame()
{
}

void ItemFrame::create(Level* level)
{
    new ItemFrame(level);
}

void ItemFrame::addDataWalkers(DataFixerUpper* fixer)
{
    ItemDataWalker* walker = new ItemDataWalker(0x0a080002ULL, L"Item");
    fixer->addWalker(DataFixTypes::eDataFixTypes_ItemInstance, (DataWalker*)walker);
}

unsigned long long ItemFrame::staticCtor()
{
    DATA_ITEM = SynchedEntityData::defineId(0x0a080002ULL, 0x0000000008000000ULL, EntityDataSerializers::ITEM_INSTANCE);
    DATA_ROTATION = SynchedEntityData::defineId(0x0a080002ULL, 0x0000000008000000ULL, EntityDataSerializers::INT);
    return 0;
}

void ItemFrame::_init()
{
    mDropChance = 1.0f;
}

void ItemFrame::defineSynchedData()
{
    if (mEntityData != NULL && DATA_ITEM != NULL)
    {
        mEntityData->createDateItem(DATA_ITEM, ItemInstance::EMPTY);
    }
    if (mEntityData != NULL && DATA_ROTATION != NULL)
    {
        mEntityData->createDateItem(DATA_ROTATION, 0);
    }
}

double ItemFrame::getPickRadius()
{
    return 0.0;
}

not_null_ptr<ItemInstance> ItemFrame::getItem()
{
    if (mEntityData != NULL && DATA_ITEM != NULL)
    {
        return mEntityData->get(DATA_ITEM);
    }
    return ItemInstance::EMPTY;
}

void ItemFrame::setItem(not_null_ptr<ItemInstance> item, bool showParticles)
{
    if (item->isEmpty())
    {
        item = ItemInstance::EMPTY;
    }
    if (mEntityData != NULL && DATA_ITEM != NULL)
    {
        mEntityData->set(DATA_ITEM, item, true);
    }
    if (showParticles)
    {
        playPlacementSound();
    }
}

void ItemFrame::setItem(not_null_ptr<ItemInstance> item)
{
    setItem(item, true);
}

bool ItemFrame::hurt(const DamageSource* source, float damage)
{
    if (mRemoved || isInvulnerableTo(source))
    {
        return false;
    }
    dropItem(boost::shared_ptr<Entity>(), true);
    return true;
}

int ItemFrame::getWidth()
{
    return 12;
}

int ItemFrame::getHeight()
{
    return 12;
}

bool ItemFrame::shouldRenderAtSqrDistance(double distanceSqr)
{
    return HangingEntity::shouldRenderAtSqrDistance(distanceSqr);
}

void ItemFrame::dropItem(boost::shared_ptr<Entity> breaker)
{
    dropItem(breaker, true);
}

void ItemFrame::removeFramedMap(not_null_ptr<ItemInstance> item)
{
    if (!item->isEmpty())
    {
        item->setFramed(boost::shared_ptr<ItemFrame>());
    }
}

void ItemFrame::dropItem(boost::shared_ptr<Entity> breaker, bool dropSelf)
{
    not_null_ptr<ItemInstance> item = getItem();
    if (!item->isEmpty())
    {
        removeFramedMap(item);
        if (dropSelf)
        {
            setItem(ItemInstance::EMPTY, false);
        }
    }
    if (dropSelf)
    {
        remove();
    }
}

void ItemFrame::playPlacementSound()
{
    playSound(SoundEvents::ITEMFRAME_PLACE, 1.0f, 1.0f);
}

void ItemFrame::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (accessor == DATA_ITEM)
    {
        not_null_ptr<ItemInstance> item = getItem();
        if (!item->isEmpty())
        {
            item->setFramed(boost::shared_ptr<ItemFrame>());
        }
    }
}

int ItemFrame::getRotation()
{
    if (mEntityData != NULL && DATA_ROTATION != NULL)
    {
        return mEntityData->get(DATA_ROTATION);
    }
    return 0;
}

void ItemFrame::setRotation(int rotation, bool update)
{
    rotation &= 7;
    if (mEntityData != NULL && DATA_ROTATION != NULL)
    {
        mEntityData->set(DATA_ROTATION, rotation, update);
    }
}

void ItemFrame::setRotation(int rotation)
{
    setRotation(rotation, true);
}

void ItemFrame::addAdditonalSaveData(CompoundTag* tag)
{
    HangingEntity::addAdditonalSaveData(tag);
    not_null_ptr<ItemInstance> item = getItem();
    if (!item->isEmpty())
    {
        CompoundTag* itemTag = new CompoundTag();
        tag->put(L"Item", item->save(itemTag));
    }
    tag->putByte(L"ItemRotation", (unsigned char)getRotation());
    tag->putFloat(L"ItemDropChance", mDropChance);
}

void ItemFrame::readAdditionalSaveData(CompoundTag* tag)
{
    HangingEntity::readAdditionalSaveData(tag);
    if (tag->contains(L"Item"))
    {
        setItem(not_null_ptr<ItemInstance>(new ItemInstance(tag->fjSafeGetCompoundTag(L"Item"))), false);
    }
    if (tag->contains(L"ItemRotation"))
    {
        setRotation(tag->getByte(L"ItemRotation"), false);
    }
    if (tag->contains(L"ItemDropChance"))
    {
        mDropChance = tag->getFloat(L"ItemDropChance");
    }
}

boost::shared_ptr<ItemFrame> ItemFrame::fjGetItemFrameForPos(Level* level, const BlockPos& pos)
{
    if (level == NULL)
        return boost::shared_ptr<ItemFrame>();
    AABB area(pos.x - 1.0, pos.y - 1.0, pos.z - 1.0,
              pos.x + 2.0, pos.y + 2.0, pos.z + 2.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        level->getEntitiesOfClass(typeid(ItemFrame), &area);
    if (entities == NULL)
        return boost::shared_ptr<ItemFrame>();
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
         it != entities->end(); ++it)
    {
        boost::shared_ptr<ItemFrame> frame = boost::dynamic_pointer_cast<ItemFrame>(*it);
        if (frame && frame->getPos().equals(pos))
            return frame;
    }
    return boost::shared_ptr<ItemFrame>();
}

bool ItemFrame::fjTryAwardMapRoom(const boost::shared_ptr<Player>& player)
{
    if (!player || mLevel == NULL || mLevel->mDimension == NULL ||
        !mLevel->mDimension->isNaturalDimension() ||
        getItem()->isEmpty() || getItem()->getItem() != Items::filledMap)
        return false;

    const Direction* direction = getDirection();
    if (direction == NULL)
        return false;
    BlockPos adjacentPos = getPos().relative(direction->getCounterClockWise(), 0);
    adjacentPos = adjacentPos.relative(Direction::UP, 0);
    boost::shared_ptr<ItemFrame> adjacent = fjGetItemFrameForPos(mLevel, adjacentPos);
    if (!adjacent || adjacent->getDirection() != direction ||
        adjacent->getItem()->isEmpty() || adjacent->getItem()->getItem() != Items::filledMap)
        return false;

    MapItem* map = static_cast<MapItem*>(adjacent->getItem()->getItem());
    boost::shared_ptr<MapItemSavedData> data = map->getSavedData(adjacent->getItem(), mLevel);
    if (!data || data->mDimension != 0 || data->mScale != 3 ||
        !data->fjIsCompletelyExplored())
        return false;

    const int mapCenterX = data->mXCenter;
    const int mapCenterZ = data->mZCenter;
    (void)mapCenterX;
    (void)mapCenterZ;

    player->awardStat(GenericStats::mapRoom(), GenericStats::param_noArgs());
    return true;
}

bool ItemFrame::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    if (!player)
    {
        return false;
    }
    not_null_ptr<ItemInstance> held = ItemInstance::EMPTY;
    if (getItem()->isEmpty() && !held->isEmpty())
    {
        setItem(held);
        return true;
    }
    if (!getItem()->isEmpty())
    {
        setRotation(getRotation() + 1);
        return true;
    }
    return false;
}

unsigned int ItemFrame::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (!getItem()->isEmpty())
    {
        return 0x4cbef584U;
    }
    return 0xffffffffU;
}

unsigned int ItemFrame::GetHitTooltip(const EntityTooltipDataHolder& holder)
{
    return holder.m_bAllowedToHurt ? 0x5fb7a25aU : 0xffffffffU;
}

int ItemFrame::getAnalogOutput()
{
    if (getItem()->isEmpty())
    {
        return 0;
    }
    return getRotation() + 1;
}

unsigned long long ItemFrame::GetType()
{
    return 0x000000000A080002ULL;
}
