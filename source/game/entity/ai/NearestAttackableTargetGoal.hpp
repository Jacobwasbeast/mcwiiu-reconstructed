#ifndef GAME_ENTITY_AI_NEARESTATTACKABLETARGETGOAL_HPP
#define GAME_ENTITY_AI_NEARESTATTACKABLETARGETGOAL_HPP

#include <typeinfo>
#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/ai/TargetGoal.hpp"
#include "game/world/Level.hpp"

class AABB;
class Entity;
class LivingEntity;
class PathfinderMob;
class Player;
class ServerPlayer;
class SubselectEntitySelector;
template <class T> class Predicate;

class NearestAttackableTargetGoal : public TargetGoal
{
public:
    class DisguisedRangeModifier : public Level::RangeModifier
    {
    public:
        DisguisedRangeModifier(PathfinderMob* mob);
        virtual ~DisguisedRangeModifier();
        virtual double apply(boost::shared_ptr<Player> player) const;
    private:
        PathfinderMob* mMob;
    };

    class DistComp
    {
    public:
        DistComp(Entity* entity);
        bool operator()(boost::shared_ptr<Entity> first,
                        boost::shared_ptr<Entity> second);
    private:
        Entity* mEntity;
    };

    NearestAttackableTargetGoal(PathfinderMob* mob, const std::type_info& targetType,
                                bool mustSee, bool mustReach, bool includeRemoved);
    NearestAttackableTargetGoal(PathfinderMob* mob, const std::type_info& targetType,
                                int randomInterval, bool mustSee, bool mustReach,
                                const Predicate<boost::shared_ptr<Entity> >* selector,
                                bool includeRemoved, bool ownsSelector);
    virtual ~NearestAttackableTargetGoal();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual std::wstring getName();
    virtual AABB* getTargetSearchArea(double distance);

private:
    const std::type_info* mTargetType;
    int mRandomInterval;
    DistComp mSorter;
    SubselectEntitySelector* mTargetSelector;
    boost::shared_ptr<LivingEntity> mTarget;
    bool mIncludeRemoved;
    DisguisedRangeModifier* mRangeModifier;
};

#endif
