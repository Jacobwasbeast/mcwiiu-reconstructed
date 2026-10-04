#include "nn/nex/DynamicGathering.hpp"

namespace nn
{
namespace nex
{

DynamicGathering::~DynamicGathering()
{
}

String DynamicGathering::GetGatheringType() const
{
    return m_Type;
}

}
}
