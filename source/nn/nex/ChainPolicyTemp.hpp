#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CHAINPOLICYTEMP_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CHAINPOLICYTEMP_HPP

namespace nn
{
namespace nex
{

template <class T>
class ChainPolicyTemp
{
public:
    static T& Next(T element)
    {
        return element->m_pTempNext;
    }

    static T& Previous(T element)
    {
        return element->m_pTempPrevious;
    }
};

}
}

#endif
