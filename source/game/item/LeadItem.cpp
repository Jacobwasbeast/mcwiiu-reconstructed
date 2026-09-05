#include "game/item/LeadItem.hpp"

#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/Boat.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

class LeashFenceKnotEntity : public Entity
{
public:
    static boost::shared_ptr<LeashFenceKnotEntity> findKnotAt(Level* level, const BlockPos& pos);
    static boost::shared_ptr<LeashFenceKnotEntity> createAndAddKnot(Level* level, const BlockPos& pos);
};

LeadItem::LeadItem()
    : Item()
{
}

LeadItem::~LeadItem()
{
}

bool LeadItem::bindPlayerMobsTest(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos)
{
    AABB* bounds = AABB::newTemp(static_cast<double>(pos.x) - 7.0,
                                  static_cast<double>(pos.y) - 7.0,
                                  static_cast<double>(pos.z) - 7.0,
                                  static_cast<double>(pos.x) + 7.0,
                                  static_cast<double>(pos.y) + 7.0,
                                  static_cast<double>(pos.z) + 7.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        level->getEntitiesOfClass(typeid(Entity), bounds);
    std::vector<boost::shared_ptr<Entity> >* boats =
        level->getEntitiesOfClass(typeid(Boat), bounds);
    entities->insert(entities->end(), boats->begin(), boats->end());

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
         it != entities->end(); ++it)
    {
        if ((*it)->canBeLeashed(player))
        {
            delete entities;
            delete boats;
            return true;
        }
    }

    delete entities;
    delete boats;
    return false;
}

bool LeadItem::bindPlayerMobs(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos)
{
    boost::shared_ptr<LeashFenceKnotEntity> knot =
        LeashFenceKnotEntity::findKnotAt(level, pos);
    AABB* bounds = AABB::newTemp(static_cast<double>(pos.x) - 7.0,
                                  static_cast<double>(pos.y) - 7.0,
                                  static_cast<double>(pos.z) - 7.0,
                                  static_cast<double>(pos.x) + 7.0,
                                  static_cast<double>(pos.y) + 7.0,
                                  static_cast<double>(pos.z) + 7.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        level->getEntitiesOfClass(typeid(Entity), bounds);
    std::vector<boost::shared_ptr<Entity> >* boats =
        level->getEntitiesOfClass(typeid(Boat), bounds);
    entities->insert(entities->end(), boats->begin(), boats->end());

    bool bound = false;
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
         it != entities->end(); ++it)
    {
        if ((*it)->canBeLeashed(player))
        {
            if (!knot)
            {
                knot = LeashFenceKnotEntity::createAndAddKnot(level, pos);
            }
            (*it)->setLeashedTo(knot, true);
            bound = true;
        }
    }

    delete entities;
    delete boats;
    return bound;
}

InteractionResult::EInteractionResult LeadItem::useOn(boost::shared_ptr<Player> player,
                                                        Level* level,
                                                        const BlockPos& pos,
                                                        InteractionHand::EInteractionHand hand,
                                                        const Direction* direction,
                                                        float hitX,
                                                        float hitY,
                                                        float hitZ,
                                                        bool simulate)
{
    if (!level->getBlockState(pos)->getBlock()->instanceof(EBLOCK_TYPE_FENCE))
    {
        return InteractionResult::eInteractionResult_Pass;
    }

    if (simulate)
    {
        bindPlayerMobsTest(player, level, pos);
    }

    if (!level->mIsClientSide)
    {
        bindPlayerMobs(player, level, pos);
    }

    return InteractionResult::eInteractionResult_Success;
}
