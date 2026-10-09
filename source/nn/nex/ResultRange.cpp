#include "nn/nex/ResultRange.hpp"

namespace nn
{
namespace nex
{

ResultRange::~ResultRange()
{
}

uint32_t ResultRange::GetOffset() const
{
    return m_uiOffset;
}

uint32_t ResultRange::GetSize() const
{
    return m_uiSize;
}

}
}
