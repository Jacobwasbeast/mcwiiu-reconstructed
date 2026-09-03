#include "game/world/block/ComparatorBlockEntity.hpp"

#include "game/data/CompoundTag.hpp"

ComparatorBlockEntity::ComparatorBlockEntity()
    : BlockEntity(), mOutputSignal(0)
{
}

ComparatorBlockEntity::~ComparatorBlockEntity()
{
}

BlockEntity* ComparatorBlockEntity::create()
{
    return new ComparatorBlockEntity();
}

unsigned int ComparatorBlockEntity::getOutputSignal()
{
    return (unsigned int)mOutputSignal;
}

void ComparatorBlockEntity::setOutputSignal(int signal)
{
    mOutputSignal = signal;
}

boost::shared_ptr<BlockEntity> ComparatorBlockEntity::clone()
{
    boost::shared_ptr<BlockEntity> result(new ComparatorBlockEntity());
    BlockEntity::clone(result);
    static_cast<ComparatorBlockEntity*>(result.get())->mOutputSignal = mOutputSignal;
    return result;
}

void ComparatorBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putInt(L"OutputSignal", mOutputSignal);
}

void ComparatorBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mOutputSignal = tag->getInt(L"OutputSignal");
}

unsigned long long ComparatorBlockEntity::GetType()
{
    return 0x0100020A00000000ULL;
}
