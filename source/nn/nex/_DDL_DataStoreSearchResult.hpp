#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORESEARCHRESULT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORESEARCHRESULT_HPP

#include <cstdint>

#include "nn/nex/DataStoreMetaInfo.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreSearchResult : public RootObject
{
public:
    _DDL_DataStoreSearchResult();
    virtual ~_DDL_DataStoreSearchResult();

    _DDL_DataStoreSearchResult& operator=(const _DDL_DataStoreSearchResult& rSource);
    bool operator==(const _DDL_DataStoreSearchResult& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreSearchResult& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreSearchResult* pDestination);

    std::uint8_t m_uiStructureVersion;
    std::uint32_t m_uiTotalCount;
    qVector<DataStoreMetaInfo> m_Result;
    std::uint8_t m_uiTotalCountType;
};

}
}

#endif
