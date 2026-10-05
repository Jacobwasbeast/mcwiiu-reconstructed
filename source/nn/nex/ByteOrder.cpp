#include "nn/nex/ByteOrder.hpp"

namespace nn
{
namespace nex
{

template <typename T>
T HostOrderToNexNetworkOrder(T value)
{
    T result = 0;
    for (unsigned int i = 0; i < sizeof(T); ++i)
        result |= static_cast<T>(((value >> (8 * i)) & 0xFF) << (8 * (sizeof(T) - 1 - i)));

    return result;
}

template unsigned int HostOrderToNexNetworkOrder<unsigned int>(unsigned int);
template unsigned short HostOrderToNexNetworkOrder<unsigned short>(unsigned short);

}
}
