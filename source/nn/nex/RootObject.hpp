#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ROOTOBJECT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ROOTOBJECT_HPP

#include <cstddef>

namespace nn
{
namespace nex
{

class RootObject
{
public:
    static void* operator new(std::size_t size);
    static void operator delete(void* memory);
    static void* operator new[](std::size_t size);
    static void operator delete[](void* memory);
};

}
}

#endif
