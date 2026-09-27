#include "game/item/FrostWalkerEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/item/EnchantmentCategory.hpp"
#include "game/item/Enchantments.hpp"
#include "game/core/Direction.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/util/Random.hpp"

FrostWalkerEnchantment::FrostWalkerEnchantment(const Rarity* rarity,
                                               arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::ARMOR_FEET, slots)
{
    setDescriptionId((int)IDS_ENCHANTMENT_FROST_WALKER);
}

int FrostWalkerEnchantment::getMinCost(int level)
{
    return level * 10;
}

int FrostWalkerEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 15;
}

bool FrostWalkerEnchantment::isTreasureOnly()
{
    return true;
}

int FrostWalkerEnchantment::getMaxLevel()
{
    return 2;
}

void FrostWalkerEnchantment::onEntityMoved(const boost::shared_ptr<LivingEntity>& entity,
                                           Level* level, const BlockPos& pos, int levelValue)
{
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
    if (!entity->mOnGround || (player && player->mAbilities.mFlying))
    {
        return;
    }

    const int radius = levelValue + 2 < 16 ? levelValue + 2 : 16;
    const double distance = (double)radius;
    const BlockPos from = pos.offset(-distance, -1.0, -distance);
    const BlockPos to = pos.offset(distance, -1.0, distance);
    Iterator<MutableBlockPos*>* positions = BlockPos::mutablesBetween(from, to);
    while (positions->hasNext())
    {
        MutableBlockPos* current = positions->next();
        if (current->distSqrToCenter(entity->mX, entity->mY, entity->mZ) <= distance * distance
            && level->isWaterAt(*current))
        {
            const BlockPos above = current->above();
            if (level->getBlockState(above)->getBlock() == Blocks::air
                && level->mayPlace(Blocks::frosted_ice, *current, false, Direction::DOWN,
                                   boost::shared_ptr<Entity>()))
            {
                level->setBlock(*current, Blocks::frosted_ice->defaultBlockState(), Block::UPDATE_ALL, false);
            }
        }
    }

    delete positions;
}

bool FrostWalkerEnchantment::checkCompatibility(const Enchantment* other) const
{
    return Enchantment::checkCompatibility(other) && other != Enchantments::DEPTH_STRIDER;
}

FrostWalkerEnchantment::~FrostWalkerEnchantment()
{
}
