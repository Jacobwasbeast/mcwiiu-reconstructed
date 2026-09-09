#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORESEARCHPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORESEARCHPARAM_HPP

#include <cstdint>

#include "nn/nex/DateTime.hpp"
#include "nn/nex/ResultRange.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreSearchParam : public RootObject
{
public:
    _DDL_DataStoreSearchParam();
    virtual ~_DDL_DataStoreSearchParam();

    _DDL_DataStoreSearchParam& operator=(const _DDL_DataStoreSearchParam& rSource);
    bool operator==(const _DDL_DataStoreSearchParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreSearchParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreSearchParam* pDestination);

    std::uint8_t m_uiStructureVersion;
    std::uint8_t m_uiSearchTarget;
    qVector<std::uint32_t> m_OwnerIds;
    std::uint8_t m_uiOwnerType;
    qVector<std::uint32_t> m_DestinationIds;
    std::uint16_t m_uiDataType;
    DateTime m_CreatedAfter;
    DateTime m_CreatedBefore;
    DateTime m_UpdatedAfter;
    DateTime m_UpdatedBefore;
    std::uint32_t m_uiReferDataId;
    qVector<String> m_Tags;
    std::uint8_t m_uiResultOrderColumn;
    std::uint8_t m_uiResultOrder;
    ResultRange m_ResultRange;
    std::uint8_t m_uiResultOption;
    std::uint32_t m_uiMinimalRatingFrequency;
    bool m_bUseCache;
    bool m_bTotalCountEnabled;
    qVector<std::uint16_t> m_DataTypes;
};

}
}

#endif
