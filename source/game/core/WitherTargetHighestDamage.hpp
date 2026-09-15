/* WitherTargetHighestDamage - generated reconstruction header. */
#ifndef RECON_C_GAME_W_WITHERTARGETHIGHESTDAMAGE_HPP
#define RECON_C_GAME_W_WITHERTARGETHIGHESTDAMAGE_HPP 1

#include "guest_ns.hpp"
#include "game/T/TargetGoal.hpp"

namespace recon {
struct WitherBoss;
struct sp_LivingEntity;

#pragma pack(push, 1)
struct WitherTargetHighestDamage : TargetGoal {
    /* no fields witnessed beyond the base */

    /* interface: 5 methods recovered, 0 without a recoverable signature;
     * implementation: 0 reconstructed bodies, 4 lowered drafts, 0 closed */
    WitherTargetHighestDamage(gptr<WitherBoss> a1);   /* reconstructed */
    ~WitherTargetHighestDamage();   /* LOWERED-DRAFT; not reconstructed */
    WitherTargetHighestDamage() {}   /* generated shell; the real ctor is dispatch-only */
    uint64_t getHighestDamageTarget();   /* LOWERED-DRAFT; not reconstructed */
    uint32_t canUse();   /* LOWERED-DRAFT; not reconstructed */
    uint8_t canContinueToUse();   /* LOWERED-DRAFT; not reconstructed */
    void start();   /* LOWERED-DRAFT; not reconstructed */
    uint32_t canAttack(gptr<sp_LivingEntity> a1, bool a2);   /* boundary stub */
};
static_assert(sizeof(WitherTargetHighestDamage) == 0x28, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
