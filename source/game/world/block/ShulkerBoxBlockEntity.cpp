#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/core/LockCode.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/EntityType.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/Level.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/Player.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/MoverType.hpp"

#include <algorithm>

#include "game/core/CensoredString.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/core/DataWalker.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/ShulkerBoxMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"

ShulkerBoxBlockEntity::ShulkerBoxBlockEntity(const DyeColor* color)
    : RandomizableLockableContainerBlockEntity()
    , WorldlyContainer()
    , mCleared(false)
    , mOpenCount(0)
    , mAnimationStatus(0)
    , mProgress(0.0f)
    , mProgressOld(0.0f)
    , mColor(color)
    , mDestroyedByCreativePlayer(false)
{
    _init(color);
}

ShulkerBoxBlockEntity::ShulkerBoxBlockEntity()
    : RandomizableLockableContainerBlockEntity()
    , WorldlyContainer()
    , mCleared(false)
    , mOpenCount(0)
    , mAnimationStatus(0)
    , mProgress(0.0f)
    , mProgressOld(0.0f)
    , mColor(NULL)
    , mDestroyedByCreativePlayer(false)
{
    _init(NULL);
}

ShulkerBoxBlockEntity::~ShulkerBoxBlockEntity()
{
    delete[] mItems.items;
}

BlockEntity* ShulkerBoxBlockEntity::create()
{
    return new ShulkerBoxBlockEntity();
}

void ShulkerBoxBlockEntity::_init(const DyeColor* color)
{
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(27, ItemInstance::EMPTY);
    mColor = color;
    mCleared = false;
    mOpenCount = 0;
    mAnimationStatus = 0;
    mProgress = 0.0f;
    mProgressOld = 0.0f;
    mDestroyedByCreativePlayer = false;
}

std::wstring ShulkerBoxBlockEntity::getName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring(L"container.shulkerBox");
}

std::wstring ShulkerBoxBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring(L"container.shulkerBox");
}

unsigned int ShulkerBoxBlockEntity::getContainerSize()
{
    return mItems.size();
}

bool ShulkerBoxBlockEntity::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        if (!mItems[i]->isEmpty())
        {
            return false;
        }
    }
    return true;
}

int ShulkerBoxBlockEntity::getMaxStackSize()
{
    return 64;
}

void ShulkerBoxBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

void ShulkerBoxBlockEntity::startOpen(boost::shared_ptr<Player> player)
{
    if (player->isSpectator())
    {
        return;
    }

    if (mOpenCount < 0)
    {
        mOpenCount = 0;
    }

    ++mOpenCount;
    mLevel->blockEvent(mPos, getBlock(), 1, mOpenCount);

    if (mOpenCount == 1)
    {
        mLevel->playSound(boost::shared_ptr<Player>(), mPos, SoundEvents::SHULKER_BOX_OPEN,
                          SoundSource::eSoundSource_Blocks, 0.5f, mLevel->mRandom->nextFloat() * 0.1f + 0.9f, 16.0f);
    }
}

void ShulkerBoxBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
    if (player->isSpectator())
    {
        return;
    }

    --mOpenCount;
    mLevel->blockEvent(mPos, getBlock(), 1, mOpenCount);

    if (mOpenCount <= 0)
    {
        mLevel->playSound(boost::shared_ptr<Player>(), mPos, SoundEvents::SHULKER_BOX_CLOSE,
                          SoundSource::eSoundSource_Blocks, 0.5f, mLevel->mRandom->nextFloat() * 0.1f + 0.9f, 16.0f);
    }
}

void ShulkerBoxBlockEntity::clearContent()
{
    RandomizableLockableContainerBlockEntity::clearContent();
    mCleared = true;
    mOpenCount = 0;
    mAnimationStatus = 0;
}

AbstractContainerMenu* ShulkerBoxBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory,
                                                          boost::shared_ptr<Player> player)
{
    boost::shared_ptr<Container> container = boost::static_pointer_cast<ShulkerBoxBlockEntity>(shared_from_this());
    return new ShulkerBoxMenu(inventory, container, player);
}

int ShulkerBoxBlockEntity::getTypeId(bool large)
{
    return ClientboundContainerOpenPacket::SHULKER_BOX;
}

arrayWithLength<int> ShulkerBoxBlockEntity::getSlotsForFace(const Direction* face)
{
    arrayWithLength<int> slots;
    return slots;
}

bool ShulkerBoxBlockEntity::canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> item,
                                                    const Direction* face)
{
    return dynamic_cast<ShulkerBoxBlock*>(Block::byItem(item->getItem())) == NULL;
}

bool ShulkerBoxBlockEntity::canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> item,
                                                   const Direction* face)
{
    return true;
}

int ShulkerBoxBlockEntity::getAnimationStatus()
{
    return mAnimationStatus;
}

void ShulkerBoxBlockEntity::setDestroyedByCreativePlayer(bool value)
{
    mDestroyedByCreativePlayer = value;
}

bool ShulkerBoxBlockEntity::isCleared()
{
    return mCleared;
}

bool ShulkerBoxBlockEntity::isDestroyedByCreativePlayer()
{
    return mDestroyedByCreativePlayer;
}

bool ShulkerBoxBlockEntity::shouldDrop()
{
    return !mDestroyedByCreativePlayer || !hasLevel() || onlyOpCanSetNbt() || mItems.size() == 0;
}

CompoundTag* ShulkerBoxBlockEntity::saveToTag(CompoundTag* tag)
{
    if (!trySaveLootTable(tag))
    {
        ContainerHelper::saveAllItems(tag, mItems, false);
    }
    mCustomName->saveToTag(tag);
    if (!tag->contains(L"Lock") && isLocked())
    {
        getLockKeyCode()->addTag(tag);
    }
    return tag;
}

float ShulkerBoxBlockEntity::getProgress(float partialTick)
{
    return mProgressOld + (mProgress - mProgressOld) * partialTick;
}

AABB* ShulkerBoxBlockEntity::getBoundingBox(const Direction* direction)
{
    float progress = getProgress(1.0f);
    double x = direction == NULL ? 0.0 : direction->getAxisDirection()->getStep() == 0 ? 0.0 : progress * direction->getAxisDirection()->getStep();
    return AABB::newTemp(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)->expand(x, x, x);
}

AABB* ShulkerBoxBlockEntity::getBoundingBox(const BlockState* state)
{
    return getBoundingBox(Direction::UP);
}

AABB* ShulkerBoxBlockEntity::getTopBoundingBox(const Direction* direction)
{
    const Direction* opposite = direction->getOpposite();
    return getBoundingBox(direction)->contract(-static_cast<double>(opposite->getStepX()),
                                                -static_cast<double>(opposite->getStepY()),
                                                -static_cast<double>(opposite->getStepZ()));
}

void ShulkerBoxBlockEntity::moveCollidedEntities()
{
    if (mLevel == NULL)
        return;
    const BlockState* state = mLevel->getBlockState(mPos);
    if (state == NULL)
        return;
    const Direction* direction = state->getValue(DirectionalBlock::FACING_PROPERTY);
    AABB* box = getTopBoundingBox(direction)->move(mPos);
    std::vector<boost::shared_ptr<Entity> > entities;
    mLevel->getEntities(boost::shared_ptr<Entity>(), box, &entities);
    Direction::Axis* axis = direction->getAxis();
    const int step = direction->getAxisDirection()->getStep();
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities.begin(); it != entities.end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        if (!entity || entity->mRemoved || entity->getPistonPushReaction() == 3)
            continue;
        double dx = 0.0;
        double dy = 0.0;
        double dz = 0.0;
        const AABB* entityBox = entity->mBoundingBox;
        if (entityBox == NULL)
            continue;
        const double offset = 0.01;
        if (axis->getId() == Direction::Axis::eAxis_X)
            dx = step > 0 ? box->mMaxX - entityBox->mMinX + offset : entityBox->mMaxX - box->mMinX + offset;
        else if (axis->getId() == Direction::Axis::eAxis_Y)
            dy = step > 0 ? box->mMaxY - entityBox->mMinY + offset : entityBox->mMaxY - box->mMinY + offset;
        else
            dz = step > 0 ? box->mMaxZ - entityBox->mMinZ + offset : entityBox->mMaxZ - box->mMinZ + offset;
        dx *= step;
        dy *= step;
        dz *= step;
        entity->move(eMoverType_ShulkerBox, dx, dy, dz, false);
    }
}

void ShulkerBoxBlockEntity::updateAnimation()
{
    mProgressOld = mProgress;
    if (mAnimationStatus == 0)
    {
        mProgress = 0.0f;
    }
    else if (mAnimationStatus == 1)
    {
        mProgress += 0.1f;
        if (mProgress >= 1.0f)
        {
            mProgress = 1.0f;
            mAnimationStatus = 2;
            moveCollidedEntities();
        }
    }
    else if (mAnimationStatus == 3)
    {
        mProgress -= 0.1f;
        if (mProgress <= 0.0f)
        {
            mProgress = 0.0f;
            mAnimationStatus = 0;
        }
    }
}

void ShulkerBoxBlockEntity::tick()
{
    updateAnimation();
    if (mAnimationStatus == 1 || mAnimationStatus == 3)
    {
        moveCollidedEntities();
    }
}

bool ShulkerBoxBlockEntity::triggerEvent(int id, int param)
{
    if (id == 1)
    {
        mOpenCount = param;
        mAnimationStatus = param == 0 ? 3 : 1;
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

void ShulkerBoxBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new ItemListDataWalker(BlockEntityType::SHULKER_BOX, L"Items"));
}

void ShulkerBoxBlockEntity::loadFromTag(CompoundTag* tag)
{
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    if (!tryLoadLootTable(tag) && tag->contains(L"Items", Tag::TAG_LIST))
    {
        ContainerHelper::loadAllItems(tag, mItems);
    }
    mCustomName->loadFromTag(tag);
}

void ShulkerBoxBlockEntity::load(CompoundTag* tag)
{
    LockableContainerBlockEntity::load(tag);
    loadFromTag(tag);
}

CompoundTag* ShulkerBoxBlockEntity::save(CompoundTag* tag)
{
    LockableContainerBlockEntity::save(tag);
    return saveToTag(tag);
}

NonNullList<not_null_ptr<ItemInstance> > ShulkerBoxBlockEntity::getItems()
{
    return mItems;
}

void ShulkerBoxBlockEntity::setItems(NonNullList<not_null_ptr<ItemInstance> > items)
{
    mItems = items;
}

const DyeColor* ShulkerBoxBlockEntity::getColor()
{
    return mColor;
}

boost::shared_ptr<Packet> ShulkerBoxBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>(new ClientboundBlockEntityDataPacket(mPos, 10, mLevel->mDimension->getType()->getId(), getUpdateTag()));
}

boost::shared_ptr<BlockEntity> ShulkerBoxBlockEntity::clone()
{
    boost::shared_ptr<ShulkerBoxBlockEntity> result(new ShulkerBoxBlockEntity());
    BlockEntity::clone(result);
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        result->mItems[i] = mItems[i]->copy();
    }
    result->mCleared = mCleared;
    result->mOpenCount = mOpenCount;
    result->mAnimationStatus = mAnimationStatus;
    result->mProgress = mProgress;
    result->mProgressOld = mProgressOld;
    result->mColor = mColor;
    result->mDestroyedByCreativePlayer = mDestroyedByCreativePlayer;
    result->mCustomName->set(mCustomName->raw(), mCustomName->getUUID(), mCustomName->isChatCensored(), mCustomName->isUUIDLocal());
    return result;
}

unsigned long long ShulkerBoxBlockEntity::GetType()
{
    return BlockEntityType::SHULKER_BOX;
}

void ShulkerBoxBlockEntity::operator=(const ShulkerBoxBlockEntity& other)
{
    RandomizableLockableContainerBlockEntity::operator=(other);
    mItems = other.mItems;
    mCleared = other.mCleared;
    mOpenCount = other.mOpenCount;
    mAnimationStatus = other.mAnimationStatus;
    mProgress = other.mProgress;
    mProgressOld = other.mProgressOld;
    mColor = other.mColor;
    mDestroyedByCreativePlayer = other.mDestroyedByCreativePlayer;
}
