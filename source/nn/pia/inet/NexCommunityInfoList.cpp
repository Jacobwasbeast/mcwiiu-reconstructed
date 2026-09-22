#include "nn/pia/inet/NexCommunityInfoList.hpp"

namespace nn { namespace pia { namespace inet {

NexCommunityInfoList::~NexCommunityInfoList()
{
    for (unsigned int index = 0; index < m_size; ++index)
        delete m_entries[index];
    delete [] m_entries;
}

NexCommunityInfoList::const_iterator NexCommunityInfoList::Begin() const { return m_entries; }
NexCommunityInfoList::iterator NexCommunityInfoList::Begin() { return m_entries; }
NexCommunityInfoList::const_iterator NexCommunityInfoList::End() const { return m_entries + m_size; }
NexCommunityInfoList::iterator NexCommunityInfoList::End() { return m_entries + m_size; }
unsigned int NexCommunityInfoList::GetSize() const { return m_size; }
unsigned int NexCommunityInfoList::GetMaxSize() const { return m_maxSize; }

void NexCommunityInfoList::Clear()
{
    for (unsigned int index = 0; index < m_size; ++index)
        m_entries[index]->Clear();
    m_size = 0;
}

}}}
