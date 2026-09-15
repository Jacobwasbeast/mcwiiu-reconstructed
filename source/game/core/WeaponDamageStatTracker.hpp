/* WeaponDamageStatTracker - generated reconstruction header. */
#ifndef RECON_C_GAME_W_WEAPONDAMAGESTATTRACKER_HPP
#define RECON_C_GAME_W_WEAPONDAMAGESTATTRACKER_HPP 1

#include "guest_ns.hpp"
#include "game/U/UntypedStatTracker.hpp"

namespace recon {
struct DamageEvent;
struct StatsUID;

#pragma pack(push, 1)
struct WeaponDamageStatTracker : UntypedStatTracker {
    be<int32_t> field_0x18;
    be<int32_t> field_0x1c;
    uint8_t pad_0x20[4];
    be<int32_t> field_0x24;
    uint8_t pad_0x28[4];
    be<uint32_t> field_0x2c;

    /* interface: 1 methods recovered, 0 without a recoverable signature;
     * implementation: 0 reconstructed bodies, 1 lowered drafts, 0 closed */
    WeaponDamageStatTracker(gptr<StatsUID> a1, uint32_t a2);   /* LOWERED-DRAFT; not reconstructed */
    ~WeaponDamageStatTracker();   /* LOWERED-DRAFT; not reconstructed */
    WeaponDamageStatTracker() {}   /* generated shell; the real ctor is dispatch-only */
    void update(gptr<DamageEvent> a1);   /* LOWERED-DRAFT; not reconstructed */
};
static_assert(sizeof(WeaponDamageStatTracker) == 0x30, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
