#include "nn/pia/common/CryptoSetting.hpp"

#include <string.h>

namespace nn { namespace pia { namespace common {

CryptoSetting::CryptoSetting()
    : m_field00(0)
{
    memset(m_key, 0, sizeof(m_key));
}

}}}
