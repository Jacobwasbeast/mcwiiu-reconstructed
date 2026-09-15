#include "game/X/XmlObject.hpp"
#include "game/X/XmlChild.hpp"
#include "_thirdparty/std___String_base.hpp"
#include "_thirdparty/std__basic_string_char__std__char_traits_char___std__allocator_char__.hpp"
#include "_thirdparty/std__basic_string_wchar_t__std__char_traits_wchar_t___std__allocator_wchar_t__.hpp"
#include "_thirdparty/std__vector_XmlChild__std__allocator_XmlChild__.hpp"
#include "_thirdparty/std__vector_std__pair_std__basic_string_wchar_t__std__char_traits_wchar_t___std__allocator_wchar_t____std__basic_string_.hpp"
#include "functions.hpp"

namespace recon {

/* {ctor}(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&) [0x02F3BA3C] - LOWERED-DRAFT; not reconstructed; allocating shell stripped */
#if defined(RECON_RUNTIME_BUILD)
XmlObject::XmlObject(gptr<wstring> a1)
{
    guest::stub(0x02F3BA3Cu, gaddr(this), (uint32_t)a1);
}
#else
XmlObject::XmlObject(gptr<wstring> a1)
{
    uint32_t _frame = native_frame_open(40);
    ST32(gaddr(this), GSTR(L"", 0x10391A60));
    this->field_0x20 = 7;
    this->field_0x8 = 0;
    this->field_0x1c = 0;
    ((wstring *)gmem(gaddr(&this->field_0x4)))->assign((int32_t)(a1), (uint32_t)(0), (uint32_t)(0xffffffff));
    ((vector_XmlChild *)gmem(_frame))->_Buy((uint32_t)(0));
    ((vector_XmlChild *)gmem((gaddr(this) + (9) * 4)))->operator=((int32_t)(_frame));
    ((vector_XmlChild *)gmem(_frame))->_Tidy();
    ((vector_pair_wstring_wstring *)gmem((_frame + 16)))->_Buy((uint32_t)(0));
    ((vector_pair_wstring_wstring *)gmem((gaddr(this) + (0xd) * 4)))->operator=((int32_t)((_frame + 16)));
    ((vector_pair_wstring_wstring *)gmem((_frame + 16)))->_Tidy();
    native_frame_close(_frame, 40);
}
#endif

/* {dtor}() [0x02F3BB90] - LOWERED-DRAFT; not reconstructed; destructor shell stripped */
#if defined(RECON_RUNTIME_BUILD)
XmlObject::~XmlObject()
{
    guest::stub(0x02F3BB90u, gaddr(this), 0u);
}
#else
XmlObject::~XmlObject()
{
    uint32_t piVar1 = 0;
    int32_t iVar2 = 0;
    int32_t iVar3 = 0;
    iVar3 = LD32(gaddr(this) + (0xb) * 4);
    iVar2 = LD32(gaddr(this) + (10) * 4);
    if ((iVar2 != iVar3))
    {
        do
        {
            if (((LD32((uint32_t)((iVar2 + 4))) == 1) && (piVar1 = (uint32_t)(((XmlChild *)gmem(iVar2))->xml()), (piVar1 != (uint32_t)(0x0)))))
            {
                (uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar1), 0) + 0xc)), (uint32_t)(piVar1), (uint32_t)(3)) >> 32);
                iVar3 = LD32(gaddr(this) + (0xb) * 4);
            }
            iVar2 = (iVar2 + 0xc);
        }
        while ((iVar2 != iVar3));
    }
    if ((7 < (uint32_t)(this->field_0x20)))
    {
        ::recon::guest_delete((int32_t)(this->field_0x18));
    }
    this->field_0x20 = 7;
    this->field_0x1c = 0;
    this->field_0x8 = 0;
}
#endif

/* child(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&) [0x02F3BCC0] - LOWERED-DRAFT; not reconstructed (4 raw IR ops, 4 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
uint32_t XmlObject::child(gptr<wstring> a1)
{
    return (uint32_t)(guest::stub(0x02F3BCC0u, gaddr(this), (uint32_t)a1) >> 32);
}
#else
uint32_t XmlObject::child(gptr<wstring> a1)
{
    uint32_t puVar1 = 0;
    int32_t iVar2 = 0;
    int32_t iVar3 = 0;
    iVar2 = LD32((uint32_t)((gaddr(this) + 0x2c)));
    iVar3 = LD32((uint32_t)((gaddr(this) + 0x28)));
    if ((iVar3 != iVar2))
    {
        do
        {
            if ((LD32((uint32_t)((iVar3 + 4))) == 1))
            {
                iVar2 = ((XmlChild *)gmem(iVar3))->xml();
                iVar2 = ::recon::equalsIgnoreCase__FRCQ2_3std78basic_string__tm__58_wQ2_3std20char_traits__tm__2_wQ2_3std18allocator__tm__2_wT1((gptr<wstring>)((iVar2 + 4)), (gptr<wstring>)(a1));
                if (((iVar2 != 0) || (a1->_Mysize == 0)))
                {
                    puVar1 = (uint32_t)(((XmlChild *)gmem(iVar3))->xml());
                    return puVar1;
                }
                iVar2 = LD32((uint32_t)((gaddr(this) + 0x2c)));
            }
            iVar3 = (iVar3 + 0xc);
        }
        while ((iVar3 != iVar2));
    }
    return 0x10529A90u;
}
#endif

/* addChildRaw(const XmlChild&) [0x02F3C660] - boundary stub; body not reconstructed */
void XmlObject::addChildRaw(gptr<XmlChild> a1)
{
    guest::stub(0x02F3C660u, gaddr(this), (uint32_t)a1);
}

/* addAttr(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&, const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>&) [0x02F3CF2C] - LOWERED-DRAFT; not reconstructed (3 open edges, 14 raw IR ops, 1 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
void XmlObject::addAttr(gptr<wstring> a1, gptr<wstring> a2)
{
    guest::stub(0x02F3CF2Cu, gaddr(this), (uint32_t)a1, (uint32_t)a2);
}
#else
void XmlObject::addAttr(gptr<wstring> a1, gptr<wstring> a2)
{
    uint32_t _frame = native_frame_open(64);
    ST32((_frame + 28), 7);
    ST16((_frame + 4), 0);
    ST32((_frame + 24), 0);
    ((wstring *)gmem(_frame))->assign((int32_t)(a1), (uint32_t)(0), (uint32_t)(0xffffffff));
    ST32((_frame + 60), 7);
    ST16((_frame + 36), 0);
    ST32((_frame + 56), 0);
    ((wstring *)gmem((_frame + 32)))->assign((int32_t)(a2), (uint32_t)(0), (uint32_t)(0xffffffff));
    ((vector_pair_wstring_wstring *)gmem((gaddr(this) + 0x34)))->push_back((uint32_t)(_frame));
    if ((7 < LD32((_frame + 60))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 52))));
    }
    ST32((_frame + 56), 0);
    ST16((_frame + 36), 0);
    ST32((_frame + 60), 7);
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    native_frame_close(_frame, 64);
    return;
    native_frame_close(_frame, 64);
}
#endif

/* size() const [0x02F3D038] - LOWERED-DRAFT; not reconstructed (3 raw IR ops, 3 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
int32_t XmlObject::size()
{
    return (int32_t)(guest::stub(0x02F3D038u, gaddr(this)) >> 32);
}
#else
int32_t XmlObject::size()
{
    int32_t iVar1 = 0;
    iVar1 = 0;
    if ((LD32((uint32_t)((gaddr(this) + 0x28))) != 0))
    {
        iVar1 = ((LD32((uint32_t)((gaddr(this) + 0x2c))) - LD32((uint32_t)((gaddr(this) + 0x28)))) / 0xc);
    }
    return iVar1;
}
#endif

/* operator[](int) const [0x02F3D4A8] - LOWERED-DRAFT; not reconstructed (4 open edges, 20 raw IR ops, 4 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
int32_t XmlObject::operator[](int32_t a1)
{
    return (int32_t)(guest::stub(0x02F3D4A8u, gaddr(this), (uint32_t)a1) >> 32);
}
#else
int32_t XmlObject::operator[](int32_t a1)
{
    uint32_t _frame = native_frame_open(72);
    uint32_t pcVar1 = 0;
    uint32_t pcVar2 = 0;
    int32_t iVar3 = 0;
    iVar3 = LD32((uint32_t)((gaddr(this) + 0x28)));
    if (((iVar3 == 0) || ((uint32_t)(((LD32((uint32_t)((gaddr(this) + 0x2c))) - iVar3) / 0xc)) <= a1)))
    {
        ST32((_frame + 68), 0xf);
        ST32((_frame + 64), 0);
        ST8((_frame + 44), 0);
        pcVar1 = (GSTR("invalid_vector_T__subscript", 0x1038B17C) + 0x1b);
        do
        {
            pcVar2 = pcVar1;
            pcVar1 = (pcVar2 + 1);
        }
        while (((uint8_t)gelem<uint8_t>((uint32_t)(pcVar1), 0) != '\0'));
        ((string *)gmem((_frame + 40)))->assign(GSTR("invalid_vector_T__subscript", 0x1038B198), (uint32_t)((pcVar2 + -(0x1038b197))));
        ST32((_frame + 4), GSTR(L"", 0x1038A82C));
        ST32((_frame + 36), 0xf);
        ST32(_frame, GSTR("unknown", 0x1038A758));
        ST8((_frame + 12), 0);
        ST32((_frame + 32), 0);
        ((string *)gmem((_frame + 8)))->assign((int32_t)((_frame + 40)), (uint32_t)(0), (uint32_t)(0xffffffff));
        ST32((_frame + 4), GSTR(L"", 0x1038A85C));
        if ((global_0x104D2910 != (uint32_t)(0x0)))
        {
            (uint32_t)(guest::indirect((uint32_t)(global_0x104D2910), (uint32_t)(_frame)) >> 32);
        }
        guest::std___Throw((uint32_t)(_frame), 0u);
        if ((0xf < LD32((_frame + 36))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 28))));
        }
        ST32((_frame + 32), 0);
        ST32((_frame + 36), 0xf);
        ST8((_frame + 12), 0);
        if ((0xf < LD32((_frame + 68))))
        {
            ::recon::guest_delete((int32_t)(LD32((_frame + 60))));
        }
        iVar3 = LD32((uint32_t)((gaddr(this) + 0x28)));
    }
    { int32_t t1 = (iVar3 + (a1 * 0xc));
    native_frame_close(_frame, 72);
    return t1; }
    native_frame_close(_frame, 72);
}
#endif

/* asString() const [0x02F3D4BC] - LOWERED-DRAFT; not reconstructed (1 open edges, 1 raw IR ops) */
#if defined(RECON_RUNTIME_BUILD)
uint64_t XmlObject::asString()
{
    return (uint64_t)guest::stub(0x02F3D4BCu, gaddr(this));
}
#else
uint64_t XmlObject::asString(uint32_t a1)
{
    uint32_t piVar1 = 0;
    piVar1 = (uint32_t)(this->operator[]((int32_t)(0)));
    return ((uint64_t)(uint32_t)((uint32_t)(guest::indirect(LD32((gelem<int32_t>((uint32_t)(piVar1), 0) + 0x14)), (uint32_t)(piVar1), (uint32_t)(a1)) >> 32)) << 32);
}
#endif

/* isString() const [0x02F3D50C] - LOWERED-DRAFT; not reconstructed (1 open edges) */
#if defined(RECON_RUNTIME_BUILD)
bool XmlObject::isString()
{
    return (bool)(guest::stub(0x02F3D50Cu, gaddr(this)) >> 32);
}
#else
bool XmlObject::isString()
{
    int32_t iVar1 = 0;
    iVar1 = guest::XmlObject_size(gaddr(this));
    return (iVar1 != 0);
}
#endif

/* toString(int, bool) const [0x02F3D8E4] - LOWERED-DRAFT; not reconstructed (33 open edges, 74 raw IR ops, 24 raise misses) */
#if defined(RECON_RUNTIME_BUILD)
gresult<wstring> XmlObject::toString(int32_t a1, bool a2)
{
    gresult<wstring> result;
    guest::stub(0x02F3D8E4u, gaddr(this), (uint32_t)result, (uint32_t)a1, (uint32_t)a2);
    return result;
}
#else
gresult<wstring> XmlObject::toString(int32_t a1, bool a2, int32_t a3)
{
    uint32_t _frame = native_frame_open(192);
    int32_t iVar1 = 0;
    uint32_t sVar2 = 0;
    uint32_t uVar3 = 0;
    int32_t iVar4 = 0;
    uint32_t puVar5 = 0;
    ST32((_frame + 28), 7);
    ST16((_frame + 4) + (0) * 2, 0);
    ST32((_frame + 24), 0);
    if ((a2 == -(1)))
    {
        ((std__String_base *)gmem(_frame))->_Xlen();
    }
    iVar1 = ((wstring *)gmem(_frame))->_Grow((uint32_t)(a2), (bool)(0));
    if ((iVar1 != 0))
    {
        ((wstring *)gmem(_frame))->_Chassign((int32_t)(0), (uint32_t)(a2), (uint16_t)(9));
        puVar5 = (_frame + 4);
        if ((7 < LD32((_frame + 28))))
        {
            puVar5 = LD32((_frame + 20));
        }
        ST32((_frame + 24), a2);
        gelem<uint16_t>((uint32_t)(puVar5), a2) = 0;
    }
    sVar2 = ::recon::wcslen((uint32_t)(GSTR(L"<", 0x1038CA7C)));
    ((wstring *)gmem(_frame))->append(GSTR(L"<", 0x1038CA7C), (uint32_t)(sVar2));
    ((wstring *)gmem(_frame))->append((int32_t)(gaddr(&this->field_0x4)), (uint32_t)(0), (uint32_t)(0xffffffff));
    iVar1 = LD32((uint32_t)((gaddr(this) + 0x38)));
    if ((iVar1 != LD32((uint32_t)((gaddr(this) + 0x3c)))))
    {
        do
        {
            sVar2 = ::recon::wcslen((uint32_t)(GSTR(L" ", 0x1038CA80)));
            ((wstring *)gmem(_frame))->append(GSTR(L" ", 0x1038CA80), (uint32_t)(sVar2));
            ((wstring *)gmem(_frame))->append((int32_t)(iVar1), (uint32_t)(0), (uint32_t)(0xffffffff));
            sVar2 = ::recon::wcslen((uint32_t)(GSTR(L"=\"", 0x1038CA90)));
            ((wstring *)gmem(_frame))->append(GSTR(L"=\"", 0x1038CA90), (uint32_t)(sVar2));
            ((wstring *)gmem(_frame))->append((int32_t)((iVar1 + 0x20)), (uint32_t)(0), (uint32_t)(0xffffffff));
            sVar2 = ::recon::wcslen((uint32_t)(GSTR(L"\"", 0x1038CA84)));
            ((wstring *)gmem(_frame))->append(GSTR(L"\"", 0x1038CA84), (uint32_t)(sVar2));
            iVar1 = (iVar1 + 0x40);
        }
        while ((iVar1 != LD32((uint32_t)((gaddr(this) + 0x3c)))));
    }
    if (((LD32((uint32_t)((gaddr(this) + 0x28))) == 0) || ((iVar1 = (LD32((uint32_t)((gaddr(this) + 0x2c))) - LD32((uint32_t)((gaddr(this) + 0x28)))), (iVar4 = (iVar1 >> 0x1f))), (((iVar1 / 0xc) + iVar4) == iVar4))))
    {
        sVar2 = ::recon::wcslen((uint32_t)(0x1038CA74u));
        ((wstring *)gmem(_frame))->append(0x1038CA74u, (uint32_t)(sVar2));
    }
    else
    {
        sVar2 = ::recon::wcslen((uint32_t)(GSTR(L">", 0x1038CA88)));
        ((wstring *)gmem(_frame))->append(GSTR(L">", 0x1038CA88), (uint32_t)(sVar2));
        iVar4 = LD32((uint32_t)((gaddr(this) + 0x28)));
        iVar1 = LD32((uint32_t)((gaddr(this) + 0x2c)));
        if ((iVar4 != iVar1))
        {
            do
            {
                if (((LD32((uint32_t)((iVar4 + 4))) == 0) || (LD32((uint32_t)((iVar4 + 4))) == 2)))
                {
                    if (((a3 == 0) && (LD32((uint32_t)((iVar4 + 4))) == 2)))
                    {
                        sVar2 = ::recon::wcslen(GSTR("___CDATA_", 0x1038CAA0));
                        ((wstring *)gmem(_frame))->append(GSTR("___CDATA_", 0x1038CAA0), (uint32_t)(sVar2));
                        ((XmlChild *)gmem(iVar4))->txt((int32_t)((_frame + 64)));
                        ::recon::escape__FQ2_3std78basic_string__tm__58_wQ2_3std20char_traits__tm__2_wQ2_3std18allocator__tm__2_w((int32_t)((_frame + 32)), (uint32_t)((_frame + 64)));
                        ((wstring *)gmem(_frame))->append((int32_t)((_frame + 32)), (uint32_t)(0), (uint32_t)(0xffffffff));
                        if ((7 < LD32((_frame + 60))))
                        {
                            ::recon::guest_delete((int32_t)(LD32((_frame + 52))));
                        }
                        ST32((_frame + 60), 7);
                        ST16((_frame + 36), 0);
                        ST32((_frame + 56), 0);
                        if ((7 < LD32((_frame + 92))))
                        {
                            ::recon::guest_delete((int32_t)(LD32((_frame + 84))));
                        }
                        ST32((_frame + 88), 0);
                        ST16((_frame + 68), 0);
                        ST32((_frame + 92), 7);
                        sVar2 = ::recon::wcslen((uint32_t)(GSTR(L"]]>", 0x1038CA6C)));
                        ((wstring *)gmem(_frame))->append(GSTR(L"]]>", 0x1038CA6C), (uint32_t)(sVar2));
                    }
                    else
                    {
                        ((XmlChild *)gmem(iVar4))->txt((int32_t)((_frame + 32)));
                        ::recon::escape__FQ2_3std78basic_string__tm__58_wQ2_3std20char_traits__tm__2_wQ2_3std18allocator__tm__2_w((int32_t)((_frame + 64)), (uint32_t)((_frame + 32)));
                        ((wstring *)gmem(_frame))->append((int32_t)((_frame + 64)), (uint32_t)(0), (uint32_t)(0xffffffff));
                        if ((7 < LD32((_frame + 92))))
                        {
                            ::recon::guest_delete((int32_t)(LD32((_frame + 84))));
                        }
                        ST16((_frame + 68), 0);
                        ST32((_frame + 88), 0);
                        ST32((_frame + 92), 7);
                        if ((7 < LD32((_frame + 60))))
                        {
                            ::recon::guest_delete((int32_t)(LD32((_frame + 52))));
                        }
                        ST32((_frame + 60), 7);
                        ST32((_frame + 56), 0);
                        ST16((_frame + 36), 0);
                    }
                }
                else
                {
                    sVar2 = ::recon::wcslen((uint32_t)(0x1038CA8Cu));
                    ((wstring *)gmem(_frame))->append(0x1038CA8Cu, (uint32_t)(sVar2));
                    uVar3 = guest::XmlChild_xml((uint32_t)(iVar4));
                    ((XmlObject *)gmem(uVar3))->toString((int32_t)((_frame + 96)), (bool)((a2 + 1)), (int32_t)(0));
                    ((wstring *)gmem(_frame))->append((int32_t)((_frame + 96)), (uint32_t)(0), (uint32_t)(0xffffffff));
                    if ((7 < LD32((_frame + 124))))
                    {
                        ::recon::guest_delete((int32_t)(LD32((_frame + 116))));
                    }
                    ST32((_frame + 124), 7);
                    ST32((_frame + 120), 0);
                    ST16((_frame + 100), 0);
                    sVar2 = ::recon::wcslen((uint32_t)(0x1038CA8Cu));
                    ((wstring *)gmem(_frame))->append(0x1038CA8Cu, (uint32_t)(sVar2));
                }
                iVar4 = (iVar4 + 0xc);
                iVar1 = LD32((uint32_t)((gaddr(this) + 0x2c)));
            }
            while ((iVar4 != iVar1));
        }
        if ((LD32((uint32_t)((iVar1 + -(8)))) == 1))
        {
            ST32((_frame + 152), 0);
            ST32((_frame + 156), 7);
            ST16((_frame + 132) + (0) * 2, 0);
            if ((a2 == -(1)))
            {
                ((std__String_base *)gmem((_frame + 128)))->_Xlen();
            }
            iVar1 = ((wstring *)gmem((_frame + 128)))->_Grow((uint32_t)(a2), (bool)(0));
            if ((iVar1 != 0))
            {
                ((wstring *)gmem((_frame + 128)))->_Chassign((int32_t)(0), (uint32_t)(a2), (uint16_t)(9));
                puVar5 = (_frame + 132);
                if ((7 < LD32((_frame + 156))))
                {
                    puVar5 = LD32((_frame + 148));
                }
                ST32((_frame + 152), a2);
                gelem<uint16_t>((uint32_t)(puVar5), a2) = 0;
            }
            ((wstring *)gmem(_frame))->append((int32_t)((_frame + 128)), (uint32_t)(0), (uint32_t)(0xffffffff));
            if ((7 < LD32((_frame + 156))))
            {
                ::recon::guest_delete((int32_t)(LD32((_frame + 148))));
            }
            ST16((_frame + 132) + (0) * 2, 0);
            ST32((_frame + 156), 7);
            ST32((_frame + 152), 0);
        }
        sVar2 = ::recon::wcslen((uint32_t)(GSTR(L"</", 0x1038CA98)));
        ((wstring *)gmem(_frame))->append(GSTR(L"</", 0x1038CA98), (uint32_t)(sVar2));
        ((wstring *)gmem(_frame))->append((int32_t)(gaddr(&this->field_0x4)), (uint32_t)(0), (uint32_t)(0xffffffff));
        sVar2 = ::recon::wcslen((uint32_t)(GSTR(L">", 0x1038CA88)));
        ((wstring *)gmem(_frame))->append(GSTR(L">", 0x1038CA88), (uint32_t)(sVar2));
        if ((LD32((uint32_t)((LD32((uint32_t)((gaddr(this) + 0x2c))) + -(8)))) == 1))
        {
            ST32((_frame + 184), 0);
            ST32((_frame + 188), 7);
            ST16((_frame + 164) + (0) * 2, 0);
            if ((a2 == -(1)))
            {
                ((std__String_base *)gmem((_frame + 160)))->_Xlen();
            }
            iVar1 = ((wstring *)gmem((_frame + 160)))->_Grow((uint32_t)(a2), (bool)(0));
            if ((iVar1 != 0))
            {
                ((wstring *)gmem((_frame + 160)))->_Chassign((int32_t)(0), (uint32_t)(a2), (uint16_t)(9));
                puVar5 = (_frame + 164);
                if ((7 < LD32((_frame + 188))))
                {
                    puVar5 = LD32((_frame + 180));
                }
                ST32((_frame + 184), a2);
                gelem<uint16_t>((uint32_t)(puVar5), a2) = 0;
            }
            ((wstring *)gmem(_frame))->append((int32_t)((_frame + 160)), (uint32_t)(0), (uint32_t)(0xffffffff));
            if ((7 < LD32((_frame + 188))))
            {
                ::recon::guest_delete((int32_t)(LD32((_frame + 180))));
            }
            ST16((_frame + 164) + (0) * 2, 0);
            ST32((_frame + 188), 7);
            ST32((_frame + 184), 0);
        }
    }
    if (((a1 != 0) || (a1 = guest::guest_new((uint32_t)(0x20)), (a1 != 0))))
    {
        ST16((uint32_t)((a1 + 4)), 0);
        ST32((uint32_t)((a1 + 0x18)), 0);
        ST32((uint32_t)((a1 + 0x1c)), 7);
        ((wstring *)gmem(a1))->assign((int32_t)(_frame), (uint32_t)(0), (uint32_t)(0xffffffff));
    }
    if ((7 < LD32((_frame + 28))))
    {
        ::recon::guest_delete((int32_t)(LD32((_frame + 20))));
    }
    native_frame_close(_frame, 192);
    return;
    native_frame_close(_frame, 192);
}
#endif

}  /* namespace recon */
