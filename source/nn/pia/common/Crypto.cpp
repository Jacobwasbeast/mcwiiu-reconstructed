#include "nn/pia/common/Crypto.hpp"

namespace nn { namespace pia { namespace common {

unsigned int Crypto::GetBlockSize(Mode mode)
{
    if (mode != 1)
        return 0;
    return 16;
}

}}}
