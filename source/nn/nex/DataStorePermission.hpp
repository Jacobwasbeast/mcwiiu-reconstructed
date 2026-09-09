#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPERMISSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPERMISSION_HPP

#include <cstdint>

#include "nn/nex/_DDL_DataStorePermission.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class DataStorePermission : public _DDL_DataStorePermission
{
public:
    DataStorePermission(std::uint8_t uiPermission = 3,
                        const qVector<std::uint32_t>& rRecipientIds = qVector<std::uint32_t>());
    virtual ~DataStorePermission();
};

}
}

#endif
