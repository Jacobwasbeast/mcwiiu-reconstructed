#include "game/world/block/MagmaBlock.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/BubbleColumnSegment.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/IceBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

MagmaBlock::MagmaBlock()
    : Block(Material::LAVA)
{
    DerivedInit();
    setLightEmission(0.2f);
    setTicking(true);
}

MagmaBlock::~MagmaBlock()
{
}

const MaterialColor* MagmaBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&)
{
    return MaterialColor::NETHER;
}

void MagmaBlock::stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity)
{
    bool frostWalker = false;
    bool setOnFire = false;
    if (!entity->isFireImmune() && (entity->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
    {
        boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(entity);
        frostWalker = true;
        if (!EnchantmentHelper::hasFrostWalker(living))
            setOnFire = true;
    }
    if (frostWalker && setOnFire)
        entity->setSecondsOnFire(1);
    Block::stepOn(level, pos, entity);
}

int MagmaBlock::getLightColor(const BlockState*, LevelSource*, const BlockPos&, int)
{
    return 0xf000f0;
}

void MagmaBlock::addToTickQueue(Level* level, const BlockPos& pos, Random* random)
{
    const int delay = getTickDelay(level);
    level->addToTickNextTick(pos, this, delay + random->nextInt(delay));
}

void MagmaBlock::tick(Level* level, const BlockPos& pos, const BlockState*, Random* random)
{
    const BlockPos above = pos.above();
    if (level->getBlock(above.above()) == Blocks::AIR && level->isEmptyWaterBlock(above))
    {
        const BlockState* liquidState = level->getLiquidBlockState(above);
        if (liquidState != NULL && liquidState->getBlock()->convertBlockStateToLegacyData(liquidState) != 0)
            return;
        if (!level->isEmptyWaterBlock(above))
            return;
    }
    BubbleColumnSegment* bubbleColumnSegment = static_cast<BubbleColumnSegment*>(Blocks::bubble_column_segment);
    if (!bubbleColumnSegment->addBubbleColumnSegment(level, pos))
    {
        Block* block = level->getBlock(above);
        bool water = block == Blocks::WATER || block == Blocks::FLOWING_WATER;
        if (water)
        {
            if (random->nextInt(8) == 0 && block == Blocks::WATER)
                IceBlock::melt(level, above);
            addToTickQueue(level, pos, random);
        }
    }
}

void MagmaBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos,
                                 Block*, const BlockPos& fromPos)
{
    if (pos.above() == fromPos && level->isEmptyWaterBlock(pos.above()))
        level->addToTickNextTick(pos, this, level->mRandom->nextInt(2, 3));

    Block* block = level->getBlock(fromPos);
    if (block == Blocks::WATER || block == Blocks::FLOWING_WATER
        || block == Blocks::ice || block == Blocks::frosted_ice)
        addToTickQueue(level, pos, level->mRandom);
}

int MagmaBlock::getTickDelay(Level*)
{
    return 160;
}

void MagmaBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    const BlockPos above = pos.above();
    if (level->getBlock(above.above()) == Blocks::AIR && level->isEmptyWaterBlock(above))
    {
        const BlockState* liquidState = level->getLiquidBlockState(above);
        if (liquidState != NULL && liquidState->getBlock()->convertBlockStateToLegacyData(liquidState) != 0)
            return;
        if (!level->isEmptyWaterBlock(above))
            return;
    }
    BubbleColumnSegment* bubbleColumnSegment = static_cast<BubbleColumnSegment*>(Blocks::bubble_column_segment);
    bubbleColumnSegment->addBubbleColumnSegment(level, pos);
}

bool MagmaBlock::isValidSpawn(const BlockState*, boost::shared_ptr<Entity> entity)
{
    return entity->isFireImmune();
}

void MagmaBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    if (!level->isRainingAt(pos.above()))
        return;

    const double x = static_cast<double>(pos.x);
    const double y = static_cast<double>(pos.y + 1);
    const double z = static_cast<double>(pos.z);
    const double velocity = 0.014;
    level->addParticle(PARTICLE_SMOKE, x + random->nextDouble(), y, z + random->nextDouble(),
                       velocity, velocity, velocity, arrayWithLength<int>());
    level->addParticle(PARTICLE_SMOKE, x + random->nextDouble(), y, z + random->nextDouble(),
                       velocity, velocity, velocity, arrayWithLength<int>());
    level->addParticle(PARTICLE_SMOKE, x + random->nextDouble(), y, z + random->nextDouble(),
                       velocity, velocity, velocity, arrayWithLength<int>());
    level->addParticle(PARTICLE_SMOKE, x + random->nextDouble(), y, z + random->nextDouble(),
                       velocity, velocity, velocity, arrayWithLength<int>());
}
