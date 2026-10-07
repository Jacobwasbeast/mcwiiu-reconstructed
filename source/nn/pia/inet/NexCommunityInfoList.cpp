#include "nn/pia/inet/NexCommunityInfoList.hpp"

#include "nn/pia/NetworkFactory.hpp"
#include "nn/pia/common/Memory.hpp"

namespace nn { namespace pia { namespace inet {

NexCommunityInfoList::~NexCommunityInfoList()
{
    for (unsigned int index = 0; index < m_maxSize; ++index)
        transport::DestroyObject(m_entries[index]);
    common::DeleteArray(m_entries);
}

NexCommunityInfoList::const_iterator NexCommunityInfoList::Begin() const { return m_entries; }
NexCommunityInfoList::iterator NexCommunityInfoList::Begin() { return m_entries; }
NexCommunityInfoList::const_iterator NexCommunityInfoList::End() const { return m_entries + m_size; }
NexCommunityInfoList::iterator NexCommunityInfoList::End() { return m_entries + m_size; }
unsigned int NexCommunityInfoList::GetSize() const { return m_size; }
unsigned int NexCommunityInfoList::GetMaxSize() const { return m_maxSize; }

void NexCommunityInfoList::Clear()
{
    m_size = 0;
    for (unsigned int index = 0; index < m_maxSize; ++index)
        m_entries[index]->Clear();
}

}}}
