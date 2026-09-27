#ifndef GAME_CORE_WITHERSKULL_HPP
#define GAME_CORE_WITHERSKULL_HPP

#include <vector>
#include "game/entity/projectile/Fireball.hpp"

class BlockPos;
class Explosion;
class HitResult;

class WitherSkull : public Fireball
{
public:
    explicit WitherSkull(Level* level);
    WitherSkull(Level* level, boost::shared_ptr<LivingEntity> owner, double x, double y, double z);
    WitherSkull(Level* level, double x, double y, double z, double xd, double yd, double zd);
    ~WitherSkull() override;
    static void create(Level* level);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);
    bool isOnFire() override;
    float getBlockExplosionResistance(Explosion* explosion, Level* level, const BlockPos& pos) override;
    bool isPickable() override;
    virtual bool isDangerous() const;
    virtual void setDangerous(bool dangerous);
    double getInertia() override;
    void onHit(const HitResult* result) override;
    void defineSynchedData() override;
    bool shouldBurn() override;
    std::vector<unsigned long long>* getEntityTypesToIgnoreCollisionsWith() override;
    unsigned long long GetType() override;
private:
    static const EntityDataAccessor<bool>* DATA_DANGEROUS;
};

#endif
