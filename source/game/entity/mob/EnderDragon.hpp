#ifndef GAME_ENTITY_MOB_ENDERDRAGON_HPP
#define GAME_ENTITY_MOB_ENDERDRAGON_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MultiEntityMob.hpp"
#include "game/core/BinaryHeap.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class AABB;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class EndDragonFight;
class EnderCrystal;
class EntityTooltipDataHolder;
class Level;
struct Node;
class Path;
class MultiEntityMobPart;
class SoundEvent;
class Vec3;

class EnderDragon : public Mob, public MultiEntityMob
{
public:
    enum EEnderdragonAction
    {
        ACTION_NONE = 0,
        ACTION_1 = 1,
        ACTION_2 = 2,
        ACTION_3 = 3,
        ACTION_4 = 4,
        ACTION_5 = 5,
        ACTION_6 = 6,
        ACTION_7 = 7
    };

    EnderDragon(Level* level);
    virtual ~EnderDragon();
    virtual std::wstring getDisplayName();
    virtual float getMaxHealth();
    virtual float getHealth();
    virtual unsigned int getDimension();
    virtual Level* getLevel();
    unsigned int getSynchedAction();
    int setSynchedAction(EEnderdragonAction action, bool force);
    unsigned int findClosestNode(double x, double y, double z);
    unsigned int findClosestNode();
    double getHeadYRotDiff(float partialTicks);
    Vec3* getHeadLookVector(float partialTicks);
    Path* reconstruct_path(Node* start, Node* end);
    Path* findPath(int start, int end, Node* target);
    void navigateToNextPathNode();
    void strafeAttackTarget();
    unsigned int findNewTarget();
    bool hurt(boost::shared_ptr<MultiEntityMobPart> part, const DamageSource* source, float amount);
    void GetInteractTooltip(boost::shared_ptr<MultiEntityMobPart> part, const EntityTooltipDataHolder& holder);
    void AddParts();
    void onCrystalDestroyed(boost::shared_ptr<EnderCrystal> crystal, const BlockPos& pos, const DamageSource* source);
    void _init();
    static void addDataWalkers(DataFixerUpper* fixer);
    static void staticCtor();
    virtual void registerAttributes();
    virtual void defineSynchedData();
    bool canRide(boost::shared_ptr<Entity> entity);
    void getLatencyPos(arrayWithLength<double> positions, int index, float partialTicks);
    double getTilt(float partialTicks);
    double getHeadYOffset(float partialTicks);
    double rotWrap(double angle);
    bool checkCrystals();
    void knockBack(std::vector<boost::shared_ptr<Entity> >* entities);
    void hurt(std::vector<boost::shared_ptr<Entity> >* entities);
    bool checkWalls(AABB* box);
    virtual void aiStep();
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool hurt(const DamageSource* source, float amount);
    virtual void reallyHurt(const DamageSource* source, float amount);
    virtual void kill();
    virtual void dropExperience(int experience);
    virtual void tickDeath();
    virtual void checkDespawn();
    virtual std::vector<boost::shared_ptr<Entity> >* getSubEntities();
    virtual bool isPickable();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual float getSoundVolume();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    double getHeadPartYOffset(int part, arrayWithLength<double> first, arrayWithLength<double> second);
    double getHeadPartYRotDiff(int part, arrayWithLength<double> first, arrayWithLength<double> second);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual unsigned long long GetType();
    static boost::shared_ptr<EnderDragon> create(Level* level);
    bool checkAttack();

    float mFlapTime;
    float mOldFlapTime;
    int mDeathTime;
    boost::shared_ptr<EnderCrystal> mBeamTarget;

private:
    double mPosHistory[64][3];
    std::vector<boost::shared_ptr<Entity> > mParts;
    BinaryHeap* mOpenSet;
    Path* mPath;
    EndDragonFight* mDragonFight;
    int mDragonPhase;
    int mTicksSinceDeath;
    boost::shared_ptr<Entity> mTarget;
    int mNearbyCrystal;
    bool mDragonKilled;
    bool mPreviouslyKilled;
    bool mHasTarget;
    bool mWasHurt;
    int mHealth;
    double mTargetX;
    double mTargetY;
    double mTargetZ;
    boost::shared_ptr<MultiEntityMobPart> mHead;
    boost::shared_ptr<MultiEntityMobPart> mNeck;
    boost::shared_ptr<MultiEntityMobPart> mBody;
    boost::shared_ptr<MultiEntityMobPart> mTail1;
    boost::shared_ptr<MultiEntityMobPart> mTail2;
    boost::shared_ptr<MultiEntityMobPart> mTail3;
    boost::shared_ptr<MultiEntityMobPart> mWing1;
    boost::shared_ptr<MultiEntityMobPart> mWing2;
    int mSittingDamageReceived;
    bool mHurtWhilePerched;
};

#endif
