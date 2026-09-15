/* VoronoiZoom - generated reconstruction header. */
#ifndef RECON_C_GAME_V_VORONOIZOOM_HPP
#define RECON_C_GAME_V_VORONOIZOOM_HPP 1

#include "guest_ns.hpp"
#include "game/L/Layer.hpp"

namespace recon {
#pragma pack(push, 1)
struct VoronoiZoom : Layer {
    /* no fields witnessed beyond the base */

    /* interface: 1 methods recovered, 0 without a recoverable signature;
     * implementation: 0 reconstructed bodies, 0 lowered drafts, 0 closed */
    VoronoiZoom(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4);   /* boundary stub */
    ~VoronoiZoom();   /* LOWERED-DRAFT; not reconstructed */
    VoronoiZoom() {}   /* generated shell; the real ctor is dispatch-only */
    uint64_t getArea(int32_t a1, int32_t a2, int32_t a3, int32_t a4);   /* boundary stub */
};
static_assert(sizeof(VoronoiZoom) == 0x28, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
