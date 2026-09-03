#ifndef SOURCE_PLATFORM_WIIU_C4JSTORAGE_HPP
#define SOURCE_PLATFORM_WIIU_C4JSTORAGE_HPP

#include <stdint.h>

class C4JStorage
{
public:
    void SetSaveDeviceSelected(uint32_t device, bool selected);
};

#endif
