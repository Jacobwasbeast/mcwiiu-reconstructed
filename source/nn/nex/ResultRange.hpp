#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RESULTRANGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RESULTRANGE_HPP

#include <cstdint>

#include "nn/nex/_DDL_ResultRange.hpp"

namespace nn
{
namespace nex
{

class ResultRange : public _DDL_ResultRange
{
public:
    ResultRange(std::uint32_t uiOffset = 0, std::uint32_t uiSize = 20);
    virtual ~ResultRange();

    virtual std::uint32_t GetOffset() const;
    virtual std::uint32_t GetSize() const;
};

}
}

#endif
