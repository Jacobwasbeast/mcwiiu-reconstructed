#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPERSISTENCEINITPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPERSISTENCEINITPARAM_HPP

#include "nn/nex/_DDL_DataStorePersistenceInitParam.hpp"

namespace nn
{
namespace nex
{

class DataStorePersistenceInitParam : public _DDL_DataStorePersistenceInitParam
{
public:
    DataStorePersistenceInitParam()
    {
    }

    static const uint16_t NO_PERSISTENCE_SLOT_ID = 0xFFFF;

    virtual ~DataStorePersistenceInitParam()
    {
    }
};

}
}

#endif
