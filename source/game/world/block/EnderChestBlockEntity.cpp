#include "game/world/block/EnderChestBlockEntity.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"

EnderChestBlockEntity::EnderChestBlockEntity()
    : BlockEntity(), mOpen(0.0F), mOpenOld(0.0F), mOpenCount(0), mTickCount(0),
      mName(new CensoredString())
{
}

EnderChestBlockEntity::~EnderChestBlockEntity()
{
}

BlockEntity* EnderChestBlockEntity::create()
{
    return new EnderChestBlockEntity();
}

void EnderChestBlockEntity::setCustomName(const std::wstring& name, const std::wstring& uuid,
                                          bool chatCensored, bool uuidLocal)
{
    if (mName)
    {
        mName->set(name, uuid, chatCensored, uuidLocal);
    }
}

void EnderChestBlockEntity::tick()
{
    ++mTickCount;
    if (mTickCount % 20 == 0 && mLevel != NULL)
    {
        mLevel->blockEvent(mPos, NULL, 1, mOpenCount);
    }

    mOpenOld = mOpen;
    if (mOpenCount > 0 && mOpen < 1.0F)
    {
        mOpen += 0.1F;
        if (mOpen > 1.0F)
        {
            mOpen = 1.0F;
        }
    }
    else if (mOpenCount <= 0 && mOpen > 0.0F)
    {
        mOpen -= 0.1F;
        if (mOpen < 0.0F)
        {
            mOpen = 0.0F;
        }
    }

    if (mLevel == NULL)
    {
        return;
    }

    Random* random = mLevel->mRandom;
    if (mOpen == 0.0F && mOpenCount > 0)
    {
        mLevel->addParticle(ePARTICLE_PORTAL, false,
                            (double)mPos.x + 0.5, (double)mPos.y + 0.8 + (double)random->nextFloat() * 0.1,
                            (double)mPos.z + 0.5, 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
    if (mOpen < 0.5F && mOpenOld >= 0.5F)
    {
        mLevel->addParticle(ePARTICLE_PORTAL, false,
                            (double)mPos.x + 0.5, (double)mPos.y + 0.8 + (double)random->nextFloat() * 0.1,
                            (double)mPos.z + 0.5, 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
}

bool EnderChestBlockEntity::triggerEvent(int id, int param)
{
    if (id == 1)
    {
        mOpenCount = param;
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

void EnderChestBlockEntity::setRemoved()
{
    BlockEntity::setRemoved();
}

void EnderChestBlockEntity::startOpen()
{
    ++mOpenCount;
    if (mLevel != NULL)
    {
        mLevel->blockEvent(mPos, NULL, 1, mOpenCount);
    }
}

void EnderChestBlockEntity::stopOpen()
{
    --mOpenCount;
    if (mLevel != NULL)
    {
        mLevel->blockEvent(mPos, NULL, 1, mOpenCount);
    }
}

bool EnderChestBlockEntity::stillValid(boost::shared_ptr<Player> player)
{
    if (player.get() == NULL || mLevel == NULL)
    {
        return false;
    }
    Entity* entity = (Entity*)player.get();
    return entity->distanceAsIfHitscanned(mPos, *entity->mBoundingBox) <= 30.25;
}

InteractionObject::NameData EnderChestBlockEntity::GetNetworkNameData()
{
    std::wstring name;
    if (mName)
    {
        name = mName->raw();
    }
    return InteractionObject::NameData(name, std::wstring(), true, false);
}

boost::shared_ptr<BlockEntity> EnderChestBlockEntity::clone()
{
    boost::shared_ptr<EnderChestBlockEntity> copy(new EnderChestBlockEntity());
    copy->mOpen = mOpen;
    copy->mOpenOld = mOpenOld;
    copy->mOpenCount = mOpenCount;
    copy->mTickCount = mTickCount;
    copy->BlockEntity::operator=(*this);
    return copy;
}

unsigned long long EnderChestBlockEntity::GetType()
{
    return 0x120020800000000ULL;
}
