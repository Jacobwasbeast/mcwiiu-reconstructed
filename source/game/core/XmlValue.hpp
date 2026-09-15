/* XmlValue - generated reconstruction header. */
#ifndef RECON_C_GAME_X_XMLVALUE_HPP
#define RECON_C_GAME_X_XMLVALUE_HPP 1

#include "guest_ns.hpp"

namespace recon {
struct wstring;

#pragma pack(push, 1)
struct XmlValue {
    uint8_t pad_0x0[4];
    be<uint8_t> field_0x4;
    uint8_t pad_0x5[7];
    be<int16_t> field_0xc;
    uint8_t pad_0xe[14];
    be<int32_t> field_0x1c;
    be<int32_t> field_0x20;
    be<int32_t> field_0x24;

    /* interface: 2 methods recovered, 0 without a recoverable signature;
     * implementation: 1 reconstructed bodies, 1 lowered drafts, 1 closed */
    ~XmlValue();   /* reconstructed */
    uint8_t isString();   /* closed */
    gresult<wstring> asString();   /* LOWERED-DRAFT; not reconstructed */
};
#pragma pack(pop)

}  /* namespace recon */

#endif
