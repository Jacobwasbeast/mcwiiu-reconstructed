#include "game/world/block/NoteBlock.hpp"

#include <cmath>
#include <vector>

#include "game/client/audio/MusicBlockEntity.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

static std::vector<const SoundEvent*> noteBlockInstruments;

NoteBlock::NoteBlock()
    : BaseEntityBlock(NULL), EntityBlock()
{
    DerivedInit();
}

NoteBlock::~NoteBlock()
{
}

boost::shared_ptr<BlockEntity> NoteBlock::newBlockEntity(Level*, int)
{
    return boost::shared_ptr<BlockEntity>(new MusicBlockEntity());
}

void NoteBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    bool powered = level->hasNeighborSignal(pos);
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    if (entity && entity->GetType() == 0x1000203ULL)
    {
        boost::shared_ptr<MusicBlockEntity> music = boost::dynamic_pointer_cast<MusicBlockEntity>(entity);
        if (powered)
        {
            music->playNote(level, BlockPos(pos));
        }
    }
}

bool NoteBlock::TestUse()
{
    return true;
}

bool NoteBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player>,
                    InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
    {
        return false;
    }
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    if (entity && entity->GetType() == 0x1000203ULL)
    {
        boost::shared_ptr<MusicBlockEntity> music = boost::dynamic_pointer_cast<MusicBlockEntity>(entity);
        music->tune();
        music->playNote(level, BlockPos(pos));
    }
    return true;
}

void NoteBlock::attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player>)
{
    if (!level->mIsClientSide)
    {
        boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
        if (entity && entity->GetType() == 0x1000203ULL)
        {
            boost::shared_ptr<MusicBlockEntity> music = boost::dynamic_pointer_cast<MusicBlockEntity>(entity);
            music->playNote(level, BlockPos(pos));
        }
    }
}

unsigned int NoteBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseItem || tooltipData.m_bCanPlace) && tooltipData.m_bCanDestroy)
    {
        return 0x1ceafe25U;
    }
    return Block::GetInteractTooltip(tooltipData);
}

unsigned int NoteBlock::GetHitTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bIsInside && !Minecraft::InMiniGame((EMiniGameId)0, true))
    {
        return 0xffffffffU;
    }
    if (tooltipData.m_bIsInside)
    {
        return 0x5fb7a25aU;
    }
    return 0x293a30d9U;
}

const SoundEvent* NoteBlock::getInstrument(int instrument)
{
    if (instrument < 0 || noteBlockInstruments.empty() || instrument >= (int)noteBlockInstruments.size())
    {
        instrument = 0;
    }
    return noteBlockInstruments.at(instrument);
}

bool NoteBlock::triggerEvent(const BlockState*, Level* level, const BlockPos& pos, int instrument, int eventParam)
{
    double pitch = std::pow(2.0, ((double)eventParam - 12.0) / 12.0);
    level->playSoundToAllPlayers(pos, getInstrument(instrument), SoundSource::eSoundSource_Blocks,
                                 3.0f, (float)pitch, 64.0f);
    arrayWithLength<int> params;
    level->addParticle(PARTICLE_NOTE, (double)pos.x + 0.5, (double)pos.y + 1.2,
                       (double)pos.z + 0.5, (double)eventParam / 24.0, 0.0, 0.0, params);
    return true;
}

EBlockRenderShape NoteBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_MODEL;
}

void NoteBlock::blockStaticCtor()
{
    noteBlockInstruments.clear();
    noteBlockInstruments.push_back(SoundEvents::NOTE_HARP);
    noteBlockInstruments.push_back(SoundEvents::NOTE_BASS);
    noteBlockInstruments.push_back(SoundEvents::NOTE_BASEDRUM);
    noteBlockInstruments.push_back(SoundEvents::NOTE_SNARE);
    noteBlockInstruments.push_back(SoundEvents::NOTE_HAT);
    noteBlockInstruments.push_back(SoundEvents::NOTE_PLING);
    noteBlockInstruments.push_back(SoundEvents::NOTE_BELL);
    noteBlockInstruments.push_back(SoundEvents::NOTE_CHIME);
    noteBlockInstruments.push_back(SoundEvents::NOTE_FLUTE);
    noteBlockInstruments.push_back(SoundEvents::NOTE_GUITAR);
}
