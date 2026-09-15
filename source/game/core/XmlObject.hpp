/* XmlObject - generated reconstruction header. */
#ifndef RECON_C_GAME_X_XMLOBJECT_HPP
#define RECON_C_GAME_X_XMLOBJECT_HPP 1

#include "guest_ns.hpp"
#include "_thirdparty/std__vector_XmlChild__std__allocator_XmlChild__.hpp"
#include "_thirdparty/std__vector_std__pair_std__basic_string_wchar_t__std__char_traits_wchar_t___std__allocator_wchar_t____std__basic_string_.hpp"

namespace recon {
struct XmlChild;
struct wstring;

#pragma pack(push, 1)
struct XmlObject {
    uint8_t pad_0x0[2];
    be<int16_t> field_0x2;
    be<int32_t> field_0x4;
    be<int16_t> field_0x8;
    uint8_t pad_0xa[14];
    be<int32_t> field_0x18;
    be<int32_t> field_0x1c;
    be<int32_t> field_0x20;
    vector_XmlChild m_0x24;
    vector_pair_wstring_wstring m_0x34;

    /* interface: 8 methods recovered, 14 without a recoverable signature;
     * implementation: 0 reconstructed bodies, 7 lowered drafts, 0 closed */
    XmlObject(gptr<wstring> a1);   /* LOWERED-DRAFT; not reconstructed */
    ~XmlObject();   /* LOWERED-DRAFT; not reconstructed */
    XmlObject() {}   /* generated shell; the real ctor is dispatch-only */
    uint32_t child(gptr<wstring> a1);   /* LOWERED-DRAFT; not reconstructed */
    void addChildRaw(gptr<XmlChild> a1);   /* boundary stub */
    void addAttr(gptr<wstring> a1, gptr<wstring> a2);   /* LOWERED-DRAFT; not reconstructed */
    int32_t size();   /* LOWERED-DRAFT; not reconstructed */
    int32_t operator[](int32_t a1);   /* LOWERED-DRAFT; not reconstructed */
    uint64_t asString();   /* LOWERED-DRAFT; not reconstructed */
    bool isString();   /* LOWERED-DRAFT; not reconstructed */
    gresult<wstring> toString(int32_t a1, bool a2);   /* LOWERED-DRAFT; not reconstructed */
    /* unrecovered: addChild(XmlObject*) [0x00000000] */
    /* unrecovered: getAttr(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&) const [0x00000000] */
    /* unrecovered: getText() const [0x00000000] */
    /* unrecovered: _getLookup(WstringLookup*) [0x00000000] */
    /* unrecovered: getLookup() [0x00000000] */
    /* unrecovered: getChildren() [0x00000000] */
    /* unrecovered: _getValues(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&, const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&, std::vector<std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>, std::allocator<std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>>>*) [0x00000000] */
    /* unrecovered: getAllValues(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&) [0x00000000] */
    /* unrecovered: getValues(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&, const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&) [0x00000000] */
    /* unrecovered: getChildren(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&) [0x00000000] */
    /* unrecovered: getChildrenRaw() const [0x00000000] */
    /* unrecovered: getAttrs() [0x00000000] */
    /* unrecovered: child(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&) const [0x02F3BDA8] */
    /* unrecovered: operator[](const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&) const [0x02F3D06C] */
};
static_assert(sizeof(XmlObject) == 0x44, "layout");
#pragma pack(pop)

}  /* namespace recon */

#endif
