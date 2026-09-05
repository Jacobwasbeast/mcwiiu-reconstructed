#include "game/world/block/CommandBlockEntity.hpp"

#include "game/core/DataOutputStream.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/util/Vec3.hpp"

CommandBlockEntity::AnonymousCommandBlock::AnonymousCommandBlock(CommandBlockEntity* entity)
    : BaseCommandBlock(), mEntity(entity)
{
}

CommandBlockEntity::AnonymousCommandBlock::~AnonymousCommandBlock()
{
}

BlockPos CommandBlockEntity::AnonymousCommandBlock::getCommandSenderBlockPosition()
{
    return mEntity->getBlockPos();
}

Vec3* CommandBlockEntity::AnonymousCommandBlock::getCommandSenderWorldPosition()
{
    BlockPos position = mEntity->getBlockPos();
    return Vec3::newTemp((double)position.x + 0.5, (double)position.y + 0.5, (double)position.z + 0.5);
}

Level* CommandBlockEntity::AnonymousCommandBlock::getCommandSenderWorld()
{
    return mEntity->getLevel();
}

void CommandBlockEntity::AnonymousCommandBlock::setCommand(const std::wstring& command)
{
    BaseCommandBlock::setCommand(command);
    onUpdated();
}

void CommandBlockEntity::AnonymousCommandBlock::onUpdated()
{
    mEntity->setChanged();
}

unsigned int CommandBlockEntity::AnonymousCommandBlock::getType()
{
    return 0;
}

void CommandBlockEntity::AnonymousCommandBlock::writeCoordinates(DataOutputStream* output)
{
    BlockPos position = mEntity->getBlockPos();
    output->writeInt(position.x);
    output->writeInt(position.y);
    output->writeInt(position.z);
}

CommandBlockEntity::CommandBlockEntity()
    : BlockEntity(), mPowered(0), mAutomatic(false), mConditionMet(0), mCommandBlock(new AnonymousCommandBlock(this))
{
}

CommandBlockEntity::CommandBlockEntity(int mode)
    : BlockEntity(), mPowered(0), mAutomatic(mode == 2), mConditionMet(0), mCommandBlock(new AnonymousCommandBlock(this))
{
}

CommandBlockEntity::~CommandBlockEntity()
{
    delete mCommandBlock;
}

BlockEntity* CommandBlockEntity::create()
{
    return new CommandBlockEntity();
}

BaseCommandBlock* CommandBlockEntity::getCommandBlock()
{
    return mCommandBlock;
}

unsigned char CommandBlockEntity::isPowered()
{
    return mPowered;
}

unsigned char CommandBlockEntity::wasConditionMet()
{
    return mConditionMet;
}

unsigned char CommandBlockEntity::isAutomatic()
{
    return mAutomatic;
}

void CommandBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mCommandBlock->save(tag);
    tag->putBoolean(L"powered", isPowered() != 0);
    tag->putBoolean(L"conditionMet", wasConditionMet() != 0);
    tag->putBoolean(L"auto", isAutomatic() != 0);
}

void CommandBlockEntity::setAutomatic(bool automatic)
{
    mAutomatic = automatic;
}

void CommandBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mCommandBlock->load(tag);
    mPowered = tag->getBoolean(L"powered");
    mConditionMet = tag->getBoolean(L"conditionMet");
    setAutomatic(tag->getBoolean(L"auto"));
}

boost::shared_ptr<Packet> CommandBlockEntity::getUpdatePacket()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return boost::shared_ptr<Packet>(new ClientboundBlockEntityDataPacket(getBlockPos(), 2, 0, tag));
}

bool CommandBlockEntity::onlyOpCanSetNbt()
{
    return true;
}

unsigned int CommandBlockEntity::getMode()
{
    return 2;
}

unsigned char CommandBlockEntity::isConditional()
{
    return false;
}

void CommandBlockEntity::clearRemoved()
{
    mData = 0;
    BlockEntity::clearRemoved();
}

boost::shared_ptr<BlockEntity> CommandBlockEntity::clone()
{
    boost::shared_ptr<BlockEntity> result(new CommandBlockEntity());
    CommandBlockEntity* copy = static_cast<CommandBlockEntity*>(result.get());
    copy->mCommandBlock->setCommand(mCommandBlock->getCommand());
    copy->mCommandBlock->setName(mCommandBlock->getName(), true);
    copy->setAutomatic(isAutomatic() != 0);
    copy->mPowered = mPowered;
    copy->mConditionMet = mConditionMet;
    BlockEntity::clone(result);
    return result;
}

void CommandBlockEntity::updateNamedPosition(const std::wstring& name, const std::wstring& value)
{
    const std::wstring token = L"<" + name + L">";
    std::wstring command = mCommandBlock->getCommand();
    std::wstring replacement = L"<" + value + L">";
    std::wstring::size_type position = command.find(token);
    bool changed = false;
    while (position != std::wstring::npos)
    {
        command.replace(position, token.size(), replacement);
        position = command.find(token, position + replacement.size());
        changed = true;
    }
    mCommandBlock->setCommand(command);
    if (changed)
    {
    static_cast<AnonymousCommandBlock*>(mCommandBlock)->onUpdated();
    }
}

unsigned long long CommandBlockEntity::GetType()
{
    return 0x0100020900000000ULL;
}
