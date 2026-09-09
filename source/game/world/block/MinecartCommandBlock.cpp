#include "game/world/block/MinecartCommandBlock.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/OverwritableWalker.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

const EntityDataAccessor<std::wstring>* MinecartCommandBlock::DATA_COMMAND_ID = NULL;
const EntityDataAccessor<std::wstring>* MinecartCommandBlock::DATA_LAST_OUTPUT_ID = NULL;

MinecartCommandBlock::~MinecartCommandBlock()
{
}

void MinecartCommandBlock::staticCtor()
{
    DATA_COMMAND_ID = SynchedEntityData::defineId<std::wstring>(0x000000000A800005ULL, ENTITY_TYPE_MINECART, EntityDataSerializers::STRING);
    DATA_LAST_OUTPUT_ID = SynchedEntityData::defineId<std::wstring>(0x000000000A800005ULL, ENTITY_TYPE_MINECART, EntityDataSerializers::STRING);
}

void MinecartCommandBlock::addDataWalkers(DataFixerUpper* dataFixerUpper)
{
    std::wstring name(L"Control");
    dataFixerUpper->addWalker(static_cast<DataFixTypes::EDataFixTypes>(4), new OverwritableWalker(0x000000000A800005ULL, name));
}

void MinecartCommandBlock::defineSynchedData()
{
    Minecart::defineSynchedData();
}

BaseCommandBlock* MinecartCommandBlock::getCommandBlock()
{
    return mCommandBlock;
}

void MinecartCommandBlock::readAdditionalSaveData(CompoundTag* tag)
{
    Minecart::readAdditionalSaveData(tag);
    mCommandBlock->load(tag);
    getEntityData()->set(DATA_COMMAND_ID, mCommandBlock->getCommand(), true);
}

void MinecartCommandBlock::addAdditonalSaveData(CompoundTag* tag)
{
    Minecart::addAdditonalSaveData(tag);
    mCommandBlock->save(tag);
}

unsigned long long MinecartCommandBlock::GetType()
{
    return 0x000000000A800005ULL;
}

const Minecart::Type* MinecartCommandBlock::getType()
{
    return Minecart::Type::COMMAND_BLOCK;
}

const BlockState* MinecartCommandBlock::getDefaultDisplayBlockState()
{
    return Blocks::command_block->defaultBlockState();
}

void MinecartCommandBlock::activateMinecart(int x, int y, int z, bool powered)
{
    if (powered && mTickCount - mLastActivatedTick > 3)
    {
        mCommandBlock->performCommand(mLevel);
        mLastActivatedTick = mTickCount;
    }
}

bool MinecartCommandBlock::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    mCommandBlock->usedBy(player);
    return false;
}

void MinecartCommandBlock::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    Entity::onSyncedDataUpdated(accessor);
    if (accessor->getId() == DATA_LAST_OUTPUT_ID->getId())
    {
        mCommandBlock->setLastOutput(getEntityData()->get(DATA_LAST_OUTPUT_ID));
    }
    else if (accessor->getId() == DATA_COMMAND_ID->getId())
    {
        mCommandBlock->setCommand(getEntityData()->get(DATA_COMMAND_ID));
    }
}

bool MinecartCommandBlock::onlyOpCanSetNbt()
{
    return true;
}

MinecartCommandBlock::MinecartBaseCommandBlock::MinecartBaseCommandBlock(const boost::shared_ptr<MinecartCommandBlock>& minecart)
    : BaseCommandBlock(), mMinecart(minecart)
{
}

MinecartCommandBlock::MinecartBaseCommandBlock::~MinecartBaseCommandBlock()
{
}

void MinecartCommandBlock::MinecartBaseCommandBlock::onUpdated()
{
    mMinecart->getEntityData()->set(MinecartCommandBlock::DATA_COMMAND_ID, getCommand(), true);
    mMinecart->getEntityData()->set(MinecartCommandBlock::DATA_LAST_OUTPUT_ID, getLastOutput(), true);
}

unsigned int MinecartCommandBlock::MinecartBaseCommandBlock::getType()
{
    return 1;
}

void MinecartCommandBlock::MinecartBaseCommandBlock::writeCoordinates(DataOutputStream* output)
{
    output->writeInt(mMinecart->getId());
}

BlockPos MinecartCommandBlock::MinecartBaseCommandBlock::getCommandSenderBlockPosition()
{
    return BlockPos(Vec3::newTemp(mMinecart->mX, mMinecart->mY + 0.5, mMinecart->mZ));
}

Vec3* MinecartCommandBlock::MinecartBaseCommandBlock::getCommandSenderWorldPosition()
{
    return Vec3::newTemp(mMinecart->mX, mMinecart->mY, mMinecart->mZ);
}

Level* MinecartCommandBlock::MinecartBaseCommandBlock::getCommandSenderWorld()
{
    return mMinecart->mLevel;
}

boost::shared_ptr<Entity> MinecartCommandBlock::MinecartBaseCommandBlock::getCommandSenderEntity()
{
    return boost::shared_ptr<Entity>(mMinecart);
}

MinecraftServer* MinecartCommandBlock::MinecartBaseCommandBlock::getServer()
{
    return mMinecart->mLevel->getServer();
}
