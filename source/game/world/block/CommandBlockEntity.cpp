#include "game/world/block/CommandBlockEntity.hpp"

#include "game/core/DataOutputStream.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CommandBlock.hpp"

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
    const BlockPos& position = mEntity->mPos;
    const BlockState* state = mEntity->mLevel->getBlockState(position);
    mEntity->getLevel()->sendBlockUpdated(position, state, state, 3, true);
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
    : BlockEntity(),
      mPowered(0),
      mAutomatic(false),
      mConditionMet(0),
      mCommandBlock(new AnonymousCommandBlock(this))
{
}

CommandBlockEntity::CommandBlockEntity(CommandBlock::Mode mode)
    : BlockEntity(),
      mPowered(0),
      mAutomatic(mode == CommandBlock::CHAIN),
      mConditionMet(0),
      mCommandBlock(new AnonymousCommandBlock(this))
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

CompoundTag* CommandBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mCommandBlock->save(tag);
    tag->putBoolean(L"powered", isPowered() != 0);
    tag->putBoolean(L"conditionMet", wasConditionMet() != 0);
    tag->putBoolean(L"auto", isAutomatic() != 0);
    return tag;
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
    CompoundTag* tag = save(new CompoundTag());
    int dimension = mLevel->mDimension->getType()->getId();
    return boost::shared_ptr<Packet>(new ClientboundBlockEntityDataPacket(mPos, 2, dimension, tag));
}

bool CommandBlockEntity::onlyOpCanSetNbt()
{
    return true;
}

unsigned int CommandBlockEntity::getMode()
{
    Block* block = getBlock();
    if (block != Blocks::command_block)
    {
        if (block == Blocks::repeating_command_block)
        {
            return 1;
        }
        if (block == Blocks::chain_command_block)
        {
            return 0;
        }
    }
    return 2;
}

unsigned char CommandBlockEntity::isConditional()
{
    const BlockState* state = mLevel->getBlockState(getBlockPos());
    if (!CommandBlock::isCommandBlock(state->getBlock()))
    {
        return 0;
    }
    return state->getValue(CommandBlock::CONDITIONAL);
}

void CommandBlockEntity::clearRemoved()
{
    mBlock = NULL;
    BlockEntity::clearRemoved();
}

boost::shared_ptr<BlockEntity> CommandBlockEntity::clone()
{
    boost::shared_ptr<BlockEntity> result(new CommandBlockEntity());
    CommandBlockEntity* copy = static_cast<CommandBlockEntity*>(result.get());
    copy->mCommandBlock->setCommand(mCommandBlock->getCommand());
    copy->mCommandBlock->setName(mCommandBlock->getName(), true);
    copy->setAutomatic(isAutomatic() != 0);
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
    return 0x0000000001000209ULL;
}
