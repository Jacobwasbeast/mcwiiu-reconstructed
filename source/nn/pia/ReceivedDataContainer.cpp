#include "nn/pia/ReceivedDataContainer.hpp"
namespace nn { namespace pia { namespace transport {
ReceivedDataContainer::ReceivedDataContainer()
    : m_list(), m_available(), m_nodeOffset(0), m_storage(0), m_count(0)
{
}
}}}
