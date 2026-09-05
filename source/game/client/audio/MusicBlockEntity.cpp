#include "game/client/audio/MusicBlockEntity.hpp"

#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"

MusicBlockEntity::MusicBlockEntity()
    : BlockEntity(), mNote(0), mPowered(false)
{
}

MusicBlockEntity::~MusicBlockEntity()
{
}

BlockEntity* MusicBlockEntity::create()
{
    return new MusicBlockEntity();
}

void MusicBlockEntity::playNote(Level* level, BlockPos pos)
{
    if (level->getBlockState(pos.above())->getMaterial() != Material::AIR)
    {
        return;
    }

    const BlockState* state = level->getBlockState(pos.below());
    Block* block = state->getBlock();
    int instrument = 0;
    Material* material = state->getMaterial();

    if (material == Material::STONE)
    {
        instrument = 1;
    }
    else if (material == Material::SAND)
    {
        instrument = 2;
    }
    else if (material == Material::GLASS)
    {
        instrument = 3;
    }
    else if (material == Material::WOOD)
    {
        instrument = 4;
    }
    else if (block == Blocks::iron_block)
    {
        instrument = 5;
    }
    else if (block == Blocks::wool)
    {
        instrument = 6;
    }
    else if (block == Blocks::gold_block)
    {
        instrument = 7;
    }
    else if (block == Blocks::clay)
    {
        instrument = 8;
    }
    else if (block == Blocks::packed_ice)
    {
        instrument = 9;
    }

    level->blockEvent(pos, Blocks::noteblock, instrument, mNote);
}

void MusicBlockEntity::tune()
{
    mNote = (unsigned char)((mNote + 1) % 25);
}

void MusicBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putByte(L"note", mNote);
    tag->putBoolean(L"powered", mPowered);
}

void MusicBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mNote = (unsigned char)Mth::clamp((int)tag->getByte(L"note"), 0, 24);
    mPowered = tag->getBoolean(L"powered");
}

boost::shared_ptr<BlockEntity> MusicBlockEntity::clone()
{
    boost::shared_ptr<BlockEntity> result(new MusicBlockEntity());
    BlockEntity::clone(result);
    MusicBlockEntity* copy = static_cast<MusicBlockEntity*>(result.get());
    copy->mNote = mNote;
    copy->mPowered = mPowered;
    return result;
}

unsigned long long MusicBlockEntity::GetType()
{
    return 0x0100020300000000ULL;
}
