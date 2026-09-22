#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_INFO_LIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_INFO_LIST_HPP

#include "nn/pia/inet/NexCommunityInfo.hpp"

namespace nn { namespace pia { namespace inet {

class NexCommunityInfoList
{
public:
    typedef NexCommunityInfo* const* const_iterator;
    typedef NexCommunityInfo** iterator;
    virtual ~NexCommunityInfoList();
    const_iterator Begin() const;
    iterator Begin();
    const_iterator End() const;
    iterator End();
    unsigned int GetSize() const;
    unsigned int GetMaxSize() const;
    void Clear();

private:
    unsigned int m_size;
    unsigned int m_maxSize;
    NexCommunityInfo** m_entries;
};

}}}

#endif
