#include "game/world/block/ChestBlockEntity.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/CompoundContainer.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/ChestMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

ChestBlockEntity::ChestBlockEntity(bool large)
    : RandomizableLockableContainerBlockEntity(),
      mLarge(large),
      mRefilledAnimationTicks(-1),
      mNeighborsChecked(false),
      mNorthNeighbour(),
      mEastNeighbour(),
      mWestNeighbour(),
      mSouthNeighbour(),
      mOpen(0.0F),
      mOpenOld(0.0F),
      mOpenCount(0),
      mTickCount(0),
      mCachedType(-1),
      mNeighbourRemoved(false)
{
    _init(large);
}

ChestBlockEntity::ChestBlockEntity(ChestBlock::Type type, bool large)
    : RandomizableLockableContainerBlockEntity(),
      mLarge(large),
      mRefilledAnimationTicks(-1),
      mNeighborsChecked(false),
      mNorthNeighbour(),
      mEastNeighbour(),
      mWestNeighbour(),
      mSouthNeighbour(),
      mOpen(0.0F),
      mOpenOld(0.0F),
      mOpenCount(0),
      mTickCount(0),
      mCachedType(type),
      mNeighbourRemoved(false)
{
    _init(large);
    mCachedType = type;
}

ChestBlockEntity::~ChestBlockEntity()
{
}

BlockEntity* ChestBlockEntity::create()
{
    return new ChestBlockEntity(false);
}

void ChestBlockEntity::_init(bool large)
{
    mLarge = large;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(27, not_null_ptr<ItemInstance>());
    mRefilledAnimationTicks = -1;
    mOpen = 0.0F;
    mOpenOld = 0.0F;
    mOpenCount = 0;
    mTickCount = 0;
    mNeighborsChecked = false;
    mCachedType = -1;
    mNeighbourRemoved = false;
}

std::wstring ChestBlockEntity::getName()
{
    if (hasCustomName())
    {
        return mCustomName ? mCustomName->get() : std::wstring();
    }
    return L"container.chest";
}

std::wstring ChestBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mCustomName ? mCustomName->get() : std::wstring();
    }
    return L"container.chest";
}

unsigned int ChestBlockEntity::getContainerSize()
{
    return 0x1b;
}

bool ChestBlockEntity::isEmpty()
{
    unsigned int i;
    for (i = 0; i < mItems.size(); ++i)
    {
        if (!mItems[i] || !mItems[i]->isEmpty())
        {
            return false;
        }
    }
    return true;
}

int ChestBlockEntity::getMaxStackSize()
{
    return 64;
}

void ChestBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

bool ChestBlockEntity::stillValid(boost::shared_ptr<Player> player)
{
    if (mNeighbourRemoved)
    {
        return false;
    }
    return RandomizableLockableContainerBlockEntity::stillValid(player);
}

void ChestBlockEntity::startOpen(boost::shared_ptr<Player> player)
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
    mLevel->updateNeighborsAt(mPos, getBlock(), false);
    if (getType() != 1)
    {
        mLevel->updateNeighborsAt(mPos.below(), getBlock(), false);
    }
}

void ChestBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
    if (player->isSpectator() || dynamic_cast<ChestBlock*>(getBlock()) == NULL)
    {
        return;
    }
    --mOpenCount;
    mLevel->blockEvent(mPos, getBlock(), 1, mOpenCount);
    mLevel->updateNeighborsAt(mPos, getBlock(), false);
    if (getType() != 1)
    {
        mLevel->updateNeighborsAt(mPos.below(), getBlock(), false);
    }
}

bool ChestBlockEntity::canPlaceItem(int, not_null_ptr<ItemInstance>)
{
    return true;
}

AbstractContainerMenu* ChestBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player)
{
    unpackLootTable(player);
    return new ChestMenu(inventory, boost::static_pointer_cast<ChestBlockEntity>(shared_from_this()), player);
}

int ChestBlockEntity::getTypeId(bool large)
{
    if (mLarge)
    {
        return 13;
    }
    return large ? 14 : 0;
}

void ChestBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    if (fixer)
    {
        fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                         (DataWalker*)new ItemListDataWalker(0x1e00201ULL, L"Items"));
    }
}

void ChestBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mCustomName->loadFromTag(tag);
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    ContainerHelper::loadAllItems(tag, mItems);
    mLarge = tag->getBoolean(L"bonus");
}

CompoundTag* ChestBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mCustomName->saveToTag(tag);
    ContainerHelper::saveAllItems(tag, mItems);
    tag->putBoolean(L"bonus", mLarge);
    return tag;
}

void ChestBlockEntity::clearCache()
{
    BlockEntity::clearCache();
    mNeighborsChecked = false;
}

void ChestBlockEntity::heyImYourNeighbor(boost::shared_ptr<ChestBlockEntity> neighbour, const Direction* direction)
{
    if (!neighbour || !direction)
    {
        mNeighborsChecked = false;
        return;
    }

    boost::shared_ptr<ChestBlockEntity> current;
    switch (direction->get3DDataValue())
    {
    case Direction::eDirection_North:
        current = mNorthNeighbour.lock();
        mNorthNeighbour = neighbour;
        break;
    case Direction::eDirection_South:
        current = mSouthNeighbour.lock();
        mSouthNeighbour = neighbour;
        break;
    case Direction::eDirection_West:
        current = mWestNeighbour.lock();
        mWestNeighbour = neighbour;
        break;
    case Direction::eDirection_East:
        current = mEastNeighbour.lock();
        mEastNeighbour = neighbour;
        break;
    default:
        return;
    }
    if (current != neighbour)
    {
        mNeighborsChecked = false;
    }
    if (mLevel)
    {
        mLevel->updateNeighborsAtExceptFromFacing(mPos, getBlock(), direction);
    }
}

boost::shared_ptr<ChestBlockEntity> ChestBlockEntity::getNeighbour()
{
    boost::shared_ptr<ChestBlockEntity> neighbour = mWestNeighbour.lock();
    if (!neighbour)
    {
        neighbour = mEastNeighbour.lock();
    }
    if (!neighbour)
    {
        neighbour = mNorthNeighbour.lock();
    }
    if (!neighbour)
    {
        neighbour = mSouthNeighbour.lock();
    }
    return neighbour;
}

void ChestBlockEntity::removeNeighbour()
{
    mNeighbourRemoved = true;
}

bool ChestBlockEntity::isSameChest(const BlockPos& position)
{
    if (!mLevel)
    {
        return false;
    }
    boost::shared_ptr<ChestBlockEntity> chest =
        boost::dynamic_pointer_cast<ChestBlockEntity>(mLevel->getBlockEntity(position));
    return chest && chest->getType() == getType();
}

boost::shared_ptr<ChestBlockEntity> ChestBlockEntity::findAndGreetChestEntity(const Direction* direction)
{
    if (!mLevel || !direction)
    {
        return boost::shared_ptr<ChestBlockEntity>();
    }
    BlockPos position = mPos.relative(direction);
    if (!isSameChest(position))
    {
        return boost::shared_ptr<ChestBlockEntity>();
    }
    boost::shared_ptr<ChestBlockEntity> neighbour =
        boost::static_pointer_cast<ChestBlockEntity>(mLevel->getBlockEntity(position));
    if (neighbour)
    {
        neighbour->heyImYourNeighbor(boost::static_pointer_cast<ChestBlockEntity>(shared_from_this()),
                                     direction->getOpposite());
    }
    return neighbour;
}

void ChestBlockEntity::checkNeighbors()
{
    if (!mNeighborsChecked && mWestNeighbour.expired() && mEastNeighbour.expired() &&
        mNorthNeighbour.expired() && mSouthNeighbour.expired())
    {
        mNeighborsChecked = true;
        mWestNeighbour = findAndGreetChestEntity(Direction::WEST);
        if (mWestNeighbour.expired())
        {
            mEastNeighbour = findAndGreetChestEntity(Direction::EAST);
            if (mEastNeighbour.expired())
            {
                mNorthNeighbour = findAndGreetChestEntity(Direction::NORTH);
                if (mNorthNeighbour.expired())
                {
                    mSouthNeighbour = findAndGreetChestEntity(Direction::SOUTH);
                }
            }
        }
    }
}

void ChestBlockEntity::tickRefilledAnimation()
{
    if (mRefilledAnimationTicks < 0)
    {
        return;
    }
    if (getLevel()->mRandom->nextInt(4) != 0)
    {
        return;
    }
    double x = (double)getBlockPos().x + (double)getLevel()->mRandom->nextFloat();
    double y = (double)getBlockPos().y + 0.7;
    double z = (double)getBlockPos().z + (double)getLevel()->mRandom->nextFloat();
    mLevel->addParticle(PARTICLE_CHEST_REFILL, true, x, y, z, 0.0, 0.1, 0.0, arrayWithLength<int>());
    --mRefilledAnimationTicks;
}

void ChestBlockEntity::tick()
{
    tickRefilledAnimation();
    checkNeighbors();
    if (mNeighbourRemoved)
    {
        mLevel->updateNeighborsAt(mPos, getBlock(), false);
        mNeighbourRemoved = false;
    }

    int x = mPos.x;
    int y = mPos.y;
    int z = mPos.z;
    ++mTickCount;

    if (!mLevel->mIsClientSide && mOpenCount != 0 && (mTickCount + x + y + z) % 200 == 0)
    {
        mOpenCount = 0;
        const AABB* range = AABB::newTemp((double)((float)x - 5.0f), (double)((float)y - 5.0f),
                                          (double)((float)z - 5.0f), (double)((float)(x + 1) + 5.0f),
                                          (double)((float)(y + 1) + 5.0f), (double)((float)(z + 1) + 5.0f));
        std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(Player), range);
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
             it != entities->end(); ++it)
        {
            boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(*it);
            ChestMenu* menu = dynamic_cast<ChestMenu*>(player->mContainerMenu);
            if (menu == NULL)
            {
                continue;
            }
            boost::shared_ptr<Container> container = menu->getContainer();
            boost::shared_ptr<Container> self =
                boost::dynamic_pointer_cast<Container>(shared_from_this());
            boost::shared_ptr<CompoundContainer> compound =
                boost::dynamic_pointer_cast<CompoundContainer>(container);
            if (container == self || (compound.get() != NULL && compound->contains(self)))
            {
                ++mOpenCount;
            }
        }
        delete entities;
    }

    mOpenOld = mOpen;
    if (mOpenCount > 0 && mOpen == 0.0f && mNorthNeighbour.expired() && mWestNeighbour.expired())
    {
        double soundX = (double)x + 0.5;
        double soundZ = (double)z + 0.5;
        if (!mEastNeighbour.expired())
        {
            soundZ += 0.5;
        }
        if (!mSouthNeighbour.expired())
        {
            soundX += 0.5;
        }
        mLevel->playSound(boost::shared_ptr<Player>(), soundX, (double)y + 0.5, soundZ,
                          SoundEvents::CHEST_OPEN, SoundSource::eSoundSource_Blocks, 0.2f,
                          mLevel->mRandom->nextFloat() * 0.1f + 0.9f, 16.0f);
    }

    if ((mOpenCount == 0 && mOpen > 0.0f) || (mOpenCount > 0 && mOpen < 1.0f))
    {
        float previous = mOpen;
        if (mOpenCount > 0)
        {
            mOpen += 0.1f;
        }
        else
        {
            mOpen -= 0.1f;
        }
        if (mOpen > 1.0f)
        {
            mOpen = 1.0f;
        }
        if (mOpen < 0.5f && previous >= 0.5f && mNorthNeighbour.expired() && mWestNeighbour.expired())
        {
            double soundX = (double)x + 0.5;
            double soundZ = (double)z + 0.5;
            if (!mEastNeighbour.expired())
            {
                soundZ += 0.5;
            }
            if (!mSouthNeighbour.expired())
            {
                soundX += 0.5;
            }
            mLevel->playSound(boost::shared_ptr<Player>(), soundX, (double)y + 0.5, soundZ,
                              SoundEvents::CHEST_CLOSE, SoundSource::eSoundSource_Blocks, 0.2f,
                              mLevel->mRandom->nextFloat() * 0.1f + 0.9f, 16.0f);
        }
        if (mOpen < 0.0f)
        {
            mOpen = 0.0f;
        }
    }
}

bool ChestBlockEntity::triggerEvent(int id, int param)
{
    if (id == 1)
    {
        mOpenCount = param;
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

void ChestBlockEntity::setRemoved()
{
    BlockEntity::setRemoved();
    checkNeighbors();
    boost::shared_ptr<ChestBlockEntity> neighbour = getNeighbour();
    if (neighbour)
    {
        neighbour->removeNeighbour();
    }
}

int ChestBlockEntity::getType()
{
    if (mCachedType == -1)
    {
        if (mLevel == NULL || dynamic_cast<ChestBlock*>(getBlock()) == NULL)
        {
            return 0;
        }
        mCachedType = ((ChestBlock*)getBlock())->mType;
    }
    return mCachedType;
}

NonNullList<not_null_ptr<ItemInstance> > ChestBlockEntity::getItems()
{
    return mItems;
}

void ChestBlockEntity::setItems(NonNullList<not_null_ptr<ItemInstance> > items)
{
    mItems = items;
}

boost::shared_ptr<BlockEntity> ChestBlockEntity::clone()
{
    boost::shared_ptr<ChestBlockEntity> result(new ChestBlockEntity((ChestBlock::Type)getType(), mLarge));
    result->setItems(mItems);
    result->mOpenCount = mOpenCount;
    result->mOpen = mOpen;
    result->mOpenOld = mOpenOld;
    result->mTickCount = mTickCount;
    return result;
}

void ChestBlockEntity::startRefilledAnimation()
{
    mRefilledAnimationTicks = 200;
}

CompoundTag* ChestBlockEntity::getUpdateTagForPistonMovement()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return tag;
}

std::wstring ChestBlockEntity::getDisplayName()
{
    return LockableContainerBlockEntity::getDisplayName();
}

unsigned long long ChestBlockEntity::GetType()
{
    return 0x0000000001e00201ULL;
}
