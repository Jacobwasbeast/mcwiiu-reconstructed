#include "vendor/__C1.hpp"

__C1::~__C1()
{
    if (m_capacity > 7)
    {
        delete[] m_data;
    }
    m_buffer[0] = 0;
    m_capacity = 7;
    m_size = 0;
}
