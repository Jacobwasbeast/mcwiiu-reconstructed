/* VertexFormatElement - generated reconstruction header. */
#ifndef RECON_C_GAME_V_VERTEXFORMATELEMENT_HPP
#define RECON_C_GAME_V_VERTEXFORMATELEMENT_HPP 1

#include "guest_ns.hpp"

namespace recon {
struct VertexFormatElement_Type;
struct VertexFormatElement_Usage;

#pragma pack(push, 1)
struct VertexFormatElement {
    be<int32_t> field_0x0;
    be<int32_t> field_0x4;
    be<int32_t> field_0x8;
    be<int32_t> field_0xc;

    /* interface: 5 methods recovered, 4 without a recoverable signature;
     * implementation: 5 reconstructed bodies, 0 lowered drafts, 4 closed */
    VertexFormatElement(gptr<VertexFormatElement> a1);   /* reconstructed */
    VertexFormatElement(int32_t a1, gptr<VertexFormatElement_Type> a2, gptr<VertexFormatElement_Usage> a3, int32_t a4);   /* reconstructed */
    VertexFormatElement() {}   /* generated shell; the real ctor is dispatch-only */
    bool isPosition();   /* closed */
    uint32_t getUsage();   /* closed */
    uint32_t getIndex();   /* closed */
    int32_t getByteSize();
    uint32_t supportsUsage(int32_t a1, gptr<VertexFormatElement_Usage> a2);   /* closed */
    /* unrecovered: toString() const [0x00000000] */
    /* unrecovered: getType() const [0x00000000] */
    /* unrecovered: getCount() const [0x00000000] */
    /* unrecovered: equals(VertexFormatElement*) const [0x00000000] */
};
static_assert(sizeof(VertexFormatElement) == 0x10, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
