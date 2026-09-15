#include "game/V/VertexFormatElement.hpp"
#include "game/V/VertexFormatElement__Type.hpp"
#include "game/V/VertexFormatElement__Usage.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(const VertexFormatElement&) [0x0339D58C] - reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
VertexFormatElement::VertexFormatElement(gptr<VertexFormatElement> a1)
{
    guest::stub(0x0339D58Cu, gaddr(this), (uint32_t)a1);
}
#else
VertexFormatElement::VertexFormatElement(gptr<VertexFormatElement> a1)
{
    uint32_t uVar1 = 0;
    uint32_t uVar2 = 0;
    uVar2 = a1->field_0x0;
    this->field_0x8 = a1->field_0x8;
    uVar1 = a1->field_0xc;
    this->field_0x0 = uVar2;
    uVar2 = a1->field_0x4;
    this->field_0xc = uVar1;
    this->field_0x4 = uVar2;
}
#endif

/* {ctor}(int, const VertexFormatElement::Type*, const VertexFormatElement::Usage*, int) [0x033A1370] - reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
VertexFormatElement::VertexFormatElement(int32_t a1, gptr<VertexFormatElement_Type> a2, gptr<VertexFormatElement_Usage> a3, int32_t a4)
{
    guest::stub(0x033A1370u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, (uint32_t)a4);
}
#else
VertexFormatElement::VertexFormatElement(int32_t a1, gptr<VertexFormatElement_Type> a2, gptr<VertexFormatElement_Usage> a3, int32_t a4)
{
    if (((this->supportsUsage((a1), (gptr<VertexFormatElement_Usage>)(a3))) == 0))
    {
        a3 = global_0x10588164;
    }
    this->field_0x0 = a2;
    this->field_0x8 = a1;
    this->field_0x4 = a3;
    this->field_0xc = a4;
}
#endif

/* isPosition() const [0x0339D5F8] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_0339D5F8
bool VertexFormatElement::isPosition()
{
    return (this->field_0x4 == global_0x10588158);
}
#else
bool VertexFormatElement::isPosition()
{
    return (bool)(guest::stub(0x0339D5F8u, gaddr(this)) >> 32);
}
#endif

/* getUsage() const [0x0339DDB8] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_0339DDB8
uint32_t VertexFormatElement::getUsage()
{
    return this->field_0x4;
}
#else
uint32_t VertexFormatElement::getUsage()
{
    return (uint32_t)(guest::stub(0x0339DDB8u, gaddr(this)) >> 32);
}
#endif

/* getIndex() const [0x0339DDC0] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_0339DDC0
uint32_t VertexFormatElement::getIndex()
{
    return this->field_0x8;
}
#else
uint32_t VertexFormatElement::getIndex()
{
    return (uint32_t)(guest::stub(0x0339DDC0u, gaddr(this)) >> 32);
}
#endif

/* getByteSize() const [0x0339DDD0] - reconstructed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_0339DDD0
int32_t VertexFormatElement::getByteSize()
{
    int32_t iVar1 = 0;
    iVar1 = ((VertexFormatElement_Type *)gmem(this->field_0x0))->getSize();
    return (iVar1 * this->field_0xc);
}
#else
int32_t VertexFormatElement::getByteSize()
{
    return (int32_t)(guest::stub(0x0339DDD0u, gaddr(this)) >> 32);
}
#endif

/* supportsUsage(int, const VertexFormatElement::Usage*) const [0x033A1348] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_033A1348
uint32_t VertexFormatElement::supportsUsage(int32_t a1, gptr<VertexFormatElement_Usage> a2)
{
    uint32_t uVar1 = 0;
    uVar1 = 1;
    if (((a1 != 0) && (a2 != global_0x10588164)))
    {
        uVar1 = 0;
    }
    return uVar1;
}
#else
uint32_t VertexFormatElement::supportsUsage(int32_t a1, gptr<VertexFormatElement_Usage> a2)
{
    return (uint32_t)(guest::stub(0x033A1348u, gaddr(this), (uint32_t)a1, (uint32_t)a2) >> 32);
}
#endif

}  /* namespace recon */
\n#include "game/V/VertexFormatElement__Type.hpp"
#include "_thirdparty/std__basic_string_wchar_t__std__char_traits_wchar_t___std__allocator_wchar_t__.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(int, std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>, int, VertexFormatElement::Type::EType) [0x033A14A0] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
VertexFormatElement_Type::VertexFormatElement_Type(int32_t a1, gptr<wstring> a2, int32_t a3, uint32_t a4)
{
    guest::stub(0x033A14A0u, gaddr(this), (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, (uint32_t)a4);
}
#else
VertexFormatElement_Type::VertexFormatElement_Type(int32_t a1, gptr<wstring> a2, int32_t a3, uint32_t a4)
{
    this->field_0x20 = 7;
    ST32(gaddr(this), a1);
    this->field_0x8 = 0;
    this->field_0x1c = 0;
    ((wstring *)gmem((gaddr(this) + (1) * 4)))->assign((int32_t)(a2), (uint32_t)(0), (uint32_t)(0xffffffff));
    this->field_0x28 = a4;
    this->field_0x24 = a3;
}
#endif

/* getSize() const [0x0339DDC8] - LOWERED-DRAFT; not reconstructed (1 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t VertexFormatElement_Type::getSize()
{
    return (uint32_t)(guest::stub(0x0339DDC8u, gaddr(this)) >> 32);
}
#else
uint32_t VertexFormatElement_Type::getSize()
{
    return LD32(gaddr(this));
}
#endif

}  /* namespace recon */
\n#include "game/V/VertexFormatElement__Usage.hpp"
#include "_thirdparty/std__basic_string_wchar_t__std__char_traits_wchar_t___std__allocator_wchar_t__.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>, VertexFormatElement::Usage::EUsage) [0x033A1408] - reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
VertexFormatElement_Usage::VertexFormatElement_Usage(gptr<wstring> a1, uint32_t a2)
{
    guest::stub(0x033A1408u, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}
#else
VertexFormatElement_Usage::VertexFormatElement_Usage(gptr<wstring> a1, uint32_t a2)
{
    this->field_0x1c = 7;
    this->field_0x4 = 0;
    this->field_0x18 = 0;
    ((wstring *)gmem(gaddr(this)))->assign((a1), 0, 0xffffffff);
    this->field_0x20 = a2;
}
#endif

/* getId() const [0x0339DDB0] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_0339DDB0
uint32_t VertexFormatElement_Usage::getId()
{
    return this->field_0x20;
}
#else
uint32_t VertexFormatElement_Usage::getId()
{
    return (uint32_t)(guest::stub(0x0339DDB0u, gaddr(this)) >> 32);
}
#endif

}  /* namespace recon */
