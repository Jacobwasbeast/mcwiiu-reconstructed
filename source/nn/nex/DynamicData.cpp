#include "nn/nex/DynamicData.hpp"

namespace nn
{
namespace nex
{

DynamicData::~DynamicData()
{
}

String DynamicData::GetDataType() const
{
    return m_Type;
}

}
}
