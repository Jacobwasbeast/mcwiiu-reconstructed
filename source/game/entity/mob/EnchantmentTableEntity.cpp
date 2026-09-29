#include "game/entity/mob/EnchantmentTableEntity.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/ui/EnchantmentMenu.hpp"
#include "game/core/Mth.hpp"
#include "game/world/Level.hpp"

EnchantmentTableEntity::EnchantmentTableEntity()
    : BlockEntity(), InteractionObject(), mRandom(new Random()),
      mCustomName(new CensoredString())
{
    mTickCount = 0;
    mBookRotation = 0.0F;
    mBookRotationOld = 0.0F;
    mBookRotationTarget = 0.0F;
    mBookRotationTargetOld = 0.0F;
    mBookOpen = 0.0F;
    mBookOpenOld = 0.0F;
    mBookSpread = 0.0F;
    mBookSpreadOld = 0.0F;
    mBookRotationAngle = 0.0F;
}

EnchantmentTableEntity::~EnchantmentTableEntity()
{
    delete mRandom;
}

BlockEntity* EnchantmentTableEntity::create()
{
    return new EnchantmentTableEntity();
}

std::wstring EnchantmentTableEntity::getName()
{
    if (hasCustomName())
        return mCustomName->get();
    return std::wstring();
}

std::wstring EnchantmentTableEntity::getCustomName()
{
    if (hasCustomName())
        return mCustomName->get();
    return std::wstring();
}

bool EnchantmentTableEntity::hasCustomName()
{
    return mCustomName->checkParentalFilter() != 0 && !mCustomName->raw().empty();
}

std::wstring EnchantmentTableEntity::getDisplayName()
{
    if (hasCustomName())
        return getCustomName();
    return getName();
}

AbstractContainerMenu* EnchantmentTableEntity::createMenu(boost::shared_ptr<Inventory> inventory,
                                                            boost::shared_ptr<Player> player)
{
    return new EnchantmentMenu(inventory, mLevel, mPos);
}

int EnchantmentTableEntity::getTypeId(bool large)
{
    return ClientboundContainerOpenPacket::ENCHANTMENT;
}

InteractionObject::NameData EnchantmentTableEntity::GetNetworkNameData()
{
    return InteractionObject::NameData(mCustomName->raw(), mCustomName->getUUID(), mCustomName->isUUIDLocal(), mCustomName->isChatCensored());
}

void EnchantmentTableEntity::setCustomName(const std::wstring& name, const std::wstring& uuid,
                                            bool chatCensored, bool uuidLocal)
{
    mCustomName->set(name, uuid, chatCensored, uuidLocal);
}

CompoundTag* EnchantmentTableEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mCustomName->saveToTag(tag);
    return tag;
}

void EnchantmentTableEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mCustomName->loadFromTag(tag);
}

void EnchantmentTableEntity::tick()
{
    boost::shared_ptr<Player> player = mLevel->getNearestPlayer(
        (double)mPos.x + 0.5, (double)mPos.y + 0.5, (double)mPos.z + 0.5,
        3.0, false, -1.0);

    mBookOpenOld = mBookOpen;
    mBookRotationOld = mBookRotation;
    float rotation = mBookRotationAngle;
    float spread = mBookSpread;
    float open = mBookOpen;
    if (player.get() == NULL)
    {
        spread += 0.02F;
        open -= 0.1F;
        mBookSpread = spread;
        mBookOpen = open;
    }
    else
    {
        Entity* entity = (Entity*)player.get();
        double angle = std::atan2(entity->mZ - ((double)mPos.z + 0.5),
                                  entity->mX - ((double)mPos.x + 0.5));
        spread = (float)angle;
        open += 0.1F;
        mBookSpread = spread;
        mBookOpen = open;
        if (mBookOpen < rotation || mRandom->nextInt(40) == 0)
            mBookRotationTarget += (float)(mRandom->nextInt(4) - mRandom->nextInt(4));
        rotation = mBookRotationTarget;
    }

    while (rotation >= 3.1415927F)
        rotation -= 6.2831855F;
    while (rotation < -3.1415927F)
        rotation += 6.2831855F;
    while (spread < -3.1415927F)
        spread += 6.2831855F;
    while (spread >= 3.1415927F)
        spread -= 6.2831855F;
    mBookSpread = spread;
    float delta = spread - rotation;
    while (delta >= 3.1415927F)
        delta -= 6.2831855F;
    while (delta < -3.1415927F)
        delta += 6.2831855F;
    mBookRotationAngle = delta * 0.4F + rotation;
    mBookOpen = Mth::clamp(mBookOpen, 0.0F, 1.0F);
    ++mTickCount;
    float targetDelta = Mth::clamp((mBookRotationTarget - mBookRotationOld) * 0.4F,
                                   -0.2F, 0.2F);
    mBookRotationTargetOld += (targetDelta - mBookRotationTargetOld) * 0.9F;
    mBookRotation = mBookRotationOld + mBookRotationTargetOld;
}

boost::shared_ptr<BlockEntity> EnchantmentTableEntity::clone()
{
    boost::shared_ptr<EnchantmentTableEntity> copy(new EnchantmentTableEntity());
    copy->mTickCount = mTickCount;
    copy->mBookRotation = mBookRotation;
    copy->mBookRotationOld = mBookRotationOld;
    copy->mBookRotationTarget = mBookRotationTarget;
    copy->mBookRotationTargetOld = mBookRotationTargetOld;
    copy->mBookOpen = mBookOpen;
    copy->mBookOpenOld = mBookOpenOld;
    copy->mBookSpread = mBookSpread;
    copy->mBookSpreadOld = mBookSpreadOld;
    copy->mBookRotationAngle = mBookRotationAngle;
    copy->mCustomName->set(mCustomName->raw(), mCustomName->getUUID(), mCustomName->isChatCensored(), mCustomName->isUUIDLocal());
    BlockEntity::clone(copy);
    return copy;
}

unsigned long long EnchantmentTableEntity::GetType()
{
    return BlockEntityType::ENCHANTMENT_TABLE_ENTITY;
}
