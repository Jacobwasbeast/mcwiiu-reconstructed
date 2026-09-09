#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_RECEIVED_DATA_CONTAINER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_RECEIVED_DATA_CONTAINER_HPP
#include "nn/pia/ListBase.hpp"
namespace nn { namespace pia { namespace transport {
class ReceivedDataContainer
{
public:
    common::ListBase m_list;
    common::ListBase m_available;
    int m_nodeOffset;
    unsigned char* m_storage;
    unsigned int m_count;
    ReceivedDataContainer();
};
}}}
#endif
