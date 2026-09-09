#ifndef SOURCE_VENDOR_MILES_MEM_HPP
#define SOURCE_VENDOR_MILES_MEM_HPP

extern "C" void AIL_debug_log(const char* pFormat, ...);
extern "C" void AIL_set_error(const char* pError);
extern "C" void* AIL_mem_alloc_lock_info(int iSize, const char* pOwnerName, int iOwnerLine);
extern "C" void AIL_mem_free_lock(void* pBlock);
extern "C" void outresuhex(unsigned int uResult);
extern "C" int AIL_ms_count();

extern int AIL_debug_enabled;
extern int AIL_debug_nested;
extern int AIL_debug_depth;

#endif
