#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QLIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QLIST_HPP

#include <list>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class qList : public std::list<T, MemAllocator<T> >, public RootObject
{
public:
    typedef std::list<T, MemAllocator<T> > _Base;
    typedef typename _Base::size_type size_type;
    typedef typename _Base::value_type value_type;

    qList() : _Base() {}

    qList(const qList<T>& other) : _Base(other) {}

    qList(const _Base& other) : _Base(other) {}

    ~qList() {}

    qList<T>& operator=(const qList<T>& other)
    {
        _Base::operator=(other);
        return *this;
    }

    qList<T>& operator=(const _Base& other)
    {
        _Base::operator=(other);
        return *this;
    }
};

}
}

#endif
