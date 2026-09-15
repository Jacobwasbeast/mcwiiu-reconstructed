/* WitherBoss - generated reconstruction header. */
#ifndef RECON_C_GAME_W_WITHERBOSS_HPP
#define RECON_C_GAME_W_WITHERBOSS_HPP 1

#include "guest_ns.hpp"
#include "game/M/Monster.hpp"

namespace recon {
struct AABB;
struct Block;
struct CompoundTag;
struct DamageSource;
struct DifficultyInstance;
struct FlyingPathNavigation;
struct Level;
struct MobEffectInstance;
struct MobGroupData;
struct ServerBossEvent;
struct sp_Entity;
struct sp_LivingEntity;
struct sp_ServerPlayer;
struct wstring;

#pragma pack(push, 1)
struct WitherBoss : Monster {
    be<uint32_t> field_0x730;   /* ptr */
    be<uint32_t> field_0x734;   /* ptr */
    be<int32_t> field_0x738;
    be<float> field_0x73c;
    be<float> field_0x740;
    be<float> field_0x744;
    be<float> field_0x748;
    be<float> field_0x74c;
    be<float> field_0x750;
    be<float> field_0x754;
    be<float> field_0x758;
    gptr<DamageSource> field_0x75c;
    be<int32_t> field_0x760;
    be<int32_t> field_0x764;
    be<int32_t> field_0x768;
    be<int32_t> field_0x76c;
    be<int32_t> field_0x770;
    be<float> field_0x774;
    be<float> field_0x778;
    be<float> field_0x77c;
    be<int32_t> field_0x780;
    be<uint8_t> field_0x784;
    uint8_t pad_0x785[3];
    be<int32_t> field_0x788;
    be<int32_t> field_0x78c;
    be<int32_t> field_0x790;
    be<float> field_0x794;
    be<int32_t> field_0x798;
    be<uint8_t> field_0x79c;
    uint8_t pad_0x79d[3];
    be<int32_t> field_0x7a0;
    be<int32_t> field_0x7a4;
    be<bool> field_0x7a8;
    uint8_t pad_0x7a9[3];
    be<int32_t> field_0x7ac;
    be<int32_t> field_0x7b0;
    be<int32_t> field_0x7b4;
    be<int32_t> field_0x7b8;
    be<int32_t> field_0x7bc;
    be<uint8_t> field_0x7c0;
    uint8_t pad_0x7c1[3];
    be<int32_t> field_0x7c4;
    be<int32_t> field_0x7c8;
    be<float> field_0x7cc;
    be<int32_t> field_0x7d0;
    be<int32_t> field_0x7d4;
    be<int32_t> field_0x7d8;
    be<int32_t> field_0x7dc;
    be<int32_t> field_0x7e0;
    be<int32_t> field_0x7e4;
    be<int32_t> field_0x7e8;
    be<bool> field_0x7ec;
    uint8_t pad_0x7ed[3];
    gptr<ServerBossEvent> field_0x7f0;
    uint8_t pad_0x7f4[4];

    /* interface: 75 methods recovered, 0 without a recoverable signature, 11 ABI artifacts;
     * implementation: 39 reconstructed bodies, 21 lowered drafts, 31 closed */
    WitherBoss(gptr<Level> a1);   /* LOWERED-DRAFT; not reconstructed */
    ~WitherBoss();   /* LOWERED-DRAFT; not reconstructed */
    WitherBoss() {}   /* generated shell; the real ctor is dispatch-only */
    static void create(gptr<Level> a1);   /* LOWERED-DRAFT; not reconstructed */
    gresult<wstring> getDisplayName();   /* LOWERED-DRAFT; not reconstructed */
    uint64_t performRangedAttack(uint32_t a1);   /* LOWERED-DRAFT; not reconstructed */
    uint32_t isAggressive();   /* closed */
    void setAggressive(bool a1);   /* closed */
    static uint32_t canDestroy(gptr<Block> a1);   /* closed */
    static void addDataWalkers(uint32_t a1, uint32_t a2);
    static uint64_t staticCtor();   /* boundary stub */
    void defineSynchedData();
    void registerAttributes();   /* LOWERED-DRAFT; not reconstructed */
    void setAerialAttack(bool a1);
    void setCustomName(gptr<wstring> a1, gptr<wstring> a2, bool a3);   /* LOWERED-DRAFT; not reconstructed */
    uint8_t getAerialAttack();   /* LOWERED-DRAFT; not reconstructed */
    void addAdditonalSaveData(gptr<CompoundTag> a1);   /* boundary stub */
    uint32_t getInvulnerableTicks();   /* LOWERED-DRAFT; not reconstructed */
    void setInvulnerableTicks(int32_t a1);
    void readAdditionalSaveData(gptr<CompoundTag> a1);   /* LOWERED-DRAFT; not reconstructed */
    void readAdditionContructionTag(gptr<CompoundTag> a1);   /* LOWERED-DRAFT; not reconstructed */
    uint32_t getAdditionContructionTag();   /* boundary stub */
    uint32_t getAlternativeTarget(int32_t a1);   /* LOWERED-DRAFT; not reconstructed */
    bool isPowered();   /* LOWERED-DRAFT; not reconstructed */
    uint64_t generateParticles();   /* boundary stub */
    uint8_t IsChargeAttacking();   /* LOWERED-DRAFT; not reconstructed */
    uint32_t canShoot();   /* closed */
    uint64_t aiStep();   /* boundary stub */
    uint32_t makeInvulnerable();   /* LOWERED-DRAFT; not reconstructed */
    void makeStuckInWeb();   /* closed */
    void startSeenByPlayer(gptr<sp_ServerPlayer> a1);   /* LOWERED-DRAFT; not reconstructed */
    void stopSeenByPlayer(gptr<sp_ServerPlayer> a1);   /* LOWERED-DRAFT; not reconstructed */
    void getPlayerParty();   /* closed */
    uint32_t getArmorValue();   /* closed */
    void changePhase();   /* LOWERED-DRAFT; not reconstructed */
    uint32_t hurt(double a1, float a2);   /* boundary stub */
    uint32_t getLightColor();   /* closed */
    uint8_t isPickable();   /* closed */
    void causeFallDamage(float a1, float a2);   /* closed */
    void addEffect(gptr<MobEffectInstance> a1, gptr<sp_Entity> a2);   /* closed */
    uint64_t setAlternativeTarget(int32_t a1, int32_t a2);   /* boundary stub */
    double getHeadYRot(int32_t a1);   /* closed */
    double getHeadXRot(int32_t a1);   /* closed */
    uint32_t getMobType();   /* closed */
    void die(gptr<DamageSource> a1);   /* LOWERED-DRAFT; not reconstructed */
    void handleEntityEvent(uint8_t a1, int32_t a2);   /* boundary stub */
    double getOverlayAlpha();   /* closed */
    double getSwellAmount(float a1);   /* closed */
    uint8_t wantsToMove();   /* closed */
    void setWantsToMove(bool a1);   /* closed */
    void setIsPathing(bool a1);   /* closed */
    void setShotDelay(int32_t a1);   /* closed */
    void heal(float a1);   /* LOWERED-DRAFT; not reconstructed */
    uint32_t canFly();   /* closed */
    uint32_t getTarget();   /* LOWERED-DRAFT; not reconstructed */
    uint32_t IsSpawning();   /* closed */
    gptr<MobGroupData> finalizeMobSpawn(gptr<DifficultyInstance> a1, gptr<MobGroupData> a2, gptr<wstring> a3);   /* LOWERED-DRAFT; not reconstructed */
    gptr<FlyingPathNavigation> createNavigation(gptr<Level> a1);   /* LOWERED-DRAFT; not reconstructed */
    void registerGoals();   /* boundary stub */
    uint32_t getAmbientSound();   /* closed */
    uint32_t getHurtSound(gptr<DamageSource> a1);   /* closed */
    uint32_t getDeathSound();   /* closed */
    uint64_t destroyBlocks(gptr<Level> a1, gptr<AABB> a2, int32_t a3);   /* boundary stub */
    void customServerAiStep();   /* boundary stub */
    void dropDeathLoot(bool a1, int32_t a2);   /* boundary stub */
    void checkDespawn();   /* closed */
    uint32_t canRide(gptr<sp_Entity> a1);   /* closed */
    uint32_t canChangeDimensions();   /* closed */
    double getHeadX(int32_t a1);
    double getHeadY(int32_t a1);   /* closed */
    double getHeadZ(int32_t a1);
    double rotlerp(float a1, float a2, float a3);   /* boundary stub */
    uint64_t performRangedAttack(int32_t a1, gptr<sp_LivingEntity> a2);   /* boundary stub */
    uint64_t performRangedAttack(int32_t a1, double a2, double a3, double a4, bool a5);   /* boundary stub */
    void getMaxHealth();
    void getHealth();
    uint32_t getDimension();   /* closed */
    uint64_t GetType();   /* closed */
};
static_assert(sizeof(WitherBoss) == 0x7f8, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
