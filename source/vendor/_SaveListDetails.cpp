#include "vendor/_SaveListDetails.hpp"

#include <cstring>

_SaveListDetails::_SaveListDetails()
{
    m_value0 = 0;
    m_value2 = 0;
    m_value1 = 0;
    std::memset(m_data, 0, 264);
    std::memset(m_data + 264, 0, 8);
}
