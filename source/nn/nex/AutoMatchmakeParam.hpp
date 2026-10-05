#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_AUTOMATCHMAKEPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_AUTOMATCHMAKEPARAM_HPP

#include "nn/nex/_DDL_AutoMatchmakeParam.hpp"

namespace nn
{
namespace nex
{

class AutoMatchmakeParam : public _DDL_AutoMatchmakeParam
{
public:
    virtual ~AutoMatchmakeParam();

    void Reset();
};

}
}

#endif
