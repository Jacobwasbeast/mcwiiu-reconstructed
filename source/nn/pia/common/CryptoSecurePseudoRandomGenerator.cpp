#include "nn/pia/common/CryptoSecurePseudoRandomGenerator.hpp"

#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace common {

CryptoSecurePseudoRandomGenerator::CryptoSecurePseudoRandomGenerator()
    : m_seed(0), m_stream(0), m_setting0(0), m_setting1(0),
      m_setting2(0), m_setting3(0)
{
    pead::Random random;
    m_seed = random.getU64();
    m_stream = random.getU64();
}

CryptoSecurePseudoRandomGenerator::~CryptoSecurePseudoRandomGenerator()
{
}

void CryptoSecurePseudoRandomGenerator::UpdateSetting(unsigned long long first,
                                                        unsigned long long second)
{
    m_setting0 = static_cast<uint32_t>(first >> 32);
    m_setting1 = static_cast<uint32_t>(first);
    m_setting2 = static_cast<uint32_t>(second >> 32);
    m_setting3 = static_cast<uint32_t>(second);
}

}}}
