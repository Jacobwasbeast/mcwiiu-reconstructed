#include "nn/nex/ResultRange.hpp"

namespace nn
{
namespace nex
{

ResultRange::ResultRange(std::uint32_t uiOffset, std::uint32_t uiSize)
{
    m_uiOffset = uiOffset;
    m_uiSize = uiSize;
}

ResultRange::~ResultRange()
{
}

std::uint32_t ResultRange::GetOffset() const
{
    return m_uiOffset;
}

std::uint32_t ResultRange::GetSize() const
{
    return m_uiSize;
}

}
}
