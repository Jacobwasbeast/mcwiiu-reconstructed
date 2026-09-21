#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LOGICALCLOCKTMPL_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LOGICALCLOCKTMPL_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

template <typename ValueType, typename DifferenceType>
class LogicalClockTmpl : public RootObject
{
public:
    LogicalClockTmpl() : m_value(0)
    {
    }

    virtual ~LogicalClockTmpl()
    {
    }

private:
    ValueType m_value;
};

}
}

#endif
