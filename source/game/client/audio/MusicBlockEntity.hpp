#ifndef GAME_CLIENT_AUDIO_MUSICBLOCKENTITY_HPP
#define GAME_CLIENT_AUDIO_MUSICBLOCKENTITY_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockEntity.hpp"

class BlockPos;
class CompoundTag;
class Level;

class MusicBlockEntity : public BlockEntity
{
public:
    MusicBlockEntity();
    virtual ~MusicBlockEntity();

    static BlockEntity* create();
    void playNote(Level* level, BlockPos pos);
    void tune();
    virtual CompoundTag* save(CompoundTag* tag);
    virtual void load(CompoundTag* tag);
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual unsigned long long GetType();

private:
    unsigned char mNote;
    bool mPowered;
};

#endif
