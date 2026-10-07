#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_INFO_LIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_INFO_LIST_HPP

#include "nn/pia/inet/NexCommunityInfo.hpp"
#include "nn/pia/session/ICommunityInfoList.hpp"

namespace nn { namespace pia { namespace inet {

class NexCommunityInfoList : public session::ICommunityInfoList
{
public:
    typedef NexCommunityInfo* const* const_iterator;
    typedef NexCommunityInfo** iterator;
    virtual ~NexCommunityInfoList();
    virtual const_iterator Begin() const;
    virtual iterator Begin();
    virtual const_iterator End() const;
    virtual iterator End();
    virtual unsigned int GetSize() const;
    virtual unsigned int GetMaxSize() const;
    virtual void Clear();

private:
    unsigned int m_size;
    unsigned int m_maxSize;
    NexCommunityInfo** m_entries;
};

}}}

#endif
