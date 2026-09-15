#include "nn/pia/common/SimpleContainer.hpp"

namespace nn { namespace pia { namespace common {

template struct SimpleContainer<nn::pia::transport::Station*, 32>;
template struct SimpleContainer<InetAddress, 4>;

SimpleContainer<nn::pia::transport::Station*, 32>::~SimpleContainer()
{
}

void SimpleContainer<nn::pia::transport::Station*, 32>::Trace(unsigned long long value) const
{
    (void)value;
}

SimpleContainer<InetAddress, 4>::~SimpleContainer()
{
}

void SimpleContainer<InetAddress, 4>::Trace(unsigned long long value) const
{
    (void)value;
}

}}}
