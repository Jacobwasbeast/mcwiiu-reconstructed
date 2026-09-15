/* Vertex - generated reconstruction header. */
#ifndef RECON_C_GAME_V_VERTEX_HPP
#define RECON_C_GAME_V_VERTEX_HPP 1

#include "guest_ns.hpp"

namespace recon {
#pragma pack(push, 1)
struct Vertex {
    be<int32_t> field_0x0;
    be<float> field_0x4;
    be<float> field_0x8;

    /* interface: 1 methods recovered, 2 without a recoverable signature;
     * implementation: 0 reconstructed bodies, 1 lowered drafts, 0 closed */
    Vertex(float a1, float a2, float a3, float a4, float a5);   /* LOWERED-DRAFT; not reconstructed */
    Vertex(double a1, float a2, float a3);   /* boundary stub */
    Vertex() {}   /* generated shell; the real ctor is dispatch-only */
    void remap(float a1, float a2);   /* LOWERED-DRAFT; not reconstructed */
    /* unrecovered: {ctor}(Vec3*, float, float) [0x00000000] */
    /* unrecovered: {dtor}() [0x00000000] */
};
static_assert(sizeof(Vertex) == 0xc, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
