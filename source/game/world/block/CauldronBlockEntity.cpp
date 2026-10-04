#include "game/world/block/CauldronBlockEntity.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/DyeColor.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potion.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"

CauldronBlockEntity::CauldronBlockEntity()
    : BlockEntity(), Container(), mPotionId(L""), mHasCustomColor(false),
      mCustomColor(0), mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(10, ItemInstance::EMPTY)),
      mCustomName(new CensoredString())
{
}

CauldronBlockEntity::~CauldronBlockEntity()
{
    delete[] mItems.items;
}

BlockEntity* CauldronBlockEntity::create()
{
    return new CauldronBlockEntity();
}

std::wstring CauldronBlockEntity::getName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return L10N::GetString(IDS_ITEM_CAULDRON);
}

std::wstring CauldronBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return L"";
}

bool CauldronBlockEntity::hasCustomName()
{
    if (!mCustomName->checkParentalFilter())
    {
        return false;
    }
    return mCustomName->raw().size() != 0;
}

std::wstring CauldronBlockEntity::getDisplayName()
{
    return hasCustomName() ? getCustomName() : getName();
}

unsigned int CauldronBlockEntity::getContainerSize()
{
    return 10;
}

bool CauldronBlockEntity::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        not_null_ptr<ItemInstance> item = mItems[i];
        if (!item->isEmpty())
        {
            return false;
        }
    }
    return true;
}

not_null_ptr<ItemInstance> CauldronBlockEntity::getItem(unsigned int slot)
{
    if (slot < mItems.size())
    {
        return mItems[slot];
    }
    return ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> CauldronBlockEntity::removeItem(unsigned int slot, int count)
{
    return ContainerHelper::removeItem(mItems, slot, count);
}

not_null_ptr<ItemInstance> CauldronBlockEntity::removeItemNoUpdate(int slot)
{
    return ContainerHelper::takeItem(mItems, slot);
}

void CauldronBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    if (slot < mItems.size())
    {
        mItems[slot] = item;
    }
    BlockEntity::setChanged();
}

int CauldronBlockEntity::getMaxStackSize()
{
    return 64;
}

bool CauldronBlockEntity::stillValid(boost::shared_ptr<Player> player)
{
    if (mLevel->getBlockEntity(mPos) != shared_from_this())
    {
        return false;
    }
    if (player->distanceAsIfHitscanned(mPos, *player->mBoundingBox) <= 30.25f)
    {
        return true;
    }
    return false;
}

void CauldronBlockEntity::startOpen(boost::shared_ptr<Player>)
{
}

void CauldronBlockEntity::stopOpen(boost::shared_ptr<Player>)
{
}

bool CauldronBlockEntity::canPlaceItem(int, not_null_ptr<ItemInstance>)
{
    return false;
}

int CauldronBlockEntity::getContainerData(int)
{
    return 0;
}

void CauldronBlockEntity::setContainerData(int, int)
{
}

int CauldronBlockEntity::getContainerDataCount()
{
    return 0;
}

void CauldronBlockEntity::clearContent()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        mItems[i] = mItems.defaultValue;
    }
}

std::wstring CauldronBlockEntity::getPotionId() const { return mPotionId; }

void CauldronBlockEntity::setPotionId(const std::wstring& potionId)
{
    mPotionId = potionId;
    BlockEntity::setChanged();
}

void CauldronBlockEntity::setCustomColor(const int& color)
{
    mCustomColor = color;
    mHasCustomColor = color != 0;
    BlockEntity::setChanged();
}

int CauldronBlockEntity::getPotionColor() const
{
    return MobEffectInstance::getColorValue(&Potion::byName(mPotionId)->getEffects());
}

int CauldronBlockEntity::getCustomColor() const { return mCustomColor; }

int CauldronBlockEntity::getColor() const
{
    if (getPotionId().compare(L"") != 0)
    {
        return getPotionColor();
    }
    if (mHasCustomColor)
    {
        return getCustomColor();
    }
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Cauldron_Water);
}

Potion::EType CauldronBlockEntity::getPotionType() const { return mPotionType; }
void CauldronBlockEntity::setPotionType(Potion::EType type) { mPotionType = type; }

int CauldronBlockEntity::getMixDyeColor()
{
    unsigned int rgb[3] = {0, 0, 0};
    unsigned int count = 0;
    if (mHasCustomColor)
    {
        rgb[2] = mCustomColor;
        rgb[0] = (rgb[2] >> 16) & 255;
        rgb[1] = (rgb[2] >> 8) & 255;
        rgb[2] &= 255;
        count = 1;
    }
    for (int slot = 2; slot < 10; ++slot)
    {
        not_null_ptr<ItemInstance> item = getItem(slot);
        if (!item->isEmpty() && item->getItem() == Items::dye)
        {
            arrayWithLength<const float> colors = DyeColor::byItemData(item->getAuxValue())->getTextureDiffuseColors();
            int amount = item->getCount();
            rgb[0] += amount * static_cast<int>(colors.data[0] * 255.0f);
            rgb[1] += amount * static_cast<int>(colors.data[1] * 255.0f);
            rgb[2] += amount * static_cast<int>(colors.data[2] * 255.0f);
            count += amount;
        }
    }
    if (static_cast<int>(count) <= 0)
    {
        return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Cauldron_Water);
    }
    return (((static_cast<int>(rgb[0]) / static_cast<int>(count)) << 8) + static_cast<int>(rgb[1]) / static_cast<int>(count) << 8)
        + static_cast<int>(rgb[2]) / static_cast<int>(count);
}

void CauldronBlockEntity::mixDyes()
{
    setCustomColor(getMixDyeColor());
    for (int slot = 2; slot < 10; ++slot)
    {
        setItem(slot, ItemInstance::EMPTY);
    }
}

void CauldronBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mCustomName->loadFromTag(tag);
    delete[] mItems.items;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    ContainerHelper::loadAllItems(tag, mItems);
    mPotionId = tag->getString(L"PotionId");
    bool splash = tag->getByte(L"IsSplash") != 0;
    if (!splash)
    {
        mPotionType = static_cast<Potion::EType>(tag->getShort(L"PotionType"));
    }
    else
    {
        mPotionType = Potion::SPLASH;
    }
    mHasCustomColor = tag->contains(L"CustomColor");
    if (mHasCustomColor)
    {
        mCustomColor = tag->getInt(L"CustomColor");
    }
}

void CauldronBlockEntity::loadFromPacket(Level* level, const BlockPos& pos, CompoundTag* tag)
{
    load(tag);
    if (level->getBlockId(pos.x, pos.y, pos.z) == BlockId::CAULDRON)
    {
        level->setData(pos, level->getBlockData(pos.x, pos.y, pos.z), 2, true);
    }
}

CompoundTag* CauldronBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mCustomName->saveToTag(tag);
    ContainerHelper::saveAllItems(tag, mItems);
    tag->putString(L"PotionId", mPotionId);
    tag->putShort(L"PotionType", static_cast<short>(mPotionType));
    if (mHasCustomColor)
    {
        tag->putInt(L"CustomColor", mCustomColor);
    }
    return tag;
}

void CauldronBlockEntity::tick() { BlockEntity::tick(); }

boost::shared_ptr<Packet> CauldronBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>(new ClientboundBlockEntityDataPacket(mPos, 12, mLevel->mDimension->getType()->getId(), getUpdateTag()));
}

CompoundTag* CauldronBlockEntity::getUpdateTag()
{
    return save(new CompoundTag());
}

boost::shared_ptr<BlockEntity> CauldronBlockEntity::clone()
{
    boost::shared_ptr<CauldronBlockEntity> result(new CauldronBlockEntity());
    BlockEntity::clone(result);
    result->mPotionId = mPotionId;
    result->mPotionType = mPotionType;
    result->mHasCustomColor = mHasCustomColor;
    result->mCustomColor = mCustomColor;
    result->mCustomName->set(mCustomName->raw(), mCustomName->getUUID(), mCustomName->isChatCensored(), mCustomName->isUUIDLocal());
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        result->mItems[i] = mItems[i]->copy();
    }
    return result;
}

void CauldronBlockEntity::setChanged() { BlockEntity::setChanged(); }
unsigned long long CauldronBlockEntity::GetType()
{
    return BlockEntityType::CAULDRON;
}
