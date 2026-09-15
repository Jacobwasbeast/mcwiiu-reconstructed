#include "nn/pia/common/CryptoSetting.hpp"

#include <cstring>

namespace nn { namespace pia { namespace common {

CryptoSetting::CryptoSetting()
    : m_mode(0)
{
    std::memset(m_key, 0, sizeof(m_key));
}

}}}
