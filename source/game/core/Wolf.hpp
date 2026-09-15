/* Wolf - generated reconstruction header. */
#ifndef RECON_C_GAME_W_WOLF_HPP
#define RECON_C_GAME_W_WOLF_HPP 1

#include "guest_ns.hpp"
#include "game/T/TamableAnimal.hpp"

namespace recon {
struct Block;
struct BlockPos;
struct CompoundTag;
struct DamageSource;
struct DyeColor;
struct Level;
struct not_null_ptr_ItemInstance;
struct sp_Entity;
struct sp_LivingEntity;
struct sp_Player;

#pragma pack(push, 1)
struct Wolf : TamableAnimal {
    be<float> field_0x788;
    be<float> field_0x78c;
    be<uint8_t> field_0x790;
    be<uint8_t> field_0x791;
    uint8_t pad_0x792[2];
    be<float> field_0x794;
    be<float> field_0x798;
    uint8_t pad_0x79c[4];

    /* interface: 44 methods recovered, 2 without a recoverable signature, 2 ABI artifacts;
     * implementation: 13 reconstructed bodies, 20 lowered drafts, 9 closed */
    Wolf(gptr<Level> a1);   /* LOWERED-DRAFT; not reconstructed */
    ~Wolf();   /* reconstructed */
    Wolf() {}   /* generated shell; the real ctor is dispatch-only */
    static void create(gptr<Level> a1);   /* LOWERED-DRAFT; not reconstructed */
    static void addDataWalkers(uint32_t a1, uint32_t a2);
    static void staticCtor();   /* LOWERED-DRAFT; not reconstructed */
    void registerGoals();   /* boundary stub */
    void registerAttributes();   /* LOWERED-DRAFT; not reconstructed */
    void setAngry(bool a1);   /* LOWERED-DRAFT; not reconstructed */
    void setTarget(gptr<sp_LivingEntity> a1);   /* LOWERED-DRAFT; not reconstructed */
    void customServerAiStep();   /* boundary stub */
    void defineSynchedData();   /* boundary stub */
    void playStepSound(gptr<BlockPos> a1, gptr<Block> a2);   /* LOWERED-DRAFT; not reconstructed */
    uint8_t isAngry();   /* LOWERED-DRAFT; not reconstructed */
    uint64_t getCollarColor();   /* LOWERED-DRAFT; not reconstructed */
    void addAdditonalSaveData(gptr<CompoundTag> a1);   /* boundary stub */
    void setCollarColor(gptr<DyeColor> a1);
    void readAdditionalSaveData(gptr<CompoundTag> a1);   /* boundary stub */
    uint32_t getAmbientSound();   /* LOWERED-DRAFT; not reconstructed */
    uint32_t getHurtSound(gptr<DamageSource> a1);   /* closed */
    uint32_t getDeathSound();   /* closed */
    double getSoundVolume();   /* closed */
    uint32_t getDefaultLootTable();   /* closed */
    void aiStep();   /* LOWERED-DRAFT; not reconstructed */
    uint8_t isInterested();   /* LOWERED-DRAFT; not reconstructed */
    void tick();   /* boundary stub */
    double getBodyRollAngle(float a1, float a2);
    double getHeadRollAngle(float a1);   /* closed */
    double getEyeHeight();   /* closed */
    uint32_t getMaxHeadXRot();   /* closed */
    uint32_t hurt(double a1, float a2);   /* boundary stub */
    int32_t doHurtTarget(gptr<sp_Entity> a1);   /* LOWERED-DRAFT; not reconstructed */
    void setTame(bool a1);   /* LOWERED-DRAFT; not reconstructed */
    uint64_t tame(gptr<sp_Player> a1, bool a2, bool a3);   /* boundary stub */
    uint32_t mobInteract(gptr<sp_Player> a1, uint32_t a2);   /* boundary stub */
    void handleEntityEvent(uint8_t a1, int32_t a2);   /* boundary stub */
    double getTailAngle();   /* LOWERED-DRAFT; not reconstructed */
    uint32_t isFood(gptr<not_null_ptr_ItemInstance> a1);   /* LOWERED-DRAFT; not reconstructed */
    uint32_t getMaxSpawnClusterSize();   /* closed */
    int32_t GetSynchedHealth();   /* LOWERED-DRAFT; not reconstructed */
    uint32_t GetInteractTooltip(uint32_t a1, uint32_t a2);   /* boundary stub */
    uint64_t getBreedOffspring(uint32_t a1);   /* LOWERED-DRAFT; not reconstructed */
    void setIsInterested(bool a1);
    uint32_t canMate(uint32_t a1);   /* LOWERED-DRAFT; not reconstructed */
    uint32_t wantsToAttack(gptr<sp_LivingEntity> a1, gptr<sp_LivingEntity> a2);   /* LOWERED-DRAFT; not reconstructed */
    uint32_t canBeLeashed(gptr<sp_Player> a1);   /* LOWERED-DRAFT; not reconstructed */
    uint64_t GetType();   /* closed */
    /* unrecovered: isWet() [0x00000000] */
    /* unrecovered: getWetShade(float) [0x00000000] */
};
static_assert(sizeof(Wolf) == 0x7a0, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
