/* WitherSkull - generated reconstruction header. */
#ifndef RECON_C_GAME_W_WITHERSKULL_HPP
#define RECON_C_GAME_W_WITHERSKULL_HPP 1

#include "guest_ns.hpp"
#include "game/F/Fireball.hpp"

namespace recon {
struct BlockPos;
struct Explosion;
struct HitResult;
struct Level;
struct vector_unsigned_long_long;

#pragma pack(push, 1)
struct WitherSkull : Fireball {
    /* no fields witnessed beyond the base */

    /* interface: 14 methods recovered, 0 without a recoverable signature, 1 ABI artifacts;
     * implementation: 5 reconstructed bodies, 8 lowered drafts, 3 closed */
    WitherSkull(gptr<Level> a1);   /* LOWERED-DRAFT; not reconstructed */
    WitherSkull(uint64_t a1, uint64_t a2, double a3, double a4, double a5);   /* boundary stub */
    WitherSkull(uint64_t a1, double a2, double a3, double a4, double a5, double a6, double a7);   /* LOWERED-DRAFT; not reconstructed */
    ~WitherSkull();   /* LOWERED-DRAFT; not reconstructed */
    WitherSkull() {}   /* generated shell; the real ctor is dispatch-only */
    static void create(gptr<Level> a1);   /* LOWERED-DRAFT; not reconstructed */
    static void staticCtor();   /* LOWERED-DRAFT; not reconstructed */
    uint32_t isOnFire();   /* closed */
    double getBlockExplosionResistance(gptr<Explosion> a1, gptr<Level> a2, gptr<BlockPos> a3);   /* LOWERED-DRAFT; not reconstructed */
    static void addDataWalkers(uint32_t a1, uint32_t a2);   /* LOWERED-DRAFT; not reconstructed */
    uint32_t isPickable();   /* LOWERED-DRAFT; not reconstructed */
    uint8_t isDangerous();   /* LOWERED-DRAFT; not reconstructed */
    uint64_t setDangerous(bool a1);   /* LOWERED-DRAFT; not reconstructed */
    double getInertia();
    void onHit(gptr<HitResult> a1);   /* boundary stub */
    void defineSynchedData();
    uint32_t shouldBurn();   /* closed */
    gptr<vector_unsigned_long_long> getEntityTypesToIgnoreCollisionsWith();   /* LOWERED-DRAFT; not reconstructed */
    uint64_t GetType();   /* closed */
};
static_assert(sizeof(WitherSkull) == 0x378, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
