#ifndef GAME_WORLD_BLOCK_CONDUITBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_CONDUITBLOCKENTITY_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"

class CompoundTag;
class Packet;

class ConduitBlockEntity : public BlockEntity
{
public:
    ConduitBlockEntity();
    virtual ~ConduitBlockEntity();

    static BlockEntity* create();
    virtual boost::shared_ptr<BlockEntity> clone();
    void _animateTick();
    void setEffectRange(int range);
    void _checkShape();
    void _applyEffects();
    void _updateTarget();
    virtual void tick();
    void loadFromPacket(Level* level, const BlockPos& position, CompoundTag* tag);
    virtual void load(CompoundTag* tag);
    virtual void save(CompoundTag* tag);
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual CompoundTag* getUpdateTag();
    void incrementAnimationValue(float amount);
    void incrementWindLevel();
    virtual unsigned long long GetType();

private:
    bool mActive;
    bool mLarge;
    int mCheckTime;
    int mNextTargetCheck;
    int mTargetId;
    int mTickCount;
    float mAnimation;
    float mRotation;
    int mEffectRange;
    int mWindLevel;
    int mTargetCooldown;
    std::vector<BlockPos> mBlocks;
};

#endif
