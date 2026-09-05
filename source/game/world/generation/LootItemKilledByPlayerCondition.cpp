#include "game/world/generation/LootItemKilledByPlayerCondition.hpp"

#include "game/core/LootTableContext.hpp"
#include "game/util/Random.hpp"

LootItemKilledByPlayerCondition::LootItemKilledByPlayerCondition(bool playerKilled)
    : playerKilled(playerKilled)
{
}

bool LootItemKilledByPlayerCondition::applies(Random* random, LootTableContext& context)
{
    boost::shared_ptr<Player> player = context.mKillerPlayer.lock();
    return (player != 0) != playerKilled;
}
