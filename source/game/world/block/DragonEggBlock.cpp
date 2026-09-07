#include "game/world/block/DragonEggBlock.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/FallingBlock.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/biome/arrayWithLength.hpp"

DragonEggBlock::DragonEggBlock() : Block(Material::EGG, MaterialColor::COLOR_BLACK)
{
    DerivedInit();
}

DragonEggBlock::~DragonEggBlock()
{
}

const AABB* DragonEggBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

void DragonEggBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    level->addToTickNextTick(pos, this, getTickDelay(level));
}

void DragonEggBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    level->addToTickNextTick(pos, this, getTickDelay(level));
}

void DragonEggBlock::checkSlide(Level* level, const BlockPos& pos)
{
    if (!FallingBlock::isFree(level->getBlockState(pos.below())) || pos.y < 0)
        return;
    if (!level->mIsClientSide && level->hasChunksAt(pos.offset(-32, -32, -32), pos.offset(32, 32, 32)))
    {
        if (!level->newFallingBlockAllowed())
        {
            level->addToTickNextTick(pos, this, getTickDelay(level), 0);
            return;
        }
        level->addEntity(boost::shared_ptr<Entity>(new FallingBlockEntity(level,
            (double)pos.x + 0.5, (double)pos.y, (double)pos.z + 0.5, defaultBlockState())));
        return;
    }
    level->removeBlock(pos);
    BlockPos target = pos.below();
    while (FallingBlock::isFree(level->getBlockState(target)))
    {
        if (target.y < 1)
            return;
        target = target.below();
    }
    if (target.y > 0)
        level->setBlockAndUpdate(target.above(), defaultBlockState());
}

void DragonEggBlock::tick(Level* level, const BlockPos& pos, const BlockState*, Random*)
{
    checkSlide(level, pos);
}

void DragonEggBlock::teleport(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != this)
        return;
    for (int attempt = 0; attempt < 500; ++attempt)
    {
        BlockPos target = pos.offset(level->mRandom->nextInt(16) - level->mRandom->nextInt(16),
                                     level->mRandom->nextInt(8) - level->mRandom->nextInt(8),
                                     level->mRandom->nextInt(16) - level->mRandom->nextInt(16));
        const BlockState* targetState = level->getBlockState(target);
        if (targetState->getMaterial() == Material::AIR)
        {
            if (level->mIsClientSide)
                return;
            level->setBlock(target, state, 2, false);
            level->removeBlock(pos);
            level->levelEvent(0x232a, target,
                              ((pos.z - target.z) & 0xff) << 16 |
                              ((pos.y - target.y) & 0xff) << 8 |
                              (pos.x - target.x) & 0xff);
            return;
        }
    }
}

bool DragonEggBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player>,
                          InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
        return false;
    teleport(level, pos);
    return true;
}

void DragonEggBlock::attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player>)
{
    teleport(level, pos);
}

int DragonEggBlock::getTickDelay(Level*)
{
    return 5;
}

bool DragonEggBlock::isSolidRender(const BlockState*) const { return false; }
bool DragonEggBlock::isCubeShaped(const BlockState*) { return false; }
bool DragonEggBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction*) { return true; }
EBlockFaceShape DragonEggBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*) { return BLOCK_FACE_UNDEFINED; }
EBlockRenderShape DragonEggBlock::getRenderShape(const BlockState*) { return (EBlockRenderShape)0x1b; }

void DragonEggBlock::generateTeleportParticles(Level* level, int x, int y, int z, int data)
{
    const signed char dx = (signed char)(data & 0xff);
    const signed char dy = (signed char)((data >> 8) & 0xff);
    const signed char dz = (signed char)((data >> 16) & 0xff);
    for (int i = 0; i < 64; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            double px = (double)x + 0.5 + (double)dx * level->mRandom->nextDouble() + level->mRandom->nextDouble() - 0.5;
            double py = (double)y - 0.5 + (double)dy * level->mRandom->nextDouble() + level->mRandom->nextDouble();
            double pz = (double)z + 0.5 + (double)dz * level->mRandom->nextDouble() + level->mRandom->nextDouble() - 0.5;
            arrayWithLength<int> params;
            level->addParticle(PARTICLE_PORTAL, px, py, pz,
                               (double)(level->mRandom->nextFloat() - 0.5f) * 0.2,
                               (double)(level->mRandom->nextFloat() - 0.5f) * 0.2,
                               (double)(level->mRandom->nextFloat() - 0.5f) * 0.2, params);
        }
    }
}
