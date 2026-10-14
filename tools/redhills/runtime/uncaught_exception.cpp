// std::uncaught_exception for the RedHills (Green Hills ABI) build.
//
// The game and its C++ library are built without exceptions and link the
// no-exception EDG runtime, so no exception object can ever be in flight:
// the stream sentries that ask see none. libsupc++'s versions read the
// Itanium unwinder's per-thread exception globals, which this runtime has no
// use for.

#include <exception>

namespace std {

bool uncaught_exception() throw()
{
    return false;
}

int uncaught_exceptions() throw()
{
    return 0;
}

}
