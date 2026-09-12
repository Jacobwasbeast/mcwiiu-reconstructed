// Startup glue for the devkitPPC/wut target build.
//
// wut supplies the RPX entry point (__rpx_start in crt0_rpx.o) and the loader
// hook __preinit_user, and its linker script places .crt0 first so the entry
// lands at the base of .text. Defining either of those here suppresses the
// archive member that carries them, which moves the entry point and produces
// an image the loader never starts. Only __init_wut is overridden, because
// wut's version does not run the C++ global constructors.

extern "C" void __init_wut_newlib();
extern "C" void __init_wut_devoptab();
extern "C" void __attribute__((weak)) __init_wut_socket();

extern "C" {
extern void (*__preinit_array_start[])();
extern void (*__preinit_array_end[])();
extern void (*__init_array_start[])();
extern void (*__init_array_end[])();
}

extern "C" void _init()
{
}

extern "C" void _fini()
{
}

// Do not replace this with newlib's __libc_init_array.
//
// wut.ld spans __init_array_start..__init_array_end across BOTH .init_array
// and the legacy .ctors format:
//
//     PROVIDE_HIDDEN (__init_array_start = .);
//     KEEP (*crtbegin.o(.ctors))        <- contributes the -1 head sentinel
//     ... .init_array and .ctors ...
//     KEEP (*(.ctors))                  <- crtend.o contributes the 0 terminator
//     PROVIDE_HIDDEN (__init_array_end = .);
//
// so the region really does begin with 0xffffffff and end with 0x00000000.
// __libc_init_array calls every slot in the range, so it branches to the head
// sentinel; bctr masks the low bits and the title dies at IP 0xfffffffc before
// a single constructor runs. Skip both sentinels instead.
//
// Forward order is correct here: modern g++ emits every real constructor into
// .init_array, and the only .ctors contributions in this link are the two
// sentinels.
static void RunInitArray(void (**first)(), void (**last)())
{
    void (**entry)() = first;
    for (; entry != last; ++entry)
    {
        unsigned int slot = (unsigned int)(*entry);
        if (slot == 0 || slot == 0xffffffffu)
            continue;
        (*entry)();
    }
}

extern "C" void __init_wut()
{
    __init_wut_newlib();
    __init_wut_devoptab();
    if (&__init_wut_socket) __init_wut_socket();
    RunInitArray(__preinit_array_start, __preinit_array_end);
    _init();
    RunInitArray(__init_array_start, __init_array_end);
}
