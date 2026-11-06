# Measuring

`tools/measure/parity.py` gives a rough progress figure you can check
yourself. It compares the original `Minecraft.Client.rpx` from your update
688 dump with the one you built, function by function.

This is separate from how the source is written. During reconstruction,
each function is A/B tested against the original function directly and
refined until it matches. The parity script is a quicker, coarser check that
anyone can run, so its percentage is only an approximate picture of
progress.

```
python tools/build.py
python tools/measure/parity.py --original <update v688 dump>/code/Minecraft.Client.rpx
```

By default it reads `build/Minecraft.Client.rpx`, its link map
`build/Minecraft.Client.map` and the `source/` tree; `--rebuilt`, `--map`
and `--source` change these. It takes about a minute. `--report FILE` writes
one row per function with the reasons for each verdict.

It needs Python 3.8 or newer and a C++ demangler: `llvm-cxxfilt` or
`c++filt` if your machine has one, otherwise the `c++filt` in the Docker
image that `tools/build.py` builds.

## What it compares

Both executables keep their function names, relocations and machine code.
The script pairs each function in the original with the function of the
same name in your build, and compares the things a different compiler leaves
unchanged:

- the functions it calls;
- how many calls it makes through a pointer (virtual calls, callbacks);
- the floating-point constants it loads, and what each is used for (an
  operand of a multiplication, an addition or subtraction, a division, a
  comparison, a store, a call argument); a constant next to the one the
  original loads counts as misread;
- the 32-bit constants it builds;
- the string literals it uses;
- the constant integer arguments of the calls both functions make;
- whether its body is far thinner than the original's.

| Verdict | Meaning |
| --- | --- |
| **Clean** | All of the above agree. |
| **Differs** | At least one disagrees; the report says which. |
| **Inlined** | The original has the function, your build has no copy of it, and `source/` defines it, so RedHills inlined it everywhere it is used. |
| **Missing** | The original has the function, and the script found no definition of it in `source/`. |
| **Trivial** | Nothing to compare, such as a small accessor. |

Inlined and trivial functions are not counted in the percentages.
Constructors, destructors and thunks are counted separately, because the
C++ ABI shapes them as much as the source does, and the virtual function
tables that constructors install are compared slot by slot. Code that each
compiler supplies itself (the C++ standard library, boost, rapidxml and the
C runtime) is not compared.

The original was built with the game's original compiler and your build
with RedHills, and the two inline different functions. The script allows
for this: if one side calls a function and the other does not, the call
still counts as matching when the callee could have been inlined there and
the other body contains what the callee does.

## What the number means

- **It checks calls, constants and strings, not behaviour.** Field offsets,
  the order of operations and branch conditions are not compared, so a clean
  function can still behave differently from the original.
- **It understates.** The two compilers sometimes produce different results
  from the same source (one folds a constant the other loads, or one keeps a
  call the other inlines in a way the script cannot follow), so some correct
  functions are reported as differing.
- **It measures the build you run.** The numbers below come from the public
  build of the commit that last updated them. Your own build of that commit
  gives the same result.

## Current result

Measured on 18 October 2026 with the public build of the commit that last
updated these numbers, against update 688's `Minecraft.Client.rpx`.

The original has 40,032 functions outside the standard library and the
runtimes. Of these, 23,851 were compared, 6,378 were inlined by RedHills and
9,803 had nothing to compare.

| Result | Functions | Share of compared |
| --- | ---: | ---: |
| Clean | 18,463 | 77.4% |
| Differs | 4,832 | 20.3% |
| Missing | 556 | 2.3% |

| Area | Clean | Differs | Clean share |
| --- | ---: | ---: | ---: |
| Game | 14,713 | 3,470 | 80.9% |
| Wii U platform layer | 551 | 287 | 65.8% |
| Mii faces (FFL) | 228 | 81 | 73.8% |
| NEX online library | 970 | 407 | 70.4% |
| Pia online library | 1,598 | 454 | 77.9% |
| Other Nintendo SDK | 43 | 5 | 89.6% |
| Sound and middleware | 340 | 115 | 74.7% |
| Cafe SDK runtime | 19 | 10 | 65.5% |
| Constructors, destructors and thunks | 4,138 | 709 | 84.6% |

The script finds definitions by reading source text, so some of the 556
missing functions are defined in a form it does not recognise.

### Virtual function tables

Of the 2,464 tables the original's constructors install, 2,441 are
identical in the rebuilt executable, and 102,993 of their 103,024 slots match. Of the
23 that differ:

- 15 differ because the original compiler gives an override with a
  covariant return type (`CompoundTag::copy`, for example) its own second
  slot, and RedHills does not yet.
- 7 are paired with the wrong table by the script (the `IdMapper` base table
  of a `MappedRegistry` against the derived `SimpleRegistry` one).
- 1 has trailing slots the rebuilt table lacks.

A further 29 tables of abstract base classes are never installed by the
rebuilt executable, because RedHills drops a store that the derived constructor
overwrites anyway. They are not compared.
