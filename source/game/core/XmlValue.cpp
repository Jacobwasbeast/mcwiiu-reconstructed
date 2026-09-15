#include "game/X/XmlValue.hpp"
#include "_thirdparty/std__basic_string_wchar_t__std__char_traits_wchar_t___std__allocator_wchar_t__.hpp"
#include "functions.hpp"

namespace recon {

/* {dtor}() [0x02FC7A84] - reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
XmlValue::~XmlValue()
{
    guest::stub(0x02FC7A84u, gaddr(this), 0u);
}
#else
XmlValue::~XmlValue()
{
    if ((7 < this->field_0x24))
    {
        ::recon::guest_delete((int32_t)(this->field_0x1c));
    }
    this->field_0x24 = 7;
    this->field_0x20 = 0;
    this->field_0xc = 0;
}
#endif

/* isString() const [0x02FC7A04] - closed */
#if !defined(RECON_RUNTIME_BUILD) || RECON_NATIVE_02FC7A04
uint8_t XmlValue::isString()
{
    return (uint8_t)this->field_0x4;
}
#else
uint8_t XmlValue::isString()
{
    return (uint8_t)(guest::stub(0x02FC7A04u, gaddr(this)) >> 32);
}
#endif

/* asString() const [0x02FC7A0C] - LOWERED-DRAFT; not reconstructed (2 open edges, 3 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
gresult<wstring> XmlValue::asString()
{
    gresult<wstring> result;
    guest::stub(0x02FC7A0Cu, gaddr(this), (uint32_t)result);
    return result;
}
#else
gresult<wstring> XmlValue::asString(int32_t a1)
{
    if (((a1 != 0) || (a1 = guest::guest_new((uint32_t)(0x20)), (a1 != 0))))
    {
        ST32((uint32_t)((a1 + 0x1c)), 7);
        ST16((uint32_t)((a1 + 4)), 0);
        ST32((uint32_t)((a1 + 0x18)), 0);
        ((wstring *)gmem(a1))->assign((int32_t)((gaddr(this) + 8)), (uint32_t)(0), (uint32_t)(0xffffffff));
    }
    return;
}
#endif

}  /* namespace recon */
