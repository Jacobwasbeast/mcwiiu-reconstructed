/* Witch - generated reconstruction header. */
#ifndef RECON_C_GAME_W_WITCH_HPP
#define RECON_C_GAME_W_WITCH_HPP 1

#include "guest_ns.hpp"
#include "game/M/Monster.hpp"

namespace recon {
struct DamageSource;
struct Level;

#pragma pack(push, 1)
struct Witch : Monster {
    be<uint32_t> vfptr;   /* vtable */
    be<int32_t> field_0x734;

    /* interface: 20 methods recovered, 1 without a recoverable signature, 5 ABI artifacts;
     * implementation: 9 reconstructed bodies, 6 lowered drafts, 8 closed */
    Witch(gptr<Level> a1);   /* reconstructed */
    ~Witch();   /* reconstructed */
    Witch() {}   /* generated shell; the real ctor is dispatch-only */
    static void create(gptr<Level> a1);   /* LOWERED-DRAFT; not reconstructed */
    uint64_t performRangedAttack(uint32_t a1);   /* boundary stub */
    uint32_t isAggressive();   /* closed */
    void setAggressive(bool a1);   /* closed */
    static uint64_t staticCtor();   /* boundary stub */
    static void addDataWalkers(uint32_t a1, uint32_t a2);
    void registerGoals();   /* boundary stub */
    void defineSynchedData();   /* LOWERED-DRAFT; not reconstructed */
    uint32_t getAmbientSound();   /* closed */
    uint32_t getHurtSound(gptr<DamageSource> a1);   /* closed */
    uint32_t getDeathSound();   /* closed */
    uint64_t setUsingItem(bool a1);   /* LOWERED-DRAFT; not reconstructed */
    uint8_t isDrinkingPotion();   /* LOWERED-DRAFT; not reconstructed */
    void registerAttributes();   /* LOWERED-DRAFT; not reconstructed */
    void aiStep();   /* boundary stub */
    uint64_t handleEntityEvent(uint8_t a1, int32_t a2);   /* boundary stub */
    double getDamageAfterMagicAbsorb(int32_t a1);   /* LOWERED-DRAFT; not reconstructed */
    uint32_t getDefaultLootTable();   /* closed */
    double getEyeHeight();   /* closed */
    uint64_t GetType();   /* closed */
    /* unrecovered: DEATH_LOOT() [static] [0x00000000] */
};
static_assert(sizeof(Witch) == 0x738, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
