#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SIMPLE_CONTAINER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SIMPLE_CONTAINER_HPP

#include "nn/pia/common/InetAddress.hpp"

namespace nn { namespace pia { namespace transport { class Station; }}}

namespace nn { namespace pia { namespace common {

template <class T, int N> struct SimpleContainer;

template <>
struct SimpleContainer<nn::pia::transport::Station*, 32>
{
    virtual ~SimpleContainer();
    virtual void Trace(unsigned long long value) const;
    unsigned int size;
    nn::pia::transport::Station* data[32];
};

template <>
struct SimpleContainer<InetAddress, 4>
{
    virtual ~SimpleContainer();
    virtual void Trace(unsigned long long value) const;
    unsigned int size;
    InetAddress data[4];
};

}}}

#endif
