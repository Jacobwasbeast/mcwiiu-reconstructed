#include "nn/nex/ResultRange.hpp"

namespace nn
{
namespace nex
{

ResultRange::ResultRange(uint32_t uiOffset, uint32_t uiSize)
{
    m_uiOffset = uiOffset;
    m_uiSize = uiSize;
}

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
