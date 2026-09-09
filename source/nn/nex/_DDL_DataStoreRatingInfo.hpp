#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATINGINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATINGINFO_HPP

#include <cstdint>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreRatingInfo : public RootObject
{
public:
    _DDL_DataStoreRatingInfo();
    virtual ~_DDL_DataStoreRatingInfo();

    _DDL_DataStoreRatingInfo& operator=(const _DDL_DataStoreRatingInfo& rSource);
    bool operator==(const _DDL_DataStoreRatingInfo& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreRatingInfo& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreRatingInfo* pDestination);

    std::uint8_t m_uiStructureVersion;
    std::int64_t m_iTotalValue;
    std::uint32_t m_uiCount;
    std::int64_t m_iInitialValue;
};

}
}

#endif
