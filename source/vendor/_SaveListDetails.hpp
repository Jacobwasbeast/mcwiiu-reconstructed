#ifndef SOURCE_VENDOR_SAVELISTDETAILS_HPP
#define SOURCE_VENDOR_SAVELISTDETAILS_HPP

#include <stdint.h>

class _SaveListDetails
{
public:
    _SaveListDetails();

private:
    uint32_t m_value0;
    uint32_t m_value1;
    uint32_t m_value2;
    unsigned char m_data[288];
};

#endif
