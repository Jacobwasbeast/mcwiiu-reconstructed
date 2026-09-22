#include "game/world/block/CauldronBlockEntity.hpp"

#include "game/core/DyeColor.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potion.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"

CauldronBlockEntity::CauldronBlockEntity()
    : BlockEntity(), Container(), mPotionId(), mPotionType(Potion::NORMAL), mHasCustomColor(false),
      mCustomColor(0), mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(10, ItemInstance::EMPTY)),
      mCustomName(new CensoredString())
{
}

CauldronBlockEntity::~CauldronBlockEntity()
{
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
    return L10N::GetString(0x5a1ba3a);
}

std::wstring CauldronBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring();
}

bool CauldronBlockEntity::hasCustomName()
{
    return mCustomName && mCustomName->checkParentalFilter() && !mCustomName->raw().empty();
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
        if (!mItems[i]->isEmpty())
        {
            return false;
        }
    }
    return true;
}

not_null_ptr<ItemInstance> CauldronBlockEntity::getItem(unsigned int slot)
{
    return slot < mItems.size() ? mItems[slot] : ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> CauldronBlockEntity::removeItem(unsigned int slot, int count)
{
    return slot < mItems.size() ? ContainerHelper::removeItem(mItems, slot, static_cast<unsigned int>(count))
                                : ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> CauldronBlockEntity::removeItemNoUpdate(int slot)
{
    return slot >= 0 && static_cast<unsigned int>(slot) < mItems.size()
        ? ContainerHelper::takeItem(mItems, static_cast<unsigned int>(slot)) : ItemInstance::EMPTY;
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
    return player->distanceAsIfHitscanned(mPos, *player->mBoundingBox) <= 30.25;
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
        mItems[i] = ItemInstance::EMPTY;
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
    Potion* potion = Potion::byName(mPotionId);
    return potion == NULL ? 0 : PotionUtils::getColor(potion);
}

int CauldronBlockEntity::getCustomColor() const { return mCustomColor; }

int CauldronBlockEntity::getColor() const
{
    if (mPotionId != L"minecraft:water")
    {
        return getPotionColor();
    }
    if (mHasCustomColor)
    {
        return mCustomColor;
    }
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Cauldron_Water);
}

Potion::EType CauldronBlockEntity::getPotionType() const { return mPotionType; }
void CauldronBlockEntity::setPotionType(Potion::EType type) { mPotionType = type; }

int CauldronBlockEntity::getMixDyeColor()
{
    unsigned int red = mHasCustomColor ? ((mCustomColor >> 16) & 255) : 0;
    unsigned int green = mHasCustomColor ? ((mCustomColor >> 8) & 255) : 0;
    unsigned int blue = mHasCustomColor ? (mCustomColor & 255) : 0;
    unsigned int count = mHasCustomColor ? 1 : 0;
    for (int slot = 2; slot <= 3; ++slot)
    {
        not_null_ptr<ItemInstance> item = getItem(static_cast<unsigned int>(slot));
        if (!item || item->isEmpty() || item->getItem() != Items::dye)
        {
            continue;
        }
        const DyeColor* dye = DyeColor::byItemData(item->getAuxValue());
        if (dye == NULL)
        {
            continue;
        }
        arrayWithLength<const float> colors = dye->getTextureDiffuseColors();
        int amount = item->getCount();
        red += amount * static_cast<int>(colors.data[0] * 255.0f);
        green += amount * static_cast<int>(colors.data[1] * 255.0f);
        blue += amount * static_cast<int>(colors.data[2] * 255.0f);
        count += amount;
    }
    if (count == 0)
    {
        return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Cauldron_Water);
    }
    return ((red / count) << 16) | ((green / count) << 8) | (blue / count);
}

void CauldronBlockEntity::mixDyes()
{
    setCustomColor(getMixDyeColor());
    for (unsigned int slot = 2; slot < 10; ++slot)
    {
        setItem(slot, ItemInstance::EMPTY);
    }
}

void CauldronBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mCustomName->loadFromTag(tag);
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(10, ItemInstance::EMPTY);
    ContainerHelper::loadAllItems(tag, mItems);
    mPotionId = tag->getString(L"PotionId");
    mPotionType = tag->getByte(L"IsSplash") == 0 ? static_cast<Potion::EType>(tag->getShort(L"PotionType")) : Potion::SPLASH;
    mHasCustomColor = tag->contains(L"CustomColor");
    if (mHasCustomColor)
    {
        mCustomColor = tag->getInt(L"CustomColor");
    }
}

void CauldronBlockEntity::loadFromPacket(Level* level, const BlockPos& pos, CompoundTag* tag)
{
    load(tag);
    if (level->getBlockId(pos.x, pos.y, pos.z) == 118)
    {
        int data = level->getBlockData(pos.x, pos.y, pos.z);
        level->setData(pos, data, 2, true);
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
    return boost::shared_ptr<Packet>(new ClientboundBlockEntityDataPacket(mPos, 12, 0, getUpdateTag()));
}

CompoundTag* CauldronBlockEntity::getUpdateTag()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return tag;
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
    return 0x0000000001000211ULL;
}
