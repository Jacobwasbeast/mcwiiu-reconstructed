#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SHA256_CONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SHA256_CONTEXT_HPP

#include "nn/pia/common/ShaBlock512BitContext.hpp"

namespace nn { namespace pia { namespace common {

class Sha256Context : public ShaBlock512BitContext
{
public:
    void ProcessBlock();
    void Initialize();
    void Update(const void* data, unsigned int size);
    void GetHash(void* hash);
    unsigned int GetHashSize() const;

private:
    unsigned int m_state[8];
};

}}}

#endif
