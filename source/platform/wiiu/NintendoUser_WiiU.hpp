#ifndef SOURCE_PLATFORM_WIIU_NINTENDOUSER_WIIU_HPP
#define SOURCE_PLATFORM_WIIU_NINTENDOUSER_WIIU_HPP

#include <stdint.h>

int CoreUser_DynLoad_DefaultAlloc__FiT1PPv(int32_t size, int32_t alignment, void** result);
void CoreUser_DynLoad_DefaultFree__FPv(void* block);

#endif
