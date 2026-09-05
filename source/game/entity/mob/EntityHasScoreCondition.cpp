#include "game/entity/mob/EntityHasScoreCondition.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Scoreboard.hpp"
#include "game/entity/mob/Entity.hpp"

bool EntityHasScoreCondition::hasScore(const boost::shared_ptr<Entity>& entity,
                                       Scoreboard* scoreboard,
                                       const std::wstring& scoreName,
                                       RandomValueBounds bounds)
{
    return true;
}

bool EntityHasScoreCondition::applies(Random* random, LootTableContext& context)
{
    boost::shared_ptr<Entity> entity = context.getEntity(entityTarget);
    if (!entity)
    {
        return false;
    }

    Scoreboard* scoreboard = NULL;
    boost::unordered_map<std::wstring, RandomValueBounds>::const_iterator it = scores.begin();
    while (it != scores.end())
    {
        if (!hasScore(entity, scoreboard, it->first, it->second))
        {
            return false;
        }
        ++it;
    }

    return true;
}
