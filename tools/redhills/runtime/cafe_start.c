/*
 * Start-up and shut-down support for the RedHills Cafe build.
 *
 * _main is the static-initialisation step the Cafe crt0 runs before main. The retail image gets it from
 * the Green Hills C++ runtime, which walks the Green Hills constructor tables. RedHills, like any ELF
 * compiler, lists static constructors in .preinit_array and .init_array and destructors in .fini_array
 * (tools/redhills/cafe.ld collects them): _main runs the first two in order, and __fini, which newlib
 * registers with atexit, runs the last in reverse.
 *
 * __gh_memclr32 is the Green Hills word-clearing helper (libarch's ind_mclr32.o) the crt0 keeps linked:
 * it zeroes a count of 32-bit words.
 */

typedef void (*cafe_init_function)(void);

extern cafe_init_function __preinit_array_start[];
extern cafe_init_function __preinit_array_end[];
extern cafe_init_function __init_array_start[];
extern cafe_init_function __init_array_end[];
extern cafe_init_function __fini_array_start[];
extern cafe_init_function __fini_array_end[];

static void run_init_functions(cafe_init_function *first, cafe_init_function *last)
{
    for (; first != last; ++first)
        (*first)();
}

void _main(void)
{
    run_init_functions(__preinit_array_start, __preinit_array_end);
    run_init_functions(__init_array_start, __init_array_end);
}

void __fini(void)
{
    cafe_init_function *entry = __fini_array_end;

    while (entry != __fini_array_start)
        (*--entry)();
}

void __gh_memclr32(unsigned int *words, unsigned int count)
{
    while (count != 0) {
        *words++ = 0;
        --count;
    }
}
