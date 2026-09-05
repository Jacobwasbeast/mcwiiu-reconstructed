#include "game/entity/mob/LeashFenceKnotEntity.hpp"

#include <cmath>
#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/Boat.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/LeadItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/FenceBlock.hpp"

LeashFenceKnotEntity::LeashFenceKnotEntity(Level* level)
    : HangingEntity(level)
{
    _init();
}

LeashFenceKnotEntity::LeashFenceKnotEntity(Level* level, const BlockPos& pos)
    : HangingEntity(level, pos)
{
    _init();
    setPos((double)pos.x + 0.5, (double)pos.y + 0.5, (double)pos.z + 0.5);
    setBoundingBox(AABB::newPermanent(mX - 0.1875, mY - 0.25 + 0.125,
                                      mZ - 0.1875, mX + 0.1875,
                                      mY + 0.25 + 0.125, mZ + 0.1875));
}

LeashFenceKnotEntity::~LeashFenceKnotEntity()
{
}

void LeashFenceKnotEntity::create(Level* level)
{
    new LeashFenceKnotEntity(level);
}

void LeashFenceKnotEntity::_init()
{
    defineSynchedData();
    fjDerivedCtorCalls();
}

void LeashFenceKnotEntity::setPos(double x, double y, double z)
{
    HangingEntity::setPos(std::floor(x) + 0.5, std::floor(y) + 0.5, std::floor(z) + 0.5);
}

void LeashFenceKnotEntity::defineSynchedData()
{
    HangingEntity::defineSynchedData();
}

void LeashFenceKnotEntity::recalculateBoundingBox()
{
    mX = (double)mPos.x + 0.5;
    mY = (double)mPos.y + 0.5;
    mZ = (double)mPos.z + 0.5;
}

void LeashFenceKnotEntity::setDirection(const Direction* direction)
{
}

int LeashFenceKnotEntity::getWidth()
{
    return 9;
}

int LeashFenceKnotEntity::getHeight()
{
    return 9;
}

double LeashFenceKnotEntity::getEyeHeight()
{
    return -0.0625;
}

bool LeashFenceKnotEntity::shouldRenderAtSqrDistance(double distanceSqr)
{
    return distanceSqr < 1024.0;
}

void LeashFenceKnotEntity::dropItem(boost::shared_ptr<Entity> breaker)
{
    playSound(SoundEvents::LEASHKNOT_BREAK, 1.0f, 1.0f);
}

void LeashFenceKnotEntity::playPlacementSound()
{
    playSound(SoundEvents::LEASHKNOT_PLACE, 1.0f, 1.0f);
}

bool LeashFenceKnotEntity::save(CompoundTag* tag)
{
    return false;
}

void LeashFenceKnotEntity::addAdditonalSaveData(CompoundTag* tag)
{
}

void LeashFenceKnotEntity::readAdditionalSaveData(CompoundTag* tag)
{
}

bool LeashFenceKnotEntity::interact(const boost::shared_ptr<Player>& player,
                                    InteractionHand::EInteractionHand hand)
{
    if (mLevel->mIsClientSide)
        return true;

    AABB* bounds = AABB::newTemp(mX - 7.0, mY - 7.0, mZ - 7.0,
                                 mX + 7.0, mY + 7.0, mZ + 7.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntitiesOfClass(typeid(Entity), bounds);
    std::vector<boost::shared_ptr<Entity> >* boats =
        mLevel->getEntitiesOfClass(typeid(Boat), bounds);
    entities->insert(entities->end(), boats->begin(), boats->end());

    bool changed = false;
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
         it != entities->end(); ++it)
    {
        if ((*it)->isLeashedToPlayer(player))
        {
            (*it)->setLeashedTo(shared_from_this(), true);
            changed = true;
        }
    }

    if (!changed && player->isAllowedToMine())
    {
        remove();
        if (player->mIsCreativeBuilding)
        {
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
                 it != entities->end(); ++it)
            {
                if ((*it)->isLeashed() && (*it)->getLeashHolder() == shared_from_this())
                    (*it)->dropLeash(true, false);
            }
        }
    }
    delete entities;
    delete boats;
    return true;
}

bool LeashFenceKnotEntity::survives()
{
    return dynamic_cast<FenceBlock*>(mLevel->getBlockState(mPos)->getBlock()) != NULL;
}

boost::shared_ptr<LeashFenceKnotEntity> LeashFenceKnotEntity::createAndAddKnot(Level* level,
                                                                                const BlockPos& pos)
{
    boost::shared_ptr<LeashFenceKnotEntity> knot(new LeashFenceKnotEntity(level, pos));
    level->addEntity(knot);
    knot->playPlacementSound();
    return knot;
}

boost::shared_ptr<LeashFenceKnotEntity> LeashFenceKnotEntity::findKnotAt(Level* level,
                                                                          const BlockPos& pos)
{
    AABB* bounds = AABB::newTemp((double)pos.x - 1.0, (double)pos.y - 1.0,
                                 (double)pos.z - 1.0, (double)pos.x + 1.0,
                                 (double)pos.y + 1.0, (double)pos.z + 1.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        level->getEntitiesOfClass(typeid(LeashFenceKnotEntity), bounds);
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
         it != entities->end(); ++it)
    {
        boost::shared_ptr<LeashFenceKnotEntity> knot =
            boost::dynamic_pointer_cast<LeashFenceKnotEntity>(*it);
        if (knot && knot->getPos().equals(pos))
        {
            delete entities;
            return knot;
        }
    }
    delete entities;
    return boost::shared_ptr<LeashFenceKnotEntity>();
}

unsigned int LeashFenceKnotEntity::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    bool bound = false;
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == 0x1a4)
    {
        boost::shared_ptr<Player> player = (boost::shared_ptr<Player>&)tooltipData.m_pPlayer;
        bound = LeadItem::bindPlayerMobsTest(player, mLevel, BlockPos(shared_from_this()));
    }
    return bound ? 0x7b938f36u : 0x551269b5u;
}

unsigned int LeashFenceKnotEntity::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return 0x551269b5u;
}

unsigned long long LeashFenceKnotEntity::GetType()
{
    return 0x0A08000300000000ULL;
}
