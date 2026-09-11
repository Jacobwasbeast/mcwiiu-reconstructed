#include <coreinit/memdefaultheap.h>

extern "C" void __init_wut_sbrk_heap(MEMHeapHandle heapHandle);
extern "C" void __init_wut_thread();
extern "C" void __init_wut_defaultheap();
extern "C" void __init_wut_newlib();
extern "C" void __libc_init_array();
extern "C" int main(int argc, char** argv);
extern "C" void exit(int code);

extern "C" void __preinit_user(MEMHeapHandle* mem1, MEMHeapHandle* foreground, MEMHeapHandle* mem2)
{
    (void)mem1;
    (void)foreground;
    __init_wut_sbrk_heap(*mem2);
    __init_wut_thread();
    __init_wut_defaultheap();
}

asm(".section .fexports,\"ax\",@0x80000001\n"
    ".align 4\n"
    ".long 1\n"
    ".long 0xa7fef0c2\n"
    ".long __preinit_user\n"
    ".long 0x10\n"
    ".string \"__preinit_user\"\n"
    ".section .text\n");

extern "C" void _init()
{
}

extern "C" void _fini()
{
}

extern "C" void __init_wut()
{
    __init_wut_newlib();
    __libc_init_array();
}

extern "C" void __fini_wut()
{
}

extern "C" void __rpx_start(int argc, char** argv)
{
    __init_wut();
    exit(main(argc, argv));
}
