#include "game/world/block/RedStoneOreBlock.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"

RedStoneOreBlock::RedStoneOreBlock(bool lit)
    : Block(Material::STONE), mLit(lit)
{
    DerivedInit();
    if (lit)
    {
        setTicking(true);
    }
}

RedStoneOreBlock::~RedStoneOreBlock()
{
}

int RedStoneOreBlock::getTickDelay(Level*)
{
    return 30;
}

void RedStoneOreBlock::attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player)
{
    interact(level, pos);
    Block::attack(level, pos, player);
}

void RedStoneOreBlock::stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity)
{
    interact(level, pos);
    Block::stepOn(level, pos, entity);
}

bool RedStoneOreBlock::TestUse()
{
    return this == Blocks::redstone_ore;
}

bool RedStoneOreBlock::use(Level* level, const BlockPos& pos, const BlockState* state,
                           boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand,
                           const Direction* direction, float hitX, float hitY, float hitZ, bool clientSide)
{
    if (clientSide)
    {
        return false;
    }
    return Block::use(level, pos, state, player, hand, direction, hitX, hitY, hitZ, false);
}

void RedStoneOreBlock::spawnParticles(Level* level, const BlockPos& pos)
{
    static const int sides = 6;
    int side;
    for (side = 0; side < sides; ++side)
    {
        BlockPos adjacent;
        double x = pos.x + 0.5;
        double y = pos.y + 0.5;
        double z = pos.z + 0.5;
        if (side == 0)
        {
            adjacent = pos.above();
            y += 0.5;
        }
        else if (side == 1)
        {
            adjacent = pos.below();
            y -= 0.5;
        }
        else if (side == 2)
        {
            adjacent = pos.south();
            z += 0.5;
        }
        else if (side == 3)
        {
            adjacent = pos.north();
            z -= 0.5;
        }
        else if (side == 4)
        {
            adjacent = pos.east();
            x += 0.5;
        }
        else
        {
            adjacent = pos.west();
            x -= 0.5;
        }
        if (level->getBlockState(adjacent)->isSolidRender())
        {
            continue;
        }
        level->addParticle(PARTICLE_RED_DUST, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
}

void RedStoneOreBlock::interact(Level* level, const BlockPos& pos)
{
    spawnParticles(level, pos);
    if (!level->mIsClientSide && this == Blocks::redstone_ore)
    {
        level->setBlock(pos, Blocks::lit_redstone_ore->defaultBlockState(), 2, false);
    }
}

void RedStoneOreBlock::tick(Level* level, const BlockPos& pos, const BlockState*, Random*)
{
    if (this == Blocks::lit_redstone_ore)
    {
        level->setBlock(pos, Blocks::redstone_ore->defaultBlockState(), 2, false);
    }
}

Item* RedStoneOreBlock::getResource(const BlockState*, Random*, int)
{
    return Items::redstone;
}

int RedStoneOreBlock::getResourceCountForLootBonus(int lootBonus, Random* random)
{
    return getResourceCount(random) + random->nextInt(lootBonus + 1);
}

int RedStoneOreBlock::getResourceCount(Random* random)
{
    return random->nextInt(2) + 4;
}

void RedStoneOreBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state,
                                      float chance, int lootBonus)
{
    Block::spawnResources(level, pos, state, chance, lootBonus);
    if (getResource(state, level->mRandom, lootBonus) != Item::byBlock(this))
    {
        popExperience(level, pos, level->mRandom->nextInt(5) + 1);
    }
}

void RedStoneOreBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    if (mLit)
    {
        spawnParticles(level, pos);
    }
}

unsigned int RedStoneOreBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_pBlockState == NULL && tooltipData.m_bUseItem)
    {
        return 3931015550U;
    }
    return Block::GetInteractTooltip(tooltipData);
}

not_null_ptr<ItemInstance> RedStoneOreBlock::getSilkTouchItemInstance(const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::redstone_ore));
}

not_null_ptr<ItemInstance> RedStoneOreBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::redstone_ore)));
}

bool RedStoneOreBlock::shouldBlockTick(Level*, const BlockPos&, const BlockState*)
{
    return this == Blocks::lit_redstone_ore;
}
