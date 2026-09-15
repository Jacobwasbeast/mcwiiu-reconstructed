/* Vex - generated reconstruction header. */
#ifndef RECON_C_GAME_V_VEX_HPP
#define RECON_C_GAME_V_VEX_HPP 1

#include "guest_ns.hpp"
#include "game/M/Monster.hpp"

namespace recon {
struct BlockPos;
struct CompoundTag;
struct DamageSource;
struct DifficultyInstance;
struct Level;
struct MobGroupData;
struct sp_Entity;
struct wstring;

#pragma pack(push, 1)
struct Vex : Monster {
    be<int32_t> field_0x730;
    be<int32_t> field_0x734;
    gptr<BlockPos> field_0x738;
    be<uint8_t> field_0x73c;
    uint8_t pad_0x73d[3];
    be<int32_t> field_0x740;
    uint8_t pad_0x744[4];

    /* interface: 29 methods recovered, 0 without a recoverable signature, 1 ABI artifacts;
     * implementation: 13 reconstructed bodies, 13 lowered drafts, 10 closed */
    Vex(gptr<Level> a1);   /* LOWERED-DRAFT; not reconstructed */
    ~Vex();   /* LOWERED-DRAFT; not reconstructed */
    Vex() {}   /* generated shell; the real ctor is dispatch-only */
    static void create(gptr<Level> a1);   /* LOWERED-DRAFT; not reconstructed */
    static uint64_t staticCtor();   /* boundary stub */
    void move(uint32_t a1, double a2, double a3, double a4, bool a5);   /* LOWERED-DRAFT; not reconstructed */
    void tick();   /* LOWERED-DRAFT; not reconstructed */
    void registerGoals();   /* LOWERED-DRAFT; not reconstructed */
    void registerAttributes();   /* LOWERED-DRAFT; not reconstructed */
    void defineSynchedData();
    static void addDataWalkers(uint32_t a1, uint32_t a2);
    void setLimitedLife(int32_t a1);   /* closed */
    void readAdditionalSaveData(gptr<CompoundTag> a1);   /* boundary stub */
    void addAdditonalSaveData(gptr<CompoundTag> a1);   /* LOWERED-DRAFT; not reconstructed */
    uint64_t getOwner();   /* boundary stub */
    gptr<BlockPos> getBoundOrigin();   /* closed */
    void setBoundOrigin(gptr<BlockPos> a1);
    bool getVexFlag(int32_t a1);   /* LOWERED-DRAFT; not reconstructed */
    uint64_t setVexFlag(int32_t a1, bool a2);   /* LOWERED-DRAFT; not reconstructed */
    bool isCharging();   /* LOWERED-DRAFT; not reconstructed */
    void setIsCharging(bool a1);   /* LOWERED-DRAFT; not reconstructed */
    uint64_t setOwner(uint32_t a1);   /* LOWERED-DRAFT; not reconstructed */
    uint32_t getAmbientSound();   /* closed */
    uint32_t getDeathSound();   /* closed */
    uint32_t getHurtSound(gptr<DamageSource> a1);   /* closed */
    uint32_t getDefaultLootTable();   /* closed */
    uint32_t getDefaultGearTable();   /* closed */
    uint32_t getLightColor();   /* closed */
    double getBrightness();   /* closed */
    uint32_t finalizeMobSpawn(gptr<DifficultyInstance> a1, gptr<MobGroupData> a2, gptr<wstring> a3);   /* LOWERED-DRAFT; not reconstructed */
    void populateDefaultEquipmentSlots(gptr<DifficultyInstance> a1);   /* LOWERED-DRAFT; not reconstructed */
    uint64_t GetType();   /* closed */

    /* virtual slot +0xae4: 1 possible targets, 0 native */
    int32_t virt_doHurtTarget(gptr<sp_Entity> a1) {
        return (int32_t)(guest::vdispatch(LD32(LD32(gaddr(this) + 0x348) + 0xae4), gaddr(this), (uint32_t)a1) >> 32);
    }
};
static_assert(sizeof(Vex) == 0x748, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
