#ifndef SOURCE_VENDOR_MILES_API_HPP
#define SOURCE_VENDOR_MILES_API_HPP

#include "vendor/MilesAsync.hpp"
#include "vendor/MilesRR.hpp"

typedef void* (*AIL_MEM_ALLOC_CALLBACK)(unsigned int uSize);
typedef void (*AIL_MEM_FREE_CALLBACK)(void* pBlock);

extern int MSS_api_started;
extern rrMutex MSS_api_mutex;
extern MilesProfileSite MSS_api_lock_wait_site;
extern MilesProfileSite MSS_api_lock_site;
extern MilesProfileSite MSS_api_unlock_site;
extern MilesProfileSite MSS_api_mutex_name_site;

extern AIL_MEM_ALLOC_CALLBACK AIL_mem_alloc_handler;
extern AIL_MEM_FREE_CALLBACK AIL_mem_free_handler;

extern char AIL_error[256];
extern int AIL_preference[37];

extern "C" void InMilesMutex();
extern "C" void OutMilesMutex();
extern "C" void AIL_lock_mutex();
extern "C" void AIL_unlock_mutex();
extern "C" void AIL_API_startup();
extern "C" int AIL_API_background();
extern "C" void AIL_API_set_error(const char* pError);
extern "C" void* AIL_API_mem_alloc_lock(unsigned int uSize);
extern "C" void AIL_API_mem_free_lock(void* pBlock);
extern "C" void* MSS_alloc_info(unsigned int uSize, unsigned int uUser, const char* pOwnerName,
                                int iOwnerLine);
extern "C" void MSS_free_info(void* pBlock);

#endif
