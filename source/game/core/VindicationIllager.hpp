/* VindicationIllager - generated reconstruction header. */
#ifndef RECON_C_GAME_V_VINDICATIONILLAGER_HPP
#define RECON_C_GAME_V_VINDICATIONILLAGER_HPP 1

#include "guest_ns.hpp"
#include "game/A/AbstractIllager.hpp"

namespace recon {
struct CompoundTag;
struct DamageSource;
struct DifficultyInstance;
struct Level;
struct MobGroupData;
struct sp_Entity;
struct wstring;

#pragma pack(push, 1)
struct VindicationIllager : AbstractIllager {
    be<uint8_t> field_0x730;
    uint8_t pad_0x731[7];

    /* interface: 21 methods recovered, 0 without a recoverable signature, 1 ABI artifacts;
     * implementation: 11 reconstructed bodies, 8 lowered drafts, 7 closed */
    VindicationIllager(gptr<Level> a1);   /* LOWERED-DRAFT; not reconstructed */
    ~VindicationIllager();   /* boundary stub */
    VindicationIllager() {}   /* generated shell; the real ctor is dispatch-only */
    static void create(gptr<Level> a1);   /* LOWERED-DRAFT; not reconstructed */
    static void addDataWalkers(uint32_t a1, uint32_t a2);
    void registerGoals();   /* boundary stub */
    void registerAttributes();   /* LOWERED-DRAFT; not reconstructed */
    void defineSynchedData();
    uint32_t getMobType();   /* closed */
    uint32_t getDefaultLootTable();   /* closed */
    uint32_t getDefaultGearTable();   /* closed */
    void isAgressive();
    void setAggressive(bool a1);
    void addAdditonalSaveData(gptr<CompoundTag> a1);   /* LOWERED-DRAFT; not reconstructed */
    void getArmPose();   /* LOWERED-DRAFT; not reconstructed */
    void readAdditionalSaveData(gptr<CompoundTag> a1);   /* LOWERED-DRAFT; not reconstructed */
    gptr<MobGroupData> finalizeMobSpawn(gptr<DifficultyInstance> a1, gptr<MobGroupData> a2, gptr<wstring> a3);   /* LOWERED-DRAFT; not reconstructed */
    void customServerAiStep();   /* LOWERED-DRAFT; not reconstructed */
    uint32_t isAlliedTo(gptr<sp_Entity> a1);   /* LOWERED-DRAFT; not reconstructed */
    void setCustomName(gptr<wstring> a1, gptr<wstring> a2, bool a3);   /* boundary stub */
    uint32_t getAmbientSound();   /* closed */
    uint32_t getDeathSound();   /* closed */
    uint32_t getHurtSound(gptr<DamageSource> a1);   /* closed */
    uint64_t GetType();   /* closed */
};
static_assert(sizeof(VindicationIllager) == 0x738, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
