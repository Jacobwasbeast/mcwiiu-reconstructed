#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_POLYMORPHICROOTOBJECT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_POLYMORPHICROOTOBJECT_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class PolymorphicRootObject : public RootObject
{
public:
    PolymorphicRootObject()
        : m_unknown0(0)
    {
    }

    virtual ~PolymorphicRootObject() {}

private:
    unsigned int m_unknown0;
};

}
}

#endif
